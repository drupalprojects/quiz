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
$p = drupal_get_path('module', 'quiz') .'/theme/';
drupal_add_js($p .'quiz_question_browser.js', 'module');
$fullOptions = array();
$table = $form;
foreach ($table['titles']['#options'] as $key => $value) {
  $fullOptions[$key] = $table['titles'][$key];
  $fullOptions[$key]['#title'] = '';
}
print drupal_render($table['ahah_target_all']);
print drupal_render($table['ahah_target']);
$rows = array();
$cols = array();
$cols[] = drupal_render($table['filters']['all']);
$cols[] = drupal_render($table['filters']['title']);
$cols[] = drupal_render($table['filters']['type']);
$cols[] = drupal_render($table['filters']['changed']);
$cols[] = drupal_render($table['filters']['name']);
$rows[] = array('data' => $cols, 'id' => 'quiz_question_browser_filters');
foreach ($table['titles']['#options'] as $key => $value) {
  $cols = array();
  $matches = array();
  preg_match('/([0-9]+)-([0-9]+)/', $key, $matches);
  $quest_nid = $matches[1];
  $quest_vid = $matches[2];
  $cols[] = array('data' => drupal_render($fullOptions[$key]), 'width' => 35);
  $cols[] = l($value, "node/$quest_nid", array('query' => array('destination' => $_GET['q']), 'attributes' => array('target' => 'blank')));
  $cols[] = $table['types'][$key]['#value'];
  $cols[] = $table['changed'][$key]['#value'];
  $cols[] = $table['names'][$key]['#value'];
  $rows[] = array('data' => $cols, 'class' => 'quiz-question-browser-row', 'id' => 'browser-'. $key);
}
print theme('table', $table['#header'], $rows, array('class' => 'browser-table'));
if (count($table['titles']['#options']) == 0)
  print t('No questions were found');
print $table['pager']['#value'];
print drupal_render($table['add_to_get']);
print drupal_render($table['ahah_target_all_end']);
?>