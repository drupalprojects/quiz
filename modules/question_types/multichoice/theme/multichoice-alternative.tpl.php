<?php

/**
 * @file
 * Handles the layout of the multichoice answering form
 *
 * Variables available:
 * - $form
 */
// Add script for using the entire alternative row as a button
drupal_add_js(drupal_get_path('module', 'multichoice') . '/js/multichoice.alternative.js');

// We want to have the checkbox in one table cell, and the title in the next.
// We store the checkbox and the titles
$options = $form['user_answer']['#options'];
$fullOptions = $titles = array();
foreach ($options as $key => $value) {
  $fullOptions[$key] = $form['user_answer'][$key];
  $titles[$key] = $form['user_answer'][$key]['#title'];
  $fullOptions[$key]['#title'] = '';
  unset($form['user_answer'][$key]);
}
unset($form['user_answer']['#options']);
print drupal_render_children($form);

// We use the stored checkboxes and titles to generate a table for the
// alternatives
foreach ($titles as $key => $value) {
  $rows[] = array(
    'class' => array('multichoice_row'),
    'data'  => array(
      array('data' => drupal_render($fullOptions[$key]), 'width' => 35, 'class' => 'selector-td'),
      $value
  ));
}
print theme('table', array('header' => array(), 'rows' => $rows));
