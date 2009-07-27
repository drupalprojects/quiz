<?php
// $Id$
/**
 * @file
 * Handles the layout of the choice creation form.
 *
 *
 * Variables available:
 * - $form
 */

?>
<?php 
$options = $form['#options'];
$fullOptions = array();
$titles = array();
foreach ($options as $key => $value) {
  $fullOptions[$key] = $form[$key];
  $titles[$key] = $form[$key]['#title'];
  $fullOptions[$key]['#title'] = '';
  unset($form[$key]);
}
unset($form['#options']);
print drupal_render($form);
foreach ($titles as $key => $value) {
  $row = array();
  $row[] = array('data' => drupal_render($fullOptions[$key]), 'width' => 20);
  $row[] = $value;
  $rows[] = $row;
}
print theme('table', NULL, $rows);
?>