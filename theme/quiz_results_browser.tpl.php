<?php
// $Id$
/**
 * @file
 * Handles the layout of the quiz question browser.
 *
 *
 * Variables available:
 * - $form
 */

// Add js
$p = drupal_get_path('module', 'quiz') .'/theme/';
drupal_add_js($p .'quiz_results_browser.js', 'module');

// We need to separate the title and the checkbox. We make a custom options array...
$full_options = array();
foreach ($form['name']['#options'] as $key => $value) {
  $full_options[$key] = $form['name'][$key];
  $full_options[$key]['#title'] = '';
}

// Print ahah targets
print drupal_render($form['ahah_target_all']);
print drupal_render($form['ahah_target']);

$rows = array();
$cols = array();

// We make the filter row
$cols[] = array('data' => drupal_render($form['filters']['all']) . drupal_render($form['filters']['name']), 'class' => 'container-inline', 'style' => 'white-space: nowrap;');
$cols[] = drupal_render($form['filters']['started']);
$cols[] = drupal_render($form['filters']['finished']);
$cols[] = drupal_render($form['filters']['duration']);
$cols[] = drupal_render($form['filters']['score']);
$rows[] = array('data' => $cols, 'id' => 'quiz-question-browser-filters');

// We make the result rows
foreach ($form['name']['#options'] as $key => $value) {
  $cols = array();
  
  // Find nid and rid
  $matches = array();
  preg_match('/([0-9]+)-([0-9]+)/', $key, $matches);
  $res_nid = $matches[1];
  $res_rid = $matches[2];
  
  // The checkbox(without the title)
  $data = '<span class = "container-inline" style = white-space: nowrap;>'. drupal_render($full_options[$key]) . $value .'</span>'; //Always shown
  $data .= '<div class = "quiz-hover-menu">'.$form['view'][$key]['#value'].'</div>';
  $cols[] = array('data' => $data, 'width' => 35);
  
  $cols[] = $form['started'][$key]['#value'];
  $cols[] = $form['finished'][$key]['#value'];
  $cols[] = $form['duration'][$key]['#value'];
  $cols[] = $form['score'][$key]['#value'];
  
  $rows[] = array('data' => $cols, 'class' => 'quiz-results-browser-row', 'id' => 'browser-'. $key);
}

print theme('table', $form['#header'], $rows, array('class' => 'browser-table'));

if (count($form['name']['#options']) == 0)
  print t('No results were found');

print $form['pager']['#value'];
print drupal_render($form['add_to_get']);
print drupal_render($form['ahah_target_all_end']);
?>