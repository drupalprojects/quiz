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
foreach ($form['titles']['#options'] as $key => $value) {
  $fullOptions[$key] = $form['titles'][$key];
  $fullOptions[$key]['#title'] = '';
}
print drupal_render($form['ahah_target_all']);
print drupal_render($form['ahah_target']);
$rows = array();
$cols = array();
$cols[] = drupal_render($form['filters']['all']);
$cols[] = drupal_render($form['filters']['title']);
$cols[] = drupal_render($form['filters']['type']);
$cols[] = drupal_render($form['filters']['changed']);
$cols[] = drupal_render($form['filters']['name']);
$rows[] = array('data' => $cols, 'id' => 'quiz_question_browser_filters');
foreach ($form['titles']['#options'] as $key => $value) {
  $cols = array();
  $matches = array();
  preg_match('/-([0-9]+)-([0-9]+)/', $key, $matches);
  $quest_nid = $matches[1];
  $quest_vid = $matches[2];
  $cols[] = array('data' => drupal_render($fullOptions[$key]), 'width' => 35);
  $cols[] = l($value, "node/$quest_nid", array('query' => array('destination' => $_GET['q'])));
  $cols[] = $form['types'][$key]['#value'];
  $cols[] = $form['changed'][$key]['#value'];
  $cols[] = $form['names'][$key]['#value'];
  $rows[] = array('data' => $cols, 'class' => 'quiz_question_browser_row');
}
print theme('table', $form['#header'], $rows);
if (count($form['titles']['#options']) == 0)
  print t('No questions were found');
print $form['pager']['#value'];
print drupal_render($form['add_to_get']);
print drupal_render($form['ahah_target_all_end']);
?>