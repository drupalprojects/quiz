<?php 
print '<div class="quiz_summary_question"><span class="quiz_question_bullet">Q:</span> ';
print drupal_render($form['question']);
print '</div>';

print '<b>'. t('Response') .':</b>';
print drupal_render($form['answer']);

print '<b>'. t('Score') .': </b>'. drupal_render($form['score']) .' '; 
print t('of') .' '. $form['max_score']['#value'] .' '. t('possible points.');
if (isset($form['feedback']))
  print '<p><b>'. t('Feedback') .': </b><div class="quiz_answer_feedback">'. drupal_render($form['feedback']) .'</div></p>';
?>