<?php
// $Id$

/**
 * Administrator interface for Quiz module.
 *
 * @file
 */


// QUIZ ADMIN
 
/**
 * Quiz Admin.
 */
function quiz_admin() {
  $results = _quiz_get_results();
  return theme('quiz_admin', $results);
}
 
 // QUIZ RESULTS ADMIN
 
 /*
  * Quiz Results Admin.
  */
function quiz_admin_results($qid) {
  $result = db_fetch_object(db_query(
    'SELECT qnp.nid FROM {quiz_node_properties} qnp, {quiz_node_results} qnrs WHERE qnrs.nid = qnp.nid AND qnrs.result_id = %d', 
    $qid
  ));
  if ($result->nid) {
    $quiz = node_load($result->nid);
    $questions = _quiz_get_answers($qid);
    $score = quiz_calculate_score($quiz, $qid);
    $summary = _quiz_get_summary_text($quiz, $score);
    return theme('quiz_admin_summary', $quiz, $questions, $score, $summary);
  }
  else {
    drupal_not_found();
  }
}

/**
 * Creates a form for quiz questions.
 *
 * Handles the manage questions tab.
 *
 * @param $qid
 *   ID of quiz to create
 * @return unknown
 */
function quiz_questions($node) {
  return drupal_get_form('quiz_questions_form', $node);
}

// EDIT QUIZ

/**
 * Handles "manage questions" tab.
 *
 * Displays form which allows questions to be assigned to the given quiz.
 *
 * @return
 *  HTML output to create page.
 */
function quiz_questions_form($context, $quiz) {

  // Set page title.
  drupal_set_title(check_plain($quiz->title));

  // Display links to create other questions.
  $form['additional_questions'] = array(
    '#type' => 'fieldset',
    '#title' => t('Create additional questions'),
    '#theme' => 'additional_questions',
  );
  foreach (_quiz_get_question_types() as $type) {
    $form['additional_questions'][$type] = array(
      '#type' => 'markup',
      '#value' => l(t($type), 'node/add/'. $type .'/'. $quiz->nid, array('title' => t('Go to '. $type .' administration'))) .' ',
    );
  }

  // Display questions 'always' on this quiz.
  $form['filtered_question_list_always'] = array(
    '#type' => 'fieldset',
    '#title' => t("Questions 'always' on this quiz"),
    '#theme' => 'quiz_filtered_questions',
    '#collapsible' => TRUE,
    'question_status' => array('#tree' => TRUE),
  );

  $form['filtered_random_question_list']['num_random_questions'] = array(
    '#type' => 'textfield',
    '#title' => t('Number questions to randomize'),
    '#size' => 3,
    '#default_value' => $quiz->number_of_random_questions,
    '#description' => t('The number of randomly selected questions to assign to this quiz.'),
  );

  $form['filtered_random_question_list']['random_term_id'] = array(
    '#type' => 'select',
    '#title' => t('Terms'),
    '#size' => 1,
    '#options' => _quiz_taxonomy_select($quiz->tid),
    '#default_value' => $quiz->tid,
    '#description' => t('Randomly select from questions with this term, or choose from the random question pool below'),
  );

  // Display questions 'random' on this quiz.
  $form['filtered_question_list_random'] = array(
    '#type' => 'fieldset',
    '#title' => t("Questions 'random' on this quiz"),
    '#theme' => 'quiz_filtered_questions',
    '#collapsible' => TRUE,
    'question_status' => array('#tree' => TRUE),
  );

  // Display filtered question list.
  $form['filtered_question_list'] = array(
    '#type' => 'fieldset',
    '#title' => t("Questions 'never' on this quiz"),
    '#theme' => 'quiz_filtered_questions',
    '#collapsible' => TRUE,
    'question_status' => array('#tree' => TRUE),
  );

  // Get all questions and their status in relation to this quiz.
  $questions = array_merge(_quiz_get_unused_questions($quiz->vid), _quiz_get_questions($quiz->vid));
  foreach ($questions as $question) {
    switch ($question->status) {
      case QUESTION_RANDOM:
        $_form =& $form['filtered_question_list_random'];
        break;

      case QUESTION_ALWAYS:
        $_form =& $form['filtered_question_list_always'];
        break;

      case QUESTION_NEVER:
        $_form =& $form['filtered_question_list'];
        break;
    }

   $_form['question_status'][$question->nid] = array(
     '#type' => 'radios',
     '#options' => array(QUESTION_RANDOM => '', QUESTION_ALWAYS => '', QUESTION_NEVER => ''),
     '#default_value' => $question->status,
   );
   $_form['question'][$question->nid] = array(
     '#type' => 'markup',
     '#value' => $question->question,
   );
   $_form['type'][$question->nid] = array(
     '#type' => 'markup',
     '#value' => $question->type,
    );
  }
  // Show the number of 'always' questions in the 'always' table header.
  $form['filtered_question_list_always']['#title'] .= ' ('. count($form['filtered_question_list_always']['type']) .')';

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
  * Submit function for quiz_questions.
  * 
  * Updates from the "manage questions" tab.
  * 
  * @param $form_id
  *  A string containing the form id.
  * @param $values
  *  Array containing the form values.
  */
function quiz_questions_form_submit($form, &$form_state) {
  // This is ugly and should be fixed.
  $quiz = node_load(arg(1));
  
  $new_revision = $form_state['values']['new_revision'];

  // Update quiz with selected question options.
  if (!quiz_update_questions($quiz, $form_state['values']['question_status'], $new_revision)) {
    form_set_error('', t('Either no questions were selected, or there was a problem updating your @quiz. Please try again.', array('@quiz' => QUIZ_NAME)));
    return;
  }

  // Check if selecting random question from pool, and not via term.
  if (empty($form_state['values']['random_term_id'])) {
   $assigned_random = 0;
   if (is_array($form_state['values']['question_status'])) {
     foreach ($form_state['values']['question_status'] as $id => $status) {
       if (QUESTION_RANDOM == $status) {
         $assigned_random++;
       }
     }
   }

   if ($form_state['values']['num_random_questions'] > $assigned_random) {
     $form_state['values']['num_random_questions'] = $assigned_random;
     drupal_set_message(t('The number of random questions for this @quiz have been lowered to %anum to match the number of questions you assigned.', array('@quiz' => QUIZ_NAME, '%anum' => $assigned_random), 'warning'));
   }
  }
  else {
    // Warn user if not enough questions available with this term_id.
    //$available_random = count(_quiz_get_random_questions($form_state['values']['num_random_questions'], $form_state['values']['random_term_id']));
    $available_random = count(_quiz_get_random_taxonomy_question_ids($form_state['values']['random_term_id'],$form_state['values']['num_random_questions']));
    if ($form_state['values']['num_random_questions'] > $available_random) {
      $form_state['values']['num_random_questions'] = $available_random;
      drupal_set_message(t('There are currently not enough questions assigned to this term (@random). Please lower the number of random quetions or assign more questions to this taxonomy term before taking this @quiz.', array('@random' => $available_random, '@quiz' => QUIZ_NAME)), 'error');
    }
  }

  

  $result = db_query("UPDATE {quiz_node_properties} SET number_of_random_questions = %d, tid = %d WHERE vid = %d AND nid = %d", $form_state['values']['num_random_questions'], $form_state['values']['random_term_id'], $quiz->vid, $quiz->nid);
  if (!$result) {
    drupal_set_message(t('There was an error updating the @quiz.', array('@quiz' => QUIZ_NAME)), 'error');
  }
  // Otherwise just give general feedback.
  else {
    drupal_set_message(t('Questions updated successfully.'));
  }
}

// Quiz Admin Settings

/**
 * Implementation of hook_settings().
 * 
 * This builds the main settings form for the quiz module.
 */
function quiz_admin_settings() {
  $form = array();
  $form['quiz_default_close'] = array(
    '#type' => 'textfield',
    '#title' => t('Default number of days before a @quiz is closed', array('@quiz' => QUIZ_NAME)),
    '#default_value' => variable_get('quiz_default_close', 30),
    '#description' => t('Supply a number of days to calculate the default close date for new quizzes.'),
  );
   
  $form['quiz_default_pass_rate'] = array(
    '#type' => 'textfield',
    '#title' => t('Default percentage needed to pass a @quiz', array('@quiz' => QUIZ_NAME)),
    '#default_value' => variable_get('quiz_default_pass_rate', 75),
    '#description' => t('Supply a number between 1 and 100 to set as the default percentage correct needed to pass a quiz. Set to 0 if you want to ignore pass/fail summary information by default.'),
  );
   
  $form['quiz_use_passfail'] = array(
    '#type' => 'checkbox',
    '#title' => t('Allow quiz creators to set a pass/fail option when creating a @quiz.', array('@quiz' => strtolower(QUIZ_NAME))),
    '#default_value' => variable_get('quiz_use_passfail', 1),
    '#description' => t('Check this to display the pass/fail options in the @quiz form. If you want to prohibit other quiz creators from changing the default pass/fail percentage set below, uncheck this option.', array('@quiz' => QUIZ_NAME)),
  );

  $form['quiz_look_feel'] = array(
    '#type' => 'fieldset',
    '#title' => t('Look and Feel Settings'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#description' => t('Control aspects of the Quiz module\'s display'),
  );
     
  $form['quiz_look_feel']['quiz_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Display name'),
    '#default_value' => QUIZ_NAME,
    '#description' => t('Change the name of the quiz type. Do you call it <em>test</em> or <em>assessment</em> instead? Change the display name of the module to something else. Currently, it is called @quiz. By default, it is called <em>Quiz</em>.', 
      array('@quiz' => QUIZ_NAME)),
    '#required' => TRUE,
  );

  $form['quiz_look_feel']['quiz_show_allowed_times'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show allowed times'),
    '#description' => t('When a user begins a new quiz, show the user the number of times they may take the test, and how many times they have already taken the test.'),
    '#default_value' => variable_get('quiz_show_allowed_times', TRUE),

  );
  
  return system_settings_form($form);
}

/**
  * Validation of the Form Settings form.
  * 
  * Checks the values for the form administration form for quiz settings.
  */
function quiz_settings_form_validate($form, &$form_state) {
  if (!is_numeric($form_state['values']['num_random_questions']) || $form_state['values']['num_random_questions'] <0 ) {
    form_set_error('num_random_questions', t('The number of random questions must be at least 0.'));
  }
  if (!is_numeric($form_state['values']['quiz_default_close']) || $form_state['values']['quiz_default_close'] <= 0) {
    form_set_error('quiz_default_close', t('The default number of days before a quiz is closed must be a number greater than 0.'));
  }
  if (!is_numeric($form_state['values']['quiz_default_pass_rate'])) {
    form_set_error('quiz_default_pass_rate', t('The pass rate value must be a number between 0% and 100%.'));
  }
  if ($form_state['values']['quiz_default_pass_rate'] > 100) {
    form_set_error('quiz_default_pass_rate', t('The pass rate value must not be more than 100%.'));
  }
  if ($form_state['values']['quiz_default_pass_rate'] < 0) {
    form_set_error('quiz_default_pass_rate', t('The pass rate value must not be less than 0%.'));
  }
}
 
 // DELETE QUIZ RESULTS
 
 /**
  * Delete Result.
  */
function quiz_admin_result_delete() {
  return drupal_get_form('quiz_admin_result_delete_form');
}

/**
 * Creates a form used for deleting a set of quiz results.
 */
function quiz_admin_result_delete_form() {
  $form['del_rid'] = array('#type' => 'hidden', '#value' => arg(2));
  return confirm_form($form,
    t('Are you sure you want to delete this @quiz result?', array('@quiz' => QUIZ_NAME)),
    'admin/quiz/results',
    t('This action cannot be undone.'),
    t('Delete'),
    t('Cancel')
  );
}
 
function quiz_admin_result_delete_form_submit($form, &$form_state) {
  db_query("DELETE FROM {quiz_node_results} WHERE result_id = %d", $form_state['values']['del_rid']);
  db_query("DELETE FROM {quiz_node_results_answers} WHERE result_id = %d", $form_state['values']['del_rid']);
  drupal_set_message(t('Deleted result.'));
  $form_state['redirect'] = 'admin/quiz/results';
  $form_state['nid'] = $node->nid;  
}
 


// THEME FUNCTIONS
// Remember to updated quiz_theme() in quiz.module

/**
 * Theme the admin results table.
 * 
 * @param $results
 *  As returned by _quiz_get_results().
 * 
 * @ingroup themeable
 */
function theme_quiz_admin($results) {
  $output = '';
  $rows = array();

  while (list($key, $result) = each($results)) {
    $rows[] = array(
      l('view', 'admin/quiz/'. $result['result_id'] .'/view') .' | '. l('delete', 'admin/quiz/'. $result['result_id'] .'/delete'),
      check_plain($result['title']),
      check_plain($result['name']),
      $result['result_id'],
      format_date($result['time_start'], 'small'),
      ($result['time_end'] > 0) ? format_date($result['time_end'], 'small') : t('In Progress'),
    );
  }

  $header = array(
    t('Action'),
    t('@quiz Title', array('@quiz' => QUIZ_NAME)),
    t('Username'),
    t('Result<br />ID'),
    t('Time Started'),
    t('Finished?'));

  if (!empty($rows)) {
    $output .= theme('table', $header, $rows);
  }
  else {
    $output .= t('No @quiz results found.', array('@quiz' => QUIZ_NAME));
  }
  return $output;
} 

/**
 * Theme the summary page for admins.
 * 
 * @param $quiz
 *  The quiz node object.
 * @param $questions
 *  The questions array as defined by _quiz_get_answers.
 * @param $score
 *  Array of score information as returned by quiz_calculate_score().
 * @param $summary
 *  Filtered text of the summary.
 * @return
 *  Themed html.
 * 
 * @ingroup themeable
 */
function theme_quiz_admin_summary($quiz, $questions, $score, $summary) {
  // Set the title here so themers can adjust.
  drupal_set_title(check_plain($quiz->title));

  // Display overall result.
  $output = '';
  $output .= '<div id="quiz_score_possible">'. t('This person got %num_correct of %question_count correct.', array('%num_correct' => $score['num_correct'], '%question_count' => $score['question_count'])) .'</div>'."\n";
  $output .= '<div id="quiz_score_percent">'. t('Total score: @score%', array('@score' => $score['percentage_score'])) .'</div><br />'."\n";
  $output .= '<div id="quiz_summary">'. $summary .'</div><br />'."\n";
  // Get the feedback for all questions.
  $output .= theme('quiz_feedback', $questions, TRUE, TRUE);
  return $output;
}
 