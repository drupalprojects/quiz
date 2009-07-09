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
function qcollection_questions($node) {
  // Set page title.
  drupal_set_title(check_plain($node->title));

  // Add JS
  $path = drupal_get_path('module', 'quiz') . '/quiz.admin.js';
  drupal_add_js($path, 'module');

  return drupal_get_form('qcollection_questions_form', $node);
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

function qcollection_questions_form($context, $quiz) {
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
  $questions = _quiz_get_questions($quiz->vid);
  $rows = array();
  $form['filtered_question_list_random']['weights'] = array('#tree' => TRUE);
  $form['filtered_question_list_always']['weights'] = array('#tree' => TRUE);
  foreach ($questions as $question) {
    $id_mod = ($question->question_status == QUESTION_RANDOM ? 'random' : 'always');
    $fieldset = 'filtered_question_list_' . $id_mod;
    $id = $id_mod . '-' . $question->nid;

    $form[$fieldset]['weights'][$id] = array(
    //'#type' => 'weight',
    //'#delta' => 60,
      '#type' => 'textfield',
      '#size' => 3,
      '#maxlength' => 4,
      '#default_value' => isset($question->weight) ? $question->weight : 0,
    );
    $form[$fieldset]['titles'][$id] = array(
    // '#value' => $question->question, // Question is too long for drag and drop.
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
  }

  // Show the number of 'always' questions in the 'always' table header.
  $always_count = isset($form['filtered_question_list_always']['titles']) ? count($form['filtered_question_list_always']['titles']) : 0;
  $form['filtered_question_list_always']['#title'] .= ' ('. $always_count .')';


  $form['new_revision'] = array(
    '#type' => 'checkbox',
    '#default_value' => in_array('revision', variable_get('node_options_quiz', array())),
    '#title' => t('New revision'),
    '#description' => t('Allow question status changes to create a new revision of the quiz?'),
  );

  $form['timestamp'] = array('#type' => 'hidden', '#value' => time());

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit questions'),
  );

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
