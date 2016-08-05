<?php

/**
 * @file
 * Theme functions for long_answer.
 */

/**
 * Theme the long_answer response form
 *
 * @param $form
 *  The response form
 */
function theme_long_answer_response_form($variables) {
  $form = $variables['form'];
  return drupal_render_children($form);
}
