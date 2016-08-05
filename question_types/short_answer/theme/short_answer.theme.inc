<?php

/**
 * @file
 * Theme functions for short_answer.
 */

/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function theme_short_answer_user_answer($variables) {
  $answer = $variables['answer'];
  $correct = $variables['correct'];
  $header = array(t('Correct Answer'), t('User Answer'));
  $row = array(array($correct, $answer));
  return theme('table', array('header' => $header, 'rows' => $row));
}

/**
 * Theme the short_answer response form
 *
 * @param $form
 *  The response form
 */
function theme_short_answer_response_form($variables) {
  $form = $variables['form'];
  return drupal_render_children($form);
}
