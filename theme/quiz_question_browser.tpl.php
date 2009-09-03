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

$header = array(NULL, t('Title'), t('Type'), t('Username'));
foreach ($form['titles']['#options'] as $key => $value) {
  $row = array();
  $matches = array();
  preg_match('/-([0-9]+)-([0-9]+)/', $key, $matches);
  $quest_nid = $matches[1];
  $quest_vid = $matches[2];
  $row[] = array('data' => drupal_render($fullOptions[$key]), 'width' => 35);
  $row[] = l($value, "node/$quest_nid", array('query' => array('destination' => $_GET['q'])));
  $row[] = $form['types'][$key]['#value'];
  $row[] = $form['names'][$key]['#value'];
  $rows[] = array('data' => $row, 'class' => 'quiz_question_browser_row');
}
print theme('table', $header, $rows);
?>