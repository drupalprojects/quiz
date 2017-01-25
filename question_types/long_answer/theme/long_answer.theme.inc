<?php

/**
 * @file
 * Theme functions for long_answer.
 */

/**
 * Theme the long_answer response form.
 *
 * @param array $variables
 *
 * @return string
 *   The rendered HTML of all children of the element.
 */
function theme_long_answer_response_form($variables) {
  $form = $variables['form'];
  return drupal_render_children($form);
}
