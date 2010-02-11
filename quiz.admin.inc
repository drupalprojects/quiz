<?php
// $Id$

/**
 * Administrator interface for Quiz module.
 *
 * @file
 */


// QUIZ ADMIN

/**
 * Displays the quizzes by title with a link to the appropriate results
 * for that specific quiz.
 *
 * @return
 *  Formatted data.
 */
function quiz_admin_quizzes() {
  global $user;
  $uid = $user->uid;
  if (user_access('view any quiz results'))
    $uid = NULL;
  $results = _quiz_get_quizzes($uid);
  return theme('quiz_admin_quizzes', $results);
}

 // QUIZ RESULTS ADMIN

 /**
  * Quiz Results Admin.
  *
  * @param $quiz
  *  The quiz node
  * @param $rid
  *  The result id
  */
function quiz_admin_results($quiz, $rid) {
  $breadcrumb = drupal_get_breadcrumb();
  // Lets add the Quiz Results to the breadcrumb array.
  $breadcrumb[] = l(t('Quiz Results'), 'admin/quiz/reports/results');

  $questions = _quiz_get_answers($rid);
  $score = quiz_calculate_score($quiz, $rid);
  $summary = _quiz_get_summary_text($quiz, $score);
  
  // Lets add the quiz title to the breadcrumb array.
  $breadcrumb[] = l($quiz->title, 'admin/quiz/reports/results/'. $rid .'/'. $result->nid);
  drupal_set_breadcrumb($breadcrumb);
  return theme('quiz_admin_summary', $quiz, $questions, $score, $summary);
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
 *  The quiz node.
 * @return
 *  HTML output to create page.
 */
function quiz_questions_form($form_state, $quiz, $options) {
  // We need to add this because of the use of ajax. Without this the form api might redirect the user
  // To the page with the ahah callback...
  $form['#action'] = url('node/'. $quiz->nid .'/questions');

  $types = _quiz_get_question_types();
  _quiz_add_fields_for_creating_questions($form, $types, $quiz);

  // Display questions on this quiz.
  $form['question_list'] = array(
    '#type' => 'fieldset',
    '#title' => (isset($options['fieldset_always']['title'])) ? $options['fieldset_always']['title'] : t('Questions on this quiz'),
    '#theme' => 'question_selection_table',
    '#collapsible' => TRUE,
    '#attributes' => array('id' => 'mq-fieldset'),
    'question_status' => array('#tree' => TRUE),
  );

  // Add randomization settings if this quiz allows randomized questions
  _quiz_add_fields_for_random_quiz($form, $quiz);

  // Build up a list of questions
  $questions_to_add = array();

  // We use $form_state[post] to avoid validation failures when questions are added using AJAX
  if (isset($form_state['post']['weights'])) {
    $questions = _quiz_get_questions_from_form_state($form_state, $questions_to_add);
  }
  else {
    // We are coming in fresh and fetches the questions currently on the quiz from the database...
    $questions = _quiz_get_questions($quiz->nid, $quiz->vid, TRUE, FALSE, FALSE);
  }
  
  if (empty($questions))
    $form['question_list']['no_questions'] = array(
      '#type' => 'markup',
      '#value' => '<div id = "no-questions">'. t('There are currently no questions in this quiz. Assign existing questions by using the question browser below. You can also use the links above to create new questions.') .'</div>',
    );

  // We add the browser and allows the browser to give us information on what questions are displayed in the browser...
  $hidden_questions = array();
  $form['question_list']['browser'] = _quiz_question_browser_form($hidden_questions, $questions_to_add, $form_state, $quiz, $types);

  // We add the questions from the browser as hidden question rows in the question list. Doing this we can have
  // the question show up in the question list instantly when a question is chosen in the browser.
  _quiz_add_hidden_questions($questions, $hidden_questions, $form_state, $quiz);

  _quiz_add_questions_to_form($form, $questions, $quiz, $types);

  // Show the number of questions in the table header.
  $always_count = 0;
  foreach ($form['question_list']['stayers'] as $stayer) {
    if ($stayer['#default_value'] === 1) $always_count++;
  }
  $form['question_list']['#title'] .= ' ('. $always_count .')';

  _quiz_add_revision_checkbox($form, $quiz);

  $form['timestamp'] = array('#type' => 'hidden', '#value' => time());

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
    '#submit' => array('quiz_questions_form_submit'),
  );

  return $form;
}

/**
 * Fields for creating new questions are added to the quiz_questions_form
 *
 * @param $form
 *   FAPI form(array)
 */
function _quiz_add_fields_for_creating_questions(&$form, &$types, &$quiz) {
  // Display links to create other questions.
  $form['additional_questions'] = array(
    '#type' => 'fieldset',
    '#title' => t('Create new question'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );

  foreach ($types as $type => $info) {
    $url_type = str_replace('_', '-', $type);
    $form['additional_questions'][$type] = array(
      '#type' => 'markup',
      '#value' => '<div class="add-questions">' . l(check_plain($info['name']), 'node/add/'. $url_type .'/'. $quiz->nid .'/'. $quiz->vid, array('attributes' => array('title' => t('Go to @name administration', array('@name' => $info['name']))))) .'</div>',
    );
  }
}

/**
 * Add fields for random quiz to the quiz_questions_form
 *
 * @param $form
 *   FAPI form array
 * @param $quiz
 *   The quiz node(object)
 */
function _quiz_add_fields_for_random_quiz(&$form, $quiz) {
  if ($quiz->randomization == 2) {
    $form['question_list']['num_random_questions'] = array(
      '#type' => 'textfield',
      '#size' => 3,
      '#maxlength' => 3,
      '#weight' => -5,
      '#title' => t('Number of random questions'),
      '#description' => t('The number of questions to be randomly selected each time someone takes this quiz'),
      '#default_value' => isset($quiz->number_of_random_questions) ? $quiz->number_of_random_questions : 10,
    );
    $terms =  _quiz_taxonomy_select($quiz->tid);
    if (!empty($terms) && function_exists('taxonomy_get_vocabularies')) {
      $form['question_list']['random_term_id'] = array(
        '#type' => 'select',
        '#title' => t('Terms'),
        '#size' => 1,
        '#options' => _quiz_taxonomy_select($quiz->tid),
        '#default_value' => $quiz->tid,
        '#description' => t('Randomly select from questions with this term, or choose from the random question pool below'),
        '#weight' => -4,
      );
    }
  }
}

/**
 * Returns the questions that was in the question list when the form was submitted using ajax.
 *
 * @param $form_state
 *   FAPI form_state(array)
 * @return $questions
 *   Array of questions as objects
 */
function _quiz_get_questions_from_form_state(&$form_state, &$questions_to_add) {
  $questions = array();
  // We first store all data from the post in a temporary array.
  // Then we fetch more data for each question from the database.
  $cur_questions = array();
  $vids = array();
  foreach ($form_state['post']['weights'] as $id => $value) {
    $cur_question = new stdClass();

    // Find nid and vid
    $matches = array();
    preg_match('/([0-9]+)-([0-9]+)/', $id, $matches);
    $cur_question->nid = $matches[1];
    if (!is_numeric($matches[2])) continue;
    $vids[] = $cur_question->vid = $matches[2];
    $cur_question->max_score = $form_state['post']['max_scores'][$id];
    $cur_question->weight = $weight;
    $cur_question->staying = $form_state['post']['stayers'][$id] === '1';
    $cur_question->question_status = QUESTION_ALWAYS;
    if($cur_question->staying == TRUE) {
      $questions_to_add[] = $id;
    }
    $cur_questions[$cur_question->nid] = $cur_question;
  }

  // Fetching the rest of the data we need for each question...
  $sql = 'SELECT n.nid, n.type, n.vid AS latest_vid, r.title
          FROM {node} n
          JOIN {node_revisions} r
          ON n.nid = r.nid
          WHERE r.vid IN ('. implode(', ', $vids) .')';

  // We use db_rewrite_sql to add node access security
  $res = db_query(db_rewrite_sql($sql));
  while ($res_o = db_fetch_object($res)) {
    $cur_questions[$res_o->nid]->type = $res_o->type;
    $cur_questions[$res_o->nid]->title = check_plain($res_o->title);
    $cur_questions[$res_o->nid]->latest_vid = $res_o->latest_vid;
    $questions[] = $cur_questions[$res_o->nid];
  }
  return $questions;
}

/**
 * Adds all information about the hidden questions to the questions array
 *
 * @see quiz_questions_form
 *
 * @param $questions
 *   The questions already added to the question list(array)
 * @param $hidden_questions
 *   The questions added to the browser(array)
 * @param $form_state
 *   FAPI form_state(array)
 */
function _quiz_add_hidden_questions(&$questions, &$hidden_questions, &$form_state, &$quiz) {
  $cur_questions = array();
  $vids = array();
  foreach ($hidden_questions as $key => $id) {
    $cur_question = new stdClass();
    $matches = array();

    // Find nid and vid
    preg_match('/([0-9]+)-([0-9]+)/', $id, $matches);
    $nid = $matches[1];
    $vid = $matches[2];

    // If a question already exists in the $questions array we won't add a new one...
    $continue = FALSE;
    foreach($questions as $question) {
      if($question->vid == $vid) {
        $continue = TRUE;
        break;
      }
    }
    if (!is_numeric($nid) || !is_numeric($vid) || $continue) continue;

    $cur_question->nid = $nid;
    $vids[] = $cur_question->vid = $vid;
    $cur_question->weight = 0;
    $cur_question->question_status = ($quiz->randomization == 2) ? QUESTION_RANDOM : QUESTION_ALWAYS;
    $cur_question->staying = isset($form_state['values']['stayers'][$id]) ? $form_state['values']['stayers'][$id] === '1' : FALSE;
    $cur_questions[$cur_question->nid] = $cur_question;
  }
  if (count($vids) > 0) {
    // We fetch the rest of the information for each question and adds node access security
    $sql = 'SELECT n.nid, n.type, r.title, p.max_score, n.vid AS latest_vid
            FROM {node} n
            JOIN {node_revisions} r
            ON n.nid = r.nid
            JOIN {quiz_question_properties} p
            ON r.vid = p.vid
            WHERE r.vid IN ('. implode(', ', $vids) .')';
    $res = db_query(db_rewrite_sql($sql));
    while ($res_o = db_fetch_object($res)) {
      $cur_questions[$res_o->nid]->type = $res_o->type;
      $cur_questions[$res_o->nid]->title = $res_o->title;
      $cur_questions[$res_o->nid]->max_score = $res_o->type == 'scale' ? 0 : $res_o->max_score;
      $cur_questions[$res_o->nid]->latest_vid = $res_o->latest_vid;
      $questions[] = $cur_questions[$res_o->nid];
    }
  }
}

/**
 * Adds the questions in the $questions array to the form
 *
 * @param $form
 *   FAPI form(array)
 * @param $questions
 *   The questions to be added to the question list(array)
 * @param $quiz
 *   The quiz node(object)
 */
function _quiz_add_questions_to_form(&$form, &$questions, &$quiz, &$question_types) {
  $form['question_list']['weights'] = array('#tree' => TRUE);
  $form['question_list']['max_scores'] = array('#tree' => TRUE);
  $form['question_list']['stayers'] = array('#tree' => TRUE);
  $form['question_list']['revision'] = array('#tree' => TRUE);
  if ($quiz->randomization == 2) $form['question_list']['compulsories'] = array('#tree' => TRUE);

  $my_dest = $_GET['q'];
  foreach ($questions as $question) {
    $fieldset = 'question_list';
    $id = $question->nid . '-' . $question->vid;

    $form[$fieldset]['weights'][$id] = array(
      '#type' => 'textfield',
      '#size' => 3,
      '#maxlength' => 4,
      '#default_value' => isset($question->weight) ? $question->weight : 0,
    );
    if ($question->type != 'quiz_directions') {
    $form[$fieldset]['max_scores'][$id] = array(
      '#type' => 'textfield',
      '#size' => 2,
      '#maxlength' => 2,
      '#default_value' => isset($question->max_score) ? $question->max_score : 0,
    );
    }
    else {
      $form[$fieldset]['max_scores'][$id] = array(
      '#type' => 'markup',
      '#value' => '',
    );
    }

    $form[$fieldset]['stayers'][$id] = array(
      '#type' => 'checkbox',
      '#default_value' => (isset($question->staying) && $question->staying === FALSE) ? 0 : 1,
      '#attributes' => array('class' => 'q-staying'),
    );
    if ($quiz->randomization == 2) {
      $form[$fieldset]['compulsories'][$id] = array(
        '#type' => 'checkbox',
        '#default_value' => isset($question->question_status) ? ($question->question_status == QUESTION_ALWAYS) ? 1: 0 : 0,
      );
    }
    $link_options = array(
      'attributes' => array('class' => 'handle-changes'),
      'html' => TRUE,
    );
    $form[$fieldset]['titles'][$id] = array('#value' => l($question->title, 'node/'. $question->nid, $link_options));
    $form[$fieldset]['types'][$id] = array(
      '#value' => $question_types[$question->type]['name'],
    );
    $form[$fieldset]['view_links'][$id] = array(
      '#value' => l(
        t('Edit'),
        'node/'. $question->nid .'/edit',
        array(
          'query' => array('destination' => $my_dest),
          'attributes' => array('class' => 'handle-changes'),
        )
      ),
    );
    $form[$fieldset]['remove_links'][$id] = array(
      '#value' => '<a href="#" class="rem-link">' . t('Remove') . '</a>',
    );
    if ($question->vid == $question->latest_vid) {
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
}

/**
 * Adds checkbox for creating new revision. Checks it by default if answers exists.
 *
 * @param $form
 *   FAPI form(array)
 * @param $quiz
 *   Quiz node(object)
 */
function _quiz_add_revision_checkbox(&$form, &$quiz) {
  // Recomend and preselect to create the quiz as a new revision if it already has been answered
  if (quiz_has_been_answered($quiz)) {
    $rev_default = TRUE;
    $rev_description = t('This quiz has been answered. To maintain correctnes of existing answer reports changes should be saved as a new revision.');
  }
  else {
    $rev_default = in_array('revision', variable_get('node_options_quiz', array()));
    $rev_description = t('Allow question status changes to create a new revision of the quiz?');
  }
  if (user_access('manual quiz revisioning')) {
    $form['new_revision'] = array(
      '#type' => 'checkbox',
      '#default_value' => $rev_default,
      '#title' => t('New revision'),
      '#description' => $rev_description,
    );
  }
  else {
    $form['new_revision'] = array(
      '#type' => 'value',
      '#value' => $rev_default,
    );
  }
}
/**
 * Creates the browser part of the quiz_questions_form
 *
 * @param $hidden_questions
 *   Array where we add the questions in the browser
 * @param $questions
 *   Questions already added to the question list(array)
 * @param $form_state
 *   FAPI form_state(array)
 * @param $quiz
 *   Quiz node(object)
 * @return form
 *   FAPI form(array)
 */
function _quiz_question_browser_form(&$hidden_questions, $questions, $form_state, $quiz, $question_types) {
  if (!is_array($question_types) || count($question_types) == 0) {
    return $form['no_questions'] = array(
      '#type' => 'markup',
      '#value' => t('No question types are enabled'),
    );
  }
  $form = array(
    '#type' => 'fieldset',
    '#title' => t('Browse for questions to add'),
    '#description' => t('Mark all the questions you want to add.') .' '
    .t('You can filter questions by using the textfields and select boxes.') .' '
    .t('You can sort by pressing the table headers.'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
    '#tree' => TRUE,
  );
  $form['table'] = array('#theme' => 'quiz_question_browser');
  $browser = &$form['table'];

  // Ahah targets
  $browser['ahah_target'] = array(
    '#type' => 'markup',
    '#value' => '<DIV ID = "ahah-target"></DIV>'
  );
  $browser['ahah_target_all'] = array(
    '#type' => 'markup',
    '#value' => '<DIV ID = "all-ahah-target">'
  );

  // Ajax use this field to send extra query strings to drupal
  $browser['add_to_get'] = array(
    '#type' => 'hidden',
    '#default_value' => '',
  );

  //Build filter part of form:
  _quiz_question_browser_add_filter_fields($browser, $question_types);

  // Create filter part of sql.
  $filter_params = array($quiz->vid);
  $filter_sql = _quiz_question_browser_prepare_filter_sql($filter_params);

  // Add querystring recieved via ajax to the $_GET array...
  $question_type_list = implode('\', \'', array_keys($question_types));
  _quiz_add_to_get($form_state['values']['browser']['table']['add_to_get']);

  // Browsers table header
  $browser['#header'] = array(
    NULL,
    array('data' => t('Title'), 'field' => 'title', 'class' => 'quiz-browser-header-title'),
    array('data' => t('Type'), 'field' => 'type', 'class' => 'quiz-browser-header-type'),
    array('data' => t('Changed'), 'field' => 'changed', 'sort' => 'desc', 'class' => 'quiz-browser-header-changed'),
    array('data' => t('Username'), 'field' => 'name', 'class' => 'quiz-browser-header-name'),
  );

  $sql = 'SELECT n.nid, n.type, n.vid, n.title, n.changed, u.name 
          FROM {node} n
          LEFT OUTER JOIN {users} u
          ON n.uid = u.uid
          WHERE n.type IN (\''. $question_type_list .'\') AND n.nid NOT IN (
            SELECT child_nid
            FROM {quiz_node_relationship}
            WHERE parent_vid = %d
          )'. $filter_sql . tablesort_sql($browser['#header']);
  $page = intval($_GET['page']);
  $num = 25;
  $pager_sql = 'SELECT COUNT(*) 
          FROM (
          SELECT nid FROM {node} n
          LEFT OUTER JOIN {users} u ON n.uid = u.uid
          WHERE n.type IN (\''. $question_type_list .'\') AND n.nid NOT IN (
            SELECT child_nid
            FROM {quiz_node_relationship}
            WHERE parent_vid = %d
          )'. $filter_sql ." LIMIT 0, ". max($page * $num + $num * 7, $num * 10) .") tbl";
  
  $res = pager_query(db_rewrite_sql($sql), $num, 0, $pager_sql, $filter_params);

  // build data part of form
  $options = array();
  while ($res_o = db_fetch_object($res)) {
    $id = $res_o->nid .'-'. $res_o->vid;
    $hidden_questions[] = $id;
    $options[$id] = check_plain($res_o->title);
    $browser['changed'][$id]['#value'] = format_date($res_o->changed, 'small');
    $browser['types'][$id]['#value'] = $question_types[$res_o->type]['name'];
    $browser['names'][$id]['#value'] = check_plain($res_o->name);
  }

  $browser['titles'] = array(
    '#title' => t('Titles'),
    '#type' => 'checkboxes',
    '#options' => $options,
    '#attributes' => array('class' => 'quiz-browser-checkbox'),
    '#default_value' => $questions,
  );
  $browser['pager'] = array(
    '#value' => '<DIV ID ="browser-pager">'. theme('pager', NULL, 20) .'</DIV>',
  );
  $browser['ahah_target_all_end'] = array(
    '#type' => 'markup',
    '#value' => '</DIV>'
  );
  return $form;
}

/**
 * Returns list of options for the changed select box
 *
 * @see _quiz_questions_browser_form()
 *
 * @return $changed_options
 *   Options for the changed select box.
 */
function _quiz_get_time_interval_options() {
  return array(
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
}

/**
 * Returns an array with sql where clauses correscponding to the options in the time filters.
 *
 * @see _quiz_questions_browser_form()
 *
 * @return $changed_timestamps
 *   array of timestamps and sql filters
 */
function _quiz_get_interval_timestamps($fieldname) {
  // Create datastructure to help create where clause in the sql for the changed filter...
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
  $changed_timestamps = array(
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

  foreach ($changed_timestamps as $key => &$val) {
    if ($val == NULL) continue;
    $val['sql'] = " AND $fieldname > ". $val[0] ." AND $fieldname < ". $val[1];
  }
  return $changed_timestamps;
}

/**
 * Returns an array with sql where clauses correscponding to the options in the time filters.
 *
 * @see _quiz_questions_browser_form()
 *
 * @return $changed_timestamps
 *   array of timestamps and sql filters
 */
function _quiz_get_duration_intervals() {
  // Create datastructure to help create where clause in the sql for the changed filter...
  $durations = array(
    NULL,
    '> 60',
    '> 300',
    '> 600',
    '> 3600',
    '> 28800',
    '> 86400',
    '< 60',
    '< 300',
    '< 600',
    '< 3600',
    '< 28800',
    '< 86400',
  );

  foreach ($durations as $key => &$val) {
    if ($val == NULL) continue;
    $durations[$key] = ' AND (time_end - time_start) '. $val;
  }
  return $durations;
}

/**
 * Returns an array with options for the time filters.
 *
 * @see _quiz_get_duration_intervals()
 *
 * @return $options
 *   array of options for the duration filter
 */
function _quiz_get_duration_options() {
  return array(
    'No filter',
    t('> 1 m'),
    t('> 5 m'),
    t('> 10 m'),
    t('> 1 h'),
    t('> 8 h'),
    t('> 24 h'),
    t('< 1 m'),
    t('< 5 m'),
    t('< 10 m'),
    t('< 1 h'),
    t('< 8 h'),
    t('< 24 h'),
  );
}

/**
 * Returns an array with options for the score filters.
 *
 * @see _quiz_get_score_intervals()
 *
 * @return $options
 *   array of options for the score filter
 */
function _quiz_get_score_options() {
  return array(
    'No filter',
    t('Passed'),
    t('Failed'),
    t('= 100 %'),
    t('> 90 %'),
    t('> 75 %'),
    t('> 50 %'),
    t('> 25 %'),
    t('< 90 %'),
    t('< 75 %'),
    t('< 50 %'),
    t('< 25 %'),
  );
}

/**
 * Returns an array with sql where clauses correscponding to the options in the time filters.
 *
 * @see _quiz_questions_browser_form()
 *
 * @return $changed_timestamps
 *   array of timestamps and sql filters
 */
function _quiz_get_score_intervals($quiz) {
// Create datastructure to help create where clause in the sql for the changed filter...
  $scores = array(
    NULL,
    '>= pass_rate',
    '< pass_rate',
    '= 100',
    '> 90',
    '> 75',
    '> 50',
    '> 25',
    '< 90',
    '< 75',
    '< 50',
    '< 25',
  );

  foreach ($scores as $key => &$val) {
    if ($val == NULL) continue;
    $scores[$key] = ' AND score '. $val;
  }
  return $scores;
}

/**
 * adds filter fields to the browser form
 *
 * @param $form
 *   FAPI form(array)
 */
function _quiz_question_browser_add_filter_fields(&$browser, &$question_types) {
  // Create options array for the type filter(select field)
  $type_options = array(
    t('No filter'),
  );
  foreach (array_keys($question_types) as $type) {
    $type_options[$type] = $question_types[$type]['name'];
  }

  // Create options array for the changed filter
  $changed_options = _quiz_get_time_interval_options();
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
      'wrapper' => 'ahah-target',
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
      'wrapper' => 'ahah-target',
      'method' => 'replace',
    ),
  );
  $filters['changed'] = array(
    '#type' => 'select',
    '#options' => $changed_options,
    '#default_value' => $_SESSION[$pre .'changed'],
    '#ahah' => array(
      'path' => 'admin/quiz/browser/ahah',
      'wrapper' => 'ahah-target',
      'method' => 'replace',
    ),
  );
  $filters['name'] = array(
    '#type' => 'textfield',
    '#size' => 10,
    '#default_value' => $_SESSION[$pre .'name'],
    '#ahah' => array(
      'path' => 'admin/quiz/browser/ahah',
      'wrapper' => 'ahah-target',
      'method' => 'replace',
      'event' => 'doneTyping',
    ),
  );
}

/**
 * Returns sql to be added in where clause in the browseres select statement
 *
 * @see _quiz_questions_browser_form()
 *
 * @param $filter_params
 *   params to be sent as parameter to db_query. (array)
 * @return $filter_sql
 *   sql to be added to where statement in browser(string)
 */
function _quiz_question_browser_prepare_filter_sql(&$filter_params) {
  $pre = 'quiz_question_browser_';
  $changed_timestamps = _quiz_get_interval_timestamps('changed');
  $filter_sql = '';
  if (drupal_strlen($_SESSION[$pre .'title']) > 0) {
    $filter_sql .= ' AND n.title LIKE \'%s%%\'';
    $filter_params[] = $_SESSION[$pre .'title'];
  }
  if (drupal_strlen($_SESSION[$pre .'name']) > 0) {
    $filter_sql .= ' AND u.name LIKE \'%s%%\'';
    $filter_params[] = $_SESSION[$pre .'name'];
  }
  if (isset($_SESSION[$pre .'type']) && ($_SESSION[$pre .'type'] !== '0')) {
    $filter_sql .= ' AND n.type = \'%s\'';
    $filter_params[] = $_SESSION[$pre .'type'];
  }
  if (isset($_SESSION[$pre .'changed'])) {
    $filter_sql .= $changed_timestamps[$_SESSION[$pre .'changed']]['sql'];
  }
  return $filter_sql;
}

/**
 * Validate that the supplied questions are real.
 */
function quiz_questions_form_validate($form, $form_state) {
  
  if (is_numeric(arg(1)) && arg(1) >= 0) {
    if (node_last_changed(intval(arg(1))) > $form_state['values']['timestamp']) {
      form_set_error('changed', t('This content has been modified by another user, changes cannot be saved.'));
    }
  }
  else {
    form_set_error('changed', t('A critical error has occured. Please report error code 28 on the quiz project page.'));
    return;
  }
  
  
  $question_types = array_keys(_quiz_get_question_types());
  $placeholders = db_placeholders($question_types, 'varchar');
  $sql = 'SELECT COUNT(nid) FROM {node} WHERE type IN (' . $placeholders . ') AND nid = %d';

  
  
  $already_checked = array();
  $weight_map = $form_state['values']['weights'];

  // Make sure the number of random questions is a positive number
  if (isset($form_state['values']['num_random_questions'])) {
    if (!is_numeric($form_state['values']['num_random_questions']) ||
         $form_state['values']['num_random_questions'] < 0) {
      form_set_error('num_random_questions', 'The number of random questions needs to be a positive number');
    }
  }

  if (empty($weight_map)) {
    form_set_error('none', 'No questions were included.');
    return;
  }

  foreach ($weight_map as $id => $weight) {
    if ($form_state['values']['stayers'][$id] == 0) continue; // The question isn't to be added...

    list($nid, $vid) = explode('-', $id, 2);

    // If a node isn't one of the questionstypes we remove it from the question list
    $params = $question_types; // Copy array.
    $params[] = $nid;
    if (db_result(db_query($sql, $params)) == 0) {
      form_set_error('none', 'One of the supplied questions was invalid. It has been removed from the quiz.');
      unset($form_state['values']['weights'][$id]);
    }

    // We also make sure that we don't have duplicate questions in the quiz.
    elseif (in_array($nid, $already_checked)) {
      form_set_error('none', 'A duplicate question has been removed. You can only ask a question once per quiz.');
      unset($form_state['values']['weights'][$id]);
    }
    else {
      $already_checked[] = $nid;
    }
  }

  // We make sure max score is a positive number
  $max_scores = $form_state['values']['max_scores'];
  foreach ($max_scores as $id => $max_score) {
    if ($form_state['values']['stayers'][$id] == 0) continue;
    if (!is_numeric($max_score)) {
      form_set_error("max_scores][$id", t('Max score needs to be numeric'));
    }
    elseif ($max_score < 0) form_set_error("max_scores][$id", t('Max score needs to be positive or zero'));
  }
}

/**
 * Update a quiz set of items with new weights and membership
 * @param $quiz
 * @param $weight_map
 * @param $removed_set
 * @param $is_new_revision
 * @return array set of questions after updating
 */
function _quiz_update_items($quiz, $weight_map, $max_scores, $is_new_revision, $refreshes, $stayers, $compulsories = NULL) {
    // Get quiz questions that will always be on the quiz:
  $questions = array();
  foreach ($weight_map as $id => $weight) {
    if ($stayers[$id] == 0) continue;
    list($nid, $vid) = explode('-', $id, 2);
    $nid = (int)$nid;
    $vid = (int)$vid;
    $question['nid'] = $nid;
    $question['vid'] = $vid;
    if (isset($compulsories)) {
      $question['state'] = ($compulsories[$id] == 1) ? QUESTION_ALWAYS : QUESTION_RANDOM;
    }
    else {
      $question['state'] = QUESTION_ALWAYS;
    }
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
  // Load the quiz node
  $quiz = node_load(intval(arg(1)));

  // Update the refresh latest quizzes table so that we know what the users latest quizzes are
  $is_new_revision = (bool) $form_state['values']['new_revision'];
  if (module_exists('quiz_question') && !$is_new_revision) quiz_question_refresh_latest_quizzes($quiz->nid);

  _quiz_question_browser_submit($form, $form_state);

  $weight_map = $form_state['values']['weights'];
  $max_scores = $form_state['values']['max_scores'];
  $refreshes = $form_state['values']['revision'];
  $stayers = $form_state['values']['stayers'];
  if (isset($form_state['values']['compulsories']))
    $compulsories = $form_state['values']['compulsories'];

  // Store what questions belong to the quiz
  $questions = _quiz_update_items($quiz, $weight_map, $max_scores, $is_new_revision, $refreshes, $stayers, $compulsories);

  $num_random = $form_state['values']['num_random_questions'];
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
    drupal_set_message(t('There was an error updating the @quiz.', array('@quiz' => QUIZ_NAME)), 'error');
  }
}

/**
 * Takes care of the browser part of the submitted form values.
 *
 * This function changes the form_state to reflect questions added via the browser.
 * (Especially if js is disabled)
 *
 *
 * @param $form
 *   FAPI form(array)
 * @param $form_state
 *   FAPI form_state(array)
 */
function _quiz_question_browser_submit($form, &$form_state) {
  // Find the biggest weight:
  $next_weight = max($form_state['values']['weights']);

  // Save the active filters in $_SESSION
  $filters = $form_state['values']['browser']['table']['filters'];
  _quiz_browser_store_filters($filters);

  // If a question is chosen in the browser, add it to the question list if it isn't already there
  if (is_array($form_state['values']['browser']['table']['titles'])) {
    foreach ($form_state['values']['browser']['table']['titles'] as $id) {
      if ($id !== 0) {
        if($form_state['values']['stayers'][$id] == 1) continue;
        $matches = array();
        preg_match('/([0-9]+)-([0-9]+)/', $id, $matches);
        $nid = $matches[1];
        $vid = $matches[2];
        $form_state['values']['weights'][$id] = ++$next_weight;
        $form_state['values']['max_scores'][$id] = quiz_question_get_max_score($nid, $vid);
        $form_state['values']['stayers'][$id] = 1;
      }
    }
  }
}

/**
 * Store values for each browser filter in $_SESSION
 *
 * @param $filters
 *   array holding the values for each filter
 */
function _quiz_browser_store_filters($filters) {
  $pre = 'quiz_question_browser_';
  $_SESSION[$pre .'title'] = $filters['title'];
  $_SESSION[$pre .'type'] = $filters['type'];
  $_SESSION[$pre .'changed'] = $filters['changed'];
  $_SESSION[$pre .'name'] = $filters['name'];
}

/**
 * Store values for each browser filter in $_SESSION
 *
 * @param $filters
 *   array holding the values for each filter
 */
function _quiz_results_mr_store_filters($filters) {
  $pre = 'quiz_results_mr_';
  $_SESSION[$pre .'name'] = $filters['name'];
  $_SESSION[$pre .'started'] = $filters['started'];
  $_SESSION[$pre .'finished'] = $filters['finished'];
  $_SESSION[$pre .'score'] = $filters['score'];
  $_SESSION[$pre .'evaluated'] = $filters['evaluated'];
  $_SESSION[$pre .'best_results'] = $filters['best_results'];
  $_SESSION[$pre .'not_in_progress'] = $filters['not_in_progress'];
}

/**
 * AHAH handler for the question browser
 */
function quiz_browser_ahah() {
  // Prepare to get the form from cache
  $form_state = array('storage' => NULL, 'submitted' => FALSE);
  $form_id = $_POST['form_build_id'];

  // Make sure the form exists
  if (!($form = form_get_cache($form_id, $form_state))) {
    form_set_error('form_token', t("Validation error, please try again. If this error persists, please contact the site administrator."));
    $output = theme('status_messages');
    print drupal_to_js(array('status' => TRUE, 'data' => $output));
    exit();
  }
  // If filter or pager has been used
  $replace_all = drupal_strlen($_POST['browser']['table']['add_to_get']) > 0;

  // Prepare to submit the form
  $args = $form['#parameters'];
  $form_id = array_shift($args);

  // We will run some of the submit handlers so we need to disable redirecting.
  $form['#redirect'] = FALSE;

  // We need to process the form, prepare for that by setting a few internals
  // variables.
  $form['#post'] = $_POST;
  $form['#programmed'] = FALSE;
  $form_state['post'] = $_POST;

  // $_REQUEST is used by drupal core. We need to remove data recieved from ajax.
  foreach ($_POST as $key => $value) {
    unset($_REQUEST[$key]);
  }
  $_POST = array();

  // We don't want any validation errors to show up. This isn't a real submit.
  _quiz_skip_validation($form);

  _quiz_browser_store_filters($form_state['post']['browser']['table']['filters']);

  // Build, validate and submit the form.
  drupal_process_form($form_id, $form, $form_state);

  //Remove any status messages the processing resulted in...
  theme('status_messages');

  // Get the form the ajax call resulted in
  $form = drupal_rebuild_form($form_id, $form_state, $args, $form_build_id);

  // We store a copy of the browser part of the form, and remove it from the main form.
  // This way we can render the form without the browser, and render the browser separatly
  $b_form = $form['question_list']['browser'];
  unset($form['question_list']['browser']);

  /*
   * $output is the string we will send to ajax, and that will be added to the page.
   * We are doing this in a special way so way have to call javascript functions to
   * update the DOM.
   */
  $output = '<script>var quizNewBuildId = '. drupal_to_js($form['#build_id']) .';';

  // We remove all visible questions from the quiz. We only want to render the hidden questions.
  foreach ($form['question_list']['stayers'] as $key => $value) {
    if (isset($value['#value']) && $value['#value'] == 1) unset($form['question_list']['titles'][$key]);
  }

  // We count the number of hidden questions
  $num_hiddens = 0;
  if (is_array($form['question_list']['titles'])) {
    foreach ($form['question_list']['titles'] as $key => $value) {
      if (preg_match('/^[0-9]+-[0-9]+/', $key)) $num_hiddens++;
    }
  }

  // We render what is left of the question list
  $sub_form_html = drupal_render($form['question_list']);

  // We pick the table rows holding the hidden questions
  $q_rows_html = _quiz_get_last_table_rows($sub_form_html, $num_hiddens);
  $output .= ' var qRowsHidden = '.drupal_to_js($q_rows_html).';';

  // We replace the entire browser. This is done when the browser is sorted
  if ($replace_all) {
    // Make sure the description and test is removed
    unset($b_form['#type']);
    unset($b_form['#value']);

    $rendered_browser = drupal_render($b_form['table']);
    $output .= ' var renderedBrowser = '. drupal_to_js(theme('status_messages') . $rendered_browser) .';';

    // Have js(jQuery) replace the browser and change the form build id in the DOM
    $output .= ' Quiz.replaceBrowser(renderedBrowser, quizNewBuildId);';

    // Add the hidden questions to the question list
    $output .= ' Quiz.addQuestions(qRowsHidden);</script>';
  }
  // Only the question rows in the browser is replaced.
  else {
    // Send status messages, new build id for the form and the new question row to the javascript:
    $b_rows = _quiz_get_browser_content(drupal_render($b_form), 'quiz-question-browser-row');
    $b_pager = drupal_render($b_form['table']['pager']);

    $output .= ' var quizBrowserRows = '. drupal_to_js($b_rows) .';';
    $output .= ' var quizBrowserPager = '. drupal_to_js($b_pager) .';';
    $output .= ' Quiz.addBrowserRows(quizBrowserRows, quizNewBuildId, quizBrowserPager);';
    $output .= ' Quiz.addQuestions(qRowsHidden);</script>';
    $output .= ' '. theme('status_messages');
  }
  drupal_json(array('status' => TRUE, 'data' => $output));
}

/**
 * AHAH handler for the results browser
 */
function quiz_results_mr_browser_ahah() {
  // Prepare to get the form from cache
  $form_state = array('storage' => NULL, 'submitted' => FALSE);
  $form_id = $_POST['form_build_id'];
  
  // Make sure the form exists
  if (!($form = form_get_cache($form_id, $form_state))) {
    form_set_error('form_token', t("Validation error, please try again. If this error persists, please contact the site administrator."));
    $output = theme('status_messages');
    print drupal_to_js(array('status' => TRUE, 'data' => $output));
    exit();
  }
  // If filter or pager has been used
  $replace_all = drupal_strlen($_POST['table']['add_to_get']) > 0;

  // Prepare to submit the form
  $args = $form['#parameters'];
  $form_id = array_shift($args);

  // We will run some of the submit handlers so we need to disable redirecting.
  $form['#redirect'] = FALSE;

  // We need to process the form, prepare for that by setting a few internals
  // variables.
  $form['#post'] = $_POST;
  $form['#programmed'] = FALSE;
  $form_state['post'] = $_POST;

  // $_REQUEST is used by drupal core. We need to remove data recieved from ajax.
  foreach ($_POST as $key => $value) {
    unset($_REQUEST[$key]);
  }
  $_POST = array();

  // We don't want any validation errors to show up. This isn't a real submit.
  _quiz_skip_validation($form);

  _quiz_results_mr_store_filters($form_state['post']['table']['filters']);

  // Build, validate and submit the form.
  drupal_process_form($form_id, $form, $form_state);

  //Remove any status messages the processing resulted in...
  theme('status_messages');

  // Get the form the ajax call resulted in
  $form = drupal_rebuild_form($form_id, $form_state, $args, $form_build_id);

  // We store a copy of the browser part of the form, and remove it from the main form.
  // This way we can render the form without the browser, and render the browser separatly
  $b_form = $form['table'];
  unset($form['table']);

  /*
   * $output is the string we will send to ajax, and that will be added to the page.
   * We are doing this in a special way so we have to call javascript functions to
   * update the DOM.
   */
  $output = '<script>var quizNewBuildId = '. drupal_to_js($form['#build_id']) .';';

  // We replace the entire browser. This is done when the browser is sorted
  if ($replace_all) {
    // Make sure the description and test is removed
    unset($b_form['#type']);
    unset($b_form['#value']);

    $rendered_browser = drupal_render($b_form);
    $output .= ' var renderedBrowser = '. drupal_to_js(theme('status_messages') . $rendered_browser) .';';

    // Have js(jQuery) replace the browser and change the form build id in the DOM
    $output .= ' Quiz.replaceBrowser(renderedBrowser, quizNewBuildId);';
  }
  
  // Only the question rows in the browser is replaced.
  else {
    // Send status messages, new build id for the form and the new question row to the javascript:
    $b_rows = _quiz_get_browser_content(drupal_render($b_form), 'quiz-results-browser-row');
    $b_pager = drupal_render($b_form['pager']);

    $output .= ' var quizBrowserRows = '. drupal_to_js($b_rows) .';';
    $output .= ' var quizBrowserPager = '. drupal_to_js($b_pager) .';';
    $output .= ' Quiz.addBrowserRows(quizBrowserRows, quizNewBuildId, quizBrowserPager);';
    $output .= ' </script>';
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

  $form['quiz_global_settings']['quiz_use_passfail'] = array(
    '#type' => 'checkbox',
    '#title' => t('Allow quiz creators to set a pass/fail option when creating a @quiz.', array('@quiz' => strtolower(QUIZ_NAME))),
    '#default_value' => variable_get('quiz_use_passfail', 1),
    '#description' => t('Check this to display the pass/fail options in the @quiz form. If you want to prohibit other quiz creators from changing the default pass/fail percentage set below, uncheck this option.', array('@quiz' => QUIZ_NAME)),
  );

  $form['quiz_global_settings']['quiz_display_feedback'] = array(
    '#type' => 'checkbox',
    '#title' => t('Display @quiz feedback.', array('@quiz' => strtolower(QUIZ_NAME))),
    '#default_value' => variable_get('quiz_display_feedback', 1),
    '#description' => t('List the user selected answer and correct answer at the end of the @quiz.', array('@quiz' => QUIZ_NAME)),
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
    '#default_value' => variable_get('quiz_email_results', 0),
    '#description' => t('Check this to send users quiz results over E-mail at the end of quiz.')
  );
  $form['quiz_email_settings']['quiz_results_to_quiz_author'] = array(
    '#type' => 'checkbox',
    '#title' => t('E-mail all users results to Quiz Author.', array('@quiz' => strtolower(QUIZ_NAME))),
    '#default_value' => variable_get('quiz_results_to_quiz_author', 0),
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

  $form['def_settings_link'] = array(
    '#type' => 'markup',
    '#value' => '<p>'. t('Default values for the quiz node-form can be edited '). l(t('here'), 'admin/quiz/settings/quiz_form') .'</p>',
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
  if (!is_numeric($form_state['values']['quiz_autotitle_length'])) {
    form_set_error('quiz_autotitle_length', t('The autotitle length value must be an integer between 0 and 128.'));
  }
  if ($form_state['values']['quiz_autotitle_length'] > 128) {
    form_set_error('quiz_autotitle_length', t('The autotitle length value must not be bigger than 128.'));
  }
  if ($form_state['values']['quiz_autotitle_length'] < 0) {
    form_set_error('quiz_autotitle_length', t('The autotitle length value must not be smaller than 0.'));
  }
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
  $path_to_module_quiz = drupal_get_path('module', 'quiz'); 	 
  $png = array(	 
    'html' => theme('image', $path_to_module_quiz . '/images/html.png', t('Export as HTML'), t('Export as HTML')), 	 
    'xml' => theme('image', $path_to_module_quiz . '/images/xml.png', t('Export as XML'), t('Export as XML')), 	 
    'csv' => theme('image', $path_to_module_quiz . '/images/csv.png', t('Export as CSV'), t('Export as CSV')), 	 
  );
  $rows = array();
  
  $exp = module_exists('results_export');
  
  while (list($key, $result) = each($results)) {
    $cols = array(
      l($result['title'], 'admin/quiz/reports/results/'. $result['nid']),
      check_plain($result['name']),
      format_date($result['created'], 'small'),
    );
    if ($exp)
      $cols[] = l($png['html'], 'admin/quiz/results_export_teaser_view/'. $result['nid'] .'/html', array('html' => TRUE)) 	 
	          . l($png['xml'], 'admin/quiz/results_export_teaser_view/'. $result['nid'] . '/xml', array('html' => TRUE)) 	 
	          . l($png['csv'], 'admin/quiz/results_export_teaser_view/'. $result['nid'] . '/csv', array('html' => TRUE));
    $rows[] = $cols;
  }

  $header = array(
    t('@quiz title', array('@quiz' => QUIZ_NAME)),
    t('Created by'),
    t('Created on')
  );
  if ($exp) {
    $header[] = t('Export');
  }
  if (!user_access('view any quiz results'))
    $no_quizzes = '<p>' . t('No @quiz that you have created was found. You do not have permission to see any other results.', array('@quiz' => QUIZ_NAME)) .'</p>';
  else {
    $no_quizzes = '<p>' . t('No @quiz found.', array('@quiz' => QUIZ_NAME)) .'</p>';
  }
  $output = (!empty($rows)) ? theme('table', $header, $rows) : $no_quizzes;
  return $output;
}

function theme_quiz_node_form($form) {
  $path = drupal_get_path('module', 'quiz') .'/theme/quiz_node_form.js';
  drupal_add_js($path);
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
  $output .= '<div id="quiz_score_percent">'. t('Total score: @score%', array('@score' => $score['percentage_score'])) .'</div>'."\n";
  if (isset($summary['passfail']))
    $output .= '<div id="quiz_summary">'. $summary['passfail'] .'</div>'."\n";
  if (isset($summary['result']))
    $output .= '<div id="quiz_summary">'. $summary['result'] .'</div>'."\n";
  // Get the feedback for all questions.
  require_once(drupal_get_path('module', 'quiz') .'/quiz.pages.inc');
  $output .= drupal_get_form('quiz_report_form', $questions, TRUE, TRUE, TRUE);
  return $output;
}

/**
 * Theme a question selection table, adding drag and drop support.
 */
function theme_question_selection_table($form) {
  drupal_add_tabledrag('question-list', 'order', 'sibling', 'question-list-weight', NULL, NULL, TRUE);

  $headers = array(t('Question'), t('Type'), t('Actions'), t('Update'), t('Max score'));
  if (isset($form['compulsories'])) $headers[] = t('Compulsory');
  $headers[] = t('Weight');
  $rows = array();
  if (!empty($form['titles'])) {
    foreach (element_children($form['titles']) as $id) {
      $form['weights'][$id]['#attributes']['class'] = 'question-list-weight';

      $rows[] = _quiz_get_question_row($form, $id);
    }
    // Make sure the same fields aren't rendered twice
    unset($form['types'], $form['view_links'], $form['remove_links'], $form['stayers']);
    unset($form['max_scores'], $form['revision'], $form['weights'], $form['titles'], $form['compulsories']);
  }
  $html_attr = array('id' => 'question-list');
  if (isset($form['no_questions']))
    $html_attr['style'] = "display:none;";
  $table = theme('table', $headers, $rows, $html_attr);
  return drupal_render($form['num_random_questions']) . drupal_render($form['random_term_id']) . $table . drupal_render($form);
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

function quiz_results_manage_results_form(&$form_state, $quiz) {
  $form = array();
  $pre = 'quiz_results_mr_';
  if (!isset($_SESSION[$pre .'not_in_progress'])) $_SESSION[$pre .'not_in_progress'] = 1;
  $form['#action'] = url('admin/quiz/reports/results/'. $quiz->nid);
  $display = isset($_GET['del']) || isset($form_state['storage']['del']) ? 'none' : 'block';
  $form['update'] = array(
    '#type' => 'fieldset',
    '#title' => t('Options'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
    '#attributes' => array(
      'class' => 'container-inline', 
      'id' => 'quiz-results-update',
      'style' => "display:$display;"
    ),
  );
  $form['update']['bulk_action'] = array(
    '#type' => 'select',
    '#options' => array(
      'def' => '',
      'del' => t('delete'),
    ),
  );
  $form['update']['update'] = array(
    '#type' => 'submit',
    '#value' => t('Update'),
  );
  $display = isset($_GET['del']) || isset($form_state['storage']['del']) ? 'block' : 'none';
  $form['confirm_delete'] = array(
    '#type' => 'fieldset',
    '#title' => t('Confirm deletion'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
    '#attributes' => array(
      'style' => "display:$display;",
      'id' => 'quiz-results-confirm-delete'
    ),
  );
  $form['confirm_delete']['help'] = array(
    '#type' => 'item',
    '#value' => t('Are you sure you want to delete all of these results?'),
    '#description' => t('This action cannot be undone'),
  );
  $form['confirm_delete']['confirm_delete'] = array(
    '#type' => 'submit',
    '#value' => t('Delete all marked results'),
  );
  $form['confirm_delete']['cancel'] = array(
    '#type' => 'markup',
    '#value' => l(t('cancel'), $_GET['q'], array('attributes' => array('id' => 'quiz-results-cancel-delete'))),
  );
  $form['special_filters'] = array(
    '#type' => 'fieldset',
    '#title' => t('Special filters'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['special_filters']['best_results'] = array(
    '#type' => 'checkbox',
    '#title' => t('Only show each users best result'),
    '#parents' => array('table', 'filters', 'best_results'),
  );
  $form['special_filters']['not_in_progress'] = array(
    '#type' => 'checkbox',
    '#title' => t('Do not show quizzes in progress'),
    '#parents' => array('table', 'filters', 'not_in_progress'),
    '#default_value' => isset($_SESSION[$pre .'not_in_progress']) ? $_SESSION[$pre . 'not_in_progress'] : 1,
  );
  $form['table'] = array('#theme' => 'quiz_results_browser', '#tree' => TRUE);
  $browser = &$form['table'];

  // Ahah targets
  $browser['ahah_target'] = array( // TODO: Needed?
    '#type' => 'markup',
    '#value' => '<DIV ID = "ahah-target"></DIV>'
  );
  $browser['ahah_target_all'] = array( // TODO: Needed?
    '#type' => 'markup',
    '#value' => '<DIV ID = "all-ahah-target">'
  );

  // Ajax use this field to send extra query strings to drupal
  $browser['add_to_get'] = array( // TODO: Needed?
    '#type' => 'hidden',
    '#default_value' => '',
  );

  //Build filter part of form:
  _quiz_results_mr_add_filter_fields($browser);

  // Add querystring recieved via ajax to the $_GET array...
  // $question_type_list = implode('\', \'', array_keys($question_types));
  _quiz_add_to_get($form_state['values']['table']['add_to_get']);

  // Browsers table header
  $browser['#header'] = array(
    array('data' => t('Username'), 'field' => 'u.uid', 'class' => 'quiz-browser-header-name'),
    array('data' => t('Started'), 'field' => 'started', 'class' => 'quiz-browser-header-started'),
    array('data' => t('Finished'), 'field' => 'finished', 'sort' => 'desc', 'class' => 'quiz-browser-header-finished'),
    array('data' => t('Score'), 'field' => 'score', 'class' => 'quiz-browser-header-score'),
    array('data' => t('Evaluated'), 'field' => 'evaluated', 'class' => 'quiz-browser-header-evaluated'),
  );

  $res = _quiz_results_mr_data_provider($browser['#header'], $quiz);

  // build data part of form
  $options = array();
  while ($res_o = db_fetch_object($res)) {
    $id = $res_o->nid .'-'. $res_o->result_id;
    $options[$id] = check_plain($res_o->name);
    $browser['hover_menu'][$id]['#value'] = _quiz_results_mr_get_hover($quiz, $res_o->result_id);
    $browser['started'][$id]['#value'] = format_date($res_o->started, 'small');
    $browser['finished'][$id]['#value'] = $res_o->finished == 0 ? t('In progress') : format_date($res_o->finished, 'small');
    $browser['duration'][$id]['#value'] = $res_o->finished == 0 ? t('In progress') : _quiz_format_duration($res_o->duration);
    $browser['score'][$id]['#value'] = $res_o->finished == 0 ? t('In progress') : check_plain($res_o->score);
    $browser['evaluated'][$id]['#value'] = $res_o->evaluated == 0 ? t('No') : t('Yes');
    $browser['pass_rate'][$id]['#value'] = $res_o->pass_rate;
  }
  $def = isset($form_state['storage']['del']) ? $form_state['storage']['del'] : array($quiz->nid .'-'. $_GET['del']);
  unset($form_state['storage']['del']);
  $browser['name'] = array(
    '#title' => t('Name'),
    '#type' => 'checkboxes',
    '#options' => $options,
    '#attributes' => array('class' => 'quiz-browser-checkbox'),
    '#default_value' => $def,
  );
  $browser['pager'] = array(
    '#value' => '<DIV ID ="browser-pager">'. theme('pager', NULL, 50) .'</DIV>',
  );
  $browser['ahah_target_all_end'] = array(
    '#type' => 'markup',
    '#value' => '</DIV>'
  );
  $form['#submit'] = array('quiz_results_mr_form_submit');
  return $form;
}

function _quiz_results_mr_data_provider($header, $quiz) {
  $filter = _quiz_results_mr_prepare_filter($quiz);
  $sql = "SELECT u.name, u.uid, qnrs.result_id, qnrs.score, qnrs.is_evaluated AS evaluated, qnrs.time_start as started, qnrs.time_end as finished, qnp.pass_rate, qnrs.time_end - qnrs.time_start as duration
          FROM {quiz_node_results} qnrs
          LEFT JOIN {users} u ON u.uid = qnrs.uid
          LEFT JOIN {quiz_node_properties} qnp ON qnrs.vid = qnp.vid
          ". $filter['join'] ."
          WHERE qnrs.nid = %d ".$filter['where'] . $filter['group'] . tablesort_sql($header) .', qnrs.result_id DESC';
  $page = intval($_GET['page']);
  $num = 50;
  $pager_sql = "SELECT COUNT(*) FROM (
                SELECT qnrs.result_id
                FROM {quiz_node_results} qnrs
                LEFT JOIN {users} u ON u.uid = qnrs.uid
                LEFT JOIN {quiz_node_properties} qnp ON qnrs.vid = qnp.vid
                ". $filter['join'] ."
                WHERE qnrs.nid = %d ".$filter['where'] . $filter['group'] . ' LIMIT 0, '. max($page * $num + $num * 7, $num * 10) .") tbl";
  
  return pager_query($sql, $num, 0, $pager_sql, $filter['params']);
}

/**
 * Returns sql to be added in where clause in the _quiz_results_mr_data_provider select statement
 *
 * @see _quiz_results_mr_data_provider()
 *
 * @param $filter_params
 *   params to be sent as parameter to db_query. (array)
 * @return $filter_sql
 *   sql to be added to where statement in browser(string)
 */
function _quiz_results_mr_prepare_filter($quiz) {
  $pre = 'quiz_results_mr_';
  $started_intervals = _quiz_get_interval_timestamps('time_start');
  $finished_intervals = _quiz_get_interval_timestamps('time_end');
  $duration_intervals = _quiz_get_duration_intervals();
  $score_intervals = _quiz_get_score_intervals($quiz);
  $filter = array(
    'params' => array('nid' => $quiz->nid),
    'join' => '',
    'where' => '',
    'group' => '',
  );
  if (drupal_strlen($_SESSION[$pre .'name']) > 0) {
    $filter['where'] .= ' AND u.name LIKE \'%s%%\'';
    $filter['params'][] = $_SESSION[$pre .'name'];
  }
  if (isset($_SESSION[$pre .'started'])) {
    $filter['where'] .= $started_intervals[$_SESSION[$pre .'started']]['sql'];
  }
  if (isset($_SESSION[$pre .'finished'])) {
    $filter['where'] .= $finished_intervals[$_SESSION[$pre .'finished']]['sql'];
  }
  if (isset($_SESSION[$pre .'score'])) {
    $filter['where'] .= $score_intervals[$_SESSION[$pre .'score']];
  }
  if (isset($_SESSION[$pre .'evaluated'])) {
    switch ($_SESSION[$pre .'evaluated']) {
      case '0': $filter['where'] .= ' AND is_evaluated = 0'; break;
      case '1': $filter['where'] .= ' AND is_evaluated = 1'; break;
    }
  }
  if ($_SESSION[$pre .'best_results'] == 1) {
    $filter['join'] .= " INNER JOIN (
                           SELECT qn.uid, MAX(qn.score) AS max_score
                           FROM quiz_node_results qn
                           GROUP BY uid
                         ) AS qnrc ON qnrs.uid = qnrc.uid AND qnrs.score = qnrc.max_score";
    $filter['group'] .= " GROUP BY qnrs.uid";
  }
  if ($_SESSION[$pre .'not_in_progress'] == 1) {
    $filter['where'] .= " AND time_end != 0";
  }
  return $filter;
}

function _quiz_results_mr_add_filter_fields(&$browser) {
  // Create options array for the changed filter
  $browser['filters'] = array();
  $filters = &$browser['filters'];
  $filters['all'] = array(
    '#type' => 'checkbox',
  );
  $pre = 'quiz_results_mr_';
  $filters['name'] = array(
    '#type' => 'textfield',
    '#size' => 12,
    '#default_value' => $_SESSION[$pre .'name'],
    '#ahah' => array(
      'path' => 'admin/quiz/reports/results/browser_ahah',
      'wrapper' => 'ahah-target',
      'method' => 'replace',
      'event' => 'doneTyping',
    ),
  );
  $filters['started'] = array(
    '#type' => 'select',
    '#options' => _quiz_get_time_interval_options(),
    '#default_value' => $_SESSION[$pre .'started'],
    '#ahah' => array(
      'path' => 'admin/quiz/reports/results/browser_ahah',
      'wrapper' => 'ahah-target',
      'method' => 'replace',
    ),
  );
  $filters['finished'] = array(
    '#type' => 'select',
    '#options' => _quiz_get_time_interval_options(),
    '#default_value' => $_SESSION[$pre .'finished'],
    '#ahah' => array(
      'path' => 'admin/quiz/reports/results/browser_ahah',
      'wrapper' => 'ahah-target',
      'method' => 'replace',
    ),
  );
  $filters['score'] = array(
    '#type' => 'select',
    '#options' => _quiz_get_score_options(),
    '#default_value' => $_SESSION[$pre .'score'],
    '#ahah' => array(
      'path' => 'admin/quiz/reports/results/browser_ahah',
      'wrapper' => 'ahah-target',
      'method' => 'replace',
    ),
  );
  $filters['evaluated'] = array(
    '#type' => 'select',
    '#options' => array('def' => t('No filter'), '1' => t('Yes'), '0' => t('No')),
    '#default_value' => $_SESSION[$pre .'evaluated'],
    '#ahah' => array(
      'path' => 'admin/quiz/reports/results/browser_ahah',
      'wrapper' => 'ahah-target',
      'method' => 'replace',
    ),
  );
}

function quiz_results_mr_form_submit($form, &$form_state) {
  if ($form_state['values']['op'] == t('Delete all marked results')) {
    foreach ($form_state['values']['table']['name'] as $value) {
      if ($value !== 0) {
        // Find nid and rid
        $matches = array();
        preg_match('/([0-9]+)-([0-9]+)/', $value, $matches);
        $res_nid = $matches[1];
        $res_rid = $matches[2];
        if ($form_state['values']['table']['filters']['best_results'] == 1)
          _quiz_delete_results($res_rid, $res_nid);
        else 
          _quiz_delete_results($res_rid);
      }
    }
    if(isset($res_rid)) drupal_set_message(t('Results have been deleted.'));
  }
  else {
    if ($form_state['values']['bulk_action'] == 'del') {
      $form_state['storage']['del'] = $form_state['values']['table']['name'];
    }
  }
  _quiz_results_mr_store_filters($form_state['values']['table']['filters']);
}

function quiz_admin_node_form(&$form_state) {
  $dummy_node = new stdClass();
  $dummy_node->def_uid = variable_get('quiz_def_uid', NULL);
  $form = quiz_form($dummy_node);
  $form['direction'] = array(
    '#type' => 'markup',
    '#value' => t('Here you can change the default quiz settings for new users.'),
    '#weight' => -10,
  );
  unset(
    $form['#quiz_check_revision_access'],
    $form['title'],
    $form['body_field'],
    $form['taking']['aid'],
    $form['taking']['addons'],
    $form['quiz_availability']['quiz_open'],
    $form['quiz_availability']['quiz_close'],
    $form['resultoptions'],
    $form['number_of_random_questions']
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#submit' => array('quiz_admin_node_form_submit'),
  );
  return $form;
}

function quiz_admin_node_form_validate($form, &$form_state) {
  $dummy_node = new stdClass();
  foreach($form_state['values'] as $key => $value) {
    $dummy_node->{$key} = $value;
  }
  $dummy_node->resultoptions = array();
  quiz_validate($dummy_node);
}

function quiz_admin_node_form_submit($form, &$form_state) {
  $form_state['values']['save_def_uid'] = variable_get('quiz_def_uid', NULL);
  _quiz_save_user_settings($form_state['values']);
}

function _quiz_results_mr_get_hover($quiz, $rid) {
  $to_return = array(l(t('view'), 'admin/quiz/reports/results/'. $quiz->nid .'/'. $rid));
  if (user_access('delete any quiz results') || user_access('delete results for own quiz'))
    $to_return[] = l(t('delete'), 'admin/quiz/reports/results/'. $quiz->nid, array(
      'query' => array('del' => $rid),
      'attributes' => array('class' => 'hover-del', 'id' => $quiz->nid .'-'. $rid .'-del'),
    ));
  return implode(' | ', $to_return);
}

function _quiz_delete_results($rid, $nid = NULL) {
  $rids = array();
  if (isset($nid)) {
    $sql = 'SELECT result_id
            FROM {quiz_node_results}
            WHERE nid = %d AND uid = (
              SELECT uid 
              FROM {quiz_node_results}
              WHERE result_id = %d
            )';
    $res = db_query($sql, $nid, $rid);
    while ($result = db_result($res)) {
      $rids[] = $result;
    }
  }
  else 
    $rids[] = $rid;
  quiz_delete_results($rids);
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
 * Helper function for theme_question_selection_table
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
  $hidden_class = ($sub_form['stayers'][$id]['#default_value'] === 0) ? ' hidden-question' : '';
  $data_array = array(
    drupal_render($sub_form['stayers'][$id]) . drupal_render($sub_form['titles'][$id]),
    $type,
    $sub_form['view_links'][$id]['#value'] . '<SPAN CLASS="q-remove" STYLE="display:none"> | '. $sub_form['remove_links'][$id]['#value'] .'</SPAN>',
    isset($sub_form['revision'][$id]) ? drupal_render($sub_form['revision'][$id]) : t("Up to date"),
    drupal_render($sub_form['max_scores'][$id])
  );
  if (isset($sub_form['compulsories'])) $data_array[] = drupal_render($sub_form['compulsories'][$id]);
  $data_array[] = drupal_render($sub_form['weights'][$id]);
  return array(
    'class' => 'q-row draggable'. $hidden_class,
    'id' => 'q-'.$id,
    'data' => $data_array
  );
}

/**
 * Finds and returns the last table rows(HTML) in a table(HTML)
 *
 * @param $table
 *   HTML string with a table
 * @param $num_rows
 *   The number of rows to return(int)
 * @return
 *   last table row in the table(html string)
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
 * @return
 *   all table rows with a certain class in a table(html string)
 */
function _quiz_get_browser_content($table, $class) {
  $matches = array();
  $n_matches = preg_match('/<tr class="'. $class .'.*<\/tr>/is', $table, $matches);
  return $matches[0];
}

/**
 * Adds variables from a querystring to $_GET
 *
 * @see quiz_browser_ahah()
 *
 * @param $to_add
 *   query string
 */
function _quiz_add_to_get($to_add){
  $pre = 'quiz_question_browser_';
  parse_str($to_add, $output);
  $vars = array('sort', 'order', 'page');
  if (empty($to_add)) 
    unset($_SESSION[$pre . 'page']);
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

/*
 * @param $time_in_sec
 *  Integers time in seconds
 * @return
 *  String time in min : sec format
 */
function _quiz_format_duration($time_in_sec) {
  $hours = intval($time_in_sec / 3600);
  $min = intval(($time_in_sec - $hours * 3600) / 60);
  $sec = $time_in_sec % 60;
  if (strlen($min) == 1) $min = '0'. $min;
  if (strlen($sec) == 1) $sec = '0'. $sec;
  return "$hours:$min:$sec";
}