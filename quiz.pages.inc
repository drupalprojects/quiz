<?php
// $Id$

/**
 * User pages.
 * @file
 */

/*
 * Quiz Results User.
 */
function quiz_user_results($result_id) {
  global $user;
  
  $sql = 'SELECT qnp.nid, qnrs.uid 
    FROM {quiz_node_properties} qnp
    INNER JOIN {quiz_node_results} qnrs 
    WHERE qnrs.nid = qnp.nid AND qnrs.result_id = %d';
  $result = db_fetch_object(db_query( $sql, $result_id));
  if ($result->nid) {
    
    // User can view own results (quiz_menu sets access to 'own results').
    // User with role 'user results' can view other user's results.
    if ($result->uid != $user->uid && !user_access('user results')) {
      drupal_access_denied();
      return;
    }
    
    $quiz = node_load($result->nid);
    $questions = _quiz_get_answers($result_id);
    $score = quiz_calculate_score($quiz, $result_id);
    $summary = _quiz_get_summary_text($quiz, $score);
    return theme('quiz_user_summary', $quiz, $questions, $score, $summary);
  }
  else {
    drupal_not_found();
  }
}

/**
 * Displays all the quizzes the user has taken part in.
 *
 * @return
 *  HTML output for page.
 */
function quiz_get_user_results($user_id) {
  global $user;
  
  if ($user_id == $user->uid || user_access('user results')) {
    $results = array();
    $sql = "SELECT n.nid, n.title, u.name, qnrs.result_id, qnrs.time_start, qnrs.time_end
      FROM {node} n 
      INNER JOIN {quiz_node_properties} qnp
      INNER JOIN {quiz_node_results} qnrs
      INNER JOIN {users} u
      WHERE n.type = 'quiz'
        AND n.nid = qnp.nid
        AND qnrs.nid = qnp.nid
        AND u.uid = qnrs.uid
        AND u.uid = %d
      ORDER BY qnrs.result_id ASC";
    
    $dbresult = db_query($sql, $user_id);
    // Create an array out of the results.
    while ($line = db_fetch_array($dbresult)) {
     $results[$line['result_id']] = $line;
    }
    return theme('quiz_get_user_results', $results);
  }
  else {
    return 'You have no permission to view the results of this user.';
  }
}

// THEME FUNCTIONS
/**
 * Theme the user results page.
 * 
 * @param $results
 *  An array of quiz information.
 * @return
 *  Themed html.
 * 
 * @ingroup themeable
 */
function theme_quiz_get_user_results($results) {
  $output = '';
  $rows = array();
  while (list($key, $result) = each($results)) {
    $rows[] = array(
      l('view', 'user/quiz/'. $result['result_id'] .'/userresults'),
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
 * Theme the filtered question list.
 * 
 * @ingroup themeable
 */
function theme_quiz_filtered_questions($form) {
  $quiz_id = is_numeric(arg(1)) ? arg(1) : NULL;
  $header = array(t('Random'), t('Always'), t('Never'), t('Question'), t('Type'), t('Edit'));
  $rows = array();

  while (list($nid, $values) = each($form['question_status'])) {
    if (is_numeric($nid)) {
      $rows[] = array(
        drupal_render($form['question_status'][$nid][QUIZ_FEEDBACK_END]),
        drupal_render($form['question_status'][$nid][QUIZ_FEEDBACK_QUESTION]),
        drupal_render($form['question_status'][$nid][QUIZ_FEEDBACK_NEVER]),
        drupal_render($form['question'][$nid]),
        drupal_render($form['type'][$nid]),
        l(t('Edit'), 'node/'. $nid .'/edit/'. $quiz_id),
      );
    }
  }
  if (!empty($rows)) {
    $output .= theme('table', $header, $rows);
  }
  else {
    $output .= t('No questions found.');
  }
  return $output;
}

/**
 * Theme a table containing array of questions and options.
 *
 * @param $questions
 *  Array of question nodes.
 * @return
 *  HTML for a table.
 * 
 * @ingroup themeable
 */
function theme_quiz_question_table($questions, $quiz_id = NULL) {
  $output = '';
  $rows = array();
  $status_descriptions = array(t('Random'), t('Always'), t('Never'));
  while (list($key, $question) = each($questions)) {
    $rows[] = array(
      $status_descriptions[$question->status],
      $question->question,
      $question->type,
      l(t('Edit'), 'node/'. $question->nid .'/edit/'. $quiz_id));
  }
  $header = array(t('Status'), t('Question'), t('Type'), t('Edit'));

  if (!empty($rows)) {
    $output .= theme('table', $header, $rows);
  }
  else {
    $output .= t('No questions found.');
  }
  return $output;
}

/**
 * Pass the correct mark to the theme so that theme authors can use an image.
 * 
 * @ingroup themeable
 */
function theme_quiz_score_correct() {
  return theme('image', drupal_get_path('module', 'quiz') .'/images/correct.gif', t('correct'));
}

/**
 * Pass the incorrect mark to the theme so that theme authors can use an image.
 *
 * @ingroup themeable
 */
function theme_quiz_score_incorrect() {
  return theme('image', drupal_get_path('module', 'quiz') .'/images/incorrect.gif', t('incorrect'));
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
function theme_quiz_progress($question_number, $num_of_question) {
  // Determine the percentage finished (not used, but left here for other implementations).
  //$progress = ($question_number*100)/$num_of_question;

  // Get the current question # by adding one.
  $current_question = $question_number + 1;

  $output  = '';
  $output .= '<div id="quiz_progress">';
  $output .= t('Question %x of %y', array('%x' => $current_question, '%y' => $num_of_question));
  $output .= '</div><br />'."\n";
  return $output;
}

/**
 * Theme a question page.
 * 
 * @param $quiz
 *  The quiz node object.
 * @param $question_node
 *  The question node.
 * @return
 *  Themed html.
 * 
 * @ingroup themeable
 */
function theme_quiz_take_question($quiz, $question_node) {
  //Calculation for quiz progress bar.
  $number_of_questions = quiz_get_number_of_questions($quiz->vid, $quiz->nid);
  $question_number = $number_of_questions - count($_SESSION['quiz_'. $quiz->nid]['quiz_questions']);
  $question_node->question_number = $question_number;
  // Set the title here in case themers want to do something different.
  drupal_set_title(check_plain($quiz->title));

  // Return the elements of the page.
  $output = '';
  $output .= theme('quiz_progress', $question_number, $number_of_questions);
  $output .= module_invoke($question_node->type, 'render_question', $question_node);
  return $output;
}

/**
 * Theme the summary page after the quiz has been completed.
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
function theme_quiz_take_summary($quiz, $questions, $score, $summary) {
  // Set the title here so themers can adjust.
  drupal_set_title(check_plain($quiz->title));
  // Display overall result.
  $output = '';
  if ($score['percentage_score']) {
    $output .= '<div id="quiz_score_possible">'. t('You got %num_correct of %question_count correct.', array('%num_correct' => $score['num_correct'], '%question_count' => $score['question_count'])) .'</div>'."\n";
    $output .= '<div id="quiz_score_percent">'. t('Your score: %score%', array('%score' => $score['percentage_score'])) .'</div><br />'."\n";
  }
  $output .= '<div id="quiz_summary">'. $summary .'</div><br />'."\n";
  // Get the feedback for all questions.
  $output .= theme('quiz_feedback', $questions, ($quiz->pass_rate > 0), TRUE);
  
  return $output;
}

/**
 * Theme the summary page for user results.
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
function theme_quiz_user_summary($quiz, $questions, $score, $summary) {
  // Set the title here so themers can adjust.
  drupal_set_title(check_plain($quiz->title));

  // Display overall result.
  $output = '';
  $output .= '<div id="quiz_score_possible">'. t('You got %num_correct of %question_count correct.', array('%num_correct' => $score['num_correct'], '%question_count' => $score['question_count'])) .'</div>'."\n";
  $output .= '<div id="quiz_score_percent">'. t('Your score was: @score%', array('@score' => $score['percentage_score'])) .'</div><br />'."\n";
  $output .= '<div id="quiz_summary">'. $summary .'</div><br />'."\n";
  // Get the feedback for all questions.
  $output .= theme('quiz_feedback', $questions, FALSE, TRUE);
  return $output;
}

/**
 * Theme the question feedback.
 * 
 * @param $questions
 *  Array of quiz objects as returned by _quiz_get_answers.
 * @param showpoints
 *  Binary flag for whether to show the actual answers or not.
 * @param $showfeedback
 *  Binary flag for whether to show question feedback.
 * @return
 *  Themed html.
 * 
 * @ingroup themeable
 */
function theme_quiz_feedback($questions, $showpoints = TRUE, $showfeedback = FALSE) {
  $header = array(t('Question Result(s)'), '');

  // Go through each of the questions.
  foreach ($questions as $question) {
    $cols = array();
    // Ask each question to render a themed report of how the user did.
    $cols[] = array('data' => theme($question->type .'_report', $question, $showpoints, $showfeedback), 'class' => 'quiz_summary_qrow');

    // Get the score result for each question only if it's a scored quiz.
    if ($showpoints) {
      $theme = ($question->correct) ? 'quiz_score_correct' : 'quiz_score_incorrect';
      $cols[] = array('data' => theme($theme), 'class' => 'quiz_summary_qcell');
    }
    // Pack all of this into a table row.
    $rows[] = array('data' => $cols, 'class' => 'quiz_summary_qrow');
  }
  return theme('table', $header, $rows);
}

/**
 * Theme feedback for one question.
 *
 * @param $quiz
 *  Quiz node (may not be needed).
 * @param $question_node
 *  The question node giving feedback for.
 * @param $answer
 *  User's response to previous question.
 * @return
 *  Themed html.
 * 
 * @ingroup themeable
 */
function theme_quiz_question_feedback($quiz, $report) {
  $output = '<div class="quiz-summary-question">';
  $output .= theme($report->type .'_feedback', $quiz, $report);
  $output .= '</div><br class="clear" />';
  return $output;
}

/**
 * Allow the option to theme the questions form.
 * 
 * @ingroup themeable
 */
function theme_quiz_questions($form) {
  $output = '';
  $output .= drupal_render($form);
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
  return t('Thanks for taking the quiz!');
}
 