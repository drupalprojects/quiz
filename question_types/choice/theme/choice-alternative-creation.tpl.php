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
$title_correct = check_plain($form['correct']['#title']);
unset($form['correct']['#title']);
$suf = $form['answer']['#required'] ? '<SPAN CLASS="form-required"> *</SPAN>' : '';
$title_answer = check_plain($form['answer']['#title']).$suf;
$form['answer']['#title'] = '';
$row[] = drupal_render($form['correct']);
$row[] = drupal_render($form['answer']);
$rows[] = $row;
$header[] = array('data' => $title_correct);
$header[] = array('data' => $title_answer);
print theme('table', $header, $rows);

//These lines make things look alot beter if user only has one input format available:
$form['format']['format']['guidelines']['#value'] = ' ';
$form['format']['3']['#value'] = ' ';
$form['advanced']['format']['format']['guidelines']['#value'] = ' ';
$form['advanced']['format']['3']['#value'] = ' ';
$form['advanced']['helper']['format']['format']['guidelines']['#value'] = ' ';
$form['advanced']['helper']['format']['3']['#value'] = ' ';

print drupal_render($form['format']);
print drupal_render($form['advanced']);
?>