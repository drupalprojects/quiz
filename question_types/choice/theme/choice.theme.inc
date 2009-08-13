<?php 
// $Id$

/**
 * The theme file for choice.
 * 
 * Sponsored by: Norwegian Centre for Telemedicine
 * Code: falcon
 *
 * @file
 * Theming functions for the choice questiontype.
 */

function theme_choice_creation_form($form) {
  $path = drupal_get_path('module', 'choice') .'/choice.js';
  drupal_add_js($path, 'module');
  $errors = form_get_errors();
  if (isset($errors['choice_multi'])) {
    $form['settings']['#collapsed'] = FALSE;
  }
  if ($errors) {
    for ($i = 0; is_array($form[$i]); $i++) {
      if (drupal_strlen(strip_tags($_POST['alternatives'][$i]['answer'])) > 0) {
        $form[$i]['#collapsed'] = FALSE;
      }
    }
  }
  return drupal_render($form);
}