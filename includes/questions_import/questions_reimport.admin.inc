<?php
// $Id$

/*
 * @file
 * Administration file for Questions Export module
 *
 */

module_load_include('inc', 'questions_import', 'questions_import.admin');


/**
 * Implementation of hook_form
 * form to upload questions
 */
function questions_reimport_form()  {
  $form['#attributes'] = array('enctype' => 'multipart/form-data');

  $form['import_id'] = array(
    '#type' => 'select',
    '#title' => t('Import'),
    '#options' => _questions_reimport_file_options(),
    '#description' => t('Select quiz import to run again'),
    '#required' => TRUE,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Reimport'),
  );

  $form['#submit'][] = 'questions_reimport_form_submit';

  return $form;
}

function _questions_reimport_file_options() {
  $imports = array();
  // Load up all import records, with filename
  $sql = "SELECT qqir.*, filename FROM {quiz_questions_import_record} qqir JOIN {files} f ON qqir.file_id = f.fid";
  $result = db_query($sql);
  while ($row = db_fetch_array($result)) {
    $imports[$row['import_id']] = $row['filename'] . ' (uploaded ' . $row['datetime'] . ')';
  }
  return $imports;
}

function questions_reimport_form_submit(&$form, &$form_state) {

  $time = 0;
  $op = '';

  $import_id = $form_state['values']['import_id'];

  $result = db_query("SELECT * FROM {quiz_questions_import_record} WHERE import_id = %d", $import_id);
  if (!($row = db_fetch_array($result))) {
    echo "error";
    return;
  }
  $fid = $row['file_id'];
  $import_type_info = explode('_', $row['importer']);
  $import_engine = $import_type_info[0];
  $import_format = $import_type_info[1];

  $start = microtime(TRUE);

  $status = array();

  // would be nice to make this a TIMESTAMP field so it's automatic, but that would bind it to MySQL
  $date = date('YmdHis');

  db_query("UPDATE {quiz_questions_import_record} SET datetime = '%s' WHERE import_id = %d", $date, $import_id);

  switch ($import_engine) {
    case 'native':
      $import_function_name = "questions_reimport_submit_$import_format";
      $count = $import_function_name($import_id);
      break;
    case 'moodle':
      $count = questions_reimport_submit_moodle_format($import_format, $import_id);
      break;
  }

  // $count contains number of questions successfully imported.
  $end = microtime(TRUE);
  $time =  substr($end - $start, 0, 6);
  if ($count) {
    drupal_set_message(t('@count questions were reimported successfully in @time seconds.', array('@count' => $count, '@time' => $time)));
    /*
     * when the questions destination is "new_quiz" questions importer will
     * create a quiz node by itself, whose properties were not defined properly.
     * so redirecting the quiz author to quiz node/$nid/edit page
     * where he/she can set the quiz node properties.
     */
  }
  return $count;
}

function questions_reimport_submit_moodle_format($format, $import_id) {
  $result = db_query("SELECT qir.importer as importer, f.filename AS filename, f.filepath AS filepath FROM {quiz_questions_import_record} AS qir RIGHT JOIN {files} AS f ON f.fid = qir.file_id WHERE qir.import_id = %d", $import_id);
  $row = db_fetch_array($result);
  $filename = $row['filepath'];

  module_load_include('php', 'quiz', "includes/moodle/question/format/$format/format");

  // e.g. qformat_webct
  $classname = "qformat_$format";
  $fHandler = new $classname();

  assert($fHandler->provide_import());

  $lines = file($filename);
  $moodle_questions = $fHandler->readquestions($lines);

  $rows = 0;

  $result = db_query("SELECT * FROM {quiz_questions_import_items} AS qii WHERE import_id = %d", $import_id);
  while ($row = db_fetch_array($result)) {
    $items[$row['index']] = $row['question_nid'];
  }

  // iterate over Moodle questions to make Drupal Quiz questions
  foreach ($moodle_questions as $mq) {
    if (isset($items[$rows])) {
      $node = node_load($items[$rows]);
    }
    //Otherwise create a new node
    else {
      $node = new stdClass();
    }
    // TODO $node->format should come from the form, same as normal import form
//    $node->format = $import_type; // node->format is the input format id (e.g. Filtered HTML=1)

    _questions_import_moodle_create_node($mq, $rows, $import_id, $node);

    if ($item_id) {
      $node->revision = TRUE;
      $node->log = "Revised by reimportation (import_id=$import_id)";
      node_save($node);
    }
    else {
      $node->log = "Created by reimportation (import_id=$import_id)";
      node_save(questions_import_node_save_static_data($node));
      _questions_import_record_item($import_id, $node->nid, $rows);
    }

    ++$rows;
  }

  return $rows;
}


/**
 * This function imports questions from Question and Test Interoperability (QTI) format file.
 * @return
 * Return the number of questions successfully imported.
 */
function questions_reimport_submit_qti12($import_id) {
  $rows = 0;

  $result = db_query("SELECT qir.importer as importer, f.filename AS filename, f.filepath AS filepath FROM {quiz_questions_import_record} AS qir RIGHT JOIN {files} AS f ON f.fid = qir.file_id WHERE qir.import_id = %d", $import_id);
  $row = db_fetch_array($result);
  $filename = $row['filepath'];
  $import_type = $row['importer'];

  $result = db_query("SELECT * FROM {quiz_questions_import_items} AS qii WHERE import_id = %d", $import_id);
  while ($row = db_fetch_array($result)) {
    $items[$row['index']] = $row['question_nid'];
  }

  $qti_items = questions_import_qti_extract_info($filename);

  // Loop through each question and import it into Drupal
  foreach ($qti_items as $item) {
    if (isset($items[$rows])) {
      $node = node_load($items[$rows]);
    }
    //Otherwise create a new node
    else {
      $node = new stdClass();
    }
    // TODO $node->format should come from the form, same as normal import form
//    $node->format = $import_type; // node->format is the input format id (e.g. Filtered HTML=1)

    questions_import_qti_create_node($item, $rows, $import_id, $node);

    if ($item_id) {
      $node->revision = TRUE;
      node_save($node);
    }
    else {
      node_save(questions_import_node_save_static_data($node));
      _questions_import_record_item($import_id, $node->nid, $rows);
    }

    ++$rows;
  }
  return $rows;
}

/**
 * @function
 * This function imports questions from Moodle Aiken format file.
 * @return
 * Return the number of questions successfully imported.
 */
function questions_reimport_submit_aiken($import_id) {
  global $user;
  $output = '';
  $line = array();

  $result = db_query("SELECT qir.importer as importer, f.filename AS filename, f.filepath AS filepath FROM {quiz_questions_import_record} AS qir RIGHT JOIN {files} AS f ON f.fid = qir.file_id WHERE qir.import_id = %d", $import_id);
  $row = db_fetch_array($result);
  $filename = $row['filepath'];
  $import_type = $row['importer'];

  $result = db_query("SELECT * FROM {quiz_questions_import_items} AS qii WHERE import_id = %d", $import_id);
  while ($row = db_fetch_array($result)) {
    $items[$row['index']] = $row['question_nid'];
  }

  $lines = file($filename);

  // while not empty of file content
  while (!empty($lines)) {
    $line = questions_import_get_next_aiken_question($lines);

    if (empty($line)) {
      // this line is empty. Get the next line
      continue;
    }

    if (isset($items[$rows])) {
      $node = node_load($items[$rows]);
    }
    //Otherwise create a new node
    else {
      $node = new stdClass();
    }
    // TODO $node->format should come from the form, same as normal import form
//    $node->format = $import_type; // node->format is the input format id (e.g. Filtered HTML=1)

    questions_import_read_node_aiken($line, $node);

    if (isset($items[$rows])) {
      $node->revision = TRUE;
      node_save($node);
    }
    else {
      node_save(questions_import_node_save_static_data($node));
      _questions_import_record_item($import_id, $node->nid, $rows);
    }

    ++$rows;
  }

  return $rows;
}


/**
 * @function
 * This function imports multichoice questions from CSV file.
 * @param $lines
 *  It contains the whole csv file content.
 * @return
 *  Return the number of questions successfully imported.
 */
function questions_reimport_submit_csv($import_id) {
  global $user;
  $rows = 0;
  $output = '';

  $result = db_query("SELECT qir.importer as importer, f.filename AS filename, f.filepath AS filepath FROM {quiz_questions_import_record} AS qir RIGHT JOIN {files} AS f ON f.fid = qir.file_id WHERE qir.import_id = %d", $import_id);
  $row = db_fetch_array($result);
  $filename = $row['filepath'];
  $import_type = $row['importer'];
  $separator = ",";

  $result = db_query("SELECT * FROM {quiz_questions_import_items} AS qii WHERE import_id = %d", $import_id);
  while ($row = db_fetch_array($result)) {
    $items[$row['index']] = $row['question_nid'];
  }

  // iterate through all the lines in the file. Each line corresponds to one quiz question
  $lines = file($filename);
  foreach ($lines as $line) {
    //If the item was succesfully imported before, load it for a revision
    if (isset($items[$rows])) {
      $node = node_load($items[$rows]);
    }
    //Otherwise create a new node
    else {
      $node = new stdClass();
    }
    // TODO $node->format should come from the form, same as normal import form
//    $node->format = $import_type; // node->format is the input format id (e.g. Filtered HTML=1)

    questions_import_read_node_csv($line, $node, $separator);

    if (isset($items[$rows])) {
      $node->revision = TRUE;
      node_save($node);
    }
    else {
      node_save(questions_import_node_save_static_data($node));
      _questions_import_record_item($import_id, $node->nid, $rows);
    }

    ++$rows;
  }

  return $rows;
}