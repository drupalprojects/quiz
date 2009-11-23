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
  $quizzes = array();
  $sql = "SELECT nr.nid, nr.vid, nr.title, n.status
          FROM {node_revisions} nr
          JOIN {quiz_node_relationship} qnr ON qnr.parent_vid = nr.vid
          JOIN {node} n ON n.nid = nr.nid
          WHERE qnr.child_vid = %d";
  $res = db_query($sql, $vid);
  while ($res_o = db_fetch_object($res)) {
    $res_o->answered = quiz_has_been_answered($res_o);
    $quizzes[] = $res_o;
  }
  $text = t('You have created a new revision of a question that belongs to %num quizzes.', array('%num' => count($quizzes)));
  $text .= ' '. t('Choose what you want to do with the different quizzes.');
  $form['intro'] = array(
    '#type' => 'markup',
    '#value' => $text,
  );
  $form['quizzes'] = array();
  foreach ($quizzes as $quiz) {
    $published = $quiz->status == 1 ? t('published') : t('unpublished');
    $answered = $quiz->answered ? t('answered') : t('unanswered');
    $options = _quiz_question_revision_options($quiz->status == 1, $quiz->answered);
    $form['quizzes'][$quiz->vid] = array(
      '#type' => 'radios',
      '#title' => check_plain($quiz->title) .' ('. $published .' '. t('and') .' '. $answered .')',
      '#default_value' => $options['default'],
      '#options' => $options['options'], 
    );
    $form['quizzes'][$quiz->vid]['#quiz_title'] = $quiz->title;
  }
  
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
  );
  $form['#submit'] = array('quiz_question_revision_actions_submit');
  return $form;
}

function _quiz_question_revision_options($published, $answered){
  $struct = array(
    'published' => array(
      'answered' => array(
        'options' => array(
	      '110' => t('Update, revise and unpublish'),
	      '111' => t('Update and revise'),
	      '101' => t('Update'),
	      '001' => t('Do nothing'),
        ),
        'default' => '111',   
      ),
      'unanswered' => array(
        'options' => array(
          '100' => t('Update and Unpublish'),
          '101' => t('Update'),
          '001' => t('Do nothing'),
        ),
        'default' => '101',
      ),
      
    ),
    'unpublished' => array(
      'answered' => array(
        'options' => array(
          '111' => t('Update, revise and publish'),
          '110' => t('Update and revise'),
          '101' => t('Update and publish'),
          '100' => t('Update'),
          '000' => t('Do nothing'),
        ),
        'default' => '110',
      ),
      'unanswered' => array(
        'options' => array(
          '101' => t('Update and publish'),
          '100' => t('Update'),
          '000' => t('Do nothing'),
        ),
        'default' => '100',
      ),
    ),    
  );
  $published = $published ? 'published' : 'unpublished';
  $answered = $answered ? 'answered' : 'unanswered';
  return $struct[$published][$answered];
}

function quiz_question_revision_actions_submit($form, &$form_state) {
  foreach ($form_state['values'] as $key => $value) {
    if (!is_numeric($key)) continue;
    switch ($value) {
      case 'ur':
        _quiz_question_revise_quiz();
        _quiz_question_update_in_quiz();
        break;
      case 'uu':
        _quiz_question_update_in_quiz();
        break;
      case 'u':
        _quiz_question();
        break;
      case 'n':
        
        break;
    }
  }
}