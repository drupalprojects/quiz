<?php
// $Id$

/**
 * @file
 * This include file implements views functionality on behalf of quiz.module
 */

/**
 * Implementation of hook_views_tables().
 */
function quiz_views_tables() {
  $tables['quiz_node_results'] = array(
    'name' => 'quiz_node_results',
    'provider' => 'internal', // won't show up in external list.
    // attach quiz results to node
    'join' => array(
      'left' => array(
        'table' => 'node',
        'field' => 'nid',
      ),
      'right' => array(
        'field' => 'nid',
      ),
     'type' => 'inner'
    ),
    'fields' => array(
      'result_id' => array(
        'name' => t('Quiz Result: Result ID'),
        'sortable' => TRUE,
        'help' => t('Display the Result ID of the Quiz Result. Optionally you can show the result id with or without a link.'),
        'handler' => 'quiz_views_handler_field_result_id',
        'option' => array(
          '#type' => 'select',
          '#options' => array(
            'link' => 'As link',
            'nolink' => 'Without link',
          ),
        ),
      ),
      'quiz_result_actions' => array(
        'field' => 'result_id',
        'name' => t('Quiz Result: Actions'),
        'sortable' => TRUE,
        'help' => t('Show a link to results, or admin actions (both view and delete) or admin view and delete separately.'),
        'handler' => 'quiz_views_handler_field_result_id',
        'option' => array(
          '#type' => 'select',
          '#options' => array(
            'onlylink' => 'view result link',
            'actions' => 'As admin actions',
            'adminview' => 'As admin view link',
            'admindelete' => 'As admin delete link',
          ),
        ),
      ),
      'time_start' => array(
        'name' => t('Quiz Result: Start Time'),
        'sortable' => TRUE,
        'handler' => views_handler_field_dates(),
        'help' => t('Display the start time of a quiz result.'),
      ),
      'time_end' => array(
        'name' => t('Quiz Result: End Time'),
        'sortable' => TRUE,
        'handler' => views_handler_field_dates(),
        'help' => t('Display the end time of a quiz result.'),
      ),
      'score' => array(
        'name' => t('Quiz Result: Score'),
        'sortable' => TRUE,
        'handler' => 'quiz_views_handler_field_score',
        'addlfields' => array('result_id'),
        'help' => t('Display the final score of a quiz result.'),
      ),
      'avg_score' => array(
        'name' => t('Quiz Result: AVG Score'),
        'sortable' => TRUE,
        'help' => t('A user\'s average score across all quizzes they have taken. This field will group results by quiz taker UID.'),
        'notafield' => TRUE,
        'handler' => 'quiz_views_handler_field_avg_score',
        'query_handler' => 'quiz_views_handler_query_avg_score',
        //'validate' => 'quiz_views_handler_avg_score_validate',
      ),
    ),
    'sorts' => array(
      'result_id' => array(
        'name' => t('Quiz Result: Result ID'),
        'help' => t('This allows you to sort by the Result ID.'),
      ),
      'score' => array(
        'name' => t('Quiz Result: Score'),
        'help' => t('This allows you to sort by the percentage needed to pass the quiz.'),
      ),
      'time_start' => array(
        'name' => t('Quiz Result: Start Time'),
        'handler' => 'views_handler_sort_date',
        'option' => views_handler_sort_date_options(),
        'help' => t('This allows you to sort by the start time of the quiz result.'),
      ),
      'time_end' => array(
        'name' => t('Quiz Result: End Time'),
        'handler' => 'views_handler_sort_date',
        'option' => views_handler_sort_date_options(),
        'help' => t('This allows you to sort by the end time of the quiz result.'),
      ),
      'avg_score' => array(
        'name' => t('Quiz Result: AVG Score'),
        'help' => t('Sort by overall score (the average across all completed quizzes by a user)'),
        'handler' => 'quiz_views_handler_sort_avg_score',
      ),
    ),
    'filters' => array(
      'quiz_user_current' => array(
        'field' => 'uid',
        'name' => t('Quiz Results: Quiz Taker Current User'),
        'operator' => 'views_handler_operator_eqneq',
        'list' => 'views_handler_filter_usercurrent',
        'list-type' => 'select',
        'help' => t('This allows you to filter by whether or not the quiz result was created by the logged in user of the view.'),
      ),
      'quiz_taker' => array(
        'field' => 'uid',
        'name' => t('Quiz Result: Quiz Taker'),
        'operator' => 'views_handler_operator_or',
        'list' => 'views_handler_filter_username',
        'value-type' => 'array',
        'help' => t('This allows you to filter quiz results by a particular user. You might not find this useful if you have a lot of users.'),
      ),
      'score' => array(
        'name' => t('Quiz Result: Score'),
        'operator' => 'views_handler_operator_gtlt',
        'option' => 'integer',
        'help' => t('This allows you to filter by the quiz result score.'),
      ),
      'time_start' => array(
        'name' => t('Quiz Result: Start Time'),
        'operator' => 'views_handler_operator_gtlt',
        'value' => views_handler_filter_date_value_form(),
        'handler' => 'views_handler_filter_timestamp',
        'option' => 'string',
        'help' => t('This allows you to sort by the start time of the quiz result.'),
      ),
      'time_end' => array(
        'name' => t('Quiz Result: End Time'),
        'operator' => 'views_handler_operator_gtlt',
        'value' => views_handler_filter_date_value_form(),
        'handler' => 'views_handler_filter_timestamp',
        'option' => 'string',
        'help' => t('This allows you to sort by the end time of the quiz result.'),
      ),
    ),
  );
    $tables['quiz_user'] = array(
      'name' => 'users',
      'provider' => 'internal', // won't show up in external list.
      // attach quiz user to results
      'join' => array(
        'type' => 'inner',
        'left' => array(
           'table' => 'quiz_node_results',
           'field' => 'uid',
        ),
        'right' => array(
          'field' => 'uid',
        ),
      ),
      'fields' => array(
        'name' => array(
          'name' => t('Quiz Result: Quiz Taker'),
          'sortable' => TRUE,
          //'option' => 'string',
          'handler' => array(
            'quiz_views_handler_field_username_text' => t('normal text'),
            'quiz_views_handler_field_username_link' => t('themed userlink'),
          ),
          'uid' => 'uid',
          'addlfields' => array('uid'),
          'help' => t('Displays the user who created this quiz result.'),
        ),
      ),
      /**
       * While this would be really great functionality, I can't for the life of me
       * figure out how to fix sorting by username
       */
      
      'sort' => array(
        'name' => array(
          'name' => t('Quiz Result: Quiz Taker'),
          'help' => t('Sort by the user who created the quiz result.'),
       //   'field' => 'name',
        ),
      ),
    );
  $tables['quiz_node_properties'] = array(
    'name' => 'quiz_node_properties',
    'provider' => 'internal', // won't show up in external list.
    // attach quiz table to node
    'join' => array(
      'left' => array(
        'table' => 'node',
        'field' => 'nid',
      ),
      'right' => array(
        'field' => 'nid',
      ),
    ),
    'fields' => array(
      'pass_rate' => array(
        'name' => t('Quiz: Passing Percentage Rate'),
        'sortable' => TRUE,
        'handler' => 'quiz_views_handler_percentage',
        'help' => t('Display the percentage needed to pass the quiz.'),
      ),
      'takes' => array(
        'name' => t('Quiz: Number of Takes Allowed'),
        'sortable' => TRUE,
        'handler' => 'views_handler_field_takes',
        'help' => t('Display the number of takes allowed on a quiz.'),
      ),
      /*
       * The database has replaced this field with the number of random questions
       * 
       * TODO add to database total number of questions per quiz, 
       * otherwise it takes too many queries if there are a lot of quizzes on a page 
       * and is a performance issue.
       */
      /*'number_of_questions' => array(
        'name' => t('Quiz: Number of Questions'),
        'sortable' => TRUE,
        'handler' => 'views_handler_field_int',
        'help' => t('Display the number of quiz questions.'),
      ),*/
      'quiz_open' => array(
        'name' => t('Quiz: Start Time'),
        'sortable' => TRUE,
        'handler' => views_handler_field_dates(),
        'help' => t('Display the start date of a quiz.'),
      ),
      'quiz_closed' => array(
        'name' => t('Quiz: End Time'),
        'sortable' => TRUE,
        'handler' => views_handler_field_dates(),
        'help' => t('Display the end date of a quiz.'),
      ),
      'quiz_always' => array(
        'name' => t('Quiz: Always Available'),
        'sortable' => TRUE,
        'handler' => views_handler_yes_no,
        'help' => t('Display the end date of a quiz.'),
      ),      
    ),
    'sorts' => array(
      'pass_rate' => array(
        'name' => t('Quiz: Passing Rate'),
        'help' => t('This allows you to sort by the percentage needed to pass the quiz.'),
      ),
      'takes' => array(
        'name' => t('Quiz: Number of Takes Allowed'),
        'help' => t('This allows you to sort by the number of takes allowed of the quiz.'),
      ),
      // changed to number of random questions
      /*'number_of_questions' => array(
        'name' => t('Quiz: Number of Questions'),
        'help' => t('This allows you to sort by the number of questions in the quiz.'),
      ),*/
      'quiz_open' => array(
        'name' => t('Quiz: Start Time'),
        'handler' => 'views_handler_sort_date',
        'option' => views_handler_sort_date_options(),
        'help' => t('This allows you to sort by the Start Date of the quiz.'),
      ),
      'quiz_closed' => array(
        'name' => t('Quiz: End Time'),
        'handler' => 'views_handler_sort_date',
        'option' => views_handler_sort_date_options(),
        'help' => t('This allows you to sort by the end date of the quiz.'),
      ),
    ),
    'filters' => array(
      'pass_rate' => array(
        'name' => t('Quiz: Passing Rate'),
        'operator' => 'views_handler_operator_gtlt',
        'option' => 'integer',
        'help' => t('This allows you to filter by the percentage needed to pass the quiz.'),
      ),
      // changed to number of random questions
      /*'number_of_questions' => array(
        'name' => t('Quiz: Number of Questions'),
        'operator' => 'views_handler_operator_gtlt',
        'option' => 'integer',
        'help' => t('This allows you to filter by the number of questions in the quiz.'),
      ),*/
      'takes' => array(
        'name' => t('Quiz: Number of Takes Allowed'),
        'operator' => 'views_handler_operator_gtlt',
        'option' => 'integer',
        'help' => t('This allows you to filter by the number of takes allowed of the quiz. Enter 0 for unlimited'),
      ),
      'quiz_open' => array(
        'name' => t('Quiz: Start Time'),
        'operator' => 'views_handler_operator_gtlt',
        'value' => views_handler_filter_date_value_form(),
        'handler' => 'views_handler_filter_timestamp',
        'option' => 'string',
        'help' => t('This allows you to sort by the Start Date of the quiz.'),
      ),
      'quiz_closed' => array(
        'name' => t('Quiz: End Time'),
        'operator' => 'views_handler_operator_yesno()',
        'value' => views_handler_filter_date_value_form(),
        'handler' => 'views_handler_filter_timestamp',
        'option' => 'string',
        'help' => t('This allows you to sort by the end date of the quiz.'),
      ),
      'quiz_always' => array(
        'name' => t('Quiz: Always Available'),
        'operator' => 'views_handler_operator_yesno',
        'value' => integer,
        'handler' => 'views_handler_yes_no',
        'option' => 'string',
        'help' => t('This allows you to sort by the end date of the quiz.'),
      ),
    ),
  );
  return $tables;
}

/**
 * Callback for quiz_views_tables(): user name as plaintext.
 */
function quiz_views_handler_field_username_text($fieldinfo, $fielddata, $value, $data) {
  return check_plain($value);
}

/**
 * Callback for quiz_views_tables(): user name as a link to the usernode.
 */
function quiz_views_handler_field_username_link($fieldinfo, $fielddata, $value, $data) {
  if ($value) {
    $obj->name = $value;
    $uidfield = $fielddata['tablename'] ."_". $fieldinfo['uid'];
    $obj->uid = $data->$uidfield;
    return theme('username', $obj);
  }
}

/**
 * Generic handler given a 1 & 0 will output as a yes for 1 and no for 0.
 */
function views_handler_yes_no($fieldinfo, $fielddata, $value, $data) {
  if ($value == '0') {
    $value = "No";
  }
  else $value = "Yes";
  return $value;
}

/**
 * Generic handler given a string will add a percentage sign to the end
 */
function quiz_views_handler_percentage($fieldinfo, $fielddata, $value, $data) {
  return $value .'%';
}
/**
 * Handler for result_id field allowing it to optionally show 
 * as the result_id with a link, the result_id with no link or
 * just a link to the result.
 *
 * Ideally, there should be some way to theme the word Results
 */
function quiz_views_handler_field_result_id($fieldinfo, $fielddata, $value, $data) {
  switch ($fielddata['options']) {
    case 'nolink':
      return check_plain($value);
    case 'onlylink':
      return l(t('Results'), "user/quiz/$value/userresults");
    case 'link':
      return l($value, "user/quiz/$value/userresults");
    case 'adminview':
      return l(t('view'), "admin/quiz/$value/view");
    case 'actions':
      return l(t('view'), "admin/quiz/$value/view") .' | '. l(t('delete'), "admin/quiz/$value/delete");
    case 'admindelete':
      return l(t('delete'), "admin/quiz/$value/delete");
  }
}

/*
 * Handler turning a score into a link to the quiz result
 * TODO this should link to the userresults page, but result_id wont follow
 * so for now, there is no link option.
 */
function quiz_views_handler_field_score($fieldinfo, $fielddata, $value, $data) {
  $value .= '%';
    return check_plain($value);
}

/**
 * Generic handler outputs a number except for 0 which outputs unlimited.
 */
function views_handler_field_takes($fieldinfo, $fielddata, $value, $data) {
  if ($value == 0) $value = 'Unlimited';
  return $value;
}
/**
 * Implementation of hook_views_arguments
 */
function quiz_views_arguments() {
  /*$arguments['number_of_questions'] = array(
	 // number of questions no longer exists in the database.
	 'name' => t('Quiz: Number of Questions'),
	 'handler' => 'views_handler_arg_noq',
	 'help' => t('The Number of Questions Argument allows you to filter to nodes by the number of questions in the quiz.'),
  ),*/
  $arguments['quiz_taker_uid'] = array(
    'name' => t('Quiz Result: UID for Quiz Taker'),
    'handler' => 'views_handler_arg_quiz_taker_uid',
    'help' => t('Filter quiz results by quiz taker.'),
    'option' => array(
      '#type' => 'select',
      '#options' => array(
        1 => t('Has taken quiz'),
        0 => t('Has not taken quiz')
      ),
    ),
  );
  return $arguments;
}

/*function views_handler_arg_noq() {
  switch($op) {
    case 'summary':
    case 'sort':
    case 'filter':
    case 'link':
    case 'title':
  }
}*/

/**
 * An argument handler for quiz_taker_uid
 */
function views_handler_arg_quiz_taker_uid($op, &$query, $argtype, $arg = '') {
  switch ($op) {
    case 'summary':
      $query->add_table('quiz_node_results', true);
      $query->add_field('uid', 'quiz_node_results');
      $fieldinfo['field'] = "quiz_node_results.uid";
      return $fieldinfo;
      break;
    case 'sort':
      $query->add_orderby('users', 'name', $argtype);
      break;
    case 'filter':
      $option = $argtype['options'];
      $uid = intval($arg);
      $query->ensure_table('quiz_node_results');
      if ($option) {
        $query->ensure_table('quiz_node_results');
        $query->add_where("quiz_node_results.uid = $uid");
      }
      else {
        $joininfo = array(
          'type' => 'LEFT',
          'left' => array(
            'table' => 'node',
            'field' => 'nid'
          ),
          'right' => array(
            'field' => 'nid'
          ),
          'extra' => array(
            'uid' => $uid
          ),
        );
        $num = $query->add_table('quiz_node_results', true, 1, $joininfo);
        $tablename = $query->get_table_name('quiz_node_results', $num);
        $query->add_where("$tablename.uid IS NULL");
      }
      break;
    case 'link':
      $name = ($query->name ? $query->name : variable_get('anonymous', t('Anonymous')));
      return l($name, "$arg/" . intval($query->uid));
    case 'title':
      if (!$query) {
        return variable_get('anonymous', t('Anonymous'));
      }
      $user = db_fetch_object(db_query("SELECT name FROM {users} WHERE uid = '%d'", $query));
      return $user->name;
  }
}

/**
 * Implementation of hook_default_views().
 */
function quiz_views_default_views() {
  // quiz_results default view, this defaults to enabled, and only viewable by the head admin.
  $view = new stdClass();
  $view->name = 'quiz_results';
  $view->description = t('Quiz Results');
  $view->access = array(
    0 => '-1',
  );
  $view->page = TRUE;
  $view->page_title = t('Quiz Results');
  $view->page_empty = t('There are no quiz results.');
  $view->page_empty_format = '1';
  $view->page_type = 'table';
  $view->url = 'admin/quiz';
  $view->use_pager = TRUE;
  $view->nodes_per_page = '25';
  $view->menu = TRUE;
  $view->menu_title = t('Quiz Results');
  $view->field = array(
    array(
      'tablename' => 'quiz_node_results',
      'field' => 'result_id',
      'label' => t('Action'),
      'options' => 'actions',
    ),
    array(
      'tablename' => 'node',
      'field' => 'title',
      'label' => t('Quiz Title'),
      'handler' => 'views_handler_field_nodelink',
      'sortable' => '1',
      'options' => 'link',
    ),
    array(
      'tablename' => 'quiz_user',
      'field' => 'name',
      'label' => t('User Name'),
      'handler' => 'quiz_views_handler_field_username_link',
      'sortable' => '1',
    ),
    array(
      'tablename' => 'quiz_node_results',
      'field' => 'result_id',
      'label' => t('Result ID'),
      'sortable' => '1',
      'options' => 'nolink',
    ),
    array(
      'tablename' => 'quiz_node_results',
      'field' => 'time_end',
      'label' => t('Finished'),
      'sortable' => '1',
      'handler' => 'views_handler_field_date_small',
    ),
    array(
      'tablename' => 'quiz_node_results',
      'field' => 'score',
      'label' => t('Score'),
      'sortable' => '1',
    ),
  );
  $view->filter = array(
    array(
      'tablename' => 'node',
      'field' => 'type',
      'operator' => 'OR',
      'options' => '',
      'value' => array(
  0 => 'quiz',
),
    ),
  );
  $view->requires = array(quiz_node_results, node, quiz_user);
  $views[$view->name] = $view;
  // myresults view
  $view = new stdClass();
  $view->name = 'myresults';
  $view->description = t('Overrides default myresults with a more flexible user result page.');
  $view->access = array(
    0 => '2',
  );
  $view->page = TRUE;
  $view->page_empty = t('This user has not taken any quizzes.');
  $view->page_empty_format = '1';
  $view->page_type = 'table';
  $view->url = 'user/$arg/myresults';
  $view->use_pager = TRUE;
  $view->nodes_per_page = '10';
  $view->sort = array(
    array(
      'tablename' => 'quiz_node_results',
      'field' => 'time_start',
      'sortorder' => 'ASC',
      'options' => 'normal',
    ),
  );
  $view->argument = array(
    array(
      'type' => 'quiz_taker_uid',
      'argdefault' => '7',
      'title' => t('%1\'s  Results'),
      'options' => '1',
    ),
  );
  $view->field = array(
    array(
      'tablename' => 'node',
      'field' => 'title',
      'label' => t('Quiz Name'),
      'handler' => 'views_handler_field_nodelink',
      'sortable' => '1',
      'defaultsort' => 'ASC',
      'options' => 'link',
    ),
    array(
      'tablename' => 'quiz_node_results',
      'field' => 'score',
      'label' => t('Score'),
    ),
    array(
      'tablename' => 'quiz_node_results',
      'field' => 'time_end',
      'label' => t('Finished?'),
      'handler' => 'views_handler_field_date_small',
    ),
    array(
      'tablename' => 'quiz_node_results',
      'field' => 'quiz_result_actions',
      'options' => 'onlylink',
    ),
  );
  $view->filter = array(
    array(
      'tablename' => 'node',
      'field' => 'type',
      'operator' => 'OR',
      'value' => array(
        0 => 'quiz',
      ),
    ),
  );
  $view->requires = array(quiz_node_results, node, quiz_user);
  $views['myresults'] = $view;
  return $views;
}

function quiz_views_handler_query_avg_score($field, $fieldinfo, &$query) {
  $query->add_field("AVG(score)", null, "avg_score");
  $query->add_groupby("quiz_node_results.uid");
  $query->add_where("score IS NOT NULL");
  $query->add_where("time_end > 0");
}

function quiz_views_handler_field_avg_score($fieldinfo, $fielddata, $value, $data) {
  return number_format($data->avg_score, 2) .'%';
}

function quiz_views_handler_sort_avg_score($op, &$query, $sortinfo, $sortdata) {
  $query->add_orderby(NULL, "AVG(score)", $sortdata['sortorder'], 'avg_score');
}
