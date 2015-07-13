<?php

/**
 * User pages.
 * @file
 */

/**
 * Show result page for a given result id
 *
 * @param $result_id
 *  Result id
 */
function quiz_user_results($result_id) {
  $quiz_result = quiz_result_load($result_id);

  if ($quiz_result->nid) {
    $quiz = node_load($quiz_result->nid, $quiz_result->vid);

    // Preserve "My results" tab.
    $item = menu_get_item("node/$quiz->nid/quiz-results");
    menu_set_item(NULL, $item);

    $questions = _quiz_get_answers($quiz, $result_id);
    $score = quiz_calculate_score($quiz, $result_id);
    $summary = _quiz_get_summary_text($quiz_result, $score);
    $data = array(
      'quiz' => $quiz,
      'questions' => $questions,
      'score' => $score,
      'summary' => $summary,
      'result_id' => $result_id,
      'account' => user_load($quiz_result->uid),
    );
    if (user_access('view own quiz results') && quiz_check_feedback($quiz_result)) {
      return theme('quiz_result', $data);
    }
    else {
      // User cannot view own results or there is no feedback enabled. Show
      // generic text.
      return t('You have finished this @quiz.', array('@quiz' => QUIZ_NAME));
    }
  }
}

/**
 * Check if any feedback should be displayed for this result.
 *
 * @param $quiz_result
 *   A quiz result.
 * @return bool
 *   TRUE if feedback needs to be displayed. FALSE otherwise.
 */
function quiz_check_feedback(QuizResult $quiz_result) {
  $quiz = node_load($quiz_result->nid, $quiz_result->vid);
  foreach (quiz_get_feedback_options() as $key => $option) {
    if (quiz_feedback_can_review($key, $quiz_result)) {
      return TRUE;
    }
  }
  return FALSE;
}

/**
 * Form for showing feedback, and for editing the feedback if necessary...
 *
 * @param $form_state
 *   FAPI form state(array)
 * @param $questions
 *   array of questions to inclide in the report
 * @return $form
 *   FAPI form array
 */
function quiz_report_form($form, $form_state, $questions, $result_id) {
  $form = array();
  // The submit button is only shown if one or more of the questions has input elements
  $show_submit = FALSE;
  foreach ($questions as $question) {
    $module = quiz_question_module_for_type($question->type);
    if (!$module) {
      return array();
    }
    $function = $module . '_report_form';
    $form_to_add = $function($question, $result_id);
    if (isset($form_to_add['submit'])) {
      $show_submit = TRUE;
      $form_to_add['#element_validate'][] = 'quiz_report_form_element_validate';
    }
    if (!isset($form_to_add['#no_report'])) {
      $form[] = $form_to_add;
    }
  }

  $form['#tree'] = TRUE;
  $form['navigation']['#type'] = 'actions';
  if ($show_submit) {
    $form['navigation']['submit'] = array(
      '#type' => 'submit',
      '#submit' => array('quiz_report_form_submit'),
      '#value' => t('Save score'),
    );
  }


  $quiz_result = quiz_result_load($result_id);

  if (count($questions) != count($quiz_result->layout)) {
    $form['#quiz_result'] = $quiz_result;
    if (empty($_SESSION['quiz'][$quiz_result->nid])) {
      // Quiz is done.
      $form['navigation']['finish'] = array(
        '#type' => 'submit',
        '#submit' => array('quiz_take_question_feedback_end_submit'),
        '#value' => t('Finish'),
      );
    }
    else {
      $form['navigation']['next'] = array(
        '#type' => 'submit',
        '#submit' => array('quiz_take_question_feedback_submit'),
        '#value' => t('Next question'),
      );
    }
  }

  return $form;
}

/**
 * Validate a single question sub-form.
 */
function quiz_report_form_element_validate(&$element, &$form_state) {
  $question = node_load($element['nid']['#value'], $element['vid']['#value']);
  $quiz_question_response = _quiz_question_response_get_instance($element['result_id']['#value'], $question);
  $quiz_question_response->getReportFormValidate($element, $form_state);
}

/**
 * Submit the report form
 */
function quiz_report_form_submit($form, &$form_state) {
  /* We go through the form state values and submit all
   * questiontypes with validation functions declared.
   */
  global $user;
  foreach ($form_state['values'] as $key => $q_values) {
    // Questions has numeric keys in the report form
    if (!is_numeric($key)) {
      continue;
    }
    // Questions store the name of the validation function with the key 'submit'
    if (!isset($q_values['submit'])) {
      continue;
    }
    // The submit function must exist
    if (!function_exists($q_values['submit'])) {
      continue;
    }

    // Load the quiz
    if (!isset($quiz)) {
      $result = db_query('SELECT nid, uid, vid FROM {quiz_node_results} WHERE result_id = :result_id', array(':result_id' => $q_values['result_id']))->fetchObject();
      $quiz = node_load($result->nid, $result->vid);
      $result_id = $q_values['result_id'];
    }

    $q_values['quiz'] = $quiz;

    // We call the submit function provided by the question
    call_user_func($q_values['submit'], $q_values);
  }
  // Scores may have been changed. We take the necessary actions
  quiz_update_total_score_fast($result_id, $quiz->vid);
  $changed = db_update('quiz_node_results')
    ->fields(array(
      'is_evaluated' => 1,
    ))
    ->condition('result_id', $result_id)
    ->execute();
  $results_got_deleted = _quiz_maintain_results($quiz, $result_id);

  // A message saying the quiz is unscored has already been set. We unset it here...
  if ($changed > 0) {
    _quiz_remove_unscored_message();
  }

  // Notify the user if results got deleted as a result of him scoring an answer.
  $add = $quiz->keep_results == QUIZ_KEEP_BEST && $results_got_deleted ? ' ' . t('Note that this @quiz is set to only keep each users best answer.', array('@quiz' => QUIZ_NAME)) : '';

  $score_data = quiz_get_score_array($result_id, $quiz->vid, TRUE);

  module_invoke_all('quiz_scored', $quiz, $score_data, $result_id);

  drupal_set_message(t('The scoring data you provided has been saved.') . $add);
  if (user_access('score taken quiz answer') && !user_access('view any quiz results')) {
    if ($result && $result->uid == $user->uid) {
      $form_state['redirect'] = 'node/' . $quiz->nid . '/quiz/results/' . $result_id;
    }
  }
}

/**
 * Helper function to remove the message saying the quiz haven't been scored
 */
function _quiz_remove_unscored_message() {
  if (!empty($_SESSION['messages']['warning'])) {
    // Search for the message, and remove it if we find it.
    foreach ($_SESSION['messages']['warning'] as $key => $val) {
      if ($val == t('This @quiz has not been scored yet.', array('@quiz' => QUIZ_NAME))) {
        unset($_SESSION['messages']['warning'][$key]);
      }
    }
    // Clean up if the message array was left empty
    if (empty($_SESSION['messages']['warning'])) {
      unset($_SESSION['messages']['warning']);
      if (empty($_SESSION['messages'])) {
        unset($_SESSION['messages']);
      }
    }
  }
}

/**
 * Updates the total score using only one mySql query.
 *
 * @param $result_id
 *  Result id
 * @param $quiz_vid
 *  Quiz node version id
 */
function quiz_update_total_score_fast($result_id, $quiz_vid) {
  $subq1 = db_select('quiz_node_results_answers', 'a');
  $subq1
    ->condition('a.result_id', $result_id)
    ->addExpression('SUM(a.points_awarded)');
  $res1 = $subq1->execute()->fetchField();

  $subq2 = db_select('quiz_node_properties', 'qnp');
  $subq2
    ->condition('qnp.vid', $quiz_vid)
    ->addField('qnp', 'max_score');
  $res2 = $subq2->execute()->fetchField();

  db_update('quiz_node_results')
    ->expression('score', 'ROUND(100*(:res1/:res2))', array(':res1' => $res1, ':res2' => $res2))
    ->condition('result_id', $result_id)
    ->execute();
}

/**
 * Returns an array of score information for a quiz
 *
 * @param unknown_type $result_id
 * @param unknown_type $quiz_vid
 * @param unknown_type $is_evaluated
 */
function quiz_get_score_array($result_id, $quiz_vid, $is_evaluated) {
  $properties = db_query('SELECT max_score, number_of_random_questions
          FROM {quiz_node_properties}
          WHERE vid = :vid', array(':vid' => $quiz_vid))->fetchObject();
  $total_score = db_query('SELECT SUM(points_awarded)
          FROM {quiz_node_results_answers}
          WHERE result_id = :result_id', array(':result_id' => $result_id))->fetchField();

  return array(
    'question_count' => $properties->number_of_random_questions + _quiz_get_num_always_questions($quiz_vid),
    'possible_score' => $properties->max_score,
    'numeric_score' => $total_score,
    'percentage_score' => ($properties->max_score == 0) ? 0 : round(($total_score * 100) / $properties->max_score),
    'is_evaluated' => $is_evaluated,
  );
}

// THEME FUNCTIONS

/**
 * Pass the correct mark to the theme so that theme authors can use an image.
 *
 * @ingroup themeable
 */
function theme_quiz_answer_result($variables) {
  $type = $variables['type'];
  return '<div class="quiz-score-icon ' . $type . '"></div>';
}

/**
 * Theme a progress indicator for use during a quiz.
 *
 * @param $question_number
 *  The position of the current question in the sessions' array.
 * @param $num_of_question
 *  The number of questions for this quiz as returned by quiz_get_number_of_questions().
 * @return
 *  Themed html.
 *
 * @ingroup themeable
 */
function theme_quiz_progress($variables) {
  drupal_add_js(drupal_get_path('module', 'quiz') . '/js/quiz.jumper.js');
  $output = '';
  $output .= '<div id="quiz-progress">';
  $text = 'Page <span id="quiz-question-number">!x</span> of <span id="quiz-num-questions">@y</span>';

  if ($variables['allow_jumping']) {
    // Show jump form.
    if ($variables['pager']) {
      $output .= theme('quiz_pager', array(
        'siblings' => variable_get('quiz_pager_siblings', 5),
        'current' => $variables['current'],
        'total' => count($variables['questions']),
      ));
    }
    else {
      $selectbox = drupal_get_form('quiz_jumper_form', $variables['questions'], $variables['current']);
      $output .= t($text, array('!x' => drupal_render($selectbox), '@y' => count($variables['questions'])));
    }
  }
  else {
    // Just text.
    $output .= t($text, array('!x' => $variables['current'], '@y' => count($variables['questions'])));
  }

  $output .= '</div>' . "\n";
  // Add div to be used by jQuery countdown
  if ($variables['time_limit']) {
    $output .= '<div class="countdown"></div>';
  }
  return $output;
}

/**
 * Theme the "no feedback" option.
 *
 * @return
 *  Themed html feedback.
 *
 * @ingroup themeable
 */
function theme_quiz_no_feedback() {
  return t('Thanks for taking the @quiz!', array('@quiz' => QUIZ_NAME));
}

/**
 * Theme the stats on the views page
 *
 * @param $node
 *   The quiz node
 */
function theme_quiz_view_stats($variables) {
  $node = $variables['node'];
  // Fetch data
  $stats = array(
    array(
      'title' => t('Questions'),
      'data' => $node->number_of_questions,
    ),
  );
  if ($node->show_attempt_stats) {
    $takes = $node->takes == 0 ? t('Unlimited') : $node->takes;
    $stats[] = array(
      'title' => t('Attempts allowed'),
      'data' => $takes,
    );
  }
  if ($node->quiz_always) {
    $stats[] = array(
      'title' => t('Available'),
      'data' => t('Always'),
    );
  }
  else {
    $stats[] = array(
      'title' => t('Opens'),
      'data' => format_date($node->quiz_open, 'short'),
    );
    $stats[] = array(
      'title' => t('Closes'),
      'data' => format_date($node->quiz_close, 'short'),
    );
  }
  if (!empty($node->pass_rate)) {
    $stats[] = array(
      'title' => t('Pass rate'),
      'data' => $node->pass_rate . ' %',
    );
  }
  if (!empty($node->time_limit)) {
    $stats[] = array(
      'title' => t('Time limit'),
      'data' => _quiz_format_duration($node->time_limit),
    );
  }
  $stats[] = array(
    'title' => t('Backwards navigation'),
    'data' => $node->backwards_navigation ? t('Allowed') : t('Forbidden'),
  );
  // Format and output the data
  $out = '<table id="quiz-view-table">' . "\n";
  foreach ($stats as $stat) {
    $out .= '<tr><td class="quiz-view-table-title"><strong>' . $stat['title'] . ':</strong></td><td class="quiz-view-table-data"><em>' . $stat['data'] . '</em></td></tr>' . "\n";
  }
  $out .= '</table>' . "\n";
  return $out;
}

/**
 * Theme the result page.
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
function theme_quiz_result($variables) {
  global $user;
  $quiz = $variables['quiz'];
  $questions = $variables['questions'];
  $score = $variables['score'];
  $summary = $variables['summary'];
  $quiz_result = quiz_result_load($variables['result_id']);
  $account = $variables['account'];

  if (!$score['is_evaluated'] && empty($_POST)) {
    $msg = t('Parts of this @quiz have not been evaluated yet. The score below is not final.', array('@quiz' => QUIZ_NAME));
    drupal_set_message($msg, 'warning');
  }

  // Display overall result.
  $output = '';
  $params = array(
    '%num_correct' => $score['numeric_score'],
    '%question_count' => $score['possible_score'],
    '!username' => ($user->uid == $account->uid) ? t('You') : theme('username', array('account' => $account)),
    '@score' => $score['percentage_score'],
    '!yourtotal' => ($user->uid == $account->uid) ? t('Your') : t('Total'),
  );

  $score_out = '';
  if (quiz_feedback_can_review('score', $quiz_result)) {
    // Show score.
    $score_out .= '<div id="quiz_score_possible">' . t('!username got %num_correct of %question_count possible points.', $params) . '</div>' . "\n";
    $score_out .= '<div id="quiz_score_percent">' . t('!yourtotal score: @score%', $params) . '</div>';
  }

  $report_form = drupal_get_form('quiz_report_form', $questions, $quiz_result->result_id);

  $quiz_feedback['#markup'] = '';
  if (quiz_feedback_can_review('quiz_feedback', $quiz_result)) {
    // Show quiz feedback.
    if (!empty($summary['passfail'])) {
      $quiz_feedback['#markup'] .= '<div id="quiz-summary">' . $summary['passfail'] . '</div>';
    }
    if (!empty($summary['result'])) {
      $quiz_feedback['#markup'] .= '<div id="quiz-summary">' . $summary['result'] . '</div>';
    }
  }
  $report_form['quiz_feedback'] = $quiz_feedback;

  $form = drupal_render($report_form);
  return $score_out . $form;
}

/**
 * Help us with special pagination.
 *
 * Why not the Drupal theme_pager()?
 *
 * It uses query strings. We have access on each menu argument (quiz question
 * number) so we unfortunately cannot use it.
 */
function _quiz_pagination_helper($total, $perpage = NULL, $current = NULL, $siblings = NULL) {
  $result = array();

  if (isset($total, $perpage) === TRUE) {
    $result = range(1, ceil($total / $perpage));

    if (isset($current, $siblings) === TRUE) {
      if (($siblings = floor($siblings / 2) * 2 + 1) >= 1) {
        $result = array_slice($result, max(0, min(count($result) - $siblings, intval($current) - ceil($siblings / 2))), $siblings);
      }
    }
  }

  return $result;
}

/**
 * Theme the quiz pager.
 */
function theme_quiz_pager($variables) {
  $total = $variables['total'];
  $current = $variables['current'];
  $siblings = $variables['siblings'];
  $items = array();
  $nid = arg(1);
  $items[] = array(
    'class' => array('pager-first'),
    'data' => l(t('first'), "node/$nid/take/1"),
  );
  foreach (_quiz_pagination_helper($total, 1, $current, $siblings) as $i) {
    if ($i == $current) {
      $items[] = array(
        'class' => array('pager-current'),
        'data' => $i,
      );
    }
    else {
      $items[] = array(
        'class' => array('pager-item'),
        'data' => l($i, "node/$nid/take/$i"),
      );
    }
  }
  $items[] = array(
    'class' => array('pager-last'),
    'data' => l(t('last'), "node/$nid/take/$total"),
  );
  return theme('item_list', array('items' => $items, 'attributes' => array('class' => array('pager'))));
}
