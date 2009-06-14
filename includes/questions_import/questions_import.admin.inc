<?php
// $Id$

/*
 * @file
 * Administration file for Questions Import module
 *
 */

require_once(drupal_get_path('module', 'quiz') .'/includes/moodle_support.php');


/*
 * Implementation of hook_form
 * form to upload questions
 */

function questions_import_form()  {
  $form['#attributes'] = array('enctype' => 'multipart/form-data');

  $form['quiz_node'] = array(
    '#type' => 'select',
    '#title' => t('Quiz Node'),
    '#options' => quiz_get_all_quiz_title(),
    '#description' => t('Select the quiz node under which you want to add questions'),
    '#required' => TRUE,
  );

  /*
  // TODO the formats specify this so this user input doesn't make sense
  $form['question_type'] = array(
    '#type' => 'select',
    '#title' => t('Question type'),
    '#options' => questions_import_question_type(),
    '#description' => t('Select the quiz question type you wish to upload (only applicable with CSV)'),
    '#required' => FALSE,
  );
  */ 

  $form['import_type'] = array(
    '#type' => 'select',
    '#title' => t('Import type'),
    '#options' => questions_import_type(),
    '#description' => t('Select the import type csv, XML'),
    '#required' => TRUE,
  );

  $form['field_separator'] = array(
    '#type' => 'textfield',
    '#title' => t('CSV Field Separator'),
    '#default_value' => t(','),
    '#description' => t('Special character used to separator the fields usually , : or ; '),
    '#size' => 3,
    //'#required' => TRUE,
  );

  //'upload' index will be used in file_check_upload()
  $form['upload'] = array(
    '#type' => 'file',
    '#title' => t('Upload'),
    '#size' => 30,
    '#description' => t('Upload the file that has quiz questions'),
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Import'),
  );

  //$form['#validate'][] = 'questions_import_form_validate';
  $form['#submit'][] = 'questions_import_form_submit';

  return $form;
}

/*
 * @return
 * this function return a list of available quiz questions type.
 */
function questions_import_question_type() {
  $list = array();
  //checks whether the multichoice module is enabled or not.
  if (module_exists('multichoice')) {
    $list['multichoice'] = t('Multichoice');
  }
  /*if (module_exists('foo_bar')) {
    $list['xml'] = t('XML');
  }*/
  return $list;
}

/*
 * @return
 * This function is intended to return a list of supported formats for import
 */
function questions_import_type() {
  $type = array(
    // native types
    'native_csv' => t('Comma Separated Values (CSV)'),
    'native_aiken' => t('Aiken'),
    'native_qti12' => t('QTI 1.2'),
    // types implemented using Moodle code
    // TODO would be nice to scan the Moodle 'format' directory for types but they don't have display names
    'moodle_aiken' => t('Aiken (by Moodle)'),
    'moodle_blackboard' => t('Blackboard 6.x (by Moodle)'),
    'moodle_examview' => t('ExamView (by Moodle)'),
    'moodle_gift' => t('GIFT (by Moodle)'),
    'moodle_hotpot' => t('Hotpotatoes 5.0 and 6.0 (by Moodle)'),
    'moodle_learnwise' => t('Learnwise (by Moodle)'),
    'moodle_webct' => t('WebCT (by Moodle)'),
  );
  return $type;
}

/*
 *
 * Implementation of hook_validate()
 */
function questions_import_form_validate($form, &$form_state) {
  $allowed_extensions = 'csv txt xml jcl';
  // it suppose to be a string which file extensions separated by space not an array
  $allowd_size = file_upload_max_size();

  $field_separator = $form_state['values']['field_separator'];
  $import_type_info = explode('_', $form_state['values']['import_type']);
  $import_engine = $import_type_info[0];
  $import_format = $import_type_info[1];
  //$question_type =  $form_state['values']['question_type'];

  $file = file_save_upload('upload');
  // creates a drupal file object

  if (!$file) {
    form_set_error('upload', 'You must select a valid file to upload.');
  }

  //there is a bug in Drupal file validator, refer  http://api.drupal.org/api/function/file_validate_extensions it Bypass validation for uid  = 1 so using custom validation functions which is same as drupal file_validate_extensions but it does not Bypass for uid = 1

  else {

    $error_msg = question_import_validate_extensions($file, $allowed_extensions);
    if ($error_msg != '') {
      form_set_error('upload', $error_msg);
    }

    $error_msg = question_import_validate_size($file, $allowd_size);
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
    }
  }
}

/*
 * @function
 * This function checks whether the Question and Test Interoperability (QTI) XML
 * file is in proper format or not.
 */

function questions_import_validate_qti12($file, $field_separator, $question_type) {
  $error_msg = '';
  $row = 0;
  $lines = file($file->filepath);
  if (empty($lines)) {
    form_set_error('xmlfile', 'File could not be uploaded. Please try again.');
  }
}


/*
 * @function
 * This function checks whether the aiken import file is in proper format or not.
 *
 */
function questions_import_validate_aiken($file, $separator, $question_type) {
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
    /*  foreach ($line as $l) {
      // yet to add more validation code
      } */
      ++$row;
    }
  }

  $error_msg .= !empty($error_msg) ? '<p>' . t('Aiken Import Failed. These lines were found to have an invalid number of fields in @filename.', array('@filename' => $file->filename)) . '</p>' : '';
  return $error_msg;
}

/*
 * @function
 * This function checks whether the csv import file is in proper format or not.
 */

function questions_import_validate_csv($file, $separator) {
  $error_msg = '';
  $row = 0;
  $lines = file($file->filepath);
  $question_type = 'multichoice';
  //reads the whole file content to an array
  if (empty($lines)) {
    return '<p>' . t('No lines were found in @filename.', array('@filename' => $file->filename)) . '</p>';
  }
  if ($question_type == 'multichoice') {
    foreach ($lines as $line) {
      $line = check_plain(trim($line));
      if (!empty($line)) {
        ++$row; // alway use pre_increment it is faster than post increment
        $fields = explode($separator, $line);
        $field_count = count($fields);
        /* a line should have minmum of four fields i.e
         * question, option1, option2, correct option
         * or maximum of 6 fields where additional two options will be option3 and option4 */
        if ($field_count < 4) {
          $error_msg .= '<p>' . t('line : ') . $row . ' '. $line . ' </p>';
        }
      }
    }
  }
  $error_msg .= !empty($error_msg) ? '<p>' . t('CSV Import Failed. These lines were found to have an invalid number of fields in @filename.', array('@filename' => $file->filename)) . '</p>' : '';
  return $error_msg;
}

/*
 * This is a generic questions import submit calls specific import function like questions_import_submit_multichoice_csv, questions_import_submit_multichoice_aiken
 *
 */

function questions_import_form_submit(&$form, &$form_state) {
  $time = 0;
  $op = '';
  $quiz_nid = $form_state['values']['quiz_node'];
  $import_type_info = explode('_', $form_state['values']['import_type']);
  $import_engine = $import_type_info[0];
  $import_format = $import_type_info[1];
  $question_type = $form_state['values']['question_type'];

  $start = microtime(TRUE);
  switch ($import_engine) {
    case 'native':
      $file = file_save_upload('upload');
      $import_function_name = "questions_import_submit_$import_format";
      $count = $import_function_name($fileform, $form_state, file($file->filepath));
      break;
    case 'moodle':
      $count = questions_import_submit_moodle_format($import_format, $form, $form_state);
      break;
  }
  // $count contains number of questions successfully imported.
  $end = microtime(TRUE);
  $time =  substr($end - $start, 0, 6);
  if ($count) {
    drupal_set_message(t('@count questions were imported successfully in @time seconds.', array('@count' => $count, '@time' => $time)));
    drupal_goto('node/' . $quiz_nid . '/questions');
  }  
  return $count;
}

function questions_import_submit_moodle_format($format, $form, $form_state) {
  // common prologue, refactor?
  $quiz_nid = $form_state['values']['quiz_node'];
  $quiz_vid = node_load($quiz_nid);
  $file = file_save_upload('upload');
  //creates drupal file object
  $lines = file($file->filepath);

  // TODO autoloader?
  global $CFG;
  require_once(drupal_get_path('module', 'quiz') ."/includes/moodle/question/format/$format/format.php");

  // e.g. qformat_webct

  $classname = "qformat_$format";
  //if (module_exists('devel')) {
    //print "instantiating '$classname'";
  //}
  $fHandler = new $classname();

  //if (module_exists('devel')) {
    //dprint_r($fHandler);
  //}

  assert($fHandler->provide_import());

  $moodle_questions = $fHandler->readquestions($lines);
  //if (module_exists('devel')) {
    //dprint_r($moodle_questions);
    // exit;
  //}

  $import_count = 0;

  // iterate over Moodle questions to make Drupal Quiz questions
  foreach ($moodle_questions as $mq) {
    questions_import_moodle_create_node($mq, $form_state);
    ++$import_count;
  }

  return $import_count;
}

function questions_import_moodle_create_node($mq, $form_state) {

  $qmap_drupal_to_moodle = array(
    // Drupal to Moodle
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
  $node->title = $mq->name;
  $node->teaser = $node->body = $mq->questiontext;
  $node->uid = $user->uid;
  $node->status = 1;
  $node->log = 'Imported with Moodle importer.';

  $quiz_nid = $form_state['values']['quiz_node'];
  $quiz_vid = node_load($quiz_nid);
  $node->quiz_id = $quiz_nid;
  $node->quiz_vid = $quiz_vid->vid;

  // already set the type of the node, this is for specialized processing
  switch (strtolower($mq->qtype)) {
    case 'match':
    /* match [answer] to [subanswer] ?

      [answer] => Array
          (
              [1] => R<SUB>1</SUB>
              [2] => R<SUB>2</SUB>
              [3] => R<SUB>3</SUB>
              [4] => R<SUB>4</SUB>
          )

      [subanswers] => Array
          (
              [1] => P<SUB>4</SUB>
              [2] => P<SUB>3</SUB>
              [3] => P<SUB>1</SUB>
              [4] => P<SUB>2</SUB>
          )
          */
          // TODO warn that matching is not supported
          break;

    case 'shortanswer':
    case 'multichoice':
      $node->number_of_answers = count($mq->answer);

      // Add answers:
      $node->answers = array();
      foreach ($mq->answer as $index => $answer) {
        $node->answers[] = array(
          // FIXME image tags are being filter, e.g. in StrongestAcid1
          'answer' => $answer,
          'feedback' => $mq->feedback[$index],
          'correct' => ($mq->fraction[$index] == 1), // TODO support fractional credit
          'result_option' => '0', // No support for personality.
        );
      }
      break;
  }
  //node_save($node);
  node_save(questions_import_node_save_static_data($node));
}


/* This function imports questions from Question and Test Interoperability (QTI) format file.
 * @return
 * Return the number of questions successfully imported.
 */
function questions_import_submit_qti($form, $form_state) {
  $file = file_save_upload('upload');
  $row = 0;
  $qti_items = questions_import_qti_extract_info($file->filepath);

  // Loop through each question and import it into Drupal
  foreach ($qti_items as $item) {
    questions_import_qti_create_node($item, $form_state);
    ++$row;
  }
  return $row;
}

/**
 * Take a description of a quiz question and turn it into a node.
 *
 * The node is saved as the appropriate Quiz question type.
 */
function questions_import_qti_create_node($item, $form_state) {
  global $user;

  $item = (object)$item;

  $node = new stdClass();
  $node->title = $item->title;
  $node->teaser = $node->body = $item->content;
  $node->uid = $user->uid;
  $node->status = 1;
  $node->log = 'Imported from QTI importer.';

  $quiz_nid = $form_state['values']['quiz_node'];
  $quiz_vid = node_load($quiz_nid);
  $node->quiz_id = $quiz_nid;
  $node->quiz_vid = $quiz_vid->vid;

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
  //node_save($node);
  node_save(questions_import_node_save_static_data($node));
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

/* This function imports questions from Moodle Aiken format file.
 * @return
 * Return the number of questions successfully imported.
 */

function questions_import_submit_aiken($form, $form_state) {
  global $user;
  $row = 0;
  $output = '';
  $line = $options = array();
  $question_type = $form_state['values']['question_type'];

  $quiz_nid = $form_state['values']['quiz_node'];
  $quiz_vid = node_load($quiz_nid);
  $file = file_save_upload('upload');
  //creates drupal file object
  $lines = file($file->filepath);
  if ($question_type == 'multichoice') {
    while (!empty($lines)) { // while not empty of file
      while ($current_line = trim(array_shift($lines))) {
        if (empty($current_line)) {
          break;
        }
        $line[] = check_plain($current_line);
      }

      // read each question and its choices and correct answer.
      // now $line[0] has question, $line[1 to n] has choices and $line[last] has correct answer.
      $question = array_shift($line);
      $answer = array_pop($line);
      // now $line is left only with choices which looks like A) Moodle B) ATutor C) Claroline D) Blackboard etc
      foreach ($line as $l) {
        $option = explode($l[1], $l);
       $options[trim($option[0])] = trim($option[1]);
      }
      //now $options['A'] => Moodle, $options['B'] => ATutor ...
      $correct = substr(trim($answer), '-1');
      $answer = $options[$correct];

      $line = array();
      $node = new stdClass();
      $node->type = $form_state['values']['question_type']; // multichoice, true/false
      $node->quiz_id = $quiz_nid;
      $node->quiz_vid = $quiz_vid->vid;
      $node->title = $node->body = $node->teaser = trim($question);
      $node->num_answers = count($options);
      $node->answers = array();
      foreach ($options as $option) {
        $node->answers[] = array(
          'correct' => (trim($answer) == trim($option)) ? 1 : 0,
          'answer' => trim($option),
          'feedback' => '', //No feedback option as of now
        );
      }
      node_save(questions_import_node_save_static_data($node));
      ++$row;
    }
  }
  /* else if ($question_type == 'true/false') {
   yet to add
  } */
  return $row;
}

/* This function imports multichoice questions from CSV file.
 * @return
 * Return the number of questions successfully imported.
 */

function questions_import_submit_csv($form, $form_state, $lines) {
  global $user;
  $row = 0;
  $output = '';
  $allowed_question_type = quiz_get_questions_type('enabled');
  $separator = $form_state['values']['field_separator'];
  $quiz_nid = $form_state['values']['quiz_node'];  
  $quiz_vid = db_result(db_query("SELECT vid FROM {node} WHERE nid = %d", $quiz_nid)); //node_load($quiz_nid);  
  // iterate through all the lines in the file.
  foreach ($lines as $line) {
    $line = trim($line);
    if (empty($line) || $line[0] === '#') {
      continue;
    }
    $node = new stdClass();
    //create node object    
    $node->quiz_id = $quiz_nid;
    $node->quiz_vid = $quiz_vid;
    $line = explode($separator, $line);
    // The line variable looks something like
    // Question Type, Question, Choice 1, Feedback Ch1, Choice 2, Feedback Ch2, ...,Correct Choice
    // eg multichoice, which of the following is an IDE ?, Ubuntu, Ubuntu is a Linux Distro, incorrect, Geany, Geany is a text editor for gnome, incorrect, Eclipse, Eclipse is an IDE, correct, Drupal, Drupal is an kick ass CMS, incorrect
    //converst cvs file fields from string to array
    $type = array_shift($line);
    if (!in_array($type, $allowed_question_type)) {
      print_r($type);
      print_r($allowed_question_type);
      drupal_set_message(t('Unable to import @type type question. You may need to enable @type module.', array('@type' => $type)),'error');
      continue;
    }
    $node->type = $type;
    $question = array_shift($line);
    $correct_answer = array_pop($line);
    $options = $line;
    $node->title = $node->body = $node->teaser = $question;
    $node->num_answers = count($options);
    $node->answers = array();
    while (!empty($options)) {
      $answer = array_shift($options);
      $feedback = array_shift($options);
      $node->answers[] = array(
        'answer' => $answer,
        'feedback' => $feedback,
        'correct' => (trim($answer) == trim($correct_answer)) ? 1 : 0,
      );
    }
    node_save(questions_import_node_save_static_data($node));
    ++$row;
  }

  /* else if ($question_type == 'true/false') {
   yet to add
  } */
  return $row;
}

/*
 * This function sets the static data pertaining to $node object
 * @return
 * Return a node object
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
    $node->format = 1;
    $node->scored_quiz = 1;
    $node->revision = 1;
    $node->op = t('Save');
    $node->preview = t('Preview');
    return $node;
}

/*
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


/*
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
