<?php

/**
 * @file
 * Preprocessors and helper functions to make theming easier.
 */
 
/**
 * Prepares variables for views exposed form templates.
 *
 * Default template: quiz-questions-browser-header.html.twig.
 *
 * @param array $vars
 *   An associative array containing:
 *   - form: A render element representing the form.
 */
function template_preprocess_quiz_questions_browser_header(&$vars) {
  $form = $vars['form'];
  $rows = array();
  //Construnct row
  $row = array();
  $row[]['data'] = $form['filters']['all'];
  $row[]['data'] = $form['filters']['title'];
  $row[]['data'] = $form['filters']['type'];
  $row[]['data'] = $form['filters']['changed'];
  $row[]['data'] = $form['filters']['name'];
  $rows[] = $row;
  //Theme table
  $vars['table'] = array(
    '#theme' => 'table',
    '#header' => $form['#header'],
    '#rows' => $rows,
  );  
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
