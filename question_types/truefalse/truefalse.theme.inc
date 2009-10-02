<?php

// $Id$

/**
 * @file
 * Themes for the truefalse module.
 */


/**
 * Implementation of theme_$type_report().
 * Theme the feedback report for a quiz. This report is generated (typically) at the end of the quiz.
 */
function theme_truefalse_report($question, $show_points, $show_feedback) {
  //drupal_set_message('called');

  if (!isset($question->answers[0])) {
    drupal_set_message(t('No result could be calculated.'), 'status');
    return;
  }

  $answer = $question->answers[0];
  $types = _quiz_get_question_types();
  $constructor = $types[$question->type]['response provider'];
  $result = new $constructor($answer['result_id'], $question);

  return $result->formatReport();
}
