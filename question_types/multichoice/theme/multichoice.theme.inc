<?php 
// $Id$

/**
 * The theme file for multichoice.
 * 
 * Sponsored by: Norwegian Centre for Telemedicine
 * Code: falcon
 *
 * @file
 * Theming functions for the multichoice question type.
 */

function theme_multichoice_creation_form($form) {
  $path = drupal_get_path('module', 'multichoice') .'/multichoice.js';
  drupal_add_js($path, 'module');
  
  // We open the settings fieldset if there is errors involving the choice_multi setting
  $errors = form_get_errors();
  if (isset($errors['choice_multi'])) {
    $form['settings']['#collapsed'] = FALSE;
  }
  
  // We open the alternative fieldsets if errors have been reported
  if ($errors) {
    for ($i = 0; is_array($form[$i]); $i++) {
      if (drupal_strlen(strip_tags($_POST['alternatives'][$i]['answer'])) > 0) {
        $form[$i]['#collapsed'] = FALSE;
      }
    }
  }
  return drupal_render($form);
}