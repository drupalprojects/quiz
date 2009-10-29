<?php
// $Id$

/**
 * @file
 * Themes the question report
 * 
 * Available variables:
 * $form - FAPI array
 * 
 * All questions are in form[x] where x is an integer.
 * Useful values:
 * $form[x]['question'] - the question as a FAPI array(usually a form field of type "markup")
 * $form[x]['score'] - the users score on the current question.(FAPI array usually of type "markup" or "textfield")
 * $form[x]['max_score'] - the max score for the current question.(FAPI array of type "value")
 * $form[x]['response'] - the users response, usually a FAPI array of type markup.
 * $form[x]['#is_correct'] - If the users response is correct(boolean)
 * $form[x]['#is_evaluated'] - If the users response has been evaluated(boolean)
 */
$td_classes = array('quiz-report-odd-td', 'quiz-report-even-td');
$td_class_i = 0;
$p = drupal_get_path('module', 'quiz') .'/theme/';
$q_image = $p. 'question_bg.png';
?>

<h2><?php print format_plural(count($questions), 'Question Result', 'Question Results');?></h2>

<table>

<?php
foreach ($form as $key => $sub_form):
  if (!is_numeric($key) || $sub_form['#no_report'] === TRUE) continue;
  unset($form[$key]);
  $c_class = ($sub_form['#is_evaluated']) ? ($sub_form['#is_correct']) ? 'q-correct' : 'q-wrong' : 'q-waiting';?>

  <tr class="quiz-report-question">
	<td class="quiz-report-question">
	  <?php /* <strong><?php print t('Question')?>: </strong> */ ?>
	  <?php print drupal_render($sub_form['question']);?>
	</td>
	
	<td class="quiz-report-score <?php print $c_class?>">
	  <span>
	  <?php print t('Score')?>
	  <?php print drupal_render($sub_form['score'])?>
      <?php print t('of') .' '. $sub_form['max_score']['#value'] ?>
	  </span>
 	</td>
  </tr>

  <tr colspan="2">
	<td colspan="2" class="quiz-report-answer">
	  <strong><?php print t('Response')?></strong>
  
      <?php print drupal_render($sub_form['response']);
      // if ($td_class_i == 1) $td_class_i = 0;
      // else $td_class_i = 1;
      ?>
    </td>
  </tr>
<?php endforeach; ?>

</table>

<div style="float:right;"><?php print drupal_render($form);?></div>