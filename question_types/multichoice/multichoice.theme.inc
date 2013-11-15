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
  $form['#attached']['js'] = array(
    drupal_get_path('module', 'multichoice') . '/js/multichoice.js',
  );

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

  $p = drupal_get_path('module', 'multichoice');
  drupal_add_css($p .'/css/multichoice.css');

  foreach ($alternatives as $i => $short) {
    $answer_markup = check_markup($short['answer'], $short['answer_format']);
    // Find the is_correct status
    $is_correct = ($short['score_if_chosen'] > $short['score_if_not_chosen']);
    $image = $is_correct ? 'correct' : 'wrong';
    if (!$show_correct) {
      $image = 'unknown';
    }

    $rows[] = array(
        array(
          'data' => array(
            '#theme' => 'html_tag',
            '#tag' => 'span',
            '#attributes' => array(
              'class' => array(
                'multichoice-icon',
                $image,
              ),
              'title' => $show_correct ?
                t('Score if chosen: @sc Score if not chosen: @nc', array(
                  '@sc' => $short['score_if_chosen'],
                  '@nc' => $short['score_if_not_chosen'])
                ) :
                t('You are not allowed to view the solution for this question'),
            ),
          ),
          'class' => array('multichoice-icon-cell'),
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
  static $css_added;
  if (!$css_added) {
    drupal_add_css(drupal_get_path('module', 'multichoice') . '/css/multichoice.css');
    $css_added = TRUE;
  }
  
  $rows = array();
  foreach ($variables['data'] as &$alternative) {
    if ($alternative['is_chosen']) {
      switch ($alternative['is_correct']) {
        case 0:
          $icon = '<span class="multichoice-icon wrong" title="' . t('This option is wrong.') . '"></span>';
          break;
        case 1:
          $icon = '<span class="multichoice-icon almost" title="' . t('This option is correct, but there is another that gives a higher score.') . '"></span>';
          break;
        case 2:
          $icon = '<span class="multichoice-icon correct" title="' . t('This option is correct.') . '"></span>';
          break;
      }
    }
    else {
      $icon = $alternative['is_correct'] == 2 ? '<span class="multichoice-icon should" title="' . t("This option is correct, but you didn't select it.") . '"></span>' : '';
    }
    $rowspan = $alternative['feedback'] ? 2 : 1;
    $rows[] = array(
      array(
        'data' => $icon,
        'rowspan' => $rowspan,
        'class' => 'selector-td multichoice-icon-cell',
      ),
      $alternative['answer']
    );
    if ($rowspan == 2) {
      $rows[] = array('<div class="multichoice-label">' . t('Feedback') . ':</div><div class="multichoice-feedback">' . $alternative['feedback'] . '</div>');
    }
  }
  return theme('table', array('header' => NULL, 'rows' => $rows));
}


function theme_multichoice_alternative_creation($variables) {
  $form = $variables['form'];
  $form['#attached']['js'] = array(
    drupal_get_path('module', 'multichoice') . '/js/multichoice-alternative-creation.js',
  );

  $output = '';

  // Get the title from the checkbox, and then unset it. We will place it as a table header
  $title_correct = check_plain($form['correct']['#title']);
  unset($form['correct']['#title']);

  // We have to add the required symbol manually
  $suf = $form['answer']['#required'] ? '<SPAN CLASS="form-required"> *</SPAN>' : '';

  // We store the title for the answer section as well
  $title_answer = check_plain($form['answer']['#title']).$suf;
  $form['answer']['#title'] = '';

  // Now we can render the table
  $row[] = drupal_render($form['correct']);
  $row[] = drupal_render($form['answer']);
  $rows[] = $row;
  $header[] = array('data' => $title_correct);
  $header[] = array('data' => $title_answer);
  $output .= theme('table', array('header' => $header, 'rows' => $rows));

  //These lines make things look alot beter if user only has one input format available:
  _quiz_format_mod($form['format']);
  _quiz_format_mod($form['advanced']['format']);
  _quiz_format_mod($form['advanced']['helper']['format']);

  $output .= drupal_render($form['format']);
  $output .= drupal_render($form['advanced']);

  return $output;
}

function theme_multichoice_alternative($variables) {
  $form = $variables['form'];
  $output = '';
  $p = drupal_get_path('module', 'multichoice');
  drupal_add_css($p .'/css/multichoice.css');

  // Add script for using the entire alternative row as a button
  drupal_add_js(
  "(function($) {
    Drupal.behaviors.multichoiceAlternativeBehavior = {
      attach: function(context, settings) {
        $('.multichoice_row')
        .once()
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
      }
    };
  })(jQuery);", 'inline');

  // We want to have the checkbox in one table cell, and the title in the next. We store the checkbox and the titles
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
  $output .= drupal_render_children($form);

  // We use the stored checkboxes and titles to generate a table for the alternatives
  foreach ($titles as $key => $value) {
    $row = array();
    $row[] = array('data' => drupal_render($fullOptions[$key]), 'width' => 35, 'class' => 'selector-td');
    $row[] = $value;
    $rows[] = array('data' => $row, 'class' => array('multichoice_row'));
  }
  $output .= theme('table', array('header' => array(), 'rows' => $rows));

  return $output;
}

function theme_multichoice_answering_form($variables) {
  $form = $variables['form'];
  return drupal_render($form);
}
