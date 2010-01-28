<?php
// $Id$

/**
 * @file
 * User page callbacks for the quiz_question module.
 */


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

function quiz_question_revision_actions($form_state, $nid, $vid) {
  if (!isset($_SESSION['quiz_question_kept']))
    drupal_goto('node/'. $nid);
  $form = array();
  $form['q_nid'] = array('#type' => 'value', '#value' => $nid);
  $form['q_vid'] = array('#type' => 'value', '#value' => $vid);
  $quiz_nids = array();
  $quiz_vids = array();
  foreach($_SESSION['quiz_question_kept'] as $nid_vid) {
    $temp = explode('-', $nid_vid);
    if (is_numeric($temp[0]) && is_numeric($temp[1])) {
      $quiz_nids[] = $temp[0];
      $quiz_vids[] = $temp[1];
    }
  }
  $quizzes = array();
  $sql = "SELECT nr.nid, nr.vid, nr.title, n.status
          FROM {node_revisions} nr
          JOIN {node} n ON n.nid = nr.nid
          WHERE nr.vid IN (". implode(',', $quiz_vids) .")";
  $res = db_query($sql);
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
    $form['quizzes'][$quiz->nid .'-'. $quiz->vid .'-'. $quiz->status .'-'. ($quiz->answered ? '1' : '0')] = array(
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
  $form['update_expl'] = array(
    '#type' => 'item',
    '#title' => t('Update'),
    '#value' => t('Replace the old revision of the question with the new revision.'),
  );
  $form['revise_expl'] = array(
    '#type' => 'item',
    '#title' => t('Revise'),
    '#value' => t('If a quiz has been answered you should make a new revision to ensure that excisting answer statistics and reports remain correct.'),
    '#description' => t('If the new revision of the question only correct spelling errors etc. you don\'t need to revise.'),
  );
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
  unset($_SESSION['quiz_question_kept']);
  foreach ($form_state['values'] as $key => $value) {
    if (!preg_match('/^[0-9]+-[0-9]+-[0,1]-[0,1]$/', $key)) continue;
    $chars = str_split($value); //FORMAT: Update[0,1], revise[0,1] and publish[0,1]
    $codes = explode('-', $key); //FORMAT: nid(int), vid(int), published[0,1] and answered[0,1]
    //If we are to revise the quiz we need to do that first...
    if ($chars[1] == '1') {
      $quiz_node = node_load((int)$codes[0], (int)$codes[1]);
      $quiz_node->revision = 1;
    }
    if (!isset($quiz_node) && $chars[2] != $codes[2])
      $quiz_node = node_load((int)$codes[0], (int)$codes[1]);
    if (isset($quiz_node)) {
      $quiz_node->auto_created = TRUE;
      $quiz_node->status = $chars[2];
      node_save($quiz_node);
      $quiz_nid = $quiz_node->nid;
      $quiz_vid = $quiz_node->vid;
    }
    else {
      $quiz_nid = (int)$codes[0];
      $quiz_vid = (int)$codes[1];
    }
    if ($chars[0] == '1') {
      // TODO: This doesn't necessarily get the newest max_score...
      $sql = 'SELECT max_score 
              FROM {quiz_node_relationship} 
              WHERE parent_vid = %d
              AND child_nid = %d';
      $res = db_query($sql, $quiz_vid, $form_state['values']['q_nid']);
      if ($max_score = db_result($res));
      else {
        $sql = 'SELECT max_score 
                FROM {quiz_question_properties} 
                WHERE vid = %d';
        $res = db_query($sql, $form_state['values']['q_vid']);
        $max_score = db_result($res);
      }
      $sql = 'DELETE FROM {quiz_node_relationship}
              WHERE parent_nid = %d AND parent_vid = %d AND child_nid = %d';
      db_query($sql, $quiz_nid, $quiz_vid, $form_state['values']['q_nid']);
      
      $sql = 'INSERT INTO {quiz_node_relationship} (parent_nid, parent_vid, child_nid, child_vid, max_score)
              VALUES(%d, %d, %d, %d, %d)';
      db_query($sql, $quiz_nid, $quiz_vid, $form_state['values']['q_nid'], $form_state['values']['q_vid'], $max_score);
    }
  }
}