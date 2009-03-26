<?php
// $Id$

/**
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
  $types = _quiz_get_question_types();
  $constructor = $types[$question->type]['response provider'];
  $result = new $constructor($answer['result_id'], $question);
  
  return $result->formatReport();
  return 'NOT FINISHED'; //$questionObject->formatReport();
  
  
  if (is_array($question)) {
    // Cast this if it is an assoc array.
    $question = (object)$question;
  }
  
  $slug = '<div class="quiz_summary_question"><span class="quiz_question_bullet">Q:</span> '. 
    check_markup($question->body) .
    '</div>';
    
  $result = '<div class="quiz_answer_feedback">';
  if ($question && !empty($question->answers)) {
    $answer = (object) current($question->answers);
    
    if ($answer->is_evaluated == 1) {
      // Show score:
      if ($show_points) {
        $args = array('@yours' => $answer->score, '@total' => $question->maximum_score);
        $result .= t('Score: @yours of @total possible points', $args);
      }
      
      // Show feedback, if any.
      if ($show_feedback && !empty($answer->feedback)) {
        $result .= '</div><div class="quiz_answer_feedback">' . $answer->feedback;
      }
      
    }
    else {
      $result .= t('This answer has not yet been scored.') . 
        '<br/>' . 
        t('Until the answer is scored, the total score will not be correct.');
    }
    
    if (user_access('score long answer')) {
      $path = sprintf('admin/quiz/score-long-answer/%s/%s', $question->vid, $answer->result_id);
      $result .= '<p>' . l(t('Score this answer'), $path) . '</p>';
    }
  }
  else {
    $result .= t('This question was not answered.');
  }
  $result .= '</div>';
  
  return $slug . $result;
}
