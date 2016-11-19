<?php

/**
 * Theme function for user response report
 *
 * @param array $variables User response data
 *
 * @return bool|string User response report
 */
function theme_quiz_h5p_response($variables) {
  static $css_added;
  if (!$css_added) {
    drupal_add_css(drupal_get_path('module', 'quiz_h5p') . '/theme/quiz_h5p.css');
    $css_added = TRUE;
  }

  if (isset($variables['description'])) {
    $description = json_decode($variables['description']);
    $description = (array) $description;
    return $description['en-US'];
  }
  else {
    return FALSE;
  }
}
