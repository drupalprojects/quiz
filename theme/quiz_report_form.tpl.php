<?php
// $Id$

/**
 * @file
 * Themes the question report
 * 
 * Available variables:
 * $form - FAPI array
 * 
 */
$td_classes = array('quiz-report-odd-td', 'quiz-report-even-td');
$td_class_i = 0;
?>
<h2><?php print format_plural(count($questions), 'Question Result', 'Question Results');?></h2>
<table>
<?php
foreach ($form as $key => $sub_form) {
  if (!is_numeric($key) || $sub_form['#no_report'] === TRUE) continue;
  unset($form[$key]);
  ?>
  <tr><td class="<?php print $td_classes[$td_class_i]?>"><table class = "quiz-report-q-header">
    <tr>
      <td valign="middle" class = "quiz-report-q-cell">
        <h3 class = "quiz-report-question-label"><?php print t('Question')?></h3>
      </td>
      <td class = "quiz-report-score-cell">
        <table class = "quiz-report-q-header">
          <tr>
            <td valign = "middle" style = "padding-right:5px;" align = "right" class = "quiz-report-score-cell"><?php print t('Score')?></td>
            <td valign = "middle" style = "width:1%; padding-right:5px;" class = "quiz-report-score-cell"><?php print drupal_render($sub_form['score'])?></td>
            <td valign = "middle" align = "left" class = "quiz-report-score-cell"><?php print t('of') .' '. $sub_form['max_score']['#value'] ?></td>
          </tr>
        </table>
      </td>
    </tr>
  </table>
  <div class="quiz-report-question"><?php print drupal_render($sub_form['question']);?></div>
  <?php $theme = ($sub_form['#is_correct']) ? t('The response is correct') : t('The response is incorrect');
  ?>
  <h3><?php print t('Response')?></h3>
  
  <?php print drupal_render($sub_form['response']);
  print($theme);
  if ($td_class_i == 1) $td_class_i = 0;
  else $td_class_i = 1;
  ?>
  </td></tr><?php 
}
?>
</table>