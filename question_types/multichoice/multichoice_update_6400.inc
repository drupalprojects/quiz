<?php 
//TODO: Test with enormous amount of content... (test with endless amount of content, make loop insert and delete...)
//TODO: Drop unused quiz tables
//TODO: Don't let quiz.install make unused quiz tables.
function _multichoice_update_6400(&$sandbox = NULL) {
  $to_return = array();
  if (!$sandbox['multichoice0']) {
    $sandbox['progress'] = 1;
    $sandbox['max'] = 6;
    $sandbox['finished'] = $sandbox['progress']/$sandbox['max'];
    _multichoice_alter_user_answers1($to_return);
    $sandbox['multichoice0'] = TRUE; 
  }
  elseif (!$sandbox['multichoice1']) {
    _multichoice_move_old_answers($to_return, $sandbox);
    $sandbox['progress'] = 2;
    $sandbox['max'] = 6;
    $sandbox['finished'] = $sandbox['progress']/$sandbox['max'];
  }
  elseif (!$sandbox['multichoice2']) {
    _multichoice_alter_user_answers2($to_return);
    $sandbox['progress'] = 3;
    $sandbox['max'] = 6;
    $sandbox['finished'] = $sandbox['progress']/$sandbox['max'];
    $sandbox['multichoice2'] = TRUE; 
  }
  elseif (!$sandbox['multichoice3']) {
    _multichoice_create_properties($to_return);
    _multichoice_alter_answers1($to_return);
    $sandbox['multichoice3'] = TRUE;
    $sandbox['progress'] = 4;
    $sandbox['max'] = 6;
    $sandbox['finished'] = $sandbox['progress']/$sandbox['max'];
  }
  elseif (!$sandbox['multichoice4']) {
    _multichoice_check_answers($to_return, $sandbox);
    $sandbox['progress'] = 5;
    $sandbox['max'] = 6;
    $sandbox['finished'] = $sandbox['progress']/$sandbox['max'];
  }
  elseif (!$sandbox['multichoice5']) {
    _multichoice_create_user_settings($to_return);
    $sandbox['multichoice5'] = TRUE;
    $sandbox['progress'] = 6;
    $sandbox['max'] = 6;
    $sandbox['finished'] = 1;
  }
  $to_return['#finished'] = $sandbox['finished'];
  return $to_return;
}
function _multichoice_alter_user_answers1(&$to_return) {
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
  if (!isset($sandbox['progress'])) {
    $sandbox['progress'] = 0;
    $sandbox['last_rid'] = 0;
    $sandbox['max'] = db_result(db_query('SELECT COUNT(DISTINCT id) FROM {quiz_multichoice_user_answers}'));
  }
  $sql = 'SELECT id, answer_id, question_nid, question_vid, result_id
          FROM {quiz_multichoice_user_answers}
          WHERE result_id > %d
          ORDER BY result_id, question_nid, question_vid';
  $res = db_query_range($sql, $sandbox['last_rid'], 0, 100);
  $prog_to_add = 0;
  while ($res_o = db_fetch_object($res)) {
    $sql = 'INSERT IGNORE INTO {quiz_multichoice_user_answer_multi}
            (user_answer_id, answer_id)
            VALUES(%d, %d)';
    db_query($sql, $res_o->id, $res_o->answer_id);
    if($res_o->question_nid == $sandbox['last_nid']
    && $res_o->question_vid == $sandbox['last_vid']
    && $res_o->result_id    == $sandbox['last_rid']) {
      $sql = 'DELETE FROM {quiz_multichoice_user_answers}
              WHERE id = %d';
      db_query($sql, $res_o->id);
    }
    else {
      $sandbox['last_nid'] = $res_o->question_nid;
      $sandbox['last_vid'] = $res_o->question_vid;
      $sandbox['last_rid'] = $res_o->result_id;
    }
    $prog_to_add++;
  }
  $sandbox['progress'] += $prog_to_add;
  if ($sandbox['progress'] >= $sandbox['max']) $sandbox['multichoice1'] = TRUE; 
}
function _multichoice_alter_user_answers2(&$to_return) {
  db_drop_field($to_return, 'quiz_multichoice_user_answers', 'answer_id');
}
function _multichoice_create_properties() {
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
function _multichoice_alter_answers1(&$to_return) {
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
    $sandbox['last_id'] = 0;
    $sandbox['last_nid'] = 0;
    $sandbox['last_vid'] = 0;
    $sandbox['num_corrects'] = 0;
    $sandbox['step_max'] = db_result(db_query('SELECT COUNT(DISTINCT id) FROM {quiz_multichoice_answers}'));
  }
  $sql = 'SELECT id, question_nid, question_vid, score_if_chosen, score_if_not_chosen
          FROM {quiz_multichoice_answers}
          WHERE id > %d
          ORDER BY question_nid, question_vid';
  $res = db_query_range($sql, $sandbox['last_id'], 0, 2);
  while ($res_o = db_fetch_object($res)) {
    if($res_o->question_nid == $sandbox['last_nid']
    && $res_o->question_vid == $sandbox['last_vid']) {
    }
    else {
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
    $sandbox['last_id'] = $res_o->id;
    $sandbox['step_progress']++;
  }
  if ($sandbox['step_progress'] >= $sandbox['step_max']) $sandbox['multichoice4'] = TRUE; 
}
function _multichoice_create_user_settings(&$to_return) {
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