<?php 
// $Id$

/**
 * The theme file for multichoice.
 * 
 * Sponsored by: Norwegian Centre for Telemedicine
 * Code: falcon
 *
 * @file
 * Theming functions for the multichoice question type.
 */

function theme_multichoice_creation_form($form) {
  $path = drupal_get_path('module', 'multichoice') .'/multichoice.js';
  drupal_add_js($path, 'module');
  
  // We open the settings fieldset if there is errors involving the choice_multi setting
  $errors = form_get_errors();
  if (isset($errors['choice_multi'])) {
    $form['settings']['#collapsed'] = FALSE;
  }
  
  // We open the alternative fieldsets if errors have been reported
  if ($errors) {
    for ($i = 0; is_array($form[$i]); $i++) {
      if (drupal_strlen(strip_tags($_POST['alternatives'][$i]['answer'])) > 0) {
        $form[$i]['#collapsed'] = FALSE;
      }
    }
  }
  return drupal_render($form);
}

/**
 * Theme the answer part of the node view
 * 
 * @param $alternatives
 *  Array of alternatives. Each alternative is also an array with all the
 *  data for each alternative.
 * @param $show_correct
 *  True if the user is allowed to view the solution
 */
function theme_multichoice_answer_node_view($alternatives, $show_correct) {
  $header = array('', '');

  foreach ($alternatives as $i => $short) {
    $answer_markup = check_markup($short['answer'], $short['answer_format'], FALSE);
    // Find the is_correct status
    $is_correct = ($short['score_if_chosen'] > $short['score_if_not_chosen']);
    $p = drupal_get_path('module', 'multichoice');
    $image = $is_correct ? 'correct' : 'wrong';
    if (!$show_correct) $image = 'unknown';
      
    $rows[] = array(
        array(
          'data' => theme('image',
            "$p/theme/images/$image.png",
            t($image),
            $show_correct ?
            t('Score if chosen: @sc Score if not chosen: @nc', array(
              '@sc' => $short['score_if_chosen'],
              '@nc' => $short['score_if_not_chosen'])
            ) :
            t('You are not allowed to veiw the solution for this question')
          ),
          'width' => 35
        ),
        $answer_markup
      );
    }
  return theme('table', $header, $rows);
}