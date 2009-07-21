<?php 
// $id$

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
}