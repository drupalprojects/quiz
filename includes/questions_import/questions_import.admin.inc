<?php
// $Id$

/**
 * @file
 * Administration file for Questions Import module
 *
 */

define('QUIZ_QUESTIONS_UPLOAD_ARCHIVE_PATH', 'quiz_questions_import_archive');

module_load_include('php', 'quiz', 'includes/moodle_support');

/**
 * @function
 * Generates link to view sample import files.
 */
function questions_import_get_sample_file_link() {
  $path = drupal_get_path('module', 'questions_import') .'/examples/';
  return array(
    '!csv' => l(t('CSV'), $path . 'csv_example.csv' ),
    '!aiken' => l(t('AIKEN'), $path . 'aiken_example.txt'),
    '!gift' => l(t('GIFT'), $path . 'gift_example.txt'),
    '!learnwise' => l(t('LEARNWISE'), $path . 'learnwise_example.xml'),
    '!qti_v1p2' => l(t('QTI 1.2'), $path . 'qti_v1p2_sample.xml'),
  );
}

/**
 * @function
 * Implementation of hook_form()
 * provides form to upload questions
 *
 * @return
 * FAPI form array
 */

function questions_import_form()  {
  // hook_init() includes the CSS and JS to create hide/show effect in form.
  $form['#attributes'] = array('enctype' => 'multipart/form-data');
  $form['quiz_destination'] = array(
    '#type' => 'fieldset',
    '#title' => t('Questions destination'),
    '#collapse' => FALSE,
  );
  $form['quiz_destination']['destination_type'] = array(
    '#type' => 'select',
    '#title' => t('Import destination'),
    // TODO option for existing item collection
    '#options' => array('new_qcollection' => t('New item collection'), 'new_quiz' => t('New quiz'), 'existing_quiz' => t('Existing quiz')),
    '#required' => TRUE,
  );
  $form['quiz_destination']['destination_title'] = array(
    '#type' => 'textfield',
    '#title' => t('Name of new node'),
    '#default_value' => t('<unnamed>'),
    '#description' => t('Name for the new quiz or item collection'),
    '#size' => 30,
    '#required' => TRUE,
  );
  $options = quiz_get_all_quiz_title();
  // $options is an array with nid as index and quiz node's title as value
  $form['quiz_destination']['quiz_node'] = array(
    '#type' => 'select',
    '#title' => t('Existing quiz'),
    '#options' => $options,
    '#description' => count($options) ? t('Select the quiz to which you want to add questions.') : t('No quiz contents were found. To !create_a_quiz go to Content Management -> Create Content -> Quiz.', array('!create_a_quiz' => l(t('create a quiz'), 'node/add/quiz'))),
  );
  $form['quiz_import'] = array(
    '#type' => 'fieldset',
    '#title' => t('Questions source'),
    '#description' => t('Questions import module allows the user with role "quiz author" to create a bulk of questions from portable text file (like CSV, XML etc) where questions are defined in some predefined format.'),
    '#collapse' => TRUE,
    '#collapsed' => FALSE,
  );
  $form['quiz_import']['import_type'] = array(
    '#type' => 'select',
    '#title' => t('Import type'),
    '#options' => questions_import_type(),
    '#description' => t('Select the import type (sample files !csv, !aiken, !gift, !learnwise, !qti_v1p2)', questions_import_get_sample_file_link()),
    '#required' => TRUE,
  );
  $form['quiz_import']['field_separator'] = array(
    '#type' => 'textfield',
    '#title' => t('CSV Field Separator'),
    '#default_value' => t(','),
    '#description' => t('Special character used to separator the fields usually , : or ; '),
    '#size' => 1,
  );
  //'upload' index will be used in file_check_upload()
  $form['quiz_import']['upload'] = array(
    '#type' => 'file',
    '#title' => t('Upload'),
    '#size' => 30,
    '#description' => t('Upload the file that has quiz questions'),
  );
  $form['advanced'] = array(
    '#type' => 'fieldset',
    '#title' => t('Advanced'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $formats = filter_formats();
  foreach ($formats as $info) {
    $format_options[$info->format] = $info->name;
  }
  $form['advanced']['input_format'] = array(
    '#type' => 'select',
    '#title' => t('Input format'),
    '#options' => $format_options,
    '#required' => TRUE,
  );
  $form['advanced']['ignore_filename_extensions'] = array(
    '#type' => 'checkbox',
    '#title' => t('Ignore filename extension'),
    '#default_value' => FALSE,
    '#description' => t('Normally this form checks that the suffix of the uploaded file.  This allows you to skip that.'),
  );
  $form['advanced']['category_vid'] = array(
    '#type' => 'select',
    '#title' => t('Vocabulary to update by question category'),
    '#options' => _questions_import_vocabulary_options(FALSE),
    '#default_value' => 'NONE',
  );
  $form['advanced']['keyword_vid'] = array(
    '#type' => 'select',
    '#title' => t('Vocabulary to update by question keywords'),
    '#options' => _questions_import_vocabulary_options(TRUE),
    '#default_value' => 'NONE',
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Import'),
  );
  $form['#validate'][] = 'questions_import_form_validate';
  $form['#submit'][] = 'questions_import_form_submit';
  return $form;
}

function _questions_import_vocabulary_options($tags_only) {
  $vocabularies = taxonomy_get_vocabularies();

  $options['NONE'] = t('None');

  foreach ($vocabularies as $vid => $vocabulary) {
    if ($tags_only && !$vocabulary->tags) continue;
    $options[$vid] = $vocabulary->name;
  }

  return $options;
}

/**
 * @return
 * This function is intended to return a list of supported formats for import
 */
function questions_import_type() {
  $type = array(
  // native types
    'native_csv' => t('Comma Separated Values (by Drupal)'),
    'native_aiken' => t('Aiken (by Drupal)'),
    'native_qti12' => t('QTI 1.2 (by Drupal)'),
  // types implemented using Moodle code
  // TODO would be nice to scan the Moodle 'format' directory for types but they don't have display names
    'moodle_aiken' => t('Aiken (by Moodle)'),
  //'moodle_blackboard' => t('Blackboard 6.x (by Moodle)'),
  //'moodle_examview' => t('ExamView (by Moodle)'),
    'moodle_gift' => t('GIFT (by Moodle)'),
  //'moodle_hotpot' => t('Hotpotatoes 6.0 and 6.0 (by Moodle)'),
    'moodle_learnwise' => t('Learnwise (by Moodle)'),
    'moodle_webct' => t('WebCT (by Moodle)'),
  // TODO fix HotPotatoes, ExamView and Blackboard
  );
  return $type;
}

/**
 * Implementation of hook_validate()
 */
function questions_import_form_validate($form, &$form_state) {

  // TODO make the allowed extensions depend on the import type
  $allowed_extensions = 'csv txt xml jcl jqz';

  // it suppose to be a string which file extensions separated by space not an array
  $allowed_size = file_upload_max_size();

  $field_separator = $form_state['values']['field_separator'];
  $import_type_info = explode('_', $form_state['values']['import_type']);
  $import_engine = $import_type_info[0];
  $import_format = $import_type_info[1];
  //$question_type =  $form_state['values']['question_type'];

  // put in a quiz_import dir within the files directory
  $import_archive_path =  file_directory_path() .'/'. QUIZ_QUESTIONS_UPLOAD_ARCHIVE_PATH;
  file_check_directory($import_archive_path, FILE_CREATE_DIRECTORY);
  // creates a drupal file object, to be saved in form_state as validated_file
  // one thread says this is bad, http://drupal.org/node/73684
  // but another says it's proper, http://www.imedstudios.com/labs/node/22
  // and it works. -ta
  $file = file_save_upload('upload', array(), $import_archive_path);

  if (!$file) {
    form_set_error('upload', 'You must select a valid file to upload.');
  }

  //there is a bug in Drupal file validator, refer  http://api.drupal.org/api/function/file_validate_extensions it Bypass validation for uid  = 1 so using custom validation functions which is same as drupal file_validate_extensions but it does not Bypass for uid = 1

  else {

    $validate_extensions = !($form_state['values']['ignore_filename_extensions']);
    if ($validate_extensions)
      $error_msg = question_import_validate_extensions($file, $allowed_extensions);
    if ($error_msg != '') {
      form_set_error('upload', $error_msg);
    }

    $error_msg = question_import_validate_size($file, $allowed_size);
    if ($error_msg != '') {
      form_set_error('upload', $error_msg);
    }

    switch ($import_engine) {
      case 'native':
        $validator_function_name = "questions_import_validate_$import_format";
        $error_msg = $validator_function_name($file, $field_separator);
        break;
      case 'moodle':
        // FIXME no validating yet for Moodle
        break;
    }

    if ($error_msg != '') {
      form_set_error('upload', $error_msg);
    } else {
      // assume upload file validates so keep it
      $success = file_set_status($file, FILE_STATUS_PERMANENT);
      if ($success) {
        $form_state['values']['validated_file'] = $file;
      } else {
        form_set_error('upload', "Failed to store file permanently");
      }
    }
  }
}

/**
 * @function
 * This function checks whether the Question and Test Interoperability (QTI) XML
 * file is in proper format or not.
 */

function questions_import_validate_qti12($file, $field_separator) {
  $error_msg = '';
  $row = 0;
  $lines = file($file->filepath);
  if (empty($lines)) {
    form_set_error('xmlfile', 'File could not be uploaded. Please try again.');
  }
}


/**
 * @function
 * This function checks whether the aiken import file is in proper format or not.
 *
 */
function questions_import_validate_aiken($file, $separator) {
  /*
   $error_msg = '';
   $row = 0;
   $lines = file($file->filepath);

   if (empty($lines) || (count($lines) < 4)) {
   return '<p>' . t('Invalid number of lines or no lines were found in @filename.', array('@filename' => $file->filename)) . '</p>';
   }

   if ($question_type = 'multichoice') {
   while (!empty($lines)) { // while not empty of file content
   while ($current_line = trim(array_shift($lines))) {
   if (empty($current_line)) {
   break;
   }
   $line[] = $current_line;
   }
   // it should have read a questions, choices and its correct answer

   if (count($line) < 4) {
   $error_msg .= '<p>' . t('Error around line : @line_number', array('@line_number' => $row)) . '</p>';
   }

   $answer = trim(array_pop($line));
   if (stristr($answer, 'ANSWER') === FALSE) {
   $error_msg .= '<p>' . t('Error around line : @line_number', array('@line_number' => $row)) . '</p>';
   }

   // now $line is left only with choices which looks like A) Moodle B) ATutor C) Claroline D) Blackboard etc
   ++$row;
   }
   }

   $error_msg .= !empty($error_msg) ? '<p>' . t('Aiken Import Failed. These lines were found to have an invalid number of fields in @filename.', array('@filename' => $file->filename)) . '</p>' : '';
   return $error_msg;
   */
}

/**
 * @function
 * This function checks whether the csv import file is in proper format or not.
 */

function questions_import_validate_csv($file, $separator) {
  $error_msg = '';
  $row = 0;
  $lines = file($file->filepath);
  //reads the whole file content to an array
  if (empty($lines)) {
    return '<p>' . t('No lines were found in @filename.', array('@filename' => $file->filename)) . '</p>';
  }

  foreach ($lines as $line) {
    $line = check_plain(trim($line));
    if (!empty($line)) {
      ++$row; // alway use pre_increment it is faster than post increment
      $fields = explode($separator, $line);
      switch ($field[0]) {
        case 'multichoice':
          $error_msg .= (count(fields) < 4) ? '<p>' . t('line : ') . $row . ' '. $line . ' </p>' : '';
          break;
        case 'true_false':
          $error_msg .= '';
      }
    }
  }

  $error_msg .= !empty($error_msg) ? '<p>' . t('CSV Import Failed. These lines were found to have an invalid number of fields in @filename.', array('@filename' => $file->filename)) . '</p>' : '';
  return $error_msg;
}

/**
 * @function
 * gets the destination node to import quiz questions.
 * @return
 * quiz node object to which questions has to be imported.
 */
function _get_destination_node($form_state) {
  global $user;

  $destination_type = $form_state['values']['destination_type'];

  if ($destination_type == 'existing_quiz') {
    $quiz_nid = $form_state['values']['quiz_node'];
    return node_load($quiz_nid);
  }

  else {
    // create a new node
    $title = $form_state['values']['destination_title'];
    list($tmp, $new_type) = explode('_', $destination_type);

    $node = new stdClass();
    $node->type = $new_type;
    $node->title = $title;
    $node->uid = $user->uid;
    $node->format = $form_state['values']['input_format'];
    $node->status = 1; // published by default
    $node->comment = 0; // comments disabled by default
    $node->log = 'Created by Quiz questions importer.';
    node_save($node);

    return $node;
  }
}

/**
 * @function
 * This is a generic questions import submit function calls specific import
 * function like questions_import_submit_csv, questions_import_submit_multichoice_aiken
 *
 * @return
 * integer number of questions imported to a quiz node.
 */
function questions_import_form_submit(&$form, &$form_state) {

  $time = 0;
  $op = '';

  $destination_node = _get_destination_node($form_state);

  $import_type_info = explode('_', $form_state['values']['import_type']);
  $import_engine = $import_type_info[0];
  $import_format = $import_type_info[1];
  $question_type = $form_state['values']['question_type'];

  $start = microtime(TRUE);

  $status = array();

  // would be nice to make this a TIMESTAMP field so it's automatic, but that would bind it to MySQL
  $date = date('YmdHis');
  $fid = $form_state['values']['validated_file']->fid;

  db_query("INSERT INTO {quiz_questions_import_record} (file_id, datetime, status_array, importer) VALUES (%d, '%s', '%s', '%s')", $fid, $date, serialize($status), $form_state['values']['import_type']);

  $import_id = db_last_insert_id('quiz_questions_import_record', 'import_id');

  switch ($import_engine) {
    case 'native':
      $file = file_save_upload('upload');
      $import_function_name = "questions_import_submit_$import_format";
      $count = $import_function_name($destination_node, $form, $form_state, $import_id);
      break;
    case 'moodle':
      $count = questions_import_submit_moodle_format($destination_node, $import_format, $form, $form_state, $import_id);
      break;
  }
  // $count contains number of questions successfully imported.
  $end = microtime(TRUE);
  $time =  substr($end - $start, 0, 6);
  if ($count) {
    drupal_set_message(t('@count questions were imported successfully in @time seconds.', array('@count' => $count, '@time' => $time)));
    /*
     * when the questions destination is "new_quiz" questions importer will
     * create a quiz node by itself, whose properties were not defined properly.
     * so redirecting the quiz author to quiz node/$nid/edit page
     * where he/she can set the quiz node properties.
     */
    $url = ($form_state['values']['destination_type'] == 'new_quiz') ? "node/$destination_node->nid/edit" : "node/$destination_node->nid";
    drupal_goto($url);
  }
  return $count;
}

function questions_import_submit_moodle_format($destination_node, $format, $form, $form_state, $import_id) {
  $file = $form_state['values']['validated_file'];

  module_load_include('php', 'quiz', "includes/moodle/question/format/$format/format");

  // e.g. qformat_webct
  $classname = "qformat_$format";
  $fHandler = new $classname();

  assert($fHandler->provide_import());

  $lines = file($file->filepath);
  $moodle_questions = $fHandler->readquestions($lines);

  $import_count = 0;

  // iterate over Moodle questions to make Drupal Quiz questions
  foreach ($moodle_questions as $mq) {
    questions_import_moodle_create_node($destination_node, $mq, $form_state, $row, $import_id);
    ++$import_count;
  }

  return $import_count;
}

function questions_import_moodle_create_node($destination_node, $mq, $form_state, $row, $import_id) {

  $qmap_drupal_to_moodle = array(
    // Drupal => Moodle
    'long_answer' => 'essay',
    'short_answer' => 'shortanswer',
    'multichoice' => 'multichoice',
    'true_false' => 'truefalse',
    'quiz_directions' => 'description',
    'matching' => 'match',
  );
  $qmap_moodle_to_drupal = array_flip($qmap_drupal_to_moodle);

  global $user;

  $node = new stdClass();
  $node->type = $qmap_moodle_to_drupal[$mq->qtype];
  if (empty($node->type)) {
    drupal_set_message(t('Skipped importation of unsupported question type "qtype"', array('qtype' => $mq-qtype)), 'warning', FALSE);
    watchdog('quiz', 'Skipped importation of Moodle question qtype', array('qtype' => $mq->qtype, 'mq' => print_r('mq', TRUE)));
    return;
  }
  $node->title = $mq->name;
  $node->teaser = $node->body = stripslashes_safe($mq->questiontext);  // a la Moodle
  $node->uid = $user->uid;
  $node->format = $form_state['values']['input_format'];
  $node->log = 'Imported with Moodle importer.';

  $node->quiz_id = $destination_node->nid;
  $node->quiz_vid = $destination_node->vid;

  // set taxonomy, if specified
  $keyword_vid = $form_state['values']['keyword_vid'];
  if (is_numeric($keyword_vid)) {
    // FIXME wrap each term in quotes in case terms have commas
    $node->taxonomy['tags'][$keyword_vid] = implode(",", $mq->keywords);
  }
  $category_vid = $form_state['values']['category_vid'];
  if (is_numeric($category_vid)) {
    // treat category as 'tags' too so taxonomy_node_save will create missing terms
    $node->taxonomy['tags'][$category_vid] = $mq->category;
  }

  // Already set the type of the node, this is for specialized processing
  switch (strtolower($mq->qtype)) {
    case 'match':
      $node->match = array();
      foreach ($mq->subquestions as $index => $sub_question) {
        $sub_answer = $mq->subanswers[$index];

        // TODO what about 'feedback' ?
        $matching_pair = array('question'=>$sub_question, 'answer' => $sub_answer);
        $node->match[] = $matching_pair;
      }
      break;

    case 'shortanswer':
      // Drupal short_answer node fields: maximum_score, correct_answer, correct_answer_evaluation
      $node->correct_answer = $mq->answer[1];
      $node->correct_answer_evaluation = ShortAnswerQuestion::ANSWER_INSENSITIVE_MATCH;
      $node->maximum_score = $mq->defaultgrade;
      break;

    case 'multichoice':
      $node->alternatives = array();

      // whether user can make multiple selections
      $node->choice_multi = ! $mq->single;

      // Add answer alternatives
      foreach ($mq->answer as $index => $answer) {
        $node->alternatives[] = array(
          // answer, answer_format, feedback_if_chosen, feedback_if_chosen_format,
          // feedback_if_not_chosen, feedback_if_not_chosen_format,
          // score_if_chosen, score_if_not_chosen
          'answer' => $answer,
          'answer_format' => $node->format,
          'feedback_if_chosen' => $mq->feedback[$index],
          'feedback_if_chosen_format' => $node->format,
          'feedback_if_not_chosen' => '',
          'feedback_if_not_chosen_format' => $node->format,
          'score_if_chosen' => $mq->fraction[$index],
          'score_if_not_chosen' => 0,
        );
      }
      break;
  }

  node_save(questions_import_node_save_static_data($node));

  db_query("INSERT INTO {quiz_questions_import_items} VALUES (%d, %d, %d)", $import_id, $node->nid, $row);
}

/**
 * This function imports questions from Question and Test Interoperability (QTI) format file.
 * @return
 * Return the number of questions successfully imported.
 */
function questions_import_submit_qti12($destination_node, $form, $form_state, $import_id) {
  $file = $form_state['values']['validated_file'];
  $row = 0;
  $qti_items = questions_import_qti_extract_info($file->filepath);

  // Loop through each question and import it into Drupal
  foreach ($qti_items as $item) {
    questions_import_qti_create_node($destination_node, $item, $form_state, $row, $import_id);
    ++$row;
  }
  return $row;
}

/**
 * Take a description of a quiz question and turn it into a node.
 *
 * The node is saved as the appropriate Quiz question type.
 */
function questions_import_qti_create_node($destination_node, $item, $form_state, $row, $import_id) {
  global $user;
  $item = (object)$item;
  $node = new stdClass();
  $node->title = $item->title;
  $node->teaser = $node->body = $item->content;
  $node->uid = $user->uid;
  $node->format = $form_state['values']['input_format'];
  $node->status = 1;
  $node->log = 'Imported from QTI importer.';
  $node->quiz_id = $destination_node->nid;
  $node->quiz_vid = $destination_node->vid;

  switch (strtolower($item->type)) {
    /*case 'explanation':
     $node->type = 'quiz_directions';
     break;
     case 'essay':
     $node->type = 'long_answer';
     $node->maximum_score = 1;
     break;*/
    case 'multiple choice':
      $node->type = 'multichoice';
      $answers = $item->answers;
      $node->number_of_answers = count($answers);
      // Add answers:
      $node->answers = array();
      foreach ($answers as $answer) {
        $node->answers[] = array(
          'answer' => $answer['text'],
          'feedback' => $answer['feedback'],
          'correct' => $answer['is_correct'],
          'result_option' => '0', // No support for personality.
        );
      }
      break;
  }
  node_save(questions_import_node_save_static_data($node));
  db_query("INSERT INTO {quiz_questions_import_items} VALUES (%d, %d, %d)", $import_id, $node->nid, $row);
}

/**
 * Given a QTI 1.2 XML file, extract quiz questions.
 */
function questions_import_qti_extract_info($file) {
  $items = array();
  foreach (qp($file, 'item') as $item) {
    $title = $item->attr('title');
    $type = $item->find('itemmetadata>qmd_itemtype')->text();
    $body = $item->end()->find('presentation>material>mattext');
    if ($body->attr('texttype') == 'text/html') {

      $bodytext = $body->text();

      if (strpos($bodytext, '<html') === FALSE) {
        drupal_set_message('Adding HTML', 'status');
        $bodytext =  '<html>' . $bodytext . '</html>';
      }

      // Load here so that errors are caught by Drupal (which has no exception handler.)
      $doc = new DOMDocument();
      $doc->loadHTML($bodytext);

      $html = qp($doc, 'body');
      $contents = $html->get(0)->childNodes;

      // Extract content from HTML and put it in a temp document.
      $newdoc = qp('<?xml version="1.0"?><div id="qti-question-body"/>');
      $i = 0;
      while ($node = $contents->item($i++)) {
        $newdoc->append($node);
      }
      $out = $newdoc->html(); // This leaves off XML declaration.
    }
    else {
      $out = $body->text();
    }

    $new_item = array(
      'title' => $title,
      'type' => $type,
      'content' => $out,
    );

    // Handle multiple choice questions:
    if (strtolower($type) == 'multiple choice') {
      $answers = array();

      // First, get all anssers and loop through them.
      $answerstexts = $item->parent('item')->find('response_lid>render_choice>response_label>material>mattext');
      foreach ($answerstexts as $answertext) {

        // As we find each answer, grab a bunch of related data. Processing-wise, this is not terribly
        // efficient, since we are hoping back and forth inside of the document. However, it is much easier
        // to do this all together.
        $text = $answertext->text();
        $index = $answertext->parent('response_label')->attr('ident');

        // This filter grabs the answer setting by index. Most of the time, index appears to be
        // an alpha char.
        $contains_filter = 'resprocessing>respcondition>conditionvar>varequal:contains(' . $index . ')';
        $correct = $answertext->parent('item')->find($contains_filter)->parent('respcondition')->find('setvar')->text();
        if ($correct == 0) {
          $feedback = $answertext->parent('item')->find('itemfeedback[ident="Wrong Answer"]>material>mattext')->text();
        }
        else {
          $feedback = 'Correct';
        }

        // Store all of this in an array.
        $answers[] = array(
          'text' => $text,
          'index' => $index,
          'is_correct' => $correct,
          'feedback' => $feedback,
        );

      }
      // Store answers
      $new_item['answers'] = $answers;
    }
    // Store questions
    $items[] = $new_item;
  }
  return $items;
}

/**
 * @function
 * This function imports questions from Moodle Aiken format file.
 * @return
 * Return the number of questions successfully imported.
 */
function questions_import_submit_aiken($destination_node, $form, $form_state, $import_id) {
  global $user;
  $row = 0;
  $output = '';
  $line = array();

  $file = $form_state['values']['validated_file'];
  $lines = file($file->filepath);

  // while not empty of file content
  while (!empty($lines)) {
    while ($current_line = questions_import_get_next_aiken_field($lines)) {
      // get the whole question fields to an array
      if (empty($current_line)) {
        // No more question fields. we have reached the end, exit from this loop.
        break;
      }
      if ($current_line[0] === '#') {
        // ho its a comment line, just ignore it
        continue;
      }
      $line[] = $current_line;
    }
    if (empty($line)) {
      // this line is empty. Get the next line
      continue;
    }
    // now $line is an array holding question fields.
    $type = questions_import_get_next_aiken_field($line);
    if (!in_array($type, quiz_get_questions_type('enabled'))) {
      drupal_set_message(t('Unable to import @type type question. You may need to enable @type module.', array('@type' => $type)),'error');
      // oops you have miss spelled question type or you are trying to import question whose module is disabled.
      // set an error message and read the next question.
      $line = array();
      //empty the $line to load the next question
      continue;
    }
    $node = new stdClass();
    $node->type = $type;
    $node->format = $form_state['values']['input_format'];
    $node->quiz_id = $destination_node->nid;
    $node->quiz_vid = $destination_node->vid;
    $node->type = $type;
    $node->title = $node->body = $node->teaser = questions_import_get_next_aiken_field($line);

    $options = array(); // clear the options array for a new question

    switch ($node->type) {
      case 'multichoice':
        $options = array();
        $answer = array_pop($line);
        // now $line is left only with choices which looks like A) Moodle B) ATutor C) Claroline D) Blackboard etc
        while (!empty($line)) {
          $l = questions_import_get_next_aiken_field($line);
          $option = explode($l[1], $l);
          $options[trim($option[0])]['choice'] = trim($option[1]);
          $feedback = questions_import_get_next_aiken_field($line);
          $options[trim($option[0])]['feedback'] = ($feedback == 'nil') ? '' : $feedback;
        }
        $correct = substr(trim($answer), '-1');
        $answer = $options[$correct]['choice'];
        $line = array();
        // empty the $line and load the next question fields into it.
        $node->num_answers = count($options);
        $node->answers = array();
        foreach ($options as $option) {
          $node->answers[] = array(
            'correct' => (trim($answer) == trim($option['choice'])) ? 1 : 0,
            'answer' => trim($option['choice']),
            'feedback' => trim($option['feedback']),
          );
        }
        break;

      case 'true_false':
        $node->correct_answer = (questions_import_get_next_aiken_field($line) == 'true') ? 1 : 0;
        $feedback = questions_import_get_next_aiken_field($line);
        $node->feedback = ($feedback === 'nil') ? '' : $feedback;
        break;

      case 'matching':
        $node->match = array();
        while (!empty($line)) {
          $row = explode(',', questions_import_get_next_aiken_field($line));
          $node->match[] = array(
            'question' => $row[0],
            'answer' => $row[1],
            'feedback' => $row[2] == 'nil' ? '' : $row[2],
          );
        }
        break;

      case 'short_answer':
        $evaluation_type = array('case sensitive match', 'case insensitive match', 'regular expression match', 'manually score match');
        $node->correct_answer = questions_import_get_next_aiken_field($line);
        $node->maximum_score = (int) questions_import_get_next_aiken_field($line);
        $node->correct_answer_evaluation = (int) array_search(questions_import_get_next_aiken_field($line), $evaluation_type);
        // if array_search return FALSE we convert it to int 0 and take the default format 'case sensitive match'
        break;

      case 'long_answer':
        $node->maximum_score = questions_import_get_next_aiken_field($line);
        break;

      case 'quiz_directions':
        break;
    }
    node_save(questions_import_node_save_static_data($node));

    db_query("INSERT INTO {quiz_questions_import_items} VALUES (%d, %d, %d)", $import_id, $node->nid, $row);

    ++$row;
  }
  return $row;
}

function questions_import_get_next_aiken_field(&$row) {
  return trim(array_shift($row));
}

/**
 * @function
 * This function imports multichoice questions from CSV file.
 * @param $lines
 *  It contains the whole csv file content.
 * @return
 *  Return the number of questions successfully imported.
 */
function questions_import_submit_csv($destination_node, $form, $form_state, $import_id) {
  global $user;
  $row = 0;
  $output = '';

  $file = $form_state['values']['validated_file'];
  $separator = $form_state['values']['field_separator'];

  // iterate through all the lines in the file. Each line corresponds to one quiz question
  $lines = file($file->filepath);
  foreach ($lines as $line) {
    $line = trim($line);
    if (empty($line) || $line[0] === '#') {
      continue;
    }

    $node = new stdClass();
    //create node object
    $node->format = $form_state['values']['input_format'];
    $node->quiz_id = $destination_node->nid;
    $node->quiz_vid = $destination_node->vid;
    $line = explode($separator, $line);
    // The line variable looks something like
    // Question Type, Question, Choice 1, Feedback Ch1, Choice 2, Feedback Ch2, ...,Correct Choice
    // eg multichoice, which of the following is an IDE ?, Ubuntu, Ubuntu is a Linux Distro, Geany, Geany is a text editor for gnome, Eclipse, Eclipse is an IDE, Drupal, Drupal is an kick ass CMS, Eclipse
    $type = questions_import_get_next_csv_field($line);
    if (!in_array($type, quiz_get_questions_type('enabled'))) {
      drupal_set_message(t('Unable to import @type type question. You may need to enable @type module.', array('@type' => $type)),'error');
      continue;
    }
    $node->type = $type;
    $question = questions_import_get_next_csv_field($line);
    switch ($type) {

      case 'multichoice':
        $correct_answer = array_pop($line);
        $node->num_answers = count($options);
        $node->answers = array();
        while (!empty($line)) {
          $answer = questions_import_get_next_csv_field($line);
          $feedback = questions_import_get_next_csv_field($line);
          $node->answers[] = array(
            'answer' => $answer,
            'feedback' => $feedback === 'nil' ? '' : $feedback,
            'correct' => (trim($answer) == trim($correct_answer)) ? 1 : 0,
          );
        }
        break;

      case 'true_false':
        $node->correct_answer = (questions_import_get_next_csv_field($line) == 'true') ? 1 : 0;
        $feedback = questions_import_get_next_csv_field($line);
        $node->feedback = $feedback === 'nil' ? '' : $feedback;
        break;

      case 'matching':
        $node->match = array();
        while (!empty($line)) {
          $match = array(
            'question' => questions_import_get_next_csv_field($line),
            'answer' => questions_import_get_next_csv_field($line),
            'feedback' => questions_import_get_next_csv_field($line),
          );
          $node->match[] = array(
            'question' => $match['question'],
            'answer' => $match['answer'],
            'feedback' => $match['feedback'] ? '' : $match['feedback']
          );
        }
        break;

      case 'long_answer':
        $node->maximum_score = questions_import_get_next_csv_field($line);
        break;

      case 'short_answer':
        $evaluation = array(
          'case sensitive match',
          'case insensitive match',
          'regular expression match',
          'manually score match'
          );
          $node->correct_answer = questions_import_get_next_csv_field($line);
          $node->maximum_score = questions_import_get_next_csv_field($line);
          $node->correct_answer_evaluation = (int) array_search(questions_import_get_next_csv_field($line), $evaluation);
          // if array_search return FALSE we convert it to int 0 and take the default format 'case sensitive match'
          break;

      case 'quiz_directions':
        break;

    }
    $node->title = $node->body = $node->teaser = $question;
    node_save(questions_import_node_save_static_data($node));

    db_query("INSERT INTO {quiz_questions_import_items} VALUES (%d, %d, %d)", $import_id, $node->nid, $row);

    ++$row;
  }
  return $row;
}

function questions_import_get_next_csv_field(&$row) {
  return trim(array_shift($row));
}

/**
 * @function
 * This function sets the static data pertaining to node object
 * @return
 * a quiz node object
 */
function questions_import_node_save_static_data(&$node) {
  global $user;
  $node->uid = $user->uid;
  $node->name = $user->name;
  $node->promote = 0;
  $node->sticky = 0;
  $node->status = 1;
  $node->comment = 0;
  $node->moderate = 0;
  $node->multiple_answers = 0;
  $node->more = 0;
  $node->validate = 1;
  $node->is_new = 1;
  $node->scored_quiz = 1;
  $node->revision = 1;
  $node->op = t('Save');
  $node->preview = t('Preview');
  return $node;
}

/**
 * @function
 * Checks the extension of import file.
 * @return
 * Return error message if the import file extension is not in $extensions array.
 */
function question_import_validate_extensions($file, $extensions) {
  global $user;
  $errors = '';
  // Bypass validation for uid  = 1.
  //if ($user->uid != 1) {
  $regex = '/\.('. ereg_replace(' +', '|', preg_quote($extensions)) .')$/i';
  if (!preg_match($regex, $file->filename)) {
    $errors = '<p>' . t('Only files with the following extensions are allowed: %files-allowed.', array('%files-allowed' => $extensions)) . '</p>';
  }
  //}
  return $errors;

}


/**
 * @function
 * Checks the size of import file.
 * @return
 * Return error message if file size exceed maximum file size or disk quota of user.
 */

function question_import_validate_size($file, $file_limit = 0, $user_limit = 0) {
  global $user;
  $errors = '';
  // Bypass validation for uid  = 1.
  //if ($user->uid != 1) {
  if ($file_limit && $file->filesize > $file_limit) {
    $errors = '<p>' . t('The file is %filesize exceeding the maximum file size of %maxsize.', array('%filesize' => format_size($file->filesize), '%maxsize' => format_size($file_limit))) . '</p>';
  }
  $total_size = file_space_used($user->uid) + $file->filesize;
  if ($user_limit && $total_size > $user_limit) {
    $errors = '<p>' . t('The file is %filesize which would exceed your disk quota of %quota.', array('%filesize' => format_size($file->filesize), '%quota' => format_size($user_limit))) . '</p>';
  }
  //}
  return $errors;
}
