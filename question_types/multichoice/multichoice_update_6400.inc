<?php 
//TODO: Drop unused quiz tables
//TODO: Don't let quiz.install make unused quiz tables.
function _multichoice_update_6400(&$sandbox = NULL) {
  $to_return = array();
  $sandbox['current_step'] = isset($sandbox['current_step']) ? $sandbox['current_step'] : 0;
  $steps = array();
  $steps[] = array('alter_user_answers1', 'next_step');
  $steps[] = array('move_old_answers');
  $steps[] = array('alter_user_answers2', 'create_properties', 'alter_answers', 'next_step');
  $steps[] = array('check_answers');
  $steps[] = array('create_user_settings', 'next_step');
  foreach ($steps[$sandbox['current_step']] as $value) {
    $function = '_multichoice_'. $value;
    $function($to_return, $sandbox);
  }
  $to_return['#finished'] = $sandbox['current_step'] / count($steps);
  return $to_return;
}
function _multichoice_alter_user_answers1(&$to_return, &$sandbox = NULL) {
  db_drop_primary_key($to_return, 'quiz_multichoice_user_answers');
  db_add_field($to_return, 'quiz_multichoice_user_answers', 'id', array(
    'type' => 'serial',
    'unsignet' => TRUE,
    'not_null' => TRUE,
  ), array('primary key' => array('id')));
  db_add_field($to_return, 'quiz_multichoice_user_answers', 'choice_order', array('type' => 'text'));
  db_create_table($to_return, 'quiz_multichoice_user_answer_multi', array(
    'fields' => array(
      'user_answer_id' => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
      ),
      'answer_id' => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
      ),
    ),
    'primary key' => array(
      'user_answer_id', 'answer_id'
    ),
  ));
}

function _multichoice_move_old_answers(&$to_return, &$sandbox = NULL) {
  if (!isset($sandbox['step_progress'])) {
    $sandbox['step_progress'] = 0;
    $sandbox['last_rid'] = -1;
    $sandbox['last_nid'] = -1;
    $sandbox['last_vid'] = -1;
    $sandbox['last_id'] = -1;
    $sandbox['step_max'] = db_result(db_query('SELECT COUNT(DISTINCT id) FROM {quiz_multichoice_user_answers}'));
  }
  $sql = 'SELECT id, answer_id, question_nid, question_vid, result_id
          FROM {quiz_multichoice_user_answers}
          WHERE result_id >= %d
          ORDER BY result_id, question_nid, question_vid, answer_id';
  $res = db_query_range($sql, $sandbox['last_rid'], 0, 50);
  $progress_to_add = 0;
  while ($res_o = db_fetch_object($res)) {
    if($res_o->question_nid == $sandbox['last_nid']
    && $res_o->question_vid == $sandbox['last_vid']
    && $res_o->result_id    == $sandbox['last_rid']) {
      if($res_o->id > $sandbox['last_id']){
        $sql2 = 'DELETE FROM {quiz_multichoice_user_answers}
                 WHERE id = %d';
        db_query($sql2, $res_o->id);
        if ($progress_to_add + $sandbox['step_progress'] + 1 >= $sandbox['step_max']) {
          $sandbox['step_progress'] = $sandbox['step_max'];
        }
      }
    }
    else {
      $sandbox['last_nid'] = $res_o->question_nid;
      $sandbox['last_vid'] = $res_o->question_vid;
      $sandbox['last_rid'] = $res_o->result_id;
      $sandbox['last_id'] = $res_o->id;
      $sandbox['step_progress'] += $progress_to_add;
      $progress_to_add = 0;
    }
    $sql = 'INSERT IGNORE INTO {quiz_multichoice_user_answer_multi}
            (user_answer_id, answer_id)
            VALUES(%d, %d)';
    db_query($sql, $sandbox['last_id'], $res_o->answer_id);
    $progress_to_add++;
  }
  if ($sandbox['step_progress'] >= $sandbox['step_max']) _multichoice_next_step($to_return, $sandbox); 
}
function _multichoice_alter_user_answers2(&$to_return, &$sandbox = NULL) {
  db_drop_field($to_return, 'quiz_multichoice_user_answers', 'answer_id');
}
function _multichoice_create_properties(&$to_return, &$sandbox = NULL) {
  db_create_table($to_return, 'quiz_multichoice_properties', array(
    'fields' => array(
      'nid' => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,  
      ),
      'vid' => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
      ),
      'choice_multi' => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'default' => 0,
      ),
      'choice_random' => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'default' => 0,
      ),
      'choice_boolean' => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
        'default' => 0,
      ),
    ),
    'primary key' => array(
      'nid', 'vid'
    ),
  ));
}
function _multichoice_alter_answers(&$to_return, &$sandbox = NULL) {
  //db_drop_primary_key($to_return, 'quiz_multichoice_answers');
  db_change_field($to_return, 'quiz_multichoice_answers', 'answer_id', 'id', array(
    'type' => 'serial',
    'unsigned' => TRUE,
    'not null' => TRUE,
  ));
  db_change_field($to_return, 'quiz_multichoice_answers', 'nid', 'question_nid', array(
    'type' => 'int',
    'unsigned' => TRUE,
    'not null' => TRUE,
  ));
  db_change_field($to_return, 'quiz_multichoice_answers', 'vid', 'question_vid', array(
    'type' => 'int',
    'unsigned' => TRUE,
    'not null' => TRUE,
  ));
  db_change_field($to_return, 'quiz_multichoice_answers', 'feedback', 'feedback_if_chosen', array(
    'type' => 'text',
  ));
  db_add_field($to_return, 'quiz_multichoice_answers', 'feedback_if_chosen_format', array(
    'type' => 'int',
  ));
  db_add_field($to_return, 'quiz_multichoice_answers', 'feedback_if_not_chosen', array(
    'type' => 'text',
  ));
  db_add_field($to_return, 'quiz_multichoice_answers', 'feedback_if_not_chosen_format', array(
    'type' => 'int',
  ));
  db_add_field($to_return, 'quiz_multichoice_answers', 'answer_format', array(
    'type' => 'int',
  ));
  db_change_field($to_return, 'quiz_multichoice_answers', 'is_correct', 'score_if_chosen', array(
    'type' => 'int',
    'unsigned' => TRUE,
    'not_null' => TRUE,
    'default' => 0,
  ));
  db_add_field($to_return, 'quiz_multichoice_answers', 'score_if_not_chosen', array(
    'type' => 'int',
    'unsigned' => TRUE,
    'not_null' => TRUE,
    'default' => 0,
  ));
  db_drop_field($to_return, 'quiz_multichoice_answers', 'result_option');
}
function _multichoice_check_answers(&$to_return, &$sandbox) {
  if (!isset($sandbox['multichoice_check_answers'])) {
    $sandbox['step_progress'] = 0;
    $sandbox['multichoice_check_answers'] = TRUE;
    $sandbox['last_nid'] = 0;
    $sandbox['last_vid'] = 0;
    $sandbox['step_max'] = db_result(db_query('SELECT COUNT(DISTINCT id) FROM {quiz_multichoice_answers}'));
  }
  $sandbox['num_corrects'] = 0;
  $sql = 'SELECT id, question_nid, question_vid, score_if_chosen, score_if_not_chosen
          FROM {quiz_multichoice_answers}
          WHERE question_nid >= %d AND question_vid >= %d
          ORDER BY question_nid, question_vid';
  $res = db_query_range($sql, $sandbox['last_nid'], $sandbox['last_vid'], 0, 50);
  $progress_to_add = 0;
  while ($res_o = db_fetch_object($res)) {
    if($res_o->question_nid == $sandbox['last_nid']
    && $res_o->question_vid == $sandbox['last_vid']) {
      if ($progress_to_add + $sandbox['step_progress'] + 1 >= $sandbox['step_max']) {
        $sandbox['step_progress'] = $sandbox['step_max'];
      }
    }
    else {
      $sandbox['step_progress'] += $progress_to_add;
      $progress_to_add = 0;
      $sandbox['last_nid'] = $res_o->question_nid;
      $sandbox['last_vid'] = $res_o->question_vid;
      $sandbox['num_corrects'] = 0;
      $sql = 'SELECT format
              FROM {node_revisions}
              WHERE nid = %d AND vid = %d';
      $res2 = db_query($sql, $res_o->question_nid, $res_o->question_vid);
      $sandbox['last_format'] = db_result($res2);
      $sql = 'INSERT INTO {quiz_multichoice_properties}
              (nid, vid, choice_boolean)
              VALUES(%d, %d, 1)';
      db_query($sql, $res_o->question_nid, $res_o->question_vid);
    }
    if ($res_o->score_if_chosen == 1) { 
      $sandbox['num_corrects']++;
      if ($sandbox['num_corrects'] == 2) {
        $sql = 'UPDATE {quiz_multichoice_properties}
                SET choice_multi = 1
                WHERE nid = %d AND vid = %d';
        db_query($sql, $res_o->question_nid, $res_o->question_vid);
      } 
    }
    $sql = 'UPDATE {quiz_multichoice_answers}
            SET answer_format = %d, feedback_if_chosen_format = %d, feedback_if_not_chosen_format = %d
            WHERE question_nid = %d AND question_vid = %d';
    $lf = $sandbox['last_format'];
    db_query($sql, $lf, $lf, $lf, $res_o->question_nid, $res_o->question_vid);
    $progress_to_add++;
  }
  $sandbox['step_progress'] += $progress_to_add;
  if ($sandbox['step_progress'] >= $sandbox['step_max']) _multichoice_next_step($to_return, $sandbox); 
}
function _multichoice_create_user_settings(&$to_return, &$sandbox = NULL) {
  db_create_table($to_return, 'quiz_multichoice_user_settings', array(
    'fields' => array(
      'uid' => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
      ),
      'last_nid' => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
      ),
      'last_vid' => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
      ),
    ),
    'primary key' => array(
      'uid'
    ),
  ));

}
function _multichoice_next_step(&$to_return, &$sandbox) {
  $sandbox['current_step']++;  
}