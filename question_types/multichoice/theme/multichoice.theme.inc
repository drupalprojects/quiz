<?php

/**
 * The theme file for multichoice.
 *
 * Sponsored by: Norwegian Centre for Telemedicine
 * Code: falcon
 *
 * @file
 * Theming functions for the multichoice question type.
 */

/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function theme_multichoice_creation_form($variables) {
  $form = $variables['form'];
  $path = drupal_get_path('module', 'multichoice') . '/multichoice.js';
  drupal_add_js($path);

  // We open the settings fieldset if there is errors involving the choice_multi setting
  $errors = form_get_errors();
  if (isset($errors['choice_multi'])) {
    $form['settings']['#collapsed'] = FALSE;
  }

  // We open the alternative fieldsets if errors have been reported
  if ($errors) {
    for ($i = 0; isset($form[$i]) && is_array($form[$i]); $i++) {
      if (drupal_strlen(strip_tags($_POST['alternatives'][$i]['answer']['value'])) > 0) {
        $form[$i]['#collapsed'] = FALSE;
      }
    }
  }
  return drupal_render_children($form);
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
function theme_multichoice_answer_node_view($variables) {
  $alternatives = $variables['alternatives'];
  $show_correct = $variables['show_correct'];
  $header = array('', '');

  foreach ($alternatives as $i => $short) {
    $answer_markup = check_markup($short['answer'], $short['answer_format'], $langcode = '' /* TODO Set this variable. */, FALSE);
    // Find the is_correct status
    $is_correct = ($short['score_if_chosen'] > $short['score_if_not_chosen']);
    $p = drupal_get_path('module', 'multichoice');
    $image = $is_correct ? 'correct' : 'wrong';
    if (!$show_correct) {
      $image = 'unknown';
    }

    $rows[] = array(
        array(
          'data' => theme('image', array('path' =>
            "$p/theme/images/$image.png", 'width' =>
            t($image), 'height' =>
            $show_correct ?
            t('Score if chosen: @sc Score if not chosen: @nc', array(
              '@sc' => $short['score_if_chosen'],
              '@nc' => $short['score_if_not_chosen'])
            ) :
            t('You are not allowed to view the solution for this question')
          )),
          'width' => 35
        ),
        $answer_markup
      );
  }
  return theme('table', array('header' => $header, 'rows' => $rows));
}

/**
 * Theme function for the multichoice report
 *
 * @param $data
 *  Array of data to be presented in the report
 */
function theme_multichoice_response($variables) {
  $data = $variables['data'];
  $rows = array();
  $p = drupal_get_path('module', 'multichoice') . '/theme/images';
  foreach ($data as $id => $alternative) {
    $cols = array();
    $rowspan = drupal_strlen($alternative['feedback']) > 0 ? 2 : 1;
    $not = '';
    if ($alternative['is_chosen']) {
      switch ($alternative['is_correct']) {
        case 0:
          $cols[] = array(
            'data' => theme('image', array('path' => "$p/wrong.png", 'alt' => t('Wrong'), 'title' => t('This answer is wrong'), 'attributes' => array('class' => 'feedback-icon'))),
            'width' => 35,
            'rowspan' => $rowspan,
            'class' => 'selector-td',
          );
          break;
        case 1:
          $cols[] = array('data' => theme('image', array('path' => "$p/almost.png", 'alt' => t('Almost correct'), 'title' => t('This answer is correct, but there is an answer that gives a higher score'), 'attributes' => array('class' => 'feedback-icon'))),
            'width' => 35,
            'rowspan' => $rowspan,
            'class' => 'selector-td',
          );
          break;
        case 2:
          $cols[] = array(
            'data' => theme('image', array('path' => "$p/correct.png", 'alt' => t('Correct'), 'title' => t('This answer is correct'), 'attributes' => array('class' => 'feedback-icon'))),
            'width' => 35,
            'rowspan' => $rowspan,
            'class' => 'selector-td',
          );
          break;
      }
    }
    else {
      if ($alternative['is_correct'] == 2) {
        $cols[] = array(
          'data' => theme('image', array('path' => "$p/should.png", 'alt' => t('Should have chosen'), 'title' => t("This answer is correct, but you didn't choose it"), 'attributes' => array('class' => 'feedback-icon'))),
          'width' => 35,
          'rowspan' => $rowspan,
          'class' => 'selector-td',
        );
      }
      else {
        $cols[] = array(
          'data' => '',
          'width' => 35,
          'rowspan' => $rowspan,
          'class' => 'selector-td',
        );
      }
    }
    $cols[] = $alternative['answer'];
    $rows[] = $cols;
    if ($rowspan == 2) {
      $rows[] = array('<strong>' . t('Feedback:') . '</strong><div class="quiz_answer_feedback">' . $alternative['feedback'] . '</div>');
    }
  }
  return theme('table', array('header' => NULL, 'rows' => $rows));
}
