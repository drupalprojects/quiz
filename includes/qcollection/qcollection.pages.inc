<?php
// $Id$

/**
 * User pages.
 * @file
 */


/**
 * Theme a table containing array of questions and options.
 *
 * @param $questionNodes
 *  Array of question nodes.
 * @return
 *  HTML for a table.
 *
 * @ingroup themeable
 */
function theme_qcollection_question_table(array $questionNodes) {
  $output = '';

  $question_types = _quiz_get_question_types();

  $rows = array();
  while (list($key, $questionNode) = each($questionNodes)) {
    // TODO include comment counts
    $info = '<div class="question_info"><div class="question_type">'.$question_types[$questionNode->type]['name'] .
    '</div>'._question_rating_snip($questionNode).'<div class="question_actions">'. l(t('View'), 'node/'. $questionNode->nid) .'</div></div>';
    $rows[] = array($questionNode->question, $info);
  }

  $header = array(t('Question'), t('Info'));

  if (!empty($rows)) {
    $output .= theme('table', $header, $rows);
  }
  else {
    $output .= t('No questions in collection.');
  }
  return "<div>$output</div>";
}

// TODO remove this from qcollection and handle in external theme
function _question_rating_snip(stdclass $questionNode) {
  global $user;

  // will only be set if ratings are activated for the question type
  $isRated = (boolean) variable_get('fivestar_style_'. $questionNode->type, NULL);

  if (!$isRated) {
   return '';
  }

  // show static stars for anonymous and interactive for logged in
  if ($user->uid == 0) {
    // anonymous user
    $stars = fivestar_static('node', $questionNode->nid, $tag = 'vote', $questionNode->type);
    $stars .= ' ('. l(t('Log in to rate'), 'user') .'.)';
  } else {
    $stars = fivestar_widget_form($questionNode);
  }

  return '<div class="fivestar">'.$stars.'</div>';
}
