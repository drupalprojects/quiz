<?php
// $Id$

/*
 * @file
 * Administration file for Questions Export module
 *
 */

module_load_include('php', 'quiz', 'includes/moodle_support');


/**
 * Implementation of hook_form
 * form to upload questions
 */
function questions_export_form()  {
  $form = array();
  $form['#attributes'] = array('enctype' => 'multipart/form-data');
  $form['quiz_questions_export'] = array(
    '#type' => 'fieldset',
    '#title' => t('Export Questions'),
    '#description' => t('Quiz questions import allows to export question and save it out-side of drupal in a portable text file format. Export file can be used as backback or source to generate question in other drupal site.'),
  );
  $option = quiz_get_all_quiz_title();
  // $options is an array with nid as index and quiz node's title as value
  $form['quiz_questions_export']['quiz_node_title'] = array(
    '#type' => 'select',
    '#title' => t('Quiz'),
    '#options' => $option,
    '#description' => count($options) ? t('Select the quiz to export its question(s).') : t('No quiz contents where found. To !create_a_quiz go to Content Management -> Create Content -> Quiz.', array('!create_a_quiz' => l(t('create a quiz'), 'node/add/quiz'))),
    '#required' => TRUE,
  );
  $form['quiz_questions_export']['exporter'] = array(
    '#type' => 'select',
    '#title' => t('Export format'),
    '#options' => _questions_exporters(),
    '#description' => t('Select the data format to export into.'),
    '#required' => TRUE,
  );
  $form['quiz_questions_export']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Export'),
  );
  $form['quiz_questions_export']['#submit'][] = 'questions_export_form_submit';
  return $form;
}

/**
 * @return list of implemented formats that can be exported (e.g. CSV, QTI, Aiken)
 */
function _questions_exporters() {
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
  $collection_node = node_load($form_state['values']['quiz_node_title']);
  $exporter = $form_state['values']['exporter'];
  _questions_export_download($collection_node, $exporter);
}

function _questions_export_download($collection_node, $exporter) {
  list($engine, $format) = explode('_', $exporter);

  // TODO use dynamic function names to eliminate this switch
  switch ($engine) {
    case 'moodle':
      $count = _questions_export_moodle($collection_node, $format);
      break;
    case 'native':
      $count = _questions_export_native($collection_node, $format);
      break;
  }
  return $count;
}

/**
 * Exports questions using Quiz module's native export engine.
 * @param $collection_node
 * @param $export_format
 * @return unknown_type
 */
function _questions_export_native($collection_node, $export_format) {
  // $questions is an array of quiz question node object
  $questions = _questions_in_quiz($collection_node);
  ob_start();
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

      case 'choice':
        // FIXME implement 'choice'
        break;

      case 'multichoice':
        foreach ($question->answers as $answer) {
          $feedback = $answer['feedback'] ? $answer['feedback'] : t('nil');
          // use 'nil' when there is no feedback
          array_push($line, $answer['answer'], $feedback);
          $correct_answer = $answer['is_correct'] ? $answer['answer'] : '';
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
    $output .= (count($line) > 2) ? implode($glue, $line) . "\n\n" : '';
    $line = array();
  }
  $filename = str_replace(' ', '_', $collection_node->title) . '.txt';
  $filepath = file_directory_temp() . '/' . $filename;
  $handle = @fopen($filepath, 'w');
  fwrite($handle, $output);
  fclose($handle);
  $headers = array('Content-Type: text/plain', 'Content-Disposition: attachment; filename='. $filename);
  ob_clean();
  file_transfer($filepath, $headers);
  ob_end_clean();
}

/*
 * @param $collection_node
 * quiz/qcollection node object
 *
 * @return
 * a list of question node objects in the given quiz node object
 */
function _questions_in_quiz($collection_node) {
  $questions = array();
  // Get all the questions (ignore `question_status`)
  $sql = "SELECT child_nid as question_nid, child_vid as question_vid
    FROM {quiz_node_relationship}
    WHERE parent_nid = %d
      AND parent_vid = %d
    ORDER BY weight";
  $result = db_query($sql, $collection_node->nid, $collection_node->vid);
  while ($row = db_fetch_array($result)) {
    $question_nodes[] = node_load($row['question_nid'], $row['question_vid']);
  }
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
      'choice' => 'multichoice',
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
    case 'choice':
    $mq->singlequestion = ! $question->choice_multi;
      foreach ($question->alternatives as $alternative) {
        $moodle_answer = new stdClass();
        $moodle_answer->answer = $alternative['answer'];
        $moodle_answer->feedback = $alternative['feedback_if_chosen'];
        $moodle_answer->fraction = $alternative['score_if_chosen'];
        $mq->options->answers[] = $moodle_answer;
      }
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
 * Exports questions using the Moodle export engine.
 *
 */
function _questions_export_moodle($collection_node, $format) {
  global $user;
  ob_start();

  $drupal_questions = _questions_in_quiz($collection_node);
  $moodle_questions = _as_moodle_questions($drupal_questions);

  $file_name = tempnam(variable_get('file_directory_temp', file_directory_temp()), 'quiz');
  $fhandle = @fopen($file_name, 'w'); // The @ suppresses errors.

  global $CFG;
  require_once(drupal_get_path('module', 'quiz') ."/includes/moodle/question/format/$format/format.php");
  $classname = "qformat_$format";
  $qformat = new $classname();
  $qformat->set_can_access_backupdata(false); // not available here

  $qformat->filename = $quiz->title;

  // process using Moodle
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
