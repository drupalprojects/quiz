<?php
// $Id$

/**
 * @file
 * Theme functions for short_answer.
 */

/**
 * Theme the list of unscored short answer questions.
 *
 * @param $unscored
 *  An array of objects, each with the quist_nid, question_vid, and result_id of an unscored question.
 */
function theme_short_answer_view_unscored($unscored) {
  $output = '';
  $header = array(
    t('Quiz ID'),
    t('Question'),
    t('Time Finished'),
    t('Action')
  );
  $rows = array();

  foreach ($unscored as $item) {
    if ($item->time_end > 0) {
      $rows[] = array(
        $item->result_id,
        $item->title,
        date('Y-m-d H:i', $item->time_end),
        l(t('score this response'), 'admin/quiz/reports/score-short-answer/' . $item->question_vid . '/' . $item->result_id),
      );
    }

  }

  if (!empty($rows)) {
    $output .= theme('table', $header, $rows);
  }
  else {
    $output .= t('There are no unscored essays.');
  }
  return $output;
}