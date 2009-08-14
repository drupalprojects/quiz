<?php


// re-use code from quiz.admin.inc
module_load_include('inc', 'quiz', 'quiz.admin');



/**
 * Creates a form for quiz questions.
 *
 * Handles the manage questions tab.
 *
 * @param $qid
 *   ID of quiz to create
 * @return
 *   String containing the form.
 */
function qcollection_items($node) {
  // Add JS
  $path = drupal_get_path('module', 'quiz') . '/quiz.admin.js';
  drupal_add_js($path, 'module');

  return drupal_get_form('qcollection_items_form', $node);
}


/**
 * Handles "manage questions" tab.
 *
 * Displays form which allows questions to be assigned to the given qcollection.
 *
 * @param $context
 *  The form context
 * @param $quiz
 *  The quiz node.
 * @return
 *  HTML output to create page.
 */
function qcollection_items_form($context, $quiz) {
  ////
  // ripped from quiz.admin.inc
  ///

  // This is a target for AHAH callbacks. Do not remove.
  $form['ahah_target'] = array(
    '#type' => 'markup',
    '#value' => '<div id="questions-always-target"></div>',
  );

  // Display links to create other questions.
  $form['additional_questions'] = array(
    '#type' => 'fieldset',
    '#title' => t('Add new question'),
    '#theme' => 'additional_questions',
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );

  $types = _quiz_get_question_types();
  foreach ($types as $type => $info) {
    $url_type = str_replace('_', '-', $type);
    $form['additional_questions'][$type] = array(
      '#type' => 'markup',
    // FIXME: This looks broken:
      '#value' => '<div class="add-questions">' . l(t($info['name']), 'node/add/'. $url_type .'/'. $quiz->nid, array('title' => t('Go to @name administration', array('@name' => $info['name'])))) .'</div>',
    );
  }

  // FIXME this should be something like "item_list" instead of 'filtered_question_list_always'
  // but qcollection is using quiz_admin_add_question_ahah() in quiz.admin.inc
  // which is hard-coded on this fieldset name
  $form['filtered_question_list_always'] = array(
    '#type' => 'fieldset',
    '#title' => t('Items in collection'),
    '#theme' => 'question_selection_table',
    'question_status' => array('#tree' => TRUE),
  );
  $form['filtered_question_list_always']['remove_from_quiz'] = array(
    '#type' => 'hidden',
    '#default_value' => '',
  );

  $form['add_existing_question'] = array(
    '#type' => 'fieldset',
    '#title' => '<strong>' . t('Find and add a question') . '</strong>',
    '#description' => t('Begin typing a question title or keyword. Suggestions will be presented based on your typing. ') .
       '<strong>' . t('You must choose one of the suggested questions.') . '</strong> ' .
  t('To add a new question, expand the "Add new question" section at top.'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
  );

  $form['add_existing_question']['always_autocomplete'] = array(
    '#type' => 'textfield',
    '#default_value' => '',
    '#size' => 60,
    '#maxlength' => 256,
    '#required' => FALSE,
    '#autocomplete_path' => 'admin/quiz/listquestions',
  );
  $form['add_existing_question']['add_to_list'] = array(
    '#type' => 'submit',
    '#value' => t('Add to quiz'),
    '#submit' => 'quiz_questions_form_submit',
    '#ahah' => array(
      'path' => 'admin/quiz/newquestion',
      'wrapper' => 'questions-always-target',
      'progress' => array('type' => 'bar', 'message' => t('Adding question...')),
  ),
  );

  // Build up a list of questions, sorted into those that are random and those that are
  // always on the quiz.
  $questions = _quiz_get_questions($quiz->nid, $quiz->vid);
  $rows = array();
  $form['filtered_question_list_random']['weights'] = array('#tree' => TRUE);
  $form['filtered_question_list_always']['weights'] = array('#tree' => TRUE);
  $form['filtered_question_list_random']['revision'] = array('#tree' => TRUE);
  $form['filtered_question_list_always']['revision'] = array('#tree' => TRUE);
  foreach ($questions as $question) {
    $id_mod = ($question->question_status == QUESTION_RANDOM ? 'random' : 'always');
    $fieldset = 'filtered_question_list_' . $id_mod;
    $id = $id_mod . '-' . $question->nid . '-' . $question->vid;

    $latest = node_load($question->nid);

    $form[$fieldset]['weights'][$id] = array(
    //'#type' => 'weight',
    //'#delta' => 60,
      '#type' => 'textfield',
      '#size' => 3,
      '#maxlength' => 4,
      '#default_value' => isset($question->weight) ? $question->weight : 0,
    );
    $form[$fieldset]['titles'][$id] = array(
      '#value' => $question->title,
    );
    $form[$fieldset]['types'][$id] = array(
      '#value' => $question->type,
    );
    $form[$fieldset]['view_links'][$id] = array(
      '#value' => l('View', 'node/' . $question->nid),
    );
    $form[$fieldset]['remove_links'][$id] = array(
    // FIXME: This does not degrade for non-JS browsers.
    //'#value' => l('Remove', 'node/' . $question->nid .'/questions/remove', array('attributes' => array('class' => 'rem-link'))),
      '#value' => '<a href="#" class="rem-link">' . t('Remove') . '</a>',
    );
    if ($question->vid == $latest->vid) {
      $update_cell = array(
        '#type' => 'markup',
        '#value' => '<em>Up to date</em>',
      );
    } else {
      $update_cell = array (
      '#type' => 'checkbox',
      '#title' => (l(t('Current'), 'node/' . $question->nid . '/revisions/' . $question->vid . '/view') . ' | ' .
                     l(t('Latest'), 'node/' . $question->nid . '/revisions/' . $latest->vid . '/view')),
      );
    }
    $form[$fieldset]['revision'][$id] = $update_cell;
  }

  // Show the number of 'always' questions in the 'always' table header.
  $always_count = isset($form['filtered_question_list_always']['titles']) ? count($form['filtered_question_list_always']['titles']) : 0;
  $form['filtered_question_list_always']['#title'] .= ' ('. $always_count .')';


  $form['new_revision'] = array(
    '#type' => 'checkbox',
    '#default_value' => in_array('revision', variable_get('node_options_quiz', array())),
    '#title' => t('New revision'),
    '#description' => t('Allow question status changes to create a new revision of the collection?'),
  );

  $form['timestamp'] = array('#type' => 'hidden', '#value' => time());

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit questions'),
  );

  $form['#submit'][] = 'qcollection_form_submit';

  return $form;
}


/**
 * Validate that the supplied questions are real.
 */
function qcollection_questions_form_validate($form, $form_state) {
  quiz_questions_form_validate($form, $form_state);
}


/**
 * Submit function for qcollection_questions.
 *
 * Updates from the "manage questions" tab.
 *
 * @param $form_id
 *  A string containing the form id.
 * @param $values
 *  Array containing the form values.
 */
function qcollection_questions_form_submit($form, &$form_state) {
  // This is ugly and should be fixed.
  $quiz = node_load(arg(1));

  $is_new_revision = (bool) $form_state['values']['new_revision'];

  $removed = explode(',', $form_state['values']['remove_from_quiz']);
  $weight_map = $form_state['values']['weights'];

  _quiz_update_items($quiz, $weight_map, $removed, $is_new_revision);
}


/**
 * Viewless action that copies the questions of the collection into
 * a new quiz node, then redirects the browser to editing the new quiz
 *
 * @param $collection_node
 */
function qcollection_export_quiz($collection_node) {
  global $user;

  $quiz_node = new stdClass();
  $quiz_node->type = 'quiz';
  $quiz_node->title = t("@quiz of '@qcollection_title' by @user at @unix_time", array('@quiz' => QUIZ_NAME, '@qcollection_title' => $collection_node->title, '@user' => $user->name, '@unix_time' => time()));
  $quiz_node->teaser = $node->body = $mq->questiontext;
  $quiz_node->uid = $user->uid;
  $quiz_node->status = 0; // unpublished by default
  $quiz_node->log = "Autogenerated from item collection nid:$collection_node->nid,vid:$collection_node->vid";
  $quiz_node->pass_rate = variable_get('quiz_default_pass_rate', 75);

  // save it, giving it nid and vid
  node_save($quiz_node);

  // TODO test whether this works in PostgreSQL and in MySQL 4.1 (the minimum for D6)
  // copy all the question relationships from the collection to the new quiz
  $sql = 'INSERT INTO {quiz_node_relationship} (parent_nid, parent_vid, child_nid, child_vid, question_status, weight)
SELECT %d as parent_nid, %d AS parent_vid, child_nid, child_vid, question_status, weight FROM {quiz_node_relationship} qnr
WHERE qnr.parent_nid = %d and qnr.parent_vid = %d';

  db_query($sql, $quiz_node->nid, $quiz_node->vid, $collection_node->nid, $collection_node->vid);

  // take them to edit the quiz that was just automatically made, e.g. node/1434/edit
  $new_nid = $quiz_node->nid;
  drupal_goto('node/'. $new_nid .'/edit');
}


/**
 * Callback for menu which returns a form for downloading.
 *
 * Handles the "Download" tab.
 *
 * @param $node
 *   Node of collection to download
 * @return
 *   String containing the form.
 */
function qcollection_download($node) {
  return drupal_get_form('qcollection_download_form', $node);
}


/**
 * Return a form for selecting the export settings
 */
function qcollection_download_form() {
  module_load_include('inc', 'questions_export', 'questions_export.admin');

  $form['exporter'] = array(
    '#type' => 'select',
    '#title' => t('Export format'),
    '#options' => _questions_exporters(),
    '#description' => t('Select the data format to export into.'),
    '#required' => TRUE,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Download'),
  );

  return $form;
}

/**
 * This generic submit handler calls specific export functions
 *
 */
function qcollection_download_form_submit(&$form, &$form_state) {
  $collection_node = $form['#parameters'][2]; // TODO must be a better way to get this
  $exporter = $form_state['values']['exporter'];
  _questions_export_download($collection_node, $exporter);
}

function qcollection_form_submit($form, &$form_state) {
  // This is ugly and should be fixed.
  $quiz = node_load(arg(1));

  $is_new_revision = (bool) $form_state['values']['new_revision'];

  $removed = explode(',', $form_state['values']['remove_from_quiz']);

  $weight_map = $form_state['values']['weights'];

  $old_vid = $quiz->vid;

  if ($set_new_revision) {
    // Create a new Quiz VID, even if nothing changed.
    $quiz->revision = 1;
    node_save($quiz);
  }

  foreach ($weight_map as $id => $weight) {
    list($state, $nid, $vid) = explode('-', $id, 3);
    $nid = (int) $nid;
    $vid = (int) $vid;

    $sql = 'DELETE FROM {quiz_node_relationship} WHERE parent_nid = %d AND parent_vid = %d AND child_nid = %d';
    $result = db_query($sql, $quiz->nid, $old_vid, $nid);

    if (in_array($id, $removed))
    {
      continue;
    }
    else {
      $question = new stdClass;

      $question->nid = $nid;
      $question->vid = $vid;
      $question->state = ($state == 'always' ? QUESTION_ALWAYS : QUESTION_RANDOM);
      $question->weight = $weight;

      if (isset($form_state['values']['revision'][$id]) && $form_state['values']['revision'][$id] == 1) {
        $sql = "INSERT INTO {quiz_node_relationship} (parent_nid, parent_vid, child_nid, child_vid, question_status, weight)
                VALUES (%d, %d, %d, (SELECT vid FROM {node} WHERE nid = %d), %d, %d)";
        if ($question->state != QUESTION_NEVER) {
          $result = db_query($sql, $quiz->nid, $quiz->vid, $question->nid, $question->nid, $question->state, $question->weight);
        }
      }
      else {
        $sql = "INSERT INTO {quiz_node_relationship} (parent_nid, parent_vid, child_nid, child_vid, question_status, weight)
                VALUES (%d, %d, %d, %d, %d, %d)";
        if ($question->state != QUESTION_NEVER) {
          $result = db_query($sql, $quiz->nid, $quiz->vid, $question->nid,$question->vid, $question->state, $question->weight);
        }
      }
    }
  }
}

