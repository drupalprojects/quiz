<?php

use Drupal\quiz\Controller\QuizQuestionManagementController;

/**
 * Administrator interface for Quiz module.
 *
 * @file
 */

/**
 * Form for managing what questions should be added to a quiz with categorized random questions.
 *
 * @param array $form_state
 *  The form state array
 * @param object $quiz
 *  The quiz node
 */
function quiz_categorized_form($form, $form_state, $quiz) {
  $form = array();
  _quiz_categorized_existing_terms_form($form, $form_state, $quiz);
  _quiz_categorized_new_term_form($form, $form_state, $quiz);
  $form['nid'] = array(
    '#type' => 'value',
    '#value' => $quiz->nid,
  );
  $form['vid'] = array(
    '#type' => 'value',
    '#value' => $quiz->vid,
  );
  $form['tid'] = array(
    '#type' => 'value',
    '#value' => NULL,
  );
  // Give the user the option to create a new revision of the quiz
  _quiz_add_revision_checkbox($form, $quiz);
  // Timestamp is needed to avoid multiple users editing the same quiz at the same time.
  $form['timestamp'] = array('#type' => 'hidden', '#default_value' => REQUEST_TIME);
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
  );
  $form['#tree'] = TRUE;
  return $form;
}

function _quiz_categorized_existing_terms_form(&$form, $form_state, $quiz) {
  $terms = _quiz_get_terms($quiz->vid);
  if ($terms) {
    if (empty($form_state['input']) && !quiz()->getQuizHelper()->buildCategoziedQuestionList($quiz)) {
      drupal_set_message(t('There are not enough questions in the requested categories.'), 'error');
    }
  }
  foreach ($terms as $term) {
    $form[$term->tid]['name'] = array(
      '#markup' => check_plain($term->name),
    );
    $form[$term->tid]['number'] = array(
      '#type' => 'textfield',
      '#size' => 3,
      '#default_value' => $term->number,
    );
    $form[$term->tid]['max_score'] = array(
      '#type' => 'textfield',
      '#size' => 3,
      '#default_value' => $term->max_score,
    );
    $form[$term->tid]['remove'] = array(
      '#type' => 'checkbox',
      '#default_value' => 0,
    );
    $form[$term->tid]['weight'] = array(
      '#type' => 'textfield',
      '#size' => 3,
      '#default_value' => $term->weight,
      '#attributes' => array(
        'class' => array('term-weight')
      ),
    );
  }
}

/**
 * Form for adding new terms to a quiz
 *
 * @see quiz_categorized_form
 */
function _quiz_categorized_new_term_form(&$form, $form_state, $quiz) {
  $form['new'] = array(
    '#type' => 'fieldset',
    '#title' => t('Add category'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
    '#tree' => FALSE,
  );
  $form['new']['term'] = array(
    '#type' => 'textfield',
    '#title' => t('Category'),
    '#description' => t('Type in the name of the term you would like to add questions from.'),
    '#autocomplete_path' => "node/$quiz->nid/questions/term_ahah",
    '#field_suffix' => '<a id="browse-for-term" href="javascript:void(0)">' . t('browse') . '</a>',
  );
  $form['new']['number'] = array(
    '#type' => 'textfield',
    '#title' => t('Number of questions'),
    '#description' => t('How many questions would you like to draw from this term?'),
  );
  $form['new']['max_score'] = array(
    '#type' => 'textfield',
    '#title' => t('Max score for each question'),
    '#description' => t('The number of points a user will be awarded for each question he gets correct.'),
    '#default_value' => 1,
  );
}

/**
 * Validate the categorized form
 */
function quiz_categorized_form_validate($form, &$form_state) {
  if (_quiz_is_int(arg(1))) {
    if (node_last_changed(arg(1)) > $form_state['values']['timestamp']) {
      form_set_error('changed', t('This content has been modified by another user, changes cannot be saved.'));
    }
  }
  else {
    form_set_error('changed', t('A critical error has occured. Please report error code 28 on the quiz project page.'));
    return;
  }
  if (!empty($form_state['values']['term'])) {
    $tid = _quiz_get_id_from_string($form_state['values']['term']);
    if ($tid === FALSE) {
      $terms = QuizQuestionManagementController::searchTerms($form_state['values']['term']);
      $num_terms = count($terms);
      if ($num_terms == 1) {
        $tid = key($terms);
      }
      elseif ($num_terms > 1) {
        form_set_error('term', t('You need to be more specific, or use the autocomplete feature. The term name you entered matches several terms: %terms', array('%terms' => implode(', ', $terms))));
      }
      elseif ($num_terms == 0) {
        form_set_error('term', t("The term name you entered doesn't match any registered question terms."));
      }
    }
    if (in_array($tid, array_keys($form))) {
      form_set_error('term', t('The category you are trying to add has already been added to this quiz.'));
    }
    else {
      form_set_value($form['tid'], $tid, $form_state);
    }

    if (!_quiz_is_int($form_state['values']['number'])) {
      form_set_error('number', t('The number of questions needs to be a positive integer'));
    }
    if (!_quiz_is_int($form_state['values']['max_score'], 0)) {
      form_set_error('max_score', t('The max score needs to be a positive integer or 0'));
    }
  }
}

/**
 * Submit the categorized form
 */
function quiz_categorized_form_submit($form, $form_state) {
  $quiz = node_load($form_state['values']['nid'], $form_state['values']['vid']);
  $quiz->number_of_random_questions = 0;
  // Update the refresh latest quizzes table so that we know what the users latest quizzes are
  if (variable_get('quiz_auto_revisioning', 1)) {
    $is_new_revision = quiz_has_been_answered($quiz);
  }
  else {
    $is_new_revision = (bool) $form_state['values']['new_revision'];
  }
  if (!empty($form_state['values']['tid'])) {
    $quiz->number_of_random_questions += _quiz_categorized_add_term($form, $form_state);
  }
  $quiz->number_of_random_questions += _quiz_categorized_update_terms($form, $form_state);
  if ($is_new_revision) {
    $quiz->revision = 1;
  }
  // We save the node to update its timestamp and let other modules react to the update.
  // We also do this in case a new revision is required...
  node_save($quiz);
}

/**
 * Update the categoriez belonging to a quiz with categorized random questions.
 *
 * Helper function for quiz_categorized_form_submit
 */
function _quiz_categorized_update_terms(&$form, &$form_state) {
  $ids = array('weight', 'max_score', 'number');
  $changed = array();
  $removed = array();
  $num_questions = 0;
  foreach ($form_state['values'] as $key => $existing) {
    if (!is_numeric($key)) {
      continue;
    }
    if (!$existing['remove']) {
      $num_questions += $existing['number'];
    }
    foreach ($ids as $id) {
      if ($existing[$id] != $form[$key][$id]['#default_value'] && !$existing['remove']) {
        $existing['nid'] = $form_state['values']['nid'];
        $existing['vid'] = $form_state['values']['vid'];
        $existing['tid'] = $key;
        if (empty($existing['weight'])) {
          $existing['weight'] = 1;
        }
        $changed[] = $form[$key]['name']['#markup'];
        drupal_write_record('quiz_terms', $existing, array('vid', 'tid'));
        break;
      }
      elseif ($existing['remove']) {
        db_delete('quiz_terms')
          ->condition('tid', $key)
          ->condition('vid', $form_state['values']['vid'])
          ->execute();
        $removed[] = $form[$key]['name']['#markup'];
        break;
      }
    }
  }
  if (!empty($changed)) {
    drupal_set_message(t('Updates were made for the following terms: %terms', array('%terms' => implode(', ', $changed))));
  }
  if (!empty($removed)) {
    drupal_set_message(t('The following terms were removed: %terms', array('%terms' => implode(', ', $removed))));
  }
  return $num_questions;
}

/**
 * Adds a term to a categorized quiz
 *
 * This is a helper function for the submit function.
 */
function _quiz_categorized_add_term($form, $form_state) {
  drupal_set_message(t('The term was added'));
  // Needs to be set to avoid error-message from db:
  $form_state['values']['weight'] = 0;
  drupal_write_record('quiz_terms', $form_state['values']);
  return $form_state['values']['number'];
}

/**
 * Searches for an id in the end of a string.
 *
 * Id should be written like "(id:23)"
 *
 * @param string $string
 *  The string where we will search for an id
 * @return int
 *  The matched integer
 */
function _quiz_get_id_from_string($string) {
  $matches = array();
  preg_match('/\(id:(\d+)\)$/', $string, $matches);
  return isset($matches[1]) ? (int) $matches[1] : FALSE;
}

/**
 * Handles "manage questions" tab.
 *
 * Displays form which allows questions to be assigned to the given quiz.
 *
 * This function is not used if the question assignment type "categorized random questions" is chosen
 *
 * @param $form_state
 *  The form state variable
 * @param $quiz
 *  The quiz node.
 * @return
 *  HTML output to create page.
 */
function quiz_questions_form($form, $form_state, $quiz) {

  $types = _quiz_get_question_types();

  _quiz_add_fields_for_creating_questions($form, $types, $quiz);

  // Display questions in this quiz.
  $form['question_list'] = array(
    '#type' => 'fieldset',
    '#title' => t('Questions in this quiz'),
    '#theme' => 'question_selection_table',
    '#collapsible' => TRUE,
    '#attributes' => array('id' => 'mq-fieldset'),
    'question_status' => array('#tree' => TRUE),
  );

  // Add randomization settings if this quiz allows randomized questions
  _quiz_add_fields_for_random_quiz($form, $quiz);

  $include_random = $quiz->randomization == 2;
  // @todo deal with $include_random
  $questions = quiz()->getQuizHelper()->getQuestions($quiz->nid, $quiz->vid);

  if (empty($questions)) {
    $form['question_list']['no_questions'] = array(
      '#markup' => '<div id = "no-questions">' . t('There are currently no questions in this quiz. Assign existing questions by using the question browser below. You can also use the links above to create new questions.') . '</div>',
    );
  }

  // We add the questions to the form array
  _quiz_add_questions_to_form($form, $questions, $quiz, $types);

  // Show the number of questions in the table header.
  $always_count = isset($form['question_list']['titles']) ? count($form['question_list']['titles']) : 0;
  $form['question_list']['#title'] .= ' (' . $always_count . ')';

  // Give the user the option to create a new revision of the quiz
  _quiz_add_revision_checkbox($form, $quiz);

  // Timestamp is needed to avoid multiple users editing the same quiz at the same time.
  $form['timestamp'] = array('#type' => 'hidden', '#default_value' => REQUEST_TIME);

  $form['actions']['#type'] = 'actions';

  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
    '#submit' => array('quiz_questions_form_submit'),
  );
  return $form;
}

/**
 * Fields for creating new questions are added to the quiz_questions_form
 *
 * @param $form
 *   FAPI form(array)
 * @param $types
 *   All the question types(array)
 * @param $quiz
 *   The quiz node
 */
function _quiz_add_fields_for_creating_questions(&$form, &$types, &$quiz) {
  // Display links to create other questions.
  $form['additional_questions'] = array(
    '#type' => 'fieldset',
    '#title' => t('Create new question'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  $url_query = drupal_get_destination();
  $url_query['quiz_nid'] = $quiz->nid;
  $url_query['quiz_vid'] = $quiz->vid;
  $create_question = FALSE;
  foreach ($types as $type => $info) {
    $url_type = str_replace('_', '-', $type);
    $options = array(
      'attributes' => array('title' => t('Create @name', array('@name' => $info['name']))),
      'query' => $url_query,
    );
    $access = node_access('create', $type);
    if ($access) {
      $create_question = TRUE;
    }
    $form['additional_questions'][$type] = array(
      '#markup' => '<div class="add-questions">' . l($info['name'], "node/add/$url_type", $options) . '</div>',
      '#access' => $access,
    );
  }
  if (!$create_question) {
    $form['additional_questions']['create'] = array(
      '#type' => 'markup',
      '#markup' => t('You have not enabled any question type module or no has permission been given to create any question.'),
      // @todo revisit UI text
    );
  }
}

/**
 * Add fields for random quiz to the quiz_questions_form
 *
 * @param $form
 *   FAPI form array
 * @param $quiz
 *   The quiz node(object)
 */
function _quiz_add_fields_for_random_quiz(&$form, $quiz) {
  if ($quiz->randomization != 2) {
    return;
  }
  $form['question_list']['random_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Settings for random questions'),
    '#collapsible' => TRUE,
  );
  $form['question_list']['random_settings']['num_random_questions'] = array(
    '#type' => 'textfield',
    '#size' => 3,
    '#maxlength' => 3,
    '#weight' => -5,
    '#title' => t('Number of random questions'),
    '#description' => t('The number of questions to be randomly selected each time someone takes this quiz'),
    '#default_value' => isset($quiz->number_of_random_questions) ? $quiz->number_of_random_questions : 10,
  );
  $form['question_list']['random_settings']['max_score_for_random'] = array(
    '#type' => 'textfield',
    '#size' => 3,
    '#maxlength' => 3,
    '#weight' => -5,
    '#title' => t('Max score for each random question'),
    '#default_value' => isset($quiz->max_score_for_random) ? $quiz->max_score_for_random : 1,
  );
  if ($quiz->randomization == 3) {
    $terms = _quiz_taxonomy_select($quiz->tid);
    if (!empty($terms) && function_exists('taxonomy_get_vocabularies')) {
      $form['question_list']['random_settings']['random_term_id'] = array(
        '#type' => 'select',
        '#title' => t('Terms'),
        '#size' => 1,
        '#options' => _quiz_taxonomy_select($quiz->tid),
        '#default_value' => $quiz->tid,
        '#description' => t('Randomly select from questions with this term, or choose from the question pool below'),
        '#weight' => -4,
      );
    }
  }
}

/**
 * Adds the questions in the $questions array to the form
 *
 * @param $form
 *   FAPI form(array)
 * @param $questions
 *   The questions to be added to the question list(array)
 * @param $quiz
 *   The quiz node(object)
 * @param $question_types
 *   array of all available question types
 */
function _quiz_add_questions_to_form(&$form, &$questions, &$quiz, &$question_types) {
  $form['question_list']['weights'] = array('#tree' => TRUE);
  $form['question_list']['qnr_ids'] = array('#tree' => TRUE);
  $form['question_list']['qnr_pids'] = array('#tree' => TRUE);
  $form['question_list']['max_scores'] = array('#tree' => TRUE);
  $form['question_list']['auto_update_max_scores'] = array('#tree' => TRUE);
  $form['question_list']['stayers'] = array('#tree' => TRUE);
  $form['question_list']['revision'] = array('#tree' => TRUE);
  if ($quiz->randomization == 2) {
    $form['question_list']['compulsories'] = array('#tree' => TRUE);
  }

  foreach ($questions as $question) {
    // @todo replace entire form with usage of question instance
    $question_node = node_load($question->nid, $question->vid);
    $instance = _quiz_question_get_instance($question_node);
    $fieldset = 'question_list';
    $id = $question->nid . '-' . $question->vid;

    $form[$fieldset]['weights'][$id] = array(
      '#type' => 'textfield',
      '#size' => 3,
      '#maxlength' => 4,
      '#default_value' => isset($question->weight) ? $question->weight : 0,
    );

    $form[$fieldset]['qnr_pids'][$id] = array(
      '#type' => 'textfield',
      '#size' => 3,
      '#maxlength' => 4,
      '#default_value' => $question->qnr_pid,
    );

    $form[$fieldset]['qnr_ids'][$id] = array(
      '#type' => 'textfield',
      '#size' => 3,
      '#maxlength' => 4,
      '#default_value' => $question->qnr_id,
    );

    // Quiz directions don't have scoring...
    $form[$fieldset]['max_scores'][$id] = array(
      '#type' => $instance->isGraded() ? 'textfield' : 'hidden',
      '#size' => 2,
      '#maxlength' => 2,
      '#disabled' => isset($question->auto_update_max_score) ? $question->auto_update_max_score : FALSE,
      '#default_value' => isset($question->max_score) ? $question->max_score : 0,
      '#states' => array(
        'disabled' => array(
          "#edit-auto-update-max-scores-$id" => array('checked' => TRUE),
        )
      ),
    );

    $form[$fieldset]['auto_update_max_scores'][$id] = array(
      '#type' => $instance->isGraded() ? 'checkbox' : 'hidden',
      '#default_value' => isset($question->auto_update_max_score) ? $question->auto_update_max_score : 0,
    );

    // Add checkboxes to remove questions in js disabled browsers...
    $form[$fieldset]['stayers'][$id] = array(
      '#type' => 'checkbox',
      '#default_value' => 0,
      '#attributes' => array('class' => array('q-staying')),
    );

    //Add checkboxes to mark compulsory questions for randomized quizzes.
    if ($quiz->randomization == 2) {
      $form[$fieldset]['compulsories'][$id] = array(
        '#type' => 'checkbox',
        '#default_value' => isset($question->question_status) ? ($question->question_status == QUESTION_ALWAYS) ? 1 : 0  : 0,
        '#attributes' => array('class' => array('q-compulsory')),
      );
    }

    if (user_access('view quiz question outside of a quiz')) {
      $link_options = array(
        'attributes' => array('class' => array('handle-changes')),
      );
      $question_titles = l($question->title, 'node/' . $question->nid, $link_options);
    }
    else {
      $question_titles = check_plain($question->title);
    }

    $form[$fieldset]['titles'][$id] = array('#markup' => $question_titles);


    $form[$fieldset]['types'][$id] = array(
      '#markup' => $question_types[$question->type]['name'],
      '#question_type' => $question->type,
    );

    $form[$fieldset]['view_links'][$id] = array(
      '#markup' => l(
        t('Edit'), 'node/' . $question->nid . '/edit', array(
        'query' => drupal_get_destination(),
        'attributes' => array('class' => array('handle-changes')),
        )
      ),
      '#access' => node_access('update', node_load($question->nid, $question->vid)),
    );
    // For js enabled browsers questions are removed by pressing a remove link
    $form[$fieldset]['remove_links'][$id] = array(
      '#markup' => '<a href="#" class="rem-link">' . t('Remove') . '</a>',
    );
    // Add a checkbox to update to the latest revision of the question
    if ($question->vid == $question->latest_vid) {
      $update_cell = array(
        '#markup' => t('<em>Up to date</em>'),
      );
    }
    else {
      $update_cell = array(
        '#type' => 'checkbox',
        '#title' => (l(t('Latest'), 'node/' . $question->nid . '/revisions/' . $question->latest_vid . '/view')
        . ' of ' .
        l(t('revisions'), 'node/' . $question->nid . '/revisions')
        ),
      );
    }
    $form[$fieldset]['revision'][$id] = $update_cell;
  }
}

/**
 * Adds checkbox for creating new revision. Checks it by default if answers exists.
 *
 * @param $form
 *   FAPI form(array)
 * @param $quiz
 *   Quiz node(object)
 */
function _quiz_add_revision_checkbox(&$form, &$quiz) {
  // Recomend and preselect to create the quiz as a new revision if it already has been answered
  if (quiz_has_been_answered($quiz)) {
    $rev_default = TRUE;
    $rev_description = t('This quiz has been answered. To maintain correctnes of existing answer reports changes should be saved as a new revision.');
  }
  else {
    $rev_default = in_array('revision', variable_get('node_options_quiz', array()));
    $rev_description = t('Allow question status changes to create a new revision of the quiz?');
  }
  if (user_access('manual quiz revisioning') && !variable_get('quiz_auto_revisioning', 1)) {
    $form['new_revision'] = array(
      '#type' => 'checkbox',
      '#default_value' => $rev_default,
      '#title' => t('New revision'),
      '#description' => $rev_description,
    );
  }
  else {
    $form['new_revision'] = array(
      '#type' => 'value',
      '#value' => $rev_default,
    );
  }
}

/**
 * Validate that the supplied questions are real.
 */
function quiz_questions_form_validate($form, $form_state) {
  if (_quiz_is_int(arg(1))) {
    if (node_last_changed(intval(arg(1))) > $form_state['values']['timestamp']) {
      form_set_error('changed', t('This content has been modified by another user, changes cannot be saved.'));
    }
  }
  else {
    form_set_error('changed', t('A critical error has occured. Please report error code 28 on the quiz project page.'));
    return;
  }

  $already_checked = array();
  $weight_map = $form_state['values']['weights'];

  // Make sure the number of random questions is a positive number
  if (isset($form_state['values']['num_random_questions']) && !_quiz_is_int($form_state['values']['num_random_questions'], 0)) {
    form_set_error('num_random_questions', 'The number of random questions needs to be a positive number');
  }

  // Make sure the max score for random questions is a positive number
  if (isset($form_state['values']['max_score_for_random']) && !_quiz_is_int($form_state['values']['max_score_for_random'], 0)) {
    form_set_error('max_score_for_random', 'The max score for random questions needs to be a positive number');
  }

  if (empty($weight_map)) {
    form_set_error('none', 'No questions were included.');
    return;
  }

  $question_types = array_keys(_quiz_get_question_types());

  foreach ($weight_map as $id => $weight) {

    list($nid, $vid) = explode('-', $id, 2);

    // If a node isn't one of the questionstypes we remove it from the question list
    $has_questions = (Boolean) db_select('node', 'n')
        ->fields('n', array('nid'))
        ->condition('type', $question_types, 'IN')
        ->addTag('node_access')
        ->condition('n.nid', $nid)
        ->execute()
        ->fetchField();
    if (!$has_questions) {
      form_set_error('none', 'One of the supplied questions was invalid. It has been removed from the quiz.');
      unset($form_state['values']['weights'][$id]);
    }

    // We also make sure that we don't have duplicate questions in the quiz.
    elseif (in_array($nid, $already_checked)) {
      form_set_error('none', 'A duplicate question has been removed. You can only ask a question once per quiz.');
      unset($form_state['values']['weights'][$id]);
    }
    else {
      $already_checked[] = $nid;
    }
  }

  // We make sure max score is a positive number
  $max_scores = $form_state['values']['max_scores'];
  foreach ($max_scores as $id => $max_score) {
    if (!_quiz_is_int($max_score, 0)) {
      form_set_error("max_scores][$id", t('Max score needs to be a positive number'));
    }
  }
}

/**
 * Update a quiz set of items with new weights and membership
 * @param $quiz
 *   The quiz node
 * @param $weight_map
 *   Weights for each question(determines the order in which the question will be taken by the quiz taker)
 * @param $max_scores
 *   Array of max scores for each question
 * @param $is_new_revision
 *   Array of boolean values determining if the question is to be updated to the newest revision
 * @param $refreshes
 *   True if we are creating a new revision of the quiz
 * @param $stayers
 *   Questions added to the quiz
 * @param $compulsories
 *   Array of boolean values determining if the question is compulsory or not.
 * @return array set of questions after updating
 */
function _quiz_update_items($quiz, $weight_map, $max_scores, $auto_update_max_scores, $is_new_revision, $refreshes, $stayers, $qnr_ids, $qnr_pids, $compulsories = NULL) {
  $questions = array();
  foreach ($weight_map as $id => $weight) {
    if ($stayers[$id]) {
      continue;
    }
    list($nid, $vid) = explode('-', $id, 2);
    $nid = (int) $nid;
    $vid = (int) $vid;
    $question = new stdClass();
    $question->nid = $nid;
    $question->vid = $vid;
    if (isset($compulsories)) {
      if ($compulsories[$id] == 1) {
        $question->state = QUESTION_ALWAYS;
      }
      else {
        $question->state = QUESTION_RANDOM;
        $max_scores[$id] = $quiz->max_score_for_random;
      }
    }
    else {
      $question->state = QUESTION_ALWAYS;
    }
    $question->weight = $weight;
    $question->max_score = $max_scores[$id];
    $question->auto_update_max_score = $auto_update_max_scores[$id];
    $question->qnr_pid = $qnr_pids[$id] > 0 ? $qnr_pids[$id] : NULL;
    $question->qnr_id = $qnr_ids[$id] > 0 ? $qnr_ids[$id] : NULL;
    $question->refresh = (isset($refreshes[$id]) && $refreshes[$id] == 1);

    // Add item as an object in the questions array.
    $questions[] = $question;
  }

  // Save questions.
  quiz()->getQuizHelper()->setQuestions($quiz, $questions, $is_new_revision);

  return $questions;
}

/**
 * Submit function for quiz_questions.
 *
 * Updates from the "manage questions" tab.
 */
function quiz_questions_form_submit($form, &$form_state) {
  // Load the quiz node
  $quiz = node_load(intval(arg(1)));
  // Update the refresh latest quizzes table so that we know what the users latest quizzes are
  if (variable_get('quiz_auto_revisioning', 1)) {
    $is_new_revision = quiz_has_been_answered($quiz);
  }
  else {
    $is_new_revision = (bool) $form_state['values']['new_revision'];
  }

  _quiz_question_browser_submit($form, $form_state);

  $weight_map = $form_state['values']['weights'];
  $qnr_pids_map = $form_state['values']['qnr_pids'];
  $qnr_ids_map = $form_state['values']['qnr_ids'];
  $max_scores = $form_state['values']['max_scores'];
  $auto_update_max_scores = $form_state['values']['auto_update_max_scores'];
  $refreshes = isset($form_state['values']['revision']) ? $form_state['values']['revision'] : NULL;
  $stayers = $form_state['values']['stayers'];
  $compulsories = isset($form_state['values']['compulsories']) ? $form_state['values']['compulsories'] : NULL;

  $num_random = isset($form_state['values']['num_random_questions']) ? $form_state['values']['num_random_questions'] : 0;


  $quiz->max_score_for_random = isset($form_state['values']['max_score_for_random']) ? $form_state['values']['max_score_for_random'] : 1;
  $term_id = isset($form_state['values']['random_term_id']) ? (int) $form_state['values']['random_term_id'] : 0;


  // Store what questions belong to the quiz
  $questions = _quiz_update_items($quiz, $weight_map, $max_scores, $auto_update_max_scores, $is_new_revision, $refreshes, $stayers, $qnr_ids_map, $qnr_pids_map, $compulsories, $stayers);

  // If using random questions and no term ID is specified, make sure we have enough.
  if (empty($term_id)) {
    $assigned_random = 0;

    foreach ($questions as $question) {
      if ($question->state == QUESTION_RANDOM) {
        ++$assigned_random;
      }
    }

    // Adjust number of random questions downward to match number of selected questions..
    if ($num_random > $assigned_random) {
      $num_random = $assigned_random;
      drupal_set_message(t('The number of random questions for this @quiz have been lowered to %anum to match the number of questions you assigned.', array('@quiz' => QUIZ_NAME, '%anum' => $assigned_random), array('langcode' => 'warning')));
    }
  }
  else {
    // Warn user if not enough questions available with this term_id.
    $available_random = count(_quiz_get_random_taxonomy_question_ids($term_id, $num_random));
    if ($num_random > $available_random) {
      $num_random = $available_random;
      drupal_set_message(t('There are currently not enough questions assigned to this term (@random). Please lower the number of random quetions or assign more questions to this taxonomy term before taking this @quiz.', array('@random' => $available_random, '@quiz' => QUIZ_NAME)), 'error');
    }
  }

  if ($quiz->type == 'quiz') {
    // Update the quiz node properties.
    $success = db_update('quiz_node_properties')
      ->fields(array(
        'number_of_random_questions' => $num_random ? $num_random : 0,
        'max_score_for_random' => $quiz->max_score_for_random,
        'tid' => $term_id,
      ))
      ->condition('vid', $quiz->vid)
      ->condition('nid', $quiz->nid)
      ->execute();

    // Get sum of max_score
    $query = db_select('quiz_node_relationship', 'qnr');
    $query->addExpression('SUM(max_score)', 'sum');
    $query->condition('parent_vid', $quiz->vid);
    $query->condition('question_status', QUESTION_ALWAYS);
    $score = $query->execute()->fetchAssoc();

    $success2 = db_update('quiz_node_properties')
      ->expression('max_score', 'max_score_for_random * number_of_random_questions + :sum', array(':sum' => (int) $score['sum']))
      ->condition('vid', $quiz->vid)
      ->execute();
  }

  if (isset($success) && isset($success2)) {
    drupal_set_message(t('Questions updated successfully.'));
  }
  else {
    drupal_set_message(t('There was an error updating the @quiz.', array('@quiz' => QUIZ_NAME)), 'error');
  }
}

/**
 * Takes care of the browser part of the submitted form values.
 *
 * This function changes the form_state to reflect questions added via the browser.
 * (Especially if js is disabled)
 *
 *
 * @param $form
 *   FAPI form(array)
 * @param $form_state
 *   FAPI form_state(array)
 */
function _quiz_question_browser_submit($form, &$form_state) {
  // Find the biggest weight:
  $next_weight = max($form_state['values']['weights']);

  // If a question is chosen in the browser, add it to the question list if it isn't already there
  if (isset($form_state['values']['browser']['table']['titles'])) {
    foreach ($form_state['values']['browser']['table']['titles'] as $id) {
      if ($id !== 0) {
        $matches = array();
        preg_match('/([0-9]+)-([0-9]+)/', $id, $matches);
        $nid = $matches[1];
        $vid = $matches[2];
        $form_state['values']['weights'][$id] = ++$next_weight;
        $form_state['values']['max_scores'][$id] = quiz_question_get_max_score($nid, $vid);
        $form_state['values']['stayers'][$id] = 1;
      }
    }
  }
}

// THEME FUNCTIONS

/**
 * Theme the quiz node form
 *
 * Adds js to enhance the ux.
 *
 * @param $form
 *   FAPI form array
 */
function theme_quiz_node_form($variables) {
  $form = $variables['form'];
  $path = drupal_get_path('module', 'quiz') . '/theme/quiz_node_form.js';
  drupal_add_js($path);
  return theme('node_form', array('form' => $form));
}

/**
 * Theme a question selection table, adding drag and drop support.
 */
function theme_question_selection_table($variables) {
  $form = $variables['form'];
  drupal_add_tabledrag('question-list', 'match', 'parent', 'qnr-pid', 'qnr-pid', 'qnr-id', TRUE, 1);
  drupal_add_tabledrag('question-list', 'order', 'sibling', 'question-list-weight');

  // Building headers
  $headers = array(t('Question'), t('Type'), t('Actions'), t('Update'), t('Max score'), t('Auto update max score'), t('Delete'));
  if (isset($form['compulsories'])) {
    $headers[] = t('Compulsory');
  }
  $headers[] = t('Weight');
  $headers[] = t('Parent ID');
  $headers[] = array(
    'data' => t('ID'),
    'class' => array('tabledrag-hide'),
  );

  // Building table body
  $rows = array();
  if (!empty($form['titles'])) {
    foreach (element_children($form['titles']) as $id) {
      $form['weights'][$id]['#attributes']['class'] = array('question-list-weight');
      $form['qnr_ids'][$id]['#attributes']['class'] = array('qnr-id');
      $form['qnr_pids'][$id]['#attributes']['class'] = array('qnr-pid');
      $rows[] = _quiz_get_question_row($form, $id);
    }
    // Make sure the same fields aren't rendered twice
    unset($form['types'], $form['view_links'], $form['remove_links'], $form['stayers']);
    unset($form['max_scores'], $form['auto_update_max_scores'], $form['revision'], $form['weights'], $form['titles'], $form['compulsories'], $form['qnr_ids'], $form['qnr_pids']);
  }
  $html_attr = array('id' => 'question-list');

  // We hide the table if no questions have been added so that jQuery can show it the moment the first question is beeing added.
  if (isset($form['no_questions'])) {
    $html_attr['style'] = "display:none;";
  }

  $table = theme('table', array('header' => $headers, 'rows' => $rows, 'attributes' => $html_attr));

  return drupal_render($form['random_settings'])
    . $table
    . drupal_render_children($form);
}

/**
 * Delete a single result, or all results for a given user and a given quiz.
 *
 * @param $result_id
 *   result if for the result to be deleted
 * @param $nid
 *   Node id for the quiz the result belongs to. If set all the users results for this quiz will be deleted.
 */
function _quiz_delete_results($result_id, $nid = NULL) {
  $result_ids = array();

  // We are to delete all results for a certain user on a certain quiz.
  if (isset($nid)) {
    // TODO: We should be able to rewrite this one to use the DBTNG.
    $res = db_query('SELECT result_id
            FROM {quiz_node_results}
            WHERE nid = :nid AND uid = (
              SELECT uid
              FROM {quiz_node_results}
              WHERE result_id = :result_id
            )', array(':nid' => $nid, ':result_id' => $result_id));

    while ($result = $res->fetchField()) {
      $result_ids[] = $result;
    }
  }
  // We are only to delete one specific result
  else {
    $result_ids[] = $result_id;
  }
  quiz()->getQuizHelper()->getResultHelper()->deleteByIds($result_ids);
}

// HELPER FUNCTIONS

/**
 * Helper function for theme_question_selection_table
 *
 * TODO: DELETE
 *
 * @see quiz_questions_form()
 * @see theme_question_selection_table()
 *
 * @param $sub_form
 *   Form definition array for a filtered questions list
 * @param $id
 *   Identifier used in $sub_form
 * @return table row
 *   Array defining a table row
 */
function _quiz_get_question_row($sub_form, $id) {
  $question_types = _quiz_get_question_types();
  $type = $sub_form['types'][$id]['#markup'];

  $action = theme('item_list', array(
    'items' => array(
      drupal_render($sub_form['view_links'][$id]),
      '<SPAN CLASS="q-remove" STYLE="display:none">' . drupal_render($sub_form['remove_links'][$id]) . '</SPAN>',
    ),
    'attributes' => array(
      'class' => array('links', 'inline'),
    ),
  ));
  $qnr_pid = $sub_form['qnr_pids'][$id]['#default_value'];
  $data_array = array(
    // The checkbox and the title
    ($qnr_pid ? theme('indentation', array('size' => 1)) : NULL) . drupal_render($sub_form['titles'][$id]),
    $type,
    $action,
    isset($sub_form['revision'][$id]) ? drupal_render($sub_form['revision'][$id]) : t("Up to date"),
    drupal_render($sub_form['max_scores'][$id]),
    drupal_render($sub_form['auto_update_max_scores'][$id]),
    drupal_render($sub_form['stayers'][$id]),
  );
  if (isset($sub_form['compulsories'])) {
    $data_array[] = drupal_render($sub_form['compulsories'][$id]);
  }
  $data_array[] = drupal_render($sub_form['weights'][$id]);
  $data_array[] = drupal_render($sub_form['qnr_pids'][$id]);
  $data_array[] = array(
    'class' => array('tabledrag-hide'),
    'data' => drupal_render($sub_form['qnr_ids'][$id]),
  );

  $leaf_class = $sub_form['types'][$id]['#question_type'] != 'quiz_page' ? 'tabledrag-leaf' : '';

  return array(
    'class' => array('q-row', 'draggable', $leaf_class),
    'id' => 'q-' . $id,
    'data' => $data_array
  );
}

/**
 * Helper function returning number of days as values and corresponding
 * number of milliseconds as array keys.
 *
 * @return
 *   array of options for when we want to delete partial quiz record values.
 */
function quiz_remove_partial_quiz_record_value() {
  return array(
    '0' => t('Never'),
    '86400' => t('1 Day'),
    '172800' => t('2 Days'),
    '259200' => t('3 Days'),
    '345600' => t('4 Days'),
    '432000' => t('5 Days'),
    '518400' => t('6 Days'),
    '604800' => t('7 Days'),
    '691200' => t('8 Days'),
    '777600' => t('9 Days'),
    '864000' => t('10 Days'),
    '950400' => t('11 Days'),
    '1036800' => t('12 Days'),
    '1123200' => t('13 Days'),
    '1209600' => t('14 Days'),
    '1296000' => t('15 Days'),
    '1382400' => t('16 Days'),
    '1468800' => t('17 Days'),
    '1555200' => t('18 Days'),
    '1641600' => t('19 Days'),
    '1728000' => t('20 Days'),
    '1814400' => t('21 Days'),
    '1900800' => t('22 Days'),
    '1987200' => t('23 Days'),
    '2073600' => t('24 Days'),
    '2160000' => t('25 Days'),
    '2246400' => t('26 Days'),
    '2332800' => t('27 Days'),
    '2419200' => t('28 Days'),
    '2505600' => t('29 Days'),
    '2592000' => t('30 Days'),
    '3024000' => t('35 Days'),
    '3456000' => t('40 Days'),
    '3888000' => t('45 Days'),
    '4320000' => t('50 Days'),
    '4752000' => t('55 Days'),
    '5184000' => t('60 Days'),
    '5616000' => t('65 Days'),
    '6048000' => t('70 Days'),
    '6480000' => t('75 Days'),
    '6912000' => t('80 Days'),
    '7344000' => t('85 Days'),
    '7776000' => t('90 Days'),
    '8208000' => t('95 Days'),
    '8640000' => t('100 Days'),
    '9072000' => t('105 Days'),
    '9504000' => t('110 Days'),
    '9936000' => t('115 Days'),
    '10368000' => t('120 Days'),
  );
}

/**
 * Adds inline js to automatically set the question's node title.
 */
function quiz_set_auto_title() {
  $max_length = variable_get('quiz_autotitle_length', 50);
  drupal_add_js('
  (function ($) {
  $(document).ready(function () {
    function quizStripTags(str) {
      return str.replace(/<\/?[^>]+>/gi, \'\');
    }

    function quizUpdateTitle() {
      var body = $("#edit-body textarea:eq(1)").val();
      if (quizStripTags(body).length > ' . $max_length . ') {
        $("#edit-title").val(quizStripTags(body).substring(0, ' . $max_length . ' - 3) + "...");
      }
      else {
        $("#edit-title").val(quizStripTags(body).substring(0, ' . $max_length . '));
      }
    }
    $("#edit-body textarea").keyup(quizUpdateTitle);
    // Do not use auto title if a title already has been set
    if($("#edit-title").val().length > 0){
      $("#edit-body textarea:eq(1)").unbind("keyup", quizUpdateTitle);
    }
    $("#edit-title").keyup(function() {
      $("#edit-body textarea:eq(1)").unbind("keyup", quizUpdateTitle);
    });
  });
  })(jQuery);
  ', array('type' => 'inline', 'scope' => JS_DEFAULT));
}

/**
 * Themes a categorized quiz form
 */
function theme_quiz_categorized_form($variables) {
  $form = $variables['form'];
  $output = '';
  $rows = array();
  foreach ($form as $key => &$existing) {
    if (!is_numeric($key)) {
      continue;
    }
    $cols = array();
    $cols[] = drupal_render($existing['name']);
    $cols[] = drupal_render($existing['number']);
    $cols[] = drupal_render($existing['max_score']);
    $cols[] = drupal_render($existing['remove']);
    $cols[] = drupal_render($existing['weight']);
    $rows[] = array(
      'data' => $cols,
      'class' => array('draggable'),
    );
  }
  if (!empty($rows)) {
    $header = array(
      t('Category'),
      t('Number of questions'),
      t('Max score per question'),
      t('Remove'),
      t('Weight'),
    );
    drupal_add_tabledrag('existing-terms', 'order', 'sibling', 'term-weight', NULL, NULL, TRUE);
    $output .= theme('table', array('header' => $header, 'rows' => $rows, 'attributes' => array('id' => 'existing-terms')));
  }
  $output .= drupal_render_children($form);
  drupal_add_js(
    "(function ($) {
     Drupal.behaviors.quiz_categorized = {
     attach: function(context) {
      $('#browse-for-term:not(.quiz-processed)').click(function(event) {
        event.preventDefault();
        $('#edit-term').focus().val('*').trigger('keyup');
      }).addClass('quiz-processed');
      $('#edit-term').click(function(){
        if ($(this).val() == '*') {
          $(this).val('');
        }
      });
    }};}(jQuery));"
    , array('type' => 'inline', 'group' => JS_DEFAULT));
  return $output;
}
