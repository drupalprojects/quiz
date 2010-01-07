<?php
// $Id$

/**
 * @file
 * Theme functions for short_answer.
 */

/**
 * Implementation of theme_$type_feedback().
 * This is basically useless for long answer questions.
 */
function theme_short_answer_feedback($quiz, $report) {
  $output = '<div class="quiz_summary_text"><strong>' . t('Q:') . '</strong>'.
    check_markup($report->body, $report->format) .
    '<br /><em>';
  if ($report->is_evaluated) {
    $output .= 'Score: ' . $report->score;
  }
  else {
    t('The answer to this question will be scored by hand.');
  }

  $output .= '</em></div>';
  return $output;
}




