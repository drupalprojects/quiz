<?php
// $Id$

/**
 * @file
 * User page callbacks for the quiz_question module.
 */


/**
 * Menu callback. Prints a listing of containers that reference the question node nodes.
 *
 * @param $qid
 *   ID of question to look up
 * @return
 *   String containing the markup.
 */
function quiz_question_references($node) {
  $parents = _quiz_question_parents($node);

  if (empty($parents)) {
    return '<div>No references.</div>';
  } else {
    $output = '<div>';
    foreach ($parents as $parent)
      $output .= node_view($parent, true);
    $output .= '</div>';
    return $output;
  }
}

/**
 * @param $question_nid
 *
 * @return
 * a list of nodes that are parents to the question node
 */
function _quiz_question_parents($question_node) {
  $parents = array();
  // Get all the questions (ignore `question_status`)
  $sql = "SELECT DISTINCT parent_nid as nid
    FROM {quiz_node_relationship}
    WHERE child_vid = %d
      AND child_nid = %d
    ORDER BY weight";
  $result = db_query($sql, $question_node->vid, $question_node->nid);
  while ($parent_node_info = db_fetch_array($result)) { // nid, vid
    $parent_nodes[] = node_load($parent_node_info['nid']);
  }
  return $parent_nodes;
}

function quiz_question_revision_actions($form_state, $vid) {
  $form = array();
  $form['#theme'] = 'quiz_question_revision_actions';
  $quizzes = array();
  $sql = "SELECT nr.nid, nr.vid, nr.title
          FROM {node_revisions} nr
          JOIN {quiz_node_relationship} qnr
          ON qnr.parent_vid = nr.vid
          WHERE qnr.child_vid = %d";
  $res = db_query($sql, $vid);
  while ($res_o = db_fetch_object($res)) {
    $quizzes[] = $res_o;
  }
  $form['#intro'] = t('You have created a new revision of a question that belongs to %num quizzes.', array('%num' => count($quizzes)));
  $form['#desc'] = t('Choose what you want to do with the different quizzes.');
  $form['quizzes'] = array();
  $options = array(0 => NULL, 1 => NULL, 2 => NULL, 3 => NULL);
  foreach ($quizzes as $quiz) {
    $form['quizzes'][$quiz->vid] = array(
      '#type' => 'radios',
      '#title' => 'test',
      '#options' => $options,
      '#default_value' => 0,
    );
    $form['quizzes'][$quiz->vid]['#quiz_title'] = $quiz->title;
  }
  $form['#quiz_header'] = array(
    t('Quiz'), 
    t('Update and revise'),
    t('Update and unpublish'),
    t('Update'),
    t('Do nothing'),
  );
  $form['explanation'] = array();
  $form['explanation'][0]['#title'] = t('Update and revise');
  $form['explanation'][0]['#expl'] = t('Updates the quiz with the new revision of the question, and makes a new revision of the quiz.');
  $form['explanation'][0]['#practical'] = t('Use this when your changes affects the difficulty, meaning or scoring of the question.');
  $form['explanation'][1]['#title'] = t('Update and unpublish');
  $form['explanation'][1]['#expl'] = t('Updates the quiz with the new revision of the question, and unpublishes the quiz.');
  $form['explanation'][1]['#practical'] = t('Use this when you plan to make changes to multiple questions in the quiz.');
  $form['explanation'][2]['#title'] = t('Update');
  $form['explanation'][2]['#expl'] = t('Updates the quiz with the new revision of the question.');
  $form['explanation'][2]['#practical'] = t('Use this when you have made unsignificant changes, like correcting spelling errors or blemishes.');
  $form['explanation'][2]['#warning'] = t('If your changes affects the scoring of the quiz you should make a new revision so that the old scoring data won\'t be affected by your changes.');
  $form['explanation'][3]['#title'] = t('Do nothing');
  $form['explanation'][3]['#expl'] = t('The quiz will not be updated');
  $form['explanation'][3]['#practical'] = t('Use this when your changes aren\'t supposed to be included in excisting quizzes.');
  
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
    '#submit' => array('quiz_question_revision_actions_submit'),
  );
  return $form;
}