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
$p = drupal_get_path('module', 'multichoice');
drupal_add_css($p .'/theme/multichoice.css', 'module', 'all');
print "
<SCRIPT type='text/javascript'>Drupal.behaviors.multichoiceAlternativeBehavior = function(context) {
  $('.multichoice_row')
  .filter(':has(:checkbox:checked)')
  .addClass('selected')
  .end()
  .click(function(event) {
    $(this).toggleClass('selected');
    if (event.target.type !== 'checkbox') {
      $(':checkbox', this).attr('checked', function() {
        return !this.checked;
      });
      $(':radio', this).attr('checked', true);
      if ($(':radio', this).html() != null) {
        $('.multichoice_row').removeClass('selected');
    	  $(this).addClass('selected');
      }
    }
  });
};</SCRIPT>";

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
  $row[] = array('data' => drupal_render($fullOptions[$key]), 'width' => 35);
  
  $row[] = $value;
  $rows[] = array('data' => $row, 'class' => 'multichoice_row');
}
print theme('table', NULL, $rows);
?>