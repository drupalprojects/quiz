<?php 
print '<div class="quiz_summary_question"><span class="quiz_question_bullet">Q(short):</span> ';
print drupal_render($form['question']);
print drupal_render($form['score']) .' '. t('of') .' '. drupal_render($form['max_score']) .' '. t('points');
?>