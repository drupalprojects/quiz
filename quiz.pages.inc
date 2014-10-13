<?php

/**
 * User pages.
 * @file
 */
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
 *  The number of questions for the quiz.
 * @return
 *  Themed html.
 *
 * @ingroup themeable
 */
function theme_quiz_progress($variables) {
  $output = '';
  $output .= '<div id="quiz-progress">';
  if ($variables['allow_jumping']) {
    // Show jump form.
    if ($variables['pager']) {
      $output .= theme('quiz_pager', array(
        'quiz'     => $variables['quiz'],
        'siblings' => variable_get('quiz_pager_siblings', 5),
        'current'  => $variables['current'],
        'total'    => count($variables['questions']),
      ));
    }
    else {
      $current_question = drupal_get_form('quiz_jumper', $variables['questions'], $variables['current']);
      $output .= drupal_render($current_question);
    }
  }
  else {
    // Just text.
    $output .= t('Question <span id="quiz-question-number">!x</span> of <span id="quiz-num-questions">@y</span>', array('!x' => $variables['current'], '@y' => count($variables['questions'])));
  }

  $output .= '</div>' . "\n";
  // Add div to be used by jQuery countdown
  if ($variables['time_limit']) {
    $output .= '<div class="countdown"></div>';
  }
  return $output;
}

/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function theme_quiz_jumper($variables) {
  drupal_add_js('
    (function ($) {
      Drupal.behaviors.quizJumper = {
        attach: function(context, settings) {
          $("#quiz-jumper:not(.quizJumper-processed)", context).show().addClass("quizJumper-processed").change(function(){
            $("#quiz-jumper #edit-submit").trigger("click");
          });
          $("#quiz-jumper-no-js:not(.quizJumper-processed)").hide().addClass("quizJumper-processed");
        }
      };
    })(jQuery);
  ', array('type' => 'inline', 'scope' => JS_DEFAULT));
  return '';
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
      'data'  => $node->number_of_questions,
    ),
  );
  if ($node->show_attempt_stats) {
    $takes = $node->takes == 0 ? t('Unlimited') : $node->takes;
    $stats[] = array(
      'title' => t('Attempts allowed'),
      'data'  => $takes,
    );
  }
  if ($node->quiz_always) {
    $stats[] = array(
      'title' => t('Available'),
      'data'  => t('Always'),
    );
  }
  else {
    $stats[] = array(
      'title' => t('Opens'),
      'data'  => format_date($node->quiz_open, 'short'),
    );
    $stats[] = array(
      'title' => t('Closes'),
      'data'  => format_date($node->quiz_close, 'short'),
    );
  }
  if (!empty($node->pass_rate)) {
    $stats[] = array(
      'title' => t('Pass rate'),
      'data'  => $node->pass_rate . ' %',
    );
  }
  if (!empty($node->time_limit)) {
    $stats[] = array(
      'title' => t('Time limit'),
      'data'  => quiz()->formatDuration($node->time_limit),
    );
  }
  $stats[] = array(
    'title' => t('Backwards navigation'),
    'data'  => $node->backwards_navigation ? t('Allowed') : t('Forbidden'),
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

  if (!$score['is_evaluated']) {
    $msg = t('Parts of this @quiz have not been evaluated yet. The score below is not final.', array('@quiz' => QUIZ_NAME));
    drupal_set_message($msg, 'warning');
  }

  // Display overall result.
  $output = '';
  $params = array(
    '%num_correct'    => $score['numeric_score'],
    '%question_count' => $score['possible_score'],
    '!username'       => ($user->uid == $account->uid) ? t('You') : theme('username', array('account' => $account)),
    '@score'          => $score['percentage_score'],
    '!yourtotal'      => ($user->uid == $account->uid) ? t('Your') : t('Total'),
  );

  // @todo should use quiz_feedback_can_review but we dont have the quiz result here.
  if (quiz_feedback_can_review('score', $quiz_result)) {
    // Show score.
    $output .= '<div id="quiz_score_possible">' . t('!username got %num_correct of %question_count possible points.', $params) . '</div>' . "\n";
    $output .= '<div id="quiz_score_percent">' . t('!yourtotal score: @score%', $params) . '</div>' . "\n";
  }

  if (quiz_feedback_can_review('quiz_feedback', $quiz_result)) {
    // Show quiz feedback.
    if (isset($summary['passfail'])) {
      $output .= '<div id="quiz_summary">' . $summary['passfail'] . '</div>' . "\n";
    }
    if (isset($summary['result'])) {
      $output .= '<div id="quiz_summary">' . $summary['result'] . '</div>' . "\n";
    }
  }

  $report_form = @drupal_get_form('Drupal\quiz\Form\QuizReportForm::staticCallback', $questions);
  $output .= drupal_render($report_form);
  return $output;
}

/**
 * Theme the quiz pager.
 */
function theme_quiz_pager($variables) {
  return quiz()
      ->getQuizHelper()
      ->getTakeJumperHelper($variables['quiz'], $variables['total'], $variables['siblings'], $variables['current'])
      ->render();
}
