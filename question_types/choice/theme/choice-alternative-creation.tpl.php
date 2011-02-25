<?php
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
if (!function_exists('_choice_format_mod')) {
  function _choice_format_mod(&$format) {
    $format['#attributes']['class'] = 'choice_filter';
    if (isset($format['format'])) {
      $format['format']['guidelines']['#value'] = ' ';
      foreach ($format as $key => $value) {
        if (is_numeric($key)) {
          $format[$key]['#value'] = ' ';
        }
      }
    }
  }
}
$p = drupal_get_path('module', 'choice');
drupal_add_js($p .'/theme/choice-alternative-creation.js', 'module');
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
_choice_format_mod($form['format']);
_choice_format_mod($form['advanced']['format']);
_choice_format_mod($form['advanced']['helper']['format']);

print drupal_render($form['format']);
print drupal_render($form['advanced']);
?>