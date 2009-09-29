<?php
// $Id$

/*
 * @file
 * Themes for the quiz question module.
 */

/**
 * Implementation of theme_$type_report().
 * Theme the feedback report for a quiz. This report is generated (typically) at the end of the quiz.
 */
function theme_quiz_question_report($question, $show_points, $show_feedback) {
  if (!isset($question->answers[0])) {
    drupal_set_message('No result could be calculated.', 'status');
    return;
  }

  $answer = $question->answers[0];
  $result = _quiz_question_response_get_instance($answer['result_id'], $question, $answer); 
  return $result->formatReport();
}

function theme_quiz_question_creation_form($form) {
  module_load_include('inc', 'quiz', 'quiz.admin');
  quiz_set_auto_title();
}
