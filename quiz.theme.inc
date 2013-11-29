<?php

/**
 * @file
 * Preprocessors and helper functions to make theming easier.
 */
 
/**
 * Theme quiz_questions_browser_header.
 */
function theme_quiz_questions_browser_header(&$vars) {
  $form = $vars['form'];

  $html = '<thead>';
  $html .= '<tr>';

  foreach ($form['#header'] as $header) {
    $html .= '<th>' . $header['data'] . '</th>';
  }
  $html .= '</tr>';
  $html .= '<tr id="quiz-question-browser-filters">';
  $html .= '<td>' . drupal_render($form['filters']['all']) . '</td>';
  $html .= '<td>' . drupal_render($form['filters']['title']) . '</td>';
  $html .= '<td>' . drupal_render($form['filters']['type']). '</td>';
  $html .= '<td>' . drupal_render($form['filters']['changed']) . '</td>';
  $html .= '<td>' . drupal_render($form['filters']['name']) . '</td>';
  $html .= '</tr>';
  $html .= '</thead>';

  return $html;
}

/*
 * Theme quiz_questions_browser_body.
 */
function theme_quiz_questions_browser_body(&$vars) {
  $form = $vars['form'];
  $html = '<tbody id="quiz-browser-body" class="browser-table">';

  $full_options = array();
  foreach ($form['titles']['#options'] as $key => $value) {
    $full_options[$key] = $form['titles'][$key];
    $full_options[$key]['#title'] = '';
  }

  // We make the question rows
  foreach ($form['titles']['#options'] as $key => $value) {

    // Find nid and vid
    $matches = array();
    preg_match('/([0-9]+)-([0-9]+)/', $key, $matches);
    $quest_nid = $matches[1];
    $quest_vid = $matches[2];

    $html .= '<tr class="quiz-question-browser-row" id="browser-' . $key . '">';
    $html .= '<td width="35">' . drupal_render($full_options[$key]) . '</td>';
    $html .= '<td>';
    $html .= l($value, "node/$quest_nid", array(
      'html' => TRUE,
      'query' => array('destination' => current_path()),
      'attributes' => array('target' => 'blank')
    ));
    $html .= '</td>';
    $html .= '<td>' . $form['types'][$key]['#value'] . '</td>';
    $html .= '<td>' . $form['changed'][$key]['#value'] . '</td>';
    $html .= '<td>' . $form['names'][$key]['#value'] . '</td>';
    $html .= '</tr>';
  }

  if (count($form['titles']['#options']) == 0) {
    print t('No questions were found');
  }

  $html .= '</tbody>';

  return $html;
}

/**
 * Theme quiz_results_browser_header.
 */
function theme_quiz_results_browser_header(&$vars) {
  $form = $vars['form'];

  $html = '<thead id="quiz-browser-head">';
  $html .= '<tr>';
  foreach ($form['#header'] as $header) {
    $html .= '<th>' . $header['data'] . '</th>';
  }

  $html .= '</tr>';
  $html .= '<tr id="quiz-question-browser-filters">';
  $html .= '<td class="container-inline" style="white-space: nowrap">';
  $html .= drupal_render($form['filters']['all']) . drupal_render($form['filters']['name']);
  $html .= '</td><td>';
  $html .= drupal_render($form['filters']['started']);
  $html .= '</td><td>';
  $html .= drupal_render($form['filters']['finished']);
  $html .= '</td><td>';
  $html .= drupal_render($form['filters']['score']);
  $html .= '</td><td>';
  $html .= drupal_render($form['filters']['evaluated']);
  $html .= '</td>';
  $html .= '</tr>';
  $html .= '</thead>';

  return $html;
}

  /**
   * Theme quiz_results_browser_body.
   */
  function theme_quiz_results_browser_body(&$vars) {
    $form = $vars['form'];
    $html = '<tbody id="quiz-browser-body">';

    // We need to separate the title and the checkbox. We make a custom options array...
    $full_options = array();
    foreach ($form['name']['#options'] as $key => $value) {
      $full_options[$key] = $form['name'][$key];
      $full_options[$key]['#title'] = '';
    }
    foreach ($form['name']['#options'] as $key => $value) {
      $html .= '<tr class="quiz-results-browser-row" id="browser-' . $key . '">';
      $html .= '<td valign="top">';

      // Find nid and rid
      $matches = array();
      preg_match('/([0-9]+)-([0-9]+)/', $key, $matches);
      $res_nid = $matches[1];
      $res_rid = $matches[2];

      $html .= '<span class = "container-inline" style = "white-space: nowrap;">' . drupal_render($full_options[$key]) . $value . '</span>';
      $html .= '<div class = "quiz-hover-menu">' . $form['hover_menu'][$key]['#value'] . '</div>';
      $html .= '</td><td valign="top">';
      $html .= $form['started'][$key]['#value'];
      $html .= '</td><td valign="top">';
      $html .= $form['finished'][$key]['#value'];

      if ($form['finished'][$key]['#value'] != t('In progress')) {
          $html .= '<br/><em>(' . t('Duration'). ': ' . $form['duration'][$key]['#value'] . ')</em>';
      }

      $html .= '</td><td valign ="top">';
      if (!is_numeric($form['score'][$key]['#value'])) {
        $html .= $form['score'][$key]['#value'];
      }
      elseif ($form['evaluated'][$key]['#value'] == t('No')) {
        $html .= t('Not evaluated');
      }
      else {
        if (!empty($form['pass_rate'][$key]['#value']) && is_numeric($form['score'][$key]['#value'])) {
          $pre_score = $form['score'][$key]['#value'] >= $form['pass_rate'][$key]['#value'] ? '<span class = "quiz-passed">' : '<span class = "quiz-failed">';
          $post_score = $form['score'][$key]['#value'] >= $form['pass_rate'][$key]['#value'] ?' %<br><em>'. t('Passed') .'</em></span>' : ' %<br><em>'. t('Failed') .'</em></span>';
        }
        else {
          $pre_score = '';
          $post_score = ' %';
        }
        $html .= $pre_score . $form['score'][$key]['#value'] . $post_score;
      }

      $html .= '</td><td valign="top">';
      $html .= $form['evaluated'][$key]['#value'];
      $html .= '</td>';
      $html .= '</tr>';
    }

    $html .= '</tbody>';

    return $html;
  }

  /**
 * Theme the quiz report form.
 */
function theme_quiz_report_form($variables) {
  $form = $variables['form'];
  $p = drupal_get_path('module', 'quiz') .'/templates/';
  $q_image = $p. 'question_bg.png';
  
  drupal_add_css(drupal_get_path('module', 'quiz') .'/quiz.css');
  
  $output = '';
  $output .= '<h2>' . t('Question Results') . '</h2>';
  $output .= '<div class="quiz-report">';

  foreach ($form as $key => $sub_form) {
    if (!is_numeric($key) || isset($sub_form['#no_report'])) continue;
    unset($form[$key]);
    $c_class = ($sub_form['#is_evaluated']) ? ($sub_form['#is_correct']) ? 'q-correct' : 'q-wrong' : 'q-waiting';
    $skipped = $sub_form['#is_skipped'] ? '<span class="quiz-report-skipped">'. t('(skipped)') .'</span>' : '';

    $output .= '<div class="dt">';
	  $output .= '<div class="quiz-report-score-container ' . $c_class. '">';
	  $output .= '<span>';
    $output .= t('Score');
    $output .= ' ' . drupal_render($sub_form['score']);
    $output .= ' ' . t('of') .' '. $sub_form['max_score']['#value'];
    $output .= '<br><em>'. $skipped .'</em>';
    $output .= '</span>';
    $output .= '</div>';
	  $output .= '<p class="quiz-report-question"><strong>' . t('Question') . ': </strong></p>';

    // Fix for html entities.
    $sub_form['question']['#markup'] = decode_entities($sub_form['question']['#markup']);
	  $output .= drupal_render($sub_form['question']);
    $output .= '</div>';
    $output .= '<div class="dd">';
    $output .= '<p><strong>' .t('Response') . ': </strong></p>';
    $output .= drupal_render($sub_form['response']);
    $output .= '</div>';
    $output .= '<div class="dd">';
    $output .= drupal_render($sub_form['answer_feedback']);
    $output .= '</div>';
  }
  $output .= '</div>';
  $output .= '<div class="quiz-score-submit">' . drupal_render_children($form). '</div>';
  
  return $output;
}
