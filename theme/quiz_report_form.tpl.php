<?php
// $Id$
$header = array(format_plural(count($questions), 'Question Result', 'Question Results'));
$rows = array();
foreach ($form as $key => $value) {
  if (!is_numeric($key)) continue;
  unset($form[$key]);
  $content = drupal_render($value);
  if (empty($content)) continue;
  $cols = array();
  $cols[] = array('data' => $content, 'class' => 'quiz_summary_qrow');
  // Get the score result for each question only if it's a scored quiz.
  if ($form['#showpoints']) {
    $theme = ($value['#is_correct']) ? 'quiz_score_correct' : 'quiz_score_incorrect';
    $cols[] = array('data' => theme($theme), 'class' => 'quiz_summary_qcell');
  }
  $rows[] = array('data' => $cols, 'class' => 'quiz_summary_qrow');
}
print theme('table', $header, $rows);
print drupal_render($form);
?>