<?php

// $Id$

/**
 * @file
 *   Themes for the matching module.
 */

/**
 * @function
 *   Themes matching question form.
 *
 * @param $form
 *   drupal form array
 */
function theme_matching_question_form($form) {
  $rows = array();
  $header = array(
    'question' => t('Question'),
    'answer' => t('Correct answer'),
    'feedback' => t('Feedback'),
  );

  foreach (element_children($form) as $key) {
    $rows[] = array(
      'question' => drupal_render($form[$key]['question']),
      'answer' => drupal_render($form[$key]['answer']),
      'feedback' => drupal_render($form[$key]['feedback']),
    );
  }
  // Theme output and display to screen.
  return theme('table', $header, $rows);
}

function theme_matching_answering_form($form) {
  $out = '<table class = "matching-tbl">';
  foreach($form as $key => $value) {
    if(is_int($key)) {
      $out .= '<tr><td class = "matching-question">'. $value['#question'];
      $out .= '</td><td class = "matching-select">'. drupal_render($value) .'</td></tr>';
    }
  }
  $out .= '</table>';
  return $out;
}