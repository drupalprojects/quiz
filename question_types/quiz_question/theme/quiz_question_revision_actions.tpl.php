<?php 
// $Id$
?>
<p><?php print $form['#intro'];?></p>
<p><?php print $form['#desc'];?></p>
<?php 
$rows = array();
foreach ($form['quizzes'] as $key => $quiz) {
  if (!is_numeric($key)) continue;
  $cols = array();
  $cols[] = $quiz['#quiz_title'];
  foreach($quiz as $sub_key => $sub_form) {
    if (!is_numeric($sub_key)) continue;
    $cols[] = drupal_render($sub_form);
  }
  $rows[] = $cols;
}
print theme('table', $form['#quiz_header'], $rows);
print drupal_render($form['submit']);
print '<br/><br/><h1>'. t('Explanation') .'</h1><br/>';
foreach ($form['explanation'] as $explanation):
?>
<b><?php print $explanation['#title']?></b><br/>
<?php print $explanation['#expl']?>
<p><i><?php print $explanation['#practical']?></i></p>
<?php if (isset($explanation['#warning'])):?>
<div style="color:#dd0000;"><?php print $explanation['#warning']?></div><br/>
<?php endif;?>
<?php endforeach;?>