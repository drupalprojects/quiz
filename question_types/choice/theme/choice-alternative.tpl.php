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
$p = drupal_get_path('module', 'choice');
drupal_add_css($p .'/theme/choice.css', 'module', 'all');
if ($form['#taking_quiz']) drupal_add_js($p .'/theme/choice_taking.js', 'module');
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
  if ($form['#taking_quiz']) {
    $row[] = array('data' => drupal_render($fullOptions[$key]), 'width' => 35);
  } else {
    if ($form['#correct_choice'][$key]) {
      $row[] = array('data' => theme('image', "$p./theme/images/correct.png", t('Correct'), t('This alternative is correct')), 'width' => 35);
    }
    else {
      $row[] = array('data' => theme('image', "$p./theme/images/wrong.png", t('Wrong'), t('This alternative is wrong')), 'width' => 35);
    }
  }
  $row[] = $value;
  $rows[] = array('data' => $row, 'class' => 'choice_row');
}
print theme('table', NULL, $rows);
?>