<?php
// $Id$

/**
 * Administrator interface for Quiz module.
 *
 * @file
 */


// QUIZ ADMIN

/**
 * Quiz Admin.
 */
function quiz_admin($nid) {
  $breadcrumb = drupal_get_breadcrumb();
  //lets add the Quiz Results to the breadcrumb array
  $breadcrumb[] = l(t('Quiz Results'), 'admin/quiz/results');
  drupal_set_breadcrumb($breadcrumb);
  $results = _quiz_get_results($nid);
  return theme('quiz_admin', $results);
}

/**
 * Displays the quizzes by title with a link to the appropriate results
 * for that specific quiz.
 *
 * @return
 *  Formatted data.
 */
function quiz_admin_quizzes() {
  $results = _quiz_get_quizzes();
  return theme('quiz_admin_quizzes', $results);
}

 // QUIZ RESULTS ADMIN

 /**
  * Quiz Results Admin.
  *
  * @param $qid
  *  The quiz result ID for a particular result.
  */
function quiz_admin_results($qid) {
  $breadcrumb = drupal_get_breadcrumb();
  // Lets add the Quiz Results to the breadcrumb array.
  $breadcrumb[] = l(t('Quiz Results'), 'admin/quiz/results');

  // We join against node because we might have multiple versions
  // of a quiz, and joining against quiz_node_properties would
  // return multiple rows with the same quiz result. Since we are
  // only hitting the {node} index, this should be fast.
  $result = db_fetch_object(db_query(
    "SELECT qnrs.nid
      FROM  {quiz_node_results} qnrs
      INNER JOIN {node} n ON qnrs.nid = n.nid
      WHERE qnrs.result_id = %d",
    $qid
  ));
  if ($result->nid) {
    $quiz = node_load($result->nid);
    $questions = _quiz_get_answers($qid);
    $score = quiz_calculate_score($quiz, $qid);
    $summary = _quiz_get_summary_text($quiz, $score);

    // Lets add the quiz title to the breadcrumb array.
    $breadcrumb[] = l($quiz->title, 'admin/quiz/' . $result->nid . '/view');
    drupal_set_breadcrumb($breadcrumb);
    return theme('quiz_admin_summary', $quiz, $questions, $score, $summary);
  }
  else {
    // Set the breadcrumb without the title since there isn't one and show error page.
    drupal_set_breadcrumb($breadcrumb);
    drupal_not_found();
  }
}

/**
 * Creates a form for quiz questions.
 *
 * Handles the manage questions tab.
 *
 * @param $qid
 *   ID of quiz to create
 * @return
 *   String containing the form.
 */
function quiz_questions($node) {
  // Set page title.
  drupal_set_title(check_plain($node->title));

  // Add JS
  $path = drupal_get_path('module', 'quiz') . '/quiz.admin.js';
  drupal_add_js($path, 'module');

  $options['random_pool']['enabled'] = TRUE;

  return drupal_get_form('quiz_questions_form', $node, $options);
}

// EDIT QUIZ

/**
 * Handles "manage questions" tab.
 *
 * Displays form which allows questions to be assigned to the given quiz.
 *
 * @param $form_state
 *  The form state variable
 * @param $quiz
 *  The quiz or qcollection node.
 * @return
 *  HTML output to create page.
 */
//TODO rename $quiz to $qcollection
function quiz_questions_form($form_state, $quiz, $options) {
  // This is a target for AHAH callbacks. Do not remove.
  $form['#action'] = '?q=node/'. $quiz->nid .'/questions';
  $form['ahah_target'] = array(
    '#type' => 'markup',
    '#value' => '<div id="questions-always-target"></div>',
  );
  // Display links to create other questions.
  $form['additional_questions'] = array(
    '#type' => 'fieldset',
    '#title' => t('Add new question'),
    '#theme' => 'additional_questions',
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );

  $types = _quiz_get_question_types();
  foreach ($types as $type => $info) {
    $url_type = str_replace('_', '-', $type);
    $form['additional_questions'][$type] = array(
      '#type' => 'markup',
      '#value' => '<div class="add-questions">' . l(t($info['name']), 'node/add/'. $url_type .'/'. $quiz->nid .'/'. $quiz->vid, array('title' => t('Go to @name administration', array('@name' => $info['name'])))) .'</div>',
    );
  }

  // Display questions 'always' on this quiz.
  $form['filtered_question_list_always'] = array(
    '#type' => 'fieldset',
    '#title' => (isset($options['fieldset_always']['title'])) ? $options['fieldset_always']['title'] : t('Questions always on this quiz'),
 //   '#theme' => 'quiz_filtered_questions',
    '#theme' => 'question_selection_table',
    '#collapsible' => (isset($options['fieldset_always']['collapsible'])) ? $options['fieldset_always']['collapsible'] : TRUE,
    'question_status' => array('#tree' => TRUE),
  );
  $hidden_questions = array();
  $form['filtered_question_list_always']['always_browse'] = _quiz_question_browser_form($hidden_questions, $form_state, $quiz, 'always');
    
  // TODO: Remove this
  $form['filtered_question_list_always']['remove_from_quiz'] = array(
    '#type' => 'hidden',
    '#default_value' => '',
  );

  // Display questions used 'randomly' on this quiz.
  if ($options['random_pool']['enabled']) {
    $form += _quiz_form_fields_random_pool($quiz);
  }

  // Build up a list of questions, sorted into those that are random and those that are
  // always on the quiz.

  // We use $form_state[post] to avoid validation failures when questions are added using AJAX
  if (isset($form_state['post']['weights'])) {
    // TODO: Add node access security
    foreach ($form_state['post']['weights'] as $id => $value) {
      $matches = array();
      preg_match('/-([0-9]+)-([0-9]+)/', $id, $matches);
      $nid = $matches[1];
      $vid = $matches[2];
      $node = node_load($nid, $vid);
      $node->max_score = $form_state['post']['max_scores'][$id];
      $node->weight = $weight;
      $q_to_add = quiz_node_map($node);
      $q_to_add->hidden = $form_state['post']['hiddens'][$id] === '1';
      $questions[] = $q_to_add;
    }
  }//TODO: Make question_status out of id...
  else {
    $questions = _quiz_get_questions($quiz->nid, $quiz->vid);
  }
  $temp_to_add = explode(',', $form_state['values']['always_browser']['questions_to_add']);
  $temp_to_add = array_merge($temp_to_add, $hidden_questions);
  $form['filtered_question_list_always']['temp_questions'] = array();
  $form['filtered_question_list_always']['hidden_questions'] = array();
  foreach ($temp_to_add as $key => $id) {
    $matches = array();
    preg_match('/-([0-9]+)-([0-9]+)/', $id, $matches);
    $nid = $matches[1];
    $vid = $matches[2];
    $continue = FALSE;
    foreach($questions as $question) {
      if($question->vid == $vid) {
        $continue = TRUE;
        break;
      }
    }
    if (!is_numeric($nid) || !is_numeric($vid) || $continue) continue; 
    $node = node_load($nid, $vid);
    $max_score = quiz_question_get_max_score($node->nid, $node->vid);
    $node->max_score = $max_score;
    $node->weight = 0; //TODO: Weight needs to be the biggest...
    $q_to_add = quiz_node_map($node);
    $form['filtered_question_list_always']['temp_questions'][$id]['#default_value'] = TRUE;
    if (in_array($id, $hidden_questions)) {
      $q_to_add->hidden = isset($form_state['values']['hiddens'][$id]) ? $form_state['values']['hiddens'][$id] === '1' : TRUE;
    }
    $questions[] = $q_to_add;
  }
  // IF new questions have been added using AJAX
  /*if (is_array($form_state['new_questions'])) {
    foreach ($form_state['new_questions'] as $quest) {
      $matches = array();
      preg_match('/id:([0-9]+)/', $quest, $matches);
      $nid = $matches[1];
      if (!is_numeric($nid) || $nid == 0) break;
      $node = node_load($nid);
      $max_score = quiz_question_get_max_score($node->nid, $node->vid);
      $node->max_score = $max_score;
      $node->weight = 0;
      foreach ($questions as $question) {
        $node->weight = max($node->weight, ($question->weight +1));
      }
      $q_to_add = quiz_node_map($node);
      $questions[] = $q_to_add;
    }
  }*/
  $rows = array();
  $form['filtered_question_list_random']['weights'] = array('#tree' => TRUE);
  $form['filtered_question_list_always']['weights'] = array('#tree' => TRUE);

  $form['filtered_question_list_random']['max_scores'] = array('#tree' => TRUE);
  $form['filtered_question_list_always']['max_scores'] = array('#tree' => TRUE);
  
  $form['filtered_question_list_random']['hiddens'] = array('#tree' => TRUE);
  $form['filtered_question_list_always']['hiddens'] = array('#tree' => TRUE);

  $form['filtered_question_list_random']['revision'] = array('#tree' => TRUE);
  $form['filtered_question_list_always']['revision'] = array('#tree' => TRUE);
  $my_dest = $_GET['q'];
  foreach ($questions as $question) {
    $id_mod = ($question->question_status == QUESTION_RANDOM ? 'random' : 'always');
    $fieldset = 'filtered_question_list_' . $id_mod;
    $id = $id_mod . '-' . $question->nid . '-' . $question->vid;

    $latest = node_load($question->nid);

    $form[$fieldset]['weights'][$id] = array(
      //'#type' => 'weight',
      //'#delta' => 60,
      '#type' => 'textfield',
      '#size' => 3,
      '#maxlength' => 4,
      '#default_value' => isset($question->weight) ? $question->weight : 0,
    );
    $form[$fieldset]['max_scores'][$id] = array(
      '#type' => 'textfield',
      '#size' => 2,
      '#maxlength' => 2,
      '#default_value' => isset($question->max_score) ? $question->max_score : 0,
    );
    $form[$fieldset]['hiddens'][$id] = array(
      '#type' => 'hidden',
      '#default_value' => (isset($question->hidden) && $question->hidden === TRUE) ? 1 : 0,
    );
    $form[$fieldset]['titles'][$id] = array(
      '#value' => l($question->title, 'node/'. $question->nid, array('html' => TRUE)),
    );
    $form[$fieldset]['types'][$id] = array(
      '#value' => $question->type,
    );
    $form[$fieldset]['view_links'][$id] = array(
      '#value' => l(t('Edit'), 'node/'. $question->nid .'/edit', array('query' => array('destination' => $my_dest))),
    );
    $form[$fieldset]['remove_links'][$id] = array(
      // FIXME: This does not degrade for non-JS browsers.
      //'#value' => l('Remove', 'node/' . $question->nid .'/questions/remove', array('attributes' => array('class' => 'rem-link'))),
      '#value' => '<a href="#" class="rem-link">' . t('Remove') . '</a>',
    );
    if ($question->vid == $latest->vid) {
      $update_cell = array(
        '#type' => 'markup',
        '#value' => t('<em>Up to date</em>'),
      );
    } else {
      $update_cell = array (
      '#type' => 'checkbox',
      '#title' => (l(t('Latest'), 'node/' . $question->nid . '/revisions/' . $latest->vid . '/view')
                  .' of '.
                  l(t('revisions'), 'node/' . $question->nid . '/revisions')
                  ),
      );
    }
    $form[$fieldset]['revision'][$id] = $update_cell;
  }
  // Show the number of 'always' questions in the 'always' table header.
  $always_count = isset($form['filtered_question_list_always']['titles']) ? count($form['filtered_question_list_always']['titles']) : 0;
  $form['filtered_question_list_always']['#title'] .= ' ('. $always_count .')';
  
  if (quiz_has_been_answered($quiz)) {
    $rev_default = TRUE;
    $rev_description = 'This quiz has been answered. To maintain correctnes of existing answer reports ';
    $rev_description .= 'changes should be saved as a new revision.';
  } 
  else {
    $rev_default = in_array('revision', variable_get('node_options_quiz', array()));
    $rev_description = 'Allow question status changes to create a new revision of the quiz?';
  }
  
  $form['new_revision'] = array(
    '#type' => 'checkbox',
    '#default_value' => $rev_default,
    '#title' => t('New revision'),
    '#description' => t($rev_description),
  );

  $form['timestamp'] = array('#type' => 'hidden', '#value' => time());

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
    '#submit' => array('quiz_questions_form_submit'),
  );

  return $form;
}

/**
 *
 * @param $quiz
 * @return array
 *   form fields for the section of the form handling random questions
 */
function _quiz_form_fields_random_pool($quiz) {
  $form['filtered_question_list_random'] = array(
    '#type' => 'fieldset',
    '#title' => t('Pool of questions randomly selected for this quiz'),
    '#theme' => 'question_selection_table',
    '#collapsible' => TRUE,
    '#collapsed' => FALSE, // Setting this to TRUE breaks tableDrag. What a drag.
    'question_status' => array('#tree' => TRUE),
  );

  $form['filtered_question_list_random']['number_of_random_questions'] = array(
    '#type' => 'textfield',
    '#title' => t('Number of questions to randomize'),
    '#size' => 3,
    '#default_value' => $quiz->number_of_random_questions,
    '#description' => t('The number of randomly selected questions that should be assigned to this quiz.'),
    '#weight' => 1,
  );

  $form['filtered_question_list_random']['random_box'] = array(
    '#type' => 'fieldset',
    '#title' => '<strong>' . t('Find and add a question') . '</strong>',
    '#description' => t('Begin typing a question title or keyword. Suggestions will be presented based on your typing. ') .
       '<strong>' . t('You must choose one of the suggested questions.') . '</strong> ' .
       t('To add a new question, use the "Create additional questions" section above.'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
    '#weight' => -1,
  );

  $form['filtered_question_list_random']['random_box']['random_autocomplete'] = array(
    '#type' => 'textfield',
    '#default_value' => '',
    '#size' => 60,
    '#maxlength' => 256,
    '#required' => FALSE,
    '#weight' => -2,
    '#autocomplete_path' => 'admin/quiz/listquestions',
  );
  $form['filtered_question_list_random']['random_box']['add_to_random'] = array(
    '#type' => 'submit',
    '#value' => t('Add to quiz'),
    '#submit' => array('quiz_add_question_submit'),
    '#weight' => -1,
    '#ahah' => array(
      'path' => 'admin/quiz/newquestion',
      'wrapper' => 'questions-always-target',
      'progress' => array('type' => 'bar', 'message' => t('Adding question...')),
    ),
  );

  $terms =  _quiz_taxonomy_select($quiz->tid);
  if (!empty($terms) && function_exists('taxonomy_get_vocabularies')) {
    $form['filtered_question_list_random']['random_term_id'] = array(
      '#type' => 'select',
      '#title' => t('Terms'),
      '#size' => 1,
      '#options' => _quiz_taxonomy_select($quiz->tid),
      '#default_value' => $quiz->tid,
      '#description' => t('Randomly select from questions with this term, or choose from the random question pool below'),
      '#weight' => 2,
    );
  }
  return $form;
}

function _quiz_question_browser_form(&$questions, $form_state, $quiz, $freq) {
  $form = array();
  $question_types = _quiz_get_question_types();
  if (!is_array($question_types) || count($question_types) == 0) {
    return $form['no_questions'] = array(
      '#type' => 'markup',
      '#value' => t('No question types are enabled'),
    );
  }
  $form[$freq .'_browser'] = array(
    '#type' => 'fieldset',
    '#title' => t('Browse for questions to add'),
    '#description' => t('Mark all the questions you want to add.') .' '
    .t('You can filter questions by using the textfields and select boxes.') .' '
    .t('You can sort by pressing the table headers.'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
    '#tree' => TRUE,
    '#theme' => 'quiz_question_browser',
  );
  $browser = &$form[$freq .'_browser'];
  
  $type_options = array(
    t('No filter'),
  );
  foreach (array_keys($question_types) as $type) {
    $type_options[$type] = $type;
  }
  $changedOptions = array(
    t('No filter'),
    t('Today'),
    t('Yesterday'),
    t('Two days ago'),
    t('This week'),
    t('Last week'),
    t('This month'),
    t('Last month'),
    t('This year'),
    t('Last year'),
    t('Long ago'),
  );
  $now = time();
  $one_day = 86400;
  $one_week = 604800;
  $timestamp_today = mktime(0,0,0,date('n'),date('j'),date('Y'));
  $weekday = date('N', $now);
  $timestamp_week = $timestamp_today - ((int)$weekday-1) * $one_day;
  
  $timestamp_month = mktime(0,0,0,date('n'),1,date('Y'));
  $timestamp_last_month = mktime(0,0,0,date('n', $timestamp_month - 1),1,date('Y', $timestamp_month - 1));
  $timestamp_year = mktime(0,0,0,1,1,date('Y'));
  $timestamp_last_year = mktime(0,0,0,1,1,date('Y', $timestamp_year - 1));
  $changedTimestamps = array(
    NULL,
    array($timestamp_today, $now),
    array($timestamp_today - $one_day * 1, $timestamp_today),
    array($timestamp_today - $one_day * 2, $timestamp_today - $one_day * 1),
    array($timestamp_week, $now),
    array($timestamp_week - $one_week, $timestamp_week),
    array($timestamp_month, $now),
    array($timestamp_last_month, $timestamp_month),
    array($timestamp_year, $now),
    array($timestamp_last_year, $timestamp_year),
    array(0, $timestamp_last_year),
  );
  foreach ($changedTimestamps as $key => &$val) {
    if ($val == NULL) continue;
    $val['sql'] = ' AND changed > '. $val[0] .' AND changed < '. $val[1]; 
  }
  //Build filter part of form:
  $browser['ahah_target'] = array(
    '#type' => 'markup',
    '#value' => '<DIV ID = "quiz-browser-ahah-target"></DIV>'
  );
  $browser['ahah_target_all'] = array(
    '#type' => 'markup',
    '#value' => '<DIV ID = "quiz-browser-all-ahah-target">'
  );
  $browser['add_to_get'] = array(
    '#type' => 'hidden',
    '#default_value' => '',
  );
  $browser['questions_to_add'] = array(
    '#type' => 'hidden',
    '#default_value' => '',
  );
  $browser['filters'] = array();
  $filters = &$browser['filters'];
  $filters['all'] = array(
    '#type' => 'checkbox',
  );
  $pre = 'quiz_question_browser_';
  $filters['title'] = array(
    '#type' => 'textfield',
    '#size' => 20,
    '#default_value' => $_SESSION[$pre .'title'],
    '#ahah' => array(
      'path' => 'admin/quiz/browser/ahah',
      'wrapper' => 'quiz-browser-ahah-target',
      'method' => 'replace',
      'event' => 'doneTyping',
    ),
  );
  $filters['type'] = array(
    '#type' => 'select',
    '#options' => $type_options,
    '#default_value' => $_SESSION[$pre .'type'],
    '#ahah' => array(
      'path' => 'admin/quiz/browser/ahah',
      'wrapper' => 'quiz-browser-ahah-target',
      'method' => 'replace',
    ),
  );
  $filters['changed'] = array(
    '#type' => 'select',
    '#options' => $changedOptions,
    '#default_value' => $_SESSION[$pre .'changed'],
    '#ahah' => array(
      'path' => 'admin/quiz/browser/ahah',
      'wrapper' => 'quiz-browser-ahah-target',
      'method' => 'replace',
    ),
  );
  $filters['name'] = array(
    '#type' => 'textfield',
    '#size' => 10,
    '#default_value' => $_SESSION[$pre .'name'],
    '#ahah' => array(
      'path' => 'admin/quiz/browser/ahah',
      'wrapper' => 'quiz-browser-ahah-target',
      'method' => 'replace',
      'event' => 'doneTyping',
    ),
  );
  $filter_array = array($quiz->nid, $quiz->vid);
  $filtering = '';
  if (drupal_strlen($_SESSION[$pre .'title']) > 0) {
    $filtering .= ' AND n.title LIKE \'%s%%\'';
    $filter_array[] = $_SESSION[$pre .'title'];
  }
  if (drupal_strlen($_SESSION[$pre .'name']) > 0) {
    $filtering .= ' AND u.name LIKE \'%s%%\'';
    $filter_array[] = $_SESSION[$pre .'name'];
  }
  if (isset($_SESSION[$pre .'type']) && ($_SESSION[$pre .'type'] !== '0')) {
    $filtering .= ' AND n.type = \'%s\'';
    $filter_array[] = $_SESSION[$pre .'type'];
  }
  if (isset($_SESSION[$pre .'changed'])) {
    $filtering .= $changedTimestamps[$_SESSION[$pre .'changed']]['sql'];
  }
  // Get data:
  $question_type_list = implode('\', \'', array_keys($question_types));
  $pager_num = 0;
  _quiz_add_to_get($form_state['values']['always_browser']['add_to_get']);
  $browser['#header'] = array(
    NULL, 
    array('data' => t('Title'), 'field' => 'n.title', 'class' => 'quiz-browser-header-title'),
    array('data' => t('Type'), 'field' => 'n.type', 'class' => 'quiz-browser-header-type'),
    array('data' => t('Changed'), 'field' => 'n.changed', 'sort' => 'desc', 'class' => 'quiz-browser-header-changed'),
    array('data' => t('Username'), 'field' => 'u.name', 'class' => 'quiz-browser-header-name'),
  );
  $sql = 'SELECT n.nid, n.type, n.vid, n.title, n.changed, u.name 
  		  FROM {node} n
  		  JOIN {users} u
  		  ON n.uid = u.uid
  		  WHERE n.type IN (\''. $question_type_list .'\') AND n.nid NOT IN (
  		    SELECT child_nid
  		    FROM {quiz_node_relationship}
  		    WHERE parent_nid = %d AND parent_vid = %d
  		  )'. $filtering . tablesort_sql($browser['#header']);
  $res = pager_query(db_rewrite_sql($sql), 20, $pager_num, NULL, $filter_array);

  // build data part of form
  $options = array();
  while ($res_o = db_fetch_object($res)) {
    $id = $freq .'-'. $res_o->nid .'-'. $res_o->vid;
    $questions[] = $id;
    $options[$id] = check_plain($res_o->title);
    $browser['changed'][$id]['#value'] = format_date($res_o->changed, 'small');
    $browser['types'][$id]['#value'] = $res_o->type;
    $browser['names'][$id]['#value'] = check_plain($res_o->name);
  }
  $browser['titles'] = array(
    '#title' => t('Titles'),
    '#type' => 'checkboxes',
    '#options' => $options,
    '#attributes' => array('class' => 'quiz-browser-checkbox'),
    '#default_value' => explode(',', $form_state['values']['always_browser']['questions_to_add']),
  );
  $browser['pager'] = array(
    '#value' => '<DIV ID ="quiz-question-browser-pager">'. theme('pager', NULL, 20) .'</DIV>',
  );
  $browser['ahah_target_all_end'] = array(
    '#type' => 'markup',
    '#value' => '</DIV>'
  );
  return $form;
}

/**
 * Validate that the supplied questions are real.
 */
function quiz_questions_form_validate($form, $form_state) {
  $question_types = array_keys(_quiz_get_question_types());
  $removed = explode(',', $form_state['values']['remove_from_quiz']);
  $placeholders = db_placeholders($question_types, 'varchar');
  $sql = 'SELECT COUNT(nid) FROM {node} WHERE type IN (' . $placeholders . ') AND nid = %d';
  $already_checked = array();

  $weight_map = $form_state['values']['weights'];
  if (empty($weight_map)) {
    form_set_error('none', 'No questions were included.');
    return;
  }
  foreach ($weight_map as $id => $weight) {
      if (in_array($id, $removed)) {
      // Ignore items that are going to be removed anyway.
      continue;
    }
    list($state, $nid) = explode('-', $id, 2);
    $params = $question_types; // Copy array.
    $params[] = $nid;

    if (db_result(db_query($sql, $params)) == 0) {
      form_set_error('none', 'One of the supplied questions was invalid. It has been removed from the quiz.');
      unset($form_state['values']['weights'][$id]);
    }
    elseif (in_array($nid, $already_checked)) {
      form_set_error('none', 'A duplicate question has been removed. You can only ask a question once per quiz.');
      unset($form_state['values']['weights'][$id]);
    }
    else {
      $already_checked[] = $nid;
    }
  }

  $max_scores = $form_state['values']['max_scores'];
  foreach ($max_scores as $id => $max_score) {
    if (in_array($id, $removed)) {
      // Ignore items that are going to be removed anyway.
      continue;
    }
    if (!is_numeric($max_score)) {
      form_set_error("max_scores][$id", t('Max score needs to be numeric'));
    }
    elseif ($max_score < 0) form_set_error("max_scores][$id", t('Max score needs to be positive or zero'));
  }
}

/**
 * Update a quiz or qcollection set of items with new weights and membership
 * @param $quiz
 * @param $weight_map
 * @param $removed_set
 * @param $is_new_revision
 * @return array set of questions after updating
 */
function _quiz_update_items($quiz, $weight_map, $max_scores, $removed_set, $is_new_revision, $refreshes, $hiddens) {
    // Get quiz questions that will always be on the quiz:
  $questions = array();
  foreach ($weight_map as $id => $weight) {
    if (in_array($id, $removed_set)) {
      // Skip items that should be removed.
      continue;
    }
    if ($hiddens[$id] == 1) continue;
    list($state, $nid, $vid) = explode('-', $id, 3);
    $nid = (int)$nid;
    $vid = (int)$vid;
    $question['nid'] = $nid;
    $question['vid'] = $vid;
    $question['state'] = ($state == 'always' ? QUESTION_ALWAYS : QUESTION_RANDOM);
    $question['weight'] = $weight;
    $question['max_score'] = $max_scores[$id];
    $question['refresh'] = (isset($refreshes[$id]) && $refreshes[$id] == 1);

    // Add item as an object in the questions array.
    $questions[] = (object)$question;
  }

  // Save questions.
  quiz_set_questions($quiz, $questions, $is_new_revision);

  return $questions;
}
/**
 * Adds questions to the quiz_questions_form using ahah.
 *
 * @param $form
 *   Standard form definition array
 * @param $form_state
 *   Standard form state...
 */
function quiz_add_question_submit($form, &$form_state) {
  if (!empty($form_state['post']['always_autocomplete'])) {
    $new_question = $form_state['post']['always_autocomplete'];
    $frequency = 'always';
  }
  elseif (!empty($form_state['post']['random_autocomplete'])) {
    $new_question = $form_state['post']['random_autocomplete'];
    $frequency = 'random';
  }
  $form_state['new_questions'][] = $new_question;
  $form_state['rebuild'] = TRUE;
}
 /**
  * Submit function for quiz_questions.
  *
  * Updates from the "manage questions" tab.
  *
  * @param $form_id
  *  A string containing the form id.
  * @param $values
  *  Array containing the form values.
  */
function quiz_questions_form_submit($form, &$form_state) {
  // FIXME This is ugly and should be fixed.
  $quiz = node_load(arg(1));
  if (module_exists('quiz_question')) quiz_question_refresh_latest_quizzes($quiz->nid);
  $is_new_revision = (bool) $form_state['values']['new_revision'];
  
  _quiz_question_browser_submit($form, $form_state);
  
  $removed = explode(',', $form_state['values']['remove_from_quiz']);
  $weight_map = $form_state['values']['weights'];
  $max_scores = $form_state['values']['max_scores'];
  $refreshes = $form_state['values']['revision'];
  $hiddens = $form_state['values']['hiddens'];
  $questions = _quiz_update_items($quiz, $weight_map, $max_scores, $removed, $is_new_revision, $refreshes, $hiddens);

  $num_random = $form_state['values']['number_of_random_questions'];
  $term_id = isset($form_state['values']['random_term_id']) ? $form_state['values']['random_term_id'] : 0;

  // If using random questions and no term ID is specified, make sure we have enough.
  if (empty($term_id)) {
   $assigned_random = 0;

   foreach ($questions as $question) {
     if ($question->state == QUESTION_RANDOM) {
       ++$assigned_random;
     }
   }

   // Adjust number of random questions downward to match number of selected questions..
   if ($num_random > $assigned_random) {
     $num_random = $assigned_random;
     drupal_set_message(t('The number of random questions for this @quiz have been lowered to %anum to match the number of questions you assigned.', array('@quiz' => QUIZ_NAME, '%anum' => $assigned_random), 'warning'));
   }
  }
  else {
    // Warn user if not enough questions available with this term_id.
    $available_random = count(_quiz_get_random_taxonomy_question_ids($term_id, $num_random));
    if ($num_random > $available_random) {
      $num_random = $available_random;
      drupal_set_message(t('There are currently not enough questions assigned to this term (@random). Please lower the number of random quetions or assign more questions to this taxonomy term before taking this @quiz.', array('@random' => $available_random, '@quiz' => QUIZ_NAME)), 'error');
    }
  }

  $success = true; // since we got this far
  if ($quiz->type == 'quiz') {
    // Update the quiz node properties.
    $sql = "UPDATE {quiz_node_properties} SET number_of_random_questions = %d, tid = %d WHERE vid = %d AND nid = %d";
    $success = db_query($sql, $num_random, $term_id, $quiz->vid, $quiz->nid);
  }

  if ($success) {
    drupal_set_message(t('Questions updated successfully.'));
  } else {
    // FIXME output the right type name for quiz OR qcollection
    drupal_set_message(t('There was an error updating the @quiz.', array('@quiz' => QUIZ_NAME)), 'error');
  }
}

function _quiz_question_browser_submit($form, &$form_state) {
  //Find the biggest weight:
  $next_weight = max($form_state['values']['weights']);
  $filters = $form_state['values']['always_browser']['filters'];
  $pre = 'quiz_question_browser_';
  _quiz_browser_store_filters($filters);
  //TODO: Consider random and always separately?
  if (is_array($form_state['values']['always_browser']['titles'])) {
    foreach ($form_state['values']['always_browser']['titles'] as $id) {
      if ($id !== 0) {
        if($form_state['values']['hiddens'][$id] == 0) continue;
        $matches = array();
        preg_match('/-([0-9]+)-([0-9]+)/', $id, $matches);
        $nid = $matches[1];
        $vid = $matches[2];
        $form_state['values']['weights'][$id] = ++$next_weight;
        $form_state['values']['max_scores'][$id] = quiz_question_get_max_score($nid, $vid);
        $form_state['values']['hiddens'][$id] = 0;
      }
    }
  }
}

function _quiz_browser_store_filters($filters) {
  $pre = 'quiz_question_browser_';
  $_SESSION[$pre .'title'] = $filters['title'];
  $_SESSION[$pre .'type'] = $filters['type'];
  $_SESSION[$pre .'changed'] = $filters['changed'];
  $_SESSION[$pre .'name'] = $filters['name'];
}
/**
 * Autocompletion function for quiz questions.
 */
function quiz_admin_list_questions_ac($vid, $string = '') {
  $type_map = _quiz_get_question_types();
  $values = array_keys($type_map);
  $placeholder = db_placeholders($values, 'varchar');
  $query = strtolower($string);

  $sql = "SELECT n.nid, n.title, n.type
    FROM {node} n
    INNER JOIN {node_revisions} r ON n.vid = r.vid
    WHERE n.type IN ($placeholder)
      AND n.status = 1
      AND (LOWER(n.title) LIKE '%%%s%%' OR LOWER(r.body) LIKE '%%%s%%')";

  array_push($values, $query, $query);
  $result = db_query_range(db_rewrite_sql($sql), $values, 0, 20);

  $matches = array();
  while ($node = db_fetch_object($result)) {
    $type = $type_map[$node->type]['name'];
    $key = sprintf('%s [type:%s, id:%d]', check_plain($node->title), check_plain($type), $node->nid);
    $matches[$key] = $key;
  }

  drupal_json($matches);
}

/**
 * AHAH form completion for adding new questions.
 */
function quiz_admin_add_question_ahah() {
  $form_state = array('storage' => NULL, 'submitted' => FALSE);
  $form_id = $_POST['form_build_id'];
  // Make sure the form exists
  if (!($form = form_get_cache($form_id, $form_state))) {
    form_set_error('form_token', t("Validation error, please try again. If this error persists, please contact the site administrator. $form_id"));
    $output = theme('status_messages');
    print drupal_to_js(array('status' => TRUE, 'data' => $output));
    exit();
  }
  $args = $form['#parameters'];
  $form_id = array_shift($args);

  // We will run some of the submit handlers so we need to disable redirecting.
  $form['#redirect'] = FALSE;

  // We need to process the form, prepare for that by setting a few internals
  // variables.
  $form['#post'] = $_POST;
  $form['#programmed'] = FALSE;
  $form_state['post'] = $_POST;

  // Find the new question:
  if (!empty($form_state['post']['always_autocomplete'])) {
    $new_question = $form_state['post']['always_autocomplete'];
    $frequency = 'always';
  }
  elseif (!empty($form_state['post']['random_autocomplete'])) {
    drupal_set_message('Adding random question', 'status');
    $new_question = $form_state['post']['random_autocomplete'];
    $frequency = 'random';
  }
  else {
    form_set_error('form_token', t('No question was found.'));
    $output = theme('status_messages');
    print drupal_to_js(array('status' => TRUE, 'data' => $output));
    exit();
  }
  $id = _quiz_get_question_id($new_question, $frequency);

  _quiz_skip_validation($form);

  // Build, validate and submit the form.
  drupal_process_form($form_id, $form, $form_state);

  // Get a new form with the new question attached
  $form = drupal_rebuild_form($form_id, $form_state, $args, $form_build_id);

  // Get the filtered question list with the new question.
  unset($form['filtered_question_list_'.$frequency]['always_browse']);
  $sub_form_html = drupal_render($form['filtered_question_list_'.$frequency]);

  // The new question is the last row in the table...
  $q_row_html = _quiz_get_last_table_rows($sub_form_html);

  // Send status messages, new build id for the form and the new question row to the javascript:
  $output = theme('status_messages');
  $output .= '<script>var newBuildId = '.drupal_to_js($form['#build_id']).';';
  $output .= ' var qRowTotal = '.drupal_to_js($q_row_html).';';
  $output .= ' Quiz.addQuestion("' . $frequency . '", qRowTotal);</script>';
  drupal_json(array('status' => TRUE, 'data' => $output));
}

/**
 * AHAH form completion for adding new questions.
 */
function quiz_browser_ahah() {
  $form_state = array('storage' => NULL, 'submitted' => FALSE);
  $replace_all = drupal_strlen($_POST['always_browser']['add_to_get']) > 0;
  $form_id = $_POST['form_build_id'];
  // Make sure the form exists
  if (!($form = form_get_cache($form_id, $form_state))) {
    form_set_error('form_token', t("Validation error, please try again. If this error persists, please contact the site administrator. $form_id"));
    $output = theme('status_messages');
    print drupal_to_js(array('status' => TRUE, 'data' => $output));
    exit();
  }
  $args = $form['#parameters'];
  $form_id = array_shift($args);

  // We will run some of the submit handlers so we need to disable redirecting.
  $form['#redirect'] = FALSE;

  // We need to process the form, prepare for that by setting a few internals
  // variables.
  $form['#post'] = $_POST;
  $form['#programmed'] = FALSE;
  $form_state['post'] = $_POST;
  foreach ($_POST as $key => $value) {
    unset($_REQUEST[$key]);
  }
  $_POST = array();
  _quiz_skip_validation($form);
  
  _quiz_browser_store_filters($form_state['post']['always_browser']['filters']);
  
  // Build, validate and submit the form.
  drupal_process_form($form_id, $form, $form_state);
  // Get a new form with the new question attached
  $form = drupal_rebuild_form($form_id, $form_state, $args, $form_build_id);
  $extra_output = '';
  
  $output = '<script>var quizNewBuildId = '. drupal_to_js($form['#build_id']) .';';
  $b_form = $form['filtered_question_list_always']['always_browse']['always_browser'];
  unset($form['filtered_question_list_always']['always_browse']); 
  foreach ($form['filtered_question_list_always']['hiddens'] as $key => $value) {
    if (isset($value['#value']) && $value['#value'] == 0) unset($form['filtered_question_list_always']['titles'][$key]);
  }
  $num_hiddens = 0;
  foreach ($form['filtered_question_list_always']['titles'] as $key => $value) {
    if (preg_match('/^always/', $key)) $num_hiddens++;
  }
  $sub_form_html = drupal_render($form['filtered_question_list_always']);
  $q_rows_html = _quiz_get_last_table_rows($sub_form_html, $num_hiddens);
  $output .= ' var qRowsHidden = '.drupal_to_js($q_rows_html).';';
  if($replace_all) {
    unset($b_form['#type']);
    unset($b_form['#value']);
    
    $rendered_browser = drupal_render($b_form);
    $output .= ' var renderedBrowser = '. drupal_to_js(theme('status_messages') . $rendered_browser) .';';
    $output .= ' Quiz.replaceBrowser(renderedBrowser, quizNewBuildId);';
    $output .= ' Quiz.addQuestions("always", qRowsHidden);</script>';
  }
  else {
    // Send status messages, new build id for the form and the new question row to the javascript:
    $b_rows = _quiz_get_browser_content(drupal_render($b_form));
    $b_pager = drupal_render($b_form['pager']);
    
    $output .= ' var quizBrowserRows = '. drupal_to_js($b_rows) .';';
    $output .= ' var quizBrowserPager = '. drupal_to_js($b_pager) .';';
    $output .= ' Quiz.addBrowserRows(quizBrowserRows, quizNewBuildId, quizBrowserPager);';
    $output .= ' Quiz.addQuestions("always", qRowsHidden);</script>';
    $output .= ' '. theme('status_messages');
  }
  drupal_json(array('status' => TRUE, 'data' => $output));
}
// Quiz Admin Settings

/**
 * Implementation of hook_settings().
 *
 * This builds the main settings form for the quiz module.
 */
function quiz_admin_settings() {
  $form = array();

  $form['quiz_global_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Global Configuration'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#description' => t('Control aspects of the Quiz module\'s display'),
  );

  $form['quiz_global_settings']['quiz_default_close'] = array(
    '#type' => 'textfield',
    '#title' => t('Default number of days before a @quiz is closed', array('@quiz' => QUIZ_NAME)),
    '#default_value' => variable_get('quiz_default_close', 30),
    '#description' => t('Supply a number of days to calculate the default close date for new quizzes.'),
  );

  $form['quiz_global_settings']['quiz_default_pass_rate'] = array(
    '#type' => 'textfield',
    '#title' => t('Default percentage needed to pass a @quiz', array('@quiz' => QUIZ_NAME)),
    '#default_value' => variable_get('quiz_default_pass_rate', 75),
    '#description' => t('Supply a number between 1 and 100 to set as the default percentage correct needed to pass a quiz. Set to 0 if you want to ignore pass/fail summary information by default.'),
  );

  $form['quiz_global_settings']['quiz_use_passfail'] = array(
    '#type' => 'checkbox',
    '#title' => t('Allow quiz creators to set a pass/fail option when creating a @quiz.', array('@quiz' => strtolower(QUIZ_NAME))),
    '#default_value' => variable_get('quiz_use_passfail', 1),
    '#description' => t('Check this to display the pass/fail options in the @quiz form. If you want to prohibit other quiz creators from changing the default pass/fail percentage set below, uncheck this option.', array('@quiz' => QUIZ_NAME)),
  );

  $form['quiz_global_settings']['quiz_max_result_options'] = array(
    '#type' => 'textfield',
    '#title' => t('Maximum Result Options'),
    '#description' => t('Set the maximum number of result options (categorizations for scoring a quiz).'),
    '#default_value' => variable_get('quiz_max_result_options', 5),
    '#size' => 2,
    '#maxlength' => 2,
    '#required' => FALSE,
  );

  // Added for support of actions and allowing the user to filter the actions dropdown by a value.
  $form['quiz_global_settings']['quiz_action_type'] = array(
    '#type' => 'textfield',
    '#title' => t('Default actions type'),
    '#size' => 25,
    '#default_value' => variable_get('quiz_action_type', 'all'),
    '#description' => t('Filter the actions dropdown by a specific type.'),
  );

  $form['quiz_global_settings']['quiz_remove_partial_quiz_record'] = array(
    '#type' => 'select',
    '#title' => t('Remove Incomplete Quiz Records (older than)'),
    '#options' => quiz_remove_partial_quiz_record_value(),
    '#description' => t('Number of days that you like to keep the incomplete quiz records'),
    '#default_value' => variable_get('quiz_remove_partial_quiz_record', quiz_remove_partial_quiz_record_value()),
  );

  $form['quiz_global_settings']['quiz_autotitle_length'] = array(
    '#type' => 'textfield',
    '#title' => t('Length of automatically set question titles'),
    '#size' => 3,
    '#maxlength' => 3,
    '#description' => t('Integer between 0 and 128. If the question creator doesn\'t set a question title the system will make a title automatically. Here you can deside how long the autotitle can be.'),
    '#default_value' => variable_get('quiz_autotitle_length', 50),
  );

  $target = array(
    'attributes' => array(
      'target' => '_blank'
    ),
  );

  $links = array(
    '!views' => l(t('Views'), 'http://drupal.org/project/views', $target),
    '!cck' => l(t('CCK'), 'http://drupal.org/project/cck', $target),
    '!jquery_countdown' => l(t('JQuery Countdown'), 'http://drupal.org/project/jquery_countdown', $target),
    '!userpoints' => l(t('UserPoints'), 'http://drupal.org/project/userpoints', $target),
    '@quiz' => QUIZ_NAME,
  );

  $form['quiz_addons'] = array(
    '#type' => 'fieldset',
    '#title' => t('Addons Configuration'),
    '#description' => t('Quiz module can integrate with other d.o modules like !views, !cck, !userpoints, !jquery_countdown. Here you can configure the way quiz module integrates to other modules. Disabled checkbox indicates modules are not enabled/installed', $links),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  $form['quiz_addons']['quiz_has_userpoints'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enable UserPoints Module Integration'),
    '#default_value' => variable_get('quiz_has_userpoints', 0),
    '#description' => t('!userpoints an *optional* module for quiz, provides way for users to gain or lose points for performing certain actions on your site like attending @quiz.  you will need to install the !userpoints module to use this feature.', $links),
    '#disabled' => !module_exists('userpoints'),
  );

  $form['quiz_addons']['quiz_has_timer'] = array(
    '#type' => 'checkbox',
    '#title' => t('Display Timer for Timed Quiz'),
    '#default_value' => variable_get('quiz_has_timer', 0),
    '#description' => t('!jquery_countdown an *optional* module for quiz used for the timer to be displayed on the user\'s page, you will need to install the !jquery_countdown module. Without this timer, the user will not know how long he or she has to complete the @quiz', $links),
    '#disabled' => !function_exists('jquery_countdown_add'),
  );

  $form['quiz_look_feel'] = array(
    '#type' => 'fieldset',
    '#title' => t('Look and Feel Settings'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#description' => t('Control aspects of the Quiz module\'s display'),
  );

  $form['quiz_look_feel']['quiz_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Display name'),
    '#default_value' => QUIZ_NAME,
    '#description' => t('Change the name of the quiz type. Do you call it <em>test</em> or <em>assessment</em> instead? Change the display name of the module to something else. Currently, it is called @quiz. By default, it is called <em>Quiz</em>.',
      array('@quiz' => QUIZ_NAME)),
    '#required' => TRUE,
  );

  $form['quiz_look_feel']['quiz_show_allowed_times'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show allowed times'),
    '#description' => t('When a user begins a new quiz, show the user the number of times they may take the test, and how many times they have already taken the test.'),
    '#default_value' => variable_get('quiz_show_allowed_times', TRUE),

  );

  $form['quiz_email_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Email Settings'),
    '#description' => t('Allows to send results to quiz author/ attendee via e-mail and configuration e-mail subject and body.'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['quiz_email_settings']['quiz_email_results'] = array(
    '#type' => 'checkbox',
    '#title' => t('E-mail users their quiz results at the end of a quiz.', array('@quiz' => strtolower(QUIZ_NAME))),
    '#default_value' => variable_get('quiz_email_results', 1),
    '#description' => t('Check this to send users quiz results over E-mail at the end of quiz.')
  );
  $form['quiz_email_settings']['quiz_results_to_quiz_author'] = array(
    '#type' => 'checkbox',
    '#title' => t('E-mail all users results to Quiz Author.', array('@quiz' => strtolower(QUIZ_NAME))),
    '#default_value' => variable_get('quiz_results_to_quiz_author', 1),
    '#description' => t('Check this to send users quiz results over E-mail to quiz author. (<em> it works only when the attendee is an anonymous user</em>).'),
  );
  $form['quiz_email_settings']['quiz_email_results_subject'] = array(
    '#type' => 'textfield',
    '#title' => t('Configure Email subject'),
    '#description' => t('This format will be used to send Email to notify results at the end of quiz.'),
    '#default_value' => variable_get('quiz_email_results_subject', quiz_email_results_format('subject')),
  );

  $form['quiz_email_settings']['quiz_email_results_body'] = array(
    '#type' => 'textarea',
    '#title' => t('Configure Email format'),
    '#description' => t('This format will be used to send Email to notify results at the end of quiz. !title !sitename !username !date  !desc !correct !total !percentage !url are placeholder.'),
    '#default_value' => variable_get('quiz_email_results_body', quiz_email_results_format('body')),
  );

  $form['#validate'][] = 'quiz_settings_form_validate';
  return system_settings_form($form);
}

/**
  * Validation of the Form Settings form.
  *
  * Checks the values for the form administration form for quiz settings.
  */
function quiz_settings_form_validate($form, &$form_state) {
  if (!is_numeric($form_state['values']['quiz_default_close']) || $form_state['values']['quiz_default_close'] <= 0) {
    form_set_error('quiz_default_close', t('The default number of days before a quiz is closed must be a number greater than 0.'));
  }
  if (!is_numeric($form_state['values']['quiz_default_pass_rate'])) {
    form_set_error('quiz_default_pass_rate', t('The pass rate value must be a number between 0% and 100%.'));
  }
  if (!is_numeric($form_state['values']['quiz_autotitle_length'])) {
    form_set_error('quiz_autotitle_length', t('The autotitle length value must be an integer between 0 and 128.'));
  }
  if ($form_state['values']['quiz_default_pass_rate'] > 100) {
    form_set_error('quiz_default_pass_rate', t('The pass rate value must not be more than 100%.'));
  }
  if ($form_state['values']['quiz_default_pass_rate'] < 0) {
    form_set_error('quiz_default_pass_rate', t('The pass rate value must not be less than 0%.'));
  }
  if ($form_state['values']['quiz_autotitle_length'] > 128) {
    form_set_error('quiz_autotitle_length', t('The autotitle length value must not be bigger than 128.'));
  }
  if ($form_state['values']['quiz_autotitle_length'] < 0) {
    form_set_error('quiz_autotitle_length', t('The autotitle length value must not be smaller than 0.'));
  }
}

 // DELETE QUIZ RESULTS

 /**
  * Delete Result.
  */
function quiz_admin_result_delete() {
  return drupal_get_form('quiz_admin_result_delete_form');
}

/**
 * Creates a form used for deleting a set of quiz results.
 */
function quiz_admin_result_delete_form() {
  $form['del_rid'] = array('#type' => 'hidden', '#value' => arg(2));
  return confirm_form($form,
    t('Are you sure you want to delete this @quiz result?', array('@quiz' => QUIZ_NAME)),
    'admin/quiz/results',
    t('This action cannot be undone.'),
    t('Delete'),
    t('Cancel')
  );
}

function quiz_admin_result_delete_form_submit($form, &$form_state) {
  $nid = db_result(db_query("SELECT nid FROM {quiz_node_results} WHERE result_id = %d",$form_state['values']['del_rid']));
  db_query("DELETE FROM {quiz_node_results} WHERE result_id = %d", $form_state['values']['del_rid']);
  db_query("DELETE FROM {quiz_node_results_answers} WHERE result_id = %d", $form_state['values']['del_rid']);
  drupal_set_message(t('Result has been Deleted.'));
  $form_state['redirect'] = 'admin/quiz/'. intval($nid) .'/view';
  $form_state['nid'] = $nid;
}



// THEME FUNCTIONS
// Remember to updated quiz_theme() in quiz.module

/**
 * Theme the admin quizzes table.
 *
 * @param $results
 *  As returned by _quiz_get_quizzes().
 *
 * @ingroup themeable
 */
function theme_quiz_admin_quizzes($results) {
  $output = '';
  $rows = array();

  while (list($key, $result) = each($results)) {
    $rows[] = array(
      l($result['title'], 'admin/quiz/'. $result['nid'] .'/view'),
      check_plain($result['name']),
      format_date($result['created'], 'small'),
    );
  }

  $header = array(
    t('@quiz title', array('@quiz' => QUIZ_NAME)),
    t('Created by'),
    t('Created on')
  );
  $output = (!empty($rows)) ? theme('table', $header, $rows) : '<p>' . t('No @quizzes found.', array('@quiz' => QUIZ_NAME)) . '</p>';
  return $output;
}


/**
 * Theme the admin results table.
 *
 * @param $results
 *  As returned by _quiz_get_results().
 *
 * @ingroup themeable
 */
function theme_quiz_admin($results) {
  $output = '';
  $quiz = current($results);
  drupal_set_title(t('@current Results', array('@current' => check_plain($quiz['title']))));
  // generates <img src="foo.bar.png" /> tag
  $path_to_module_quiz = drupal_get_path('module', 'quiz');
  $png = array(
    'view' => theme('image',  $path_to_module_quiz . '/images/view.png', t('View User Answers and Correct Answers'), t('View User Answers and Correct Answers')),
    'delete' => theme('image', $path_to_module_quiz . '/images/delete.png', t('Delete this Result Record'), t('Delete this Result Record')),
    'html' => theme('image', $path_to_module_quiz . '/images/html.png', t('Export as HTML'), t('Export as HTML')),
    'xml' => theme('image', $path_to_module_quiz . '/images/xml.png', t('Export as XML'), t('Export as XML')),
    'csv' => theme('image', $path_to_module_quiz . '/images/csv.png', t('Export as CSV'), t('Export as CSV')),
  );
  if (module_exists('results_export')) {
    $export_teaser_view = array(
      'html_png' => l($png['html'], 'admin/quiz/results_export_teaser_view/'. $quiz['nid'] . '/html', array('html' => TRUE)),
      'xml_png' => l($png['xml'], 'admin/quiz/results_export_teaser_view/'. $quiz['nid'] . '/xml', array('html' => TRUE)),
      'csv_png' => l($png['csv'], 'admin/quiz/results_export_teaser_view/'. $quiz['nid'] . '/csv', array('html' => TRUE)),
    );
  }
  while (list($key, $result) = each($results)) {
    $action = array(
      'view_png' => l($png['view'], 'admin/quiz/reports/' . $result['result_id'] . '/results', array('html' => TRUE)),
      'delete_png' => l($png['delete'], 'admin/quiz/' . $result['result_id'] . '/delete', array('html' => TRUE)),
    );
    if (module_exists('results_export')) {
      $export_full_view = array(
        'html_png' => l($png['html'], 'admin/quiz/results_export_full_view/'. $result['result_id'] . '/html', array('html' => TRUE)),
        //'xml_png' => l($png['xml'], 'admin/quiz/results_export_full_view/'. $result['result_id'] . '/xml', array('html' => TRUE)),
        //'csv_png' => l($png['csv'], 'admin/quiz/results_export_full_view/'. $result['result_id'] . '/csv', array('html' => TRUE)),
      );
    }
    $rows[] = array(
      implode(' ', $action),
      check_plain($result['name']),
      $result['result_id'],
      format_date($result['time_start'], 'small'),
      ($result['time_end'] > 0) ? format_date($result['time_end'], 'small') : t('In Progress'),
      ($result['time_end'] > 0) ? quiz_get_time_taken_in_minutes($result['time_end'] - $result['time_start']) : quiz_get_time_taken_in_minutes(time() - $result['time_start']),
      ($result['time_end'] > 0) ? $result['score'] : t('--'),
      module_exists('results_export') ? implode(' ', $export_full_view) : '',
    );
  }

  $header = array(
    t('Action'),
    t('Username'),
    t('Result<br />ID'),
    t('Time <br />Started <br /> (m/d/y - h/m)'),
    t('Finished? <br /> (m/d/y - h/m)'),
    t('Time <br />Taken <br /> (min:sec)'),
    t('Score'),
    module_exists('results_export') ? t('Export') : '',
  );

  if (!empty($rows)) {
    $output .= module_exists('results_export') ? '<div id="export-table"><p>' . t('Export this Table') . '</p>'. implode(' ', $export_teaser_view) . '</div>' : '';
    $output .= theme('table', $header, $rows);
  }
  else {
    $output .= t('No @quiz results found.', array('@quiz' => QUIZ_NAME));
  }
  return $output;
}

/*
 * @param $time_in_sec
 *  Integers time in seconds
 * @return
 *  String time in min : sec format
 */
function quiz_get_time_taken_in_minutes($time_in_sec) {
  $min = intval($time_in_sec / 60);
  $sec = $time_in_sec % 60;
  return "$min : $sec";
}


/**
 * Theme the summary page for admins.
 *
 * @param $quiz
 *  The quiz node object.
 * @param $questions
 *  The questions array as defined by _quiz_get_answers.
 * @param $score
 *  Array of score information as returned by quiz_calculate_score().
 * @param $summary
 *  Filtered text of the summary.
 * @return
 *  Themed html.
 *
 * @ingroup themeable
 */
function theme_quiz_admin_summary($quiz, $questions, $score, $summary) {
  // Set the title here so themers can adjust.
  drupal_set_title(check_plain($quiz->title));

  if (!$score['is_evaluated']) {
    drupal_set_message(t('This quiz has not been scored yet.'), 'error');
  }

  // Display overall result.
  $output = '';
  $params = array('%num_correct' => $score['numeric_score'], '%question_count' => $score['possible_score']);
  $output .= '<div id="quiz_score_possible">'. t('This person got %num_correct of %question_count possible points.', $params) .'</div>'."\n";
  $output .= '<div id="quiz_score_percent">'. t('Total score: @score%', array('@score' => $score['percentage_score'])) .'</div><br />'."\n";
  $output .= '<div id="quiz_summary">'. check_markup($summary, $quiz->format) .'</div><br />'."\n";
  // Get the feedback for all questions.
  $output .= theme('quiz_feedback', $questions, TRUE, TRUE);
  return $output;
}

/**
 * Theme a question selection table, adding drag and drop support.
 */
function theme_question_selection_table($form) {
  // This is a temporary hack.
  static $table_counter = 0;
  ++$table_counter;

  //drupal_add_tabledrag('questions-order-' . $table_counter, 'order', 'sibling', 'question-order-weight', 'question-order-weight-' . $table_counter, NULL, TRUE);
  drupal_add_tabledrag('questions-order-' . $table_counter, 'order', 'sibling', 'question-order-weight-' . $table_counter, NULL, NULL, TRUE);

  $headers = array(t('Question'), t('Type'), t('Actions'), t('Update'), t('Max score'), t('Weight'));

  $rows = array();
  if (!empty($form['titles'])) {
    foreach (element_children($form['titles']) as $id) {
      $form['weights'][$id]['#attributes']['class'] = 'question-order-weight question-order-weight-' . $table_counter;

      $rows[] = _quiz_get_question_row($form, $id);
      unset($form['types'][$id], $form['view_links'][$id], $form['remove_links'][$id], $form['hiddens'][$id]);
      unset($form['max_scores'][$id], $form['revision'][$id], $form['weights'][$id], $form['titles'][$id]);
    }
  }
  $table = theme('table', $headers, $rows, array('id' => 'questions-order-' . $table_counter));
  return $table . drupal_render($form);
}

function quiz_remove_partial_quiz_record_value() {
  /*$list = array();
  $list[0] = t('Never');
  for ($i=1; $i<31; $i++) {
    $list[$i * 86400] = $i . ' ' . t('Day(s)');
  }
  for ($i=35; $i<125; $i+=5) {
    $list[$i * 86400] = $i . ' ' . t('Day(s)');
  }*/
  //return $list;
  return array(
    '0' => t('Never'),
    '86400' => t('1 Day'),
    '172800' => t('2 Days'),
    '259200' => t('3 Days'),
    '345600' => t('4 Days'),
    '432000' => t('5 Days'),
    '518400' => t('6 Days'),
    '604800' => t('7 Days'),
    '691200' => t('8 Days'),
    '777600' => t('9 Days'),
    '864000' => t('10 Days'),
    '950400' => t('11 Days'),
    '1036800' => t('12 Days'),
    '1123200' => t('13 Days'),
    '1209600' => t('14 Days'),
    '1296000' => t('15 Days'),
    '1382400' => t('16 Days'),
    '1468800' => t('17 Days'),
    '1555200' => t('18 Days'),
    '1641600' => t('19 Days'),
    '1728000' => t('20 Days'),
    '1814400' => t('21 Days'),
    '1900800' => t('22 Days'),
    '1987200' => t('23 Days'),
    '2073600' => t('24 Days'),
    '2160000' => t('25 Days'),
    '2246400' => t('26 Days'),
    '2332800' => t('27 Days'),
    '2419200' => t('28 Days'),
    '2505600' => t('29 Days'),
    '2592000' => t('30 Days'),
    '3024000' => t('35 Days'),
    '3456000' => t('40 Days'),
    '3888000' => t('45 Days'),
    '4320000' => t('50 Days'),
    '4752000' => t('55 Days'),
    '5184000' => t('60 Days'),
    '5616000' => t('65 Days'),
    '6048000' => t('70 Days'),
    '6480000' => t('75 Days'),
    '6912000' => t('80 Days'),
    '7344000' => t('85 Days'),
    '7776000' => t('90 Days'),
    '8208000' => t('95 Days'),
    '8640000' => t('100 Days'),
    '9072000' => t('105 Days'),
    '9504000' => t('110 Days'),
    '9936000' => t('115 Days'),
    '10368000' => t('120 Days'),
  );
}

/*
 * Adds inline js to automatically set the question's node title.
 */
function quiz_set_auto_title() {
  drupal_add_js('
  $(document).ready(function () {
    function quizUpdateTitle() {
      $("#edit-title").val($("#edit-body").val().substring(0, '. variable_get('quiz_autotitle_length', 50) .'));
    }
    $("#edit-body").keyup(quizUpdateTitle);
    if($("#edit-title").val().length > 0){
      $("#edit-body").unbind("keyup", quizUpdateTitle);
    }
    $("#edit-title").keyup(function() {
      $("#edit-body").unbind("keyup", quizUpdateTitle);
    });
  });
  ', 'inline');
}

/**
 * Recursive helper function to set the validated property. (Taken from the skip validation module.)
 *
 * @param &$elements
 *   The elements that are currently being processed.
 */
function _quiz_skip_validation(&$elements) {
  $elements['#validated'] = TRUE;
  foreach (element_children($elements) as $key) {
    _quiz_skip_validation($elements[$key]);
  }
}

/**
 *
 *
 * @see quiz_questions_form()
 * @see theme_question_selection_table()
 *
 * @param $sub_form
 *   Form definition array for a filtered questions list
 * @param $id
 *   Identifier used in $sub_form
 * @return table row
 *   Array defining a table row
 */
function _quiz_get_question_row($sub_form, $id) {
  $question_types = _quiz_get_question_types();
  $type = $sub_form['types'][$id]['#value'];
  $hidden_class = ($sub_form['hiddens'][$id]['#default_value'] === 1) ? ' hidden-question' : '';
  $temp_class = ($sub_form['temp_questions'][$id]['#value'] === TRUE ? ' drag-previous quiz-temp' : '');
  return array(
    'class' => 'draggable'. $hidden_class . $temp_class,
    'id' => $id,
    'data' => array(
      drupal_render($sub_form['hiddens'][$id]) . drupal_render($sub_form['titles'][$id]),
      $type,
      $sub_form['view_links'][$id]['#value'] . ' | ' . $sub_form['remove_links'][$id]['#value'],
      isset($sub_form['revision'][$id]) ? drupal_render($sub_form['revision'][$id]) : t("Up to date"),
      drupal_render($sub_form['max_scores'][$id]),
      drupal_render($sub_form['weights'][$id]),
    ),
  );
}

/**
 * Converts question description(from autocomplete field) into question id(used in form definition)
 *
 * @param $new_question
 *   Question description from autocomplete field
 * @param $frequency
 *   Random or always.
 * @return question id
 *   String used as identifier in quiz_questions_form
 */
function _quiz_get_question_id($new_question, $frequency) {
  $matches = array();
  preg_match('/id:([0-9]+)/', $new_question, $matches);
  $nid = $matches[1];
  $sql = 'SELECT vid FROM {node} n where n.nid = %d';
  $vid = db_result(db_query($sql, $nid));
  return $frequency .'-'. $nid .'-'. $vid;
}

/**
 * Finds and returns the last table row(HTML) in a table(HTML)
 *
 * @param $table
 *   HTML string with a table
 * @return last table row in the table
 */
function _quiz_get_last_table_rows($table, $num_rows = 1) {
  $matches = array();
  $num_matches = preg_match_all('/<tr.*?<\/tr>/is', $table, $matches);
  $to_return = '';
  $num_rows_to_return = ($num_matches > $num_rows) ? $num_rows : $num_matches;
  for ($i = $num_matches - 1; $i > $num_matches - 1 - $num_rows_to_return; $i--) {
    $to_return .= $matches[0][$i];
  }
  return $to_return;
}

/**
 * Finds and returns all table rows with a certain class(HTML) in a table(HTML)
 *
 * @param $table
 *   HTML string with a table
 * @return all table rows with a certain class in a table
 */
function _quiz_get_browser_content($table) {
  $matches = array();
  $n_matches = preg_match('/<tr class="quiz_question_browser_row.*<\/tr>/is', $table, $matches);
  return $matches[0];
}

function _quiz_add_to_get($to_add){
  $pre = 'quiz_question_browser_';
  parse_str($to_add, $output);
  $vars = array('sort', 'order', 'page');
  foreach ($vars as $value) {
    if (isset($_GET[$value])) continue;
    if (isset($output[$value])) {
      $_GET[$value] = $_SESSION[$pre . $value] = $output[$value];
    }
    elseif (isset($_SESSION[$pre . $value])) {
      $_GET[$value] = $_SESSION[$pre . $value];
    }
  }
} 
function _quiz_browser_count_temps($temp_string) {
  $temps = explode(',', $temp_string);
  $to_return = 0;
  foreach ($temps as $value) {
    $to_return += preg_match('/-[0-9]+-[0-9]+/', $value);
  }
  return $to_return;
}