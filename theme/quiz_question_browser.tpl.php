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
//TODO: Move this to a separate file
print "
<SCRIPT type='text/javascript'>Drupal.behaviors.quizQuestionBrowserBehavior = function(context) {
  $('.quiz_question_browser_row')
  .filter(':has(:checkbox:checked)')
  .addClass('selected')
  .end()
  .click(function(event) {
    $(this).toggleClass('selected');
    if (event.target.type !== 'checkbox') {
      $(':checkbox', this).attr('checked', function() {
        return !this.checked;
      });
      $(':radio', this).attr('checked', true);
      if ($(':radio', this).html() != null) {
        $('.multichoice_row').removeClass('selected');
    	  $(this).addClass('selected');
      }
    }
  });
};</SCRIPT>";
$fullOptions = array();
foreach ($form['titles']['#options'] as $key => $value) {
  $fullOptions[$key] = $form['titles'][$key];
  $fullOptions[$key]['#title'] = '';
}
$rows = array();
$cols = array();
$cols[] = drupal_render($form['filters']['all']);
$cols[] = drupal_render($form['filters']['title']);
$cols[] = drupal_render($form['filters']['type']);
$cols[] = drupal_render($form['filters']['changed']);
$cols[] = drupal_render($form['filters']['name']);
$rows[] = array('data' => $cols);
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
print drupal_render($form['filters']);
print theme('table', $form['#header'], $rows);
print $form['pager']['#value'];
?>