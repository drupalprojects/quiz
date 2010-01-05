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
    '#options' => array('new_quiz' => t('New quiz'), 'existing_collection' => t('Existing quiz')),
    '#required' => TRUE,
  );
  $form['quiz_destination']['destination_title'] = array(
    '#type' => 'textfield',
    '#title' => t('Name of new node'),
    '#default_value' => t('<unnamed>'),
    '#description' => t('Name for the new quiz'),
    '#size' => 30,
    '#required' => TRUE,
  );
  $options = quiz_get_collection_options();
  // $options is an array with nid as index and quiz node's title as value
  $form['quiz_destination']['collection_node'] = array(
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
    '#description' => t('Select the import type'),
    '#required' => TRUE,
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
  $allowed_extensions = 'txt xml jcl jqz';

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
 * gets the destination node to which to import quiz questions.
 * @return
 * quiz node object to which questions has to be imported.
 */
function _get_destination_node($form_state) {
  global $user;

  $destination_type = $form_state['values']['destination_type'];

  if ($destination_type == 'existing_collection') {
    $quiz_nid = $form_state['values']['collection_node'];
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

  $import_id = _questions_import_generate_record($form_state['values']['validated_file']->fid,
                                                $form_state['values']['import_type']);

  $count = questions_import_submit_moodle_format($destination_node, $import_format, $form, $form_state, $import_id);
      
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
    switch ($form_state['values']['destination_type']) {
      case 'new_quiz':
         drupal_goto("node/$destination_node->nid/edit");
         break;
      default:
        drupal_goto("node/$destination_node->nid");
    }
  } else {
    drupal_set_message("No questions were imported.");
  }
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
    $node = new stdClass();
    $node->weight = $import_count;
    $node->collection_nid = $destination_node->nid;
    $node->collection_vid = $destination_node->vid;
    $node->format = $form_state['values']['input_format'];

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

    _questions_import_moodle_create_node($mq, $row, $import_id, $node);

    node_save(questions_import_node_save_static_data($node));
    _questions_import_record_item($import_id, $node->nid, $import_count);

    // give cc:attributionName to the source, if available
    if (!empty($mq->source) &&
    module_exists('creativecommons') &&
    creativecommons_node_type_is_enabled($node->type)) {
      $licenseUri = ''; // empty license for All Rights Reserved, for now
      $metadata['attributionName'] = $mq->source;
      $metadata['type'] = 'InteractiveResource';

      $cc = new creativecommons_license($licenseUri, NULL, $metadata);
      $cc->save($node->nid, 'update');
    }

    ++$import_count;
  }

  return $import_count;
}

function _questions_import_moodle_create_node ($mq, $row, $import_id, &$node) {
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

  $node->type = $qmap_moodle_to_drupal[$mq->qtype];
  if (empty($node->type)) {
    drupal_set_message(t('Skipped importation of unsupported question type "qtype"', array('qtype' => $mq-qtype)), 'warning', FALSE);
    watchdog('quiz', 'Skipped importation of Moodle question qtype', array('qtype' => $mq->qtype, 'mq' => print_r('mq', TRUE)));
    return;
  }
  $node->title = $mq->name;
  $node->teaser = $node->body = stripslashes_safe($mq->questiontext);  // a la Moodle
  $node->uid = $user->uid;
  $node->log = 'Imported with Moodle importer.';

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

function _questions_import_record_item ($import_id, $question_nid, $index) {
  db_query("INSERT INTO {quiz_questions_import_items} VALUES (%d, %d, %d)", $import_id, $question_nid, $index);
}

function _questions_import_generate_record ($file_id, $importer, $status = NULL) {
  // would be nice to make date a TIMESTAMP field so it's automatic, but that would bind it to MySQL
  db_query("INSERT INTO {quiz_questions_import_record} (file_id, datetime, status_array, importer) VALUES (%d, '%s', '%s', '%s')",
    $file_id,
    date('YmdHis'),
    serialize($status),
    $importer);
  return db_last_insert_id('quiz_questions_import_record', 'import_id');
}

