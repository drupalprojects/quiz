<?php

/**
 * User pages.
 * @file
 */


/**
 * Theme a table containing array of questions and options.
 *
 * @param $questions
 *  Array of question nodes.
 * @return
 *  HTML for a table.
 *
 * @ingroup themeable
 */
function theme_qcollection_question_table($questions, $quiz_id = NULL) {
  $output = '';

  $question_types = _quiz_get_question_types();

  $rows = array();
  while (list($key, $question) = each($questions)) {
    // TODO include comment counts
    $info = '<div class="question_info"><div class="question_type">'.$question_types[$question->type]['name'] .
    '</div>'._question_rating_snip($question).'<div class="question_actions">'. l(t('View'), 'node/'. $question->nid) .'</div></div>';
    $rows[] = array($question->question, $info);
  }

  $header = array(t('Question prompt'), t('Info'));

  if (!empty($rows)) {
    $output .= theme('table', $header, $rows);
  }
  else {
    $output .= t('No questions in collection.');
  }
  return "<div>$output</div>";
}

function _question_rating_snip($question) {
  // will only be set if ratings are activated for the question type
  $isRated = (boolean) variable_get('fivestar_style_'. $question->type, NULL);

  if ($isRated)
    return '<div class="fivestar">'.fivestar_widget_form($question).'</div>';
  else
    return '';
}
