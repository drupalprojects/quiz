<?php
// $Id$

/*
 * @file
 * Administration file for Questions Export module
 *
 */

require_once(drupal_get_path('module', 'quiz') .'/includes/moodle_support.php');

/**
 * Implementation of hook_form
 * form to upload questions
 */
function questions_export_form()  {
  $form['#attributes'] = array('enctype' => 'multipart/form-data');

  $form['quiz_node'] = array(
    '#type' => 'select',
    '#title' => t('Quiz'),
    '#options' => quiz_get_all_quiz_title(),
    '#description' => t('Select the quiz node to export.'),
    '#required' => TRUE,
  );

  $form['export_format'] = array(
    '#type' => 'select',
    '#title' => t('Export format'),
    '#options' => questions_export_formats(),
    '#description' => t('Select the data format to export into.'),
    '#required' => TRUE,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Export'),
  );

  $form['#submit'][] = 'questions_export_form_submit';

  return $form;
}

/**
 * @return list of implemented formats that can be exported (e.g. CSV, QTI, Aiken)
 */
function questions_export_formats() {
  $type = array(
    'native_csv' => t('Comma Separated Values (by Drupal)'),
    'native_aiken' => t('Aiken (by Drupal)'),
    'moodle_gift' => t('Moodle GIFT (by Moodle)'),
    'moodle_qti2' => t('QTI 2.0 (by Moodle)'),
    'moodle_xml' => t('Moodle XML (by Moodle)'),

  );
  return $type;
}


/**
 * This generic submit handler calls specific export functions
 *
 */
function questions_export_form_submit(&$form, &$form_state) {
  $time = 0;
  $op = '';
  $quiz_nid = $form_state['values']['quiz_node'];
  $export_format_info = explode('_', $form_state['values']['export_format']);
  $export_engine = $export_format_info[0];
  $export_format = $export_format_info[1];

  switch ($export_engine) {
    case 'moodle':
      $count = questions_export_submit_moodle($export_format, $form, $form_state);
      break;
    case 'native':
      $count = questions_export_submit_drupal($quiz_nid, $export_format);
      break;
  }
}


function questions_export_submit_drupal($quiz_nid, $export_format) {
  ob_start();
  $questions = _questions_in_quiz($quiz_nid);
  // $questions is an array of quiz question node object
  $quiz_title = db_result(db_query('SELECT title FROM {node} WHERE nid = %d', $quiz_nid));
  $glue = ($export_format === 'aiken') ? "\n" : ',';
  // if the export type is 'aiken' use new line ("\n") else use comma (",") as glue for implode
  // iterate through all the questions and generate file content.
  foreach ($questions as $question) {
    $line = array($question->type, $question->body);
    switch ($question->type) {
      case 'matching':
        foreach ($question->answer as $answer) {
          $feedback = $answer['feedback'] ? $answer['feedback'] : t('nil');
          // use 'nil' when there is no feedback
          array_push($line, $answer['question'], $answer['answer'], $feedback);
        }
        break;

      case 'multichoice':
        foreach ($question->answers as $answer) {
          $feedback = $answer['feedback'] ? $answer['feedback'] : t('nil');
          // use 'nil' when there is no feedback
          array_push($line, $answer['answer'], $feedback);
          $correct_answer = $answer['is_correct'] ? $answer['answer'] : '';
          print_r($correct_answer);
        }
        array_push($line, $correct_answer);
        break;

      case 'true_false':
        $feedback = $question->feedback ? $feedback : t('nil');
        // use 'nil' when there is no feedback
        array_push($line, $question->correct_answer ? 'true' : 'false', $feedback);
        break;

      case 'short_answer':
        $evaluation = array(
          'case sensitive match',
          'case insensitive match',
          'regular expression match',
          'manually score match'
        );
        array_push($line, $question->correct_answer, $question->maximum_score, $evaluation[$question->correct_answer_evaluation]);
        break;

      case 'long_answer':
        array_push($line, $question->maximum_score);
        break;
    }
    $output .= (count($line) > 2) ? implode($glue, $line) . "\r\n" : '';
    $line = array();
  }
  $filename = str_replace(' ', '_', $quiz_title) . '.txt';
  $filepath = file_directory_temp() . $filename;
  $handle = @fopen($filepath, 'w');
  fwrite($handle, $output);
  fclose($handle);
  $headers = array('Content-Type: text/plain', 'Content-Disposition: attachment; filename='. $filename);
  ob_clean();
  file_transfer($filepath, $headers);
  ob_end_clean();
}

/*
 * @param $quiz
 * quiz node object
 *
 * @return
 * a list of question node objects in the give quiz node object
 */
function _questions_in_quiz($quiz_nid) {
  $questions = array();
  $quiz_vid = db_result(db_query("SELECT vid FROM {node} WHERE nid = %d", $quiz_nid));
  // Get all the questions (ignore `question_status`)
  $sql = "SELECT child_nid as nid, child_vid as vid
    FROM {quiz_node_relationship}
    WHERE parent_vid = %d
      AND parent_nid = %d
    ORDER BY weight";
  $result = db_query($sql, $quiz_vid, $quiz_nid);
  while ($question_node_ids = db_fetch_array($result)) { // nid, vid
    $question_nodes[] = node_load($question_node_ids['vid']); // TODO do we need the version?
  }
  // OMG FIXME this loop is #bad if someone has a quiz with lot questions
  return $question_nodes;
}

function _as_moodle_questions($drupal_questions) {
  $moodle_questions = array(); // moodle questions
  // TODO turn them into Moodle model

  foreach ($drupal_questions as $dq) {
    $mq = new stdClass();
    $mq->id = "drupal_nid:$dq->nid, drupal_vid:$dq->vid, author:$dq->name";
    $mq->name = $dq->title;
    $mq->questiontext = $dq->body;
    $mq->questiontextformat = FORMAT_HTML; // FIXME where does Drupal Quiz store this?

    $qtypeMap = array(
      // Drupal to Moodle
      'long_answer' => 'essay',
      'short_answer' => 'shortanswer',
      'multichoice' => 'multichoice',
      'true_false' => 'truefalse',
      'quiz_directions' => 'description',
      'matching' => 'match',
      );
    $mq->qtype = $qtypeMap[$dq->type];

    $mq->options = new stdClass();
    $mq->options->answers = array();
    switch($dq->type) { // TODO refactor this into the questiontype classes
      case 'matching':
        // TODO why doesn't matching have an ->answers property?
        for ($i = 0; !empty($dq->$i); $i += 1) {
          $match_answer = $dq->$i;
          $questiontext = $match_answer->question;
          $answertext = $match_answer->answer;
          $matches[] = (object) compact('questiontext', 'answertext');
        }
        $mq->options->subquestions = $matches;
        break;
      case 'multichoice':
        $mq->singlequestion = ($dq->multiple_answers) ? 0 : 1;
        foreach ($dq->answers as $drupal_answer) {
          $moodle_answer = new stdClass();
          $moodle_answer->answer = $drupal_answer['answer'];
          $moodle_answer->feedback = $drupal_answer['feedback'];
          $moodle_answer->fraction = ($drupal_answer['is_correct']) ? 1 : 0;
          $mq->options->answers[] = $moodle_answer;
        }
        break;
      case 'true_false':
        $moodle_true_answer = new stdClass();
        $moodle_true_answer->fraction = ($dq->correct_answer) ? 1 : 0;
        $mq->options->answers[] = $moodle_true_answer;

        $moodle_false_answer = new stdClass();
        $moodle_false_answer->fraction = ($dq->correct_answer) ? 0 : 1;
        $mq->options->answers[] = $moodle_false_answer;

        $mq->options->trueanswer = 0; // indices above
        $mq->options->falseanswer = 1;
        break;
    }
    $moodle_questions[] = $mq;
  }

  return $moodle_questions;
}

/**
 * Exports questions to a GIFT file.
 *
 */
function questions_export_submit_moodle($format, &$form, &$form_state) {
  global $user;
  ob_start();

  $quiz_nid = $form_state['values']['quiz_node'];


  $drupal_questions = _questions_in_quiz($quiz_nid);
  $moodle_questions = _as_moodle_questions($drupal_questions);

  $file_name = tempnam(variable_get('file_directory_temp', file_directory_temp()), 'quiz');
  $fhandle = @fopen($file_name, 'w'); // The @ suppresses errors.

  global $CFG;
  require_once(drupal_get_path('module', 'quiz') ."/includes/moodle/question/format/$format/format.php");
  $classname = "qformat_$format";
  $qformat = new $classname();
  $qformat->set_can_access_backupdata(false); // not available here

  $qformat->filename = $quiz->title;

/*
  // import individually
  foreach ($moodle_questions as $question) {
    $informat = $qformat->writequestion($question);
    fwrite($fhandle, $informat);
  }
  $dlname = "$quiz->title.gift.txt";
  $headers = array("Content-Type: text/plain",
                   "Content-Disposition: attachment; filename=\"$dlname\"" );
  // fwrite($fhandle, print_r($drupal_questions,true));  // debug
*/

  // process using moodle
  $qformat->setQuestions($moodle_questions);
  if (! $qformat->exportpreprocess()) {   // Do anything before that we need to
      print_error('exporterror', 'quiz', $thispageurl->out());
  }

  if (! $qformat->exportprocess()) {         // Process the export data
      print_error('exporterror', 'quiz', $thispageurl->out());
  }

  if (! $qformat->exportpostprocess()) {         // In case anything needs to be done after
      print_error('exporterror', 'quiz', $thispageurl->out());
  }

  // send the finished file
  // FIXME this isn't very robust, but it's what Moodle's question/export.php does
  $filename = $qformat->filename . $qformat->export_file_extension();
  $filepath = $qformat->question_get_export_dir() . '/' . $filename;

  // print $filepath;exit;

  // TODO set Content-Type based on export format.  wish that Moodle included that in its formats.
  switch ($qformat->export_file_extension()) {
    case 'zip':
      $content_type = 'application/zip';
      break;
    case 'xml':
      $content_type = 'text/xml';
      break;
    case 'txt':
        $content_type = 'text/plain';
        break;
  }
  $headers = array("Content-Type: $content_type",
                   "Content-Disposition: attachment; filename=\"$filename\"" );

  ob_clean();
  file_transfer($filepath, $headers);
  ob_end_clean();

  $url = file_create_url($filepath); // for future reference
}

