<?php

/**
 * @file
 * Quiz hook implementations
 */

/**
 * Implements hook_help().
 */
function quiz_help($path, $arg) {
  // This is moved on an experimental basis.
  include_once DRUPAL_ROOT . '/' . drupal_get_path('module', 'quiz') . '/quiz.help.inc';
  return _quiz_help($path, $arg);
}

/**
 * Implements hook_views_api().
 */
function quiz_views_api() {
  return array(
    'api' => 2,
    'path' => QUIZ_VIEWS_DIR,
  );
}

/**
 * Implements hook_ctools_plugin_directory() to let the system know
 * where our task and task_handler plugins are.
 */
function quiz_ctools_plugin_directory($owner, $plugin_type) {
  if ($owner === 'page_manager') {
    return 'plugins/page_manager/' . $plugin_type;
  }
  if ($owner === 'ctools' && $plugin_type === 'content_types') {
    return 'plugins/content_types';
  }
}

/**
 * Implements hook_ctools_plugin_api().
 */
function quiz_ctools_plugin_api($module, $api) {
  if ($module === 'page_manager' && $api === 'pages_default') {
    return array('version' => 1);
  }
}

/**
 * Implements hook_node_info().
 */
function quiz_node_info() {
  return array(
    'quiz' => array(
      'name' => t('@quiz', array("@quiz" => QUIZ_NAME)),
      'base' => 'quiz',
      'description' => t('Create interactive quizzes for site visitors'),
    )
  );
}

/**
 * Implements hook_entity_info().
 */
function quiz_entity_info() {
  return array(
    'quiz_result' => array(
      'label' => t('Quiz result'),
      'controller class' => 'EntityAPIController',
      'base table' => 'quiz_node_results',
      'entity keys' => array(
        'id' => 'result_id',
      ),
      'views controller class' => 'EntityDefaultViewsController',
    ),
    'quiz_result_answer' => array(
      'label' => t('Quiz result answer'),
      'controller class' => 'EntityAPIController',
      'base table' => 'quiz_node_results_answers',
      'entity keys' => array(
        'id' => 'result_answer_id',
      ),
      'views controller class' => 'EntityDefaultViewsController',
    ),
    'quiz_question_relationship' => array(
      'label' => t('Quiz question relationship'),
      'controller class' => 'EntityAPIController',
      'base table' => 'quiz_node_relationship',
      'entity keys' => array(
        'id' => 'qnr_id',
      ),
      'views controller class' => 'EntityDefaultViewsController',
    ),
    'quiz' => array(
      'label' => t('Quiz properties'),
      'controller class' => 'EntityAPIController',
      'base table' => 'quiz_node_properties',
      'entity keys' => array(
        'id' => 'qnp_id',
      ),
      'views controller class' => 'EntityDefaultViewsController',
    ),
  );
}

/**
 * Implements hook_entity_info_alter().
 */
function quiz_entity_info_alter(&$info) {
  // Add a new view mode for displaying the question when taking a quiz
  $info['node']['view modes'] += array(
    'question' => array(
      'label' => t('Question'),
      'custom settings' => TRUE,
    ),
  );
}

/**
 * Implements hook_entity_property_info_alter().
 *
 * Define our special schema fields and relationships.
 */
function quiz_entity_property_info_alter(&$info) {
  $info['quiz_question_relationship']['properties']['child_nid']['type'] = 'node';
  $info['quiz_question_relationship']['properties']['child_vid']['type'] = 'integer';
  $info['quiz_question_relationship']['properties']['parent_nid']['type'] = 'node';
  $info['quiz_question_relationship']['properties']['parent_vid']['type'] = 'integer';

  $info['quiz_result_answer']['properties']['result_id']['type'] = 'quiz_result';
  $info['quiz_result_answer']['properties']['question_nid']['type'] = 'node';
  $info['quiz_result_answer']['properties']['is_correct']['type'] = 'boolean';
  $info['quiz_result_answer']['properties']['is_skipped']['type'] = 'boolean';
  $info['quiz_result_answer']['properties']['is_doubtful']['type'] = 'boolean';
  $info['quiz_result_answer']['properties']['answer_timestamp']['type'] = 'date';

  $info['quiz_result']['properties']['nid']['type'] = 'node';
  $info['quiz_result']['properties']['vid']['type'] = 'integer';
  $info['quiz_result']['properties']['uid']['type'] = 'user';
  $info['quiz_result']['properties']['time_start']['label'] = 'Date started';
  $info['quiz_result']['properties']['time_start']['type'] = 'date';
  $info['quiz_result']['properties']['time_end']['label'] = 'Date finished';
  $info['quiz_result']['properties']['time_end']['type'] = 'date';
  $info['quiz_result']['properties']['released']['type'] = 'date';
  $info['quiz_result']['properties']['score']['label'] = 'Score';
  $info['quiz_result']['properties']['score']['type'] = 'integer';
  $info['quiz_result']['properties']['is_invalid']['label'] = 'Invalid';
  $info['quiz_result']['properties']['is_invalid']['type'] = 'boolean';
  $info['quiz_result']['properties']['is_evaluated']['label'] = 'Evaluated';
  $info['quiz_result']['properties']['is_evaluated']['type'] = 'boolean';
  $info['quiz_result']['properties']['time_left']['type'] = 'duration';

  $info['quiz']['properties']['quiz_open']['label'] = 'Open date';
  $info['quiz']['properties']['quiz_open']['type'] = 'date';
  $info['quiz']['properties']['quiz_close']['label'] = 'Close date';
  $info['quiz']['properties']['quiz_close']['type'] = 'date';
}

/**
 * Implements hook_field_extra_fields().
 */
function quiz_field_extra_fields() {
  $extra['node']['quiz'] = array(
    'display' => array(
      'take' => array(
        'label' => t('Take quiz button'),
        'description' => t('The take button.'),
        'weight' => 10,
      ),
      'stats' => array(
        'label' => t('Quiz summary'),
        'description' => t('Quiz summary'),
        'weight' => 9,
      ),
    ),
    'form' => array(
      'taking' => array(
        'label' => t('Taking options'),
        'description' => t('Fieldset for customizing how a quiz is taken'),
        'weight' => 0,
      ),
      'quiz_availability' => array(
        'label' => t('Availability options'),
        'description' => t('Fieldset for customizing when a quiz is available'),
        'weight' => 0,
      ),
      'summaryoptions' => array(
        'label' => t('Summary options'),
        'description' => t('Fieldset for customizing summaries in the quiz reports'),
        'weight' => 0,
      ),
      'resultoptions' => array(
        'label' => t('Result options'),
        'description' => t('Fieldset for customizing result comments in quiz reports'),
        'weight' => 0,
      ),
      'remember_settings' => array(
        'label' => t('Remember settings'),
        'description' => t('Checkbox for remembering quiz settings'),
        'weight' => 0,
      ),
      'remember_global' => array(
        'label' => t('Remember as global'),
        'description' => t('Checkbox for remembering quiz settings'),
        'weight' => 0,
      ),
    ),
  );
  return $extra;
}

/**
 * Implements hook_permission().
 */
function quiz_permission() {
  return array(
    // Configure quiz:
    'administer quiz configuration' => array(
      'title' => t('Administer quiz configuration'),
      'description' => t('Control the various settings and behaviours of quiz'),
      'restrict access' => TRUE,
    ),
    // Managing quizzes:
    'access quiz' => array(
      'title' => t('Take quiz'),
      'description' => t('Can access (take) all quizzes.'),
    ),
    // viewing results:
    'view any quiz results' => array(
      'title' => t('View any quiz results'),
      'description' => t('Can view results for all quizzes and users.'),
    ),
    'view own quiz results' => array(
      'title' => t('View own quiz results'),
      'description' => t('Quiz takers can view their own results, also when quiz is not passed.'),
    ),
    'view results for own quiz' => array(
      'title' => t('View results for own quiz'),
      'description' => t('Quiz makers can view results for their own quizzes.'),
    ),
    // deleting results:
    'delete any quiz results' => array(
      'title' => t('Delete any quiz results'),
    ),
    'delete results for own quiz' => array(
      'title' => t('Delete own quiz results'),
    ),
    // scoring:
    'score any quiz' => array(
      'title' => t('Score any quiz'),
    ),
    'score own quiz' => array(
      'title' => t('Score own quiz'),
    ),
    'score taken quiz answer' => array(
      'title' => t('score taken quiz answer'),
      'description' => t('Allows attendee to score questions needing manual evaluation.'),
    ),
    // Allow a quiz question to be viewed outside of a test.
    'view quiz question outside of a quiz' => array(
      'title' => t('View quiz question outside of a quiz'),
      'description' => t('Questions can only be accessed through taking a quiz (not as individual nodes) unless this permission is given.'),
    ),
    // Allow the user to see the correct answer, when viewed outside a quiz
    'view any quiz question correct response' => array(
      'title' => t('View any quiz question correct response'),
      'description' => t('Allow the user to see the correct answer, when viewed outside a quiz.'),
    ),
    // Allows users to pick a name for their questions. Otherwise this is auto
    // generated.
    'edit question titles' => array(
      'title' => t('Edit question titles'),
      'description' => t('Questions automatically get a title based on the question text. This allows titles to be set manually.'),
    ),
    // Control revisioning, only assign this permission to users who understand
    // who permissions work. Note: If a quiz or question is changed and not
    // revisioned you will also change existing result reports.
    'manual quiz revisioning' => array(
      'title' => t('Manual quiz revisioning'),
      'description' => t('Quizzes are revisioned automatically each time they are changed. This allows you to do revisions manually.'),
    ),
  );
}

/**
 * Implements hook_admin_paths().
 */
function quiz_admin_paths() {
  return array(
    'node/*/quiz' => TRUE,
    'node/*/quiz/*' => TRUE,
  );
}

/**
 * Implements hook_action_info().
 */
function quiz_action_info() {
  $actions = array();

  $actions['quiz_add_question_to_quiz'] = array(
    'type' => 'node',
    'label' => t('Add questions to quiz'),
    'configurable' => FALSE,
    'vbo_configurable' => FALSE,
  );

  return $actions;
}

/**
 * Implementation of hook_cron().
 *
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function quiz_cron() {
  // Remove old quiz results that haven't been finished.
  $rm_time = variable_get('quiz_remove_partial_quiz_record', '0');

  if ($rm_time) { // $time = 0 for never.
    db_delete('quiz_node_results')
      ->condition('time_end', 0)
      ->where('(:request_time - time_start) > :remove_time', array(
        ':request_time' => REQUEST_TIME,
        ':remove_time' => $rm_time
      ))
      ->execute();
  }
}

/**
 * Implements hook_menu().
 */
function quiz_menu() {
  // Take quiz.
  $items['node/%quiz_menu/take'] = array(
    'title' => 'Take',
    'page callback' => 'quiz_take',
    'page arguments' => array(1),
    'access callback' => 'quiz_take_access',
    'access arguments' => array(1),
    'type' => MENU_LOCAL_TASK,
  );

  // Take question.
  // @todo Thought - the 4th argument could be a "page" instead of a question
  // number
  $items['node/%quiz_menu/take/%question_number'] = array(
    'title' => 'Take',
    'page callback' => 'quiz_take_question',
    'page arguments' => array(1, 3),
    'access callback' => 'quiz_take_question_access',
    'access arguments' => array(1, 3),
  );

  // Feedback
  $items['node/%quiz_menu/take/%question_number/feedback'] = array(
    'title' => 'Feedback',
    'page callback' => 'quiz_take_question_feedback',
    'page arguments' => array(1, 3),
    'access callback' => 'quiz_question_feedback_access',
    'access arguments' => array(1, 3),
  );

  // Admin pages.
  $items['admin/quiz'] = array(
    'title' => '@quiz',
    'title arguments' => array('@quiz' => QUIZ_NAME),
    'description' => 'View results, score answers, run reports and edit configurations.',
    'page callback' => 'system_admin_menu_block_page',
    'access arguments' => array('administer quiz configuration', 'score any quiz', 'score own quiz', 'view any quiz results', 'view results for own quiz'),
    'access callback' => 'quiz_access_multi_or',
    'type' => MENU_NORMAL_ITEM,
    'file' => 'system.admin.inc',
    'file path' => drupal_get_path('module', 'system'),
  );

  $items['admin/quiz/settings'] = array(
    'title' => '@quiz settings',
    'title arguments' => array('@quiz' => QUIZ_NAME),
    'description' => 'Change settings for the all Quiz project modules.',
    'page callback' => 'system_admin_menu_block_page',
    'access arguments' => array('administer quiz configuration'),
    'type' => MENU_NORMAL_ITEM,
    'file' => 'system.admin.inc',
    'file path' => drupal_get_path('module', 'system'),
  );

  $items['admin/quiz/settings/config'] = array(
    'title' => '@quiz configuration',
    'title arguments' => array('@quiz' => QUIZ_NAME),
    'description' => 'Configure the Quiz module.',
    'page callback' => 'drupal_get_form',
    'page arguments' => array('quiz_admin_settings'),
    'access arguments' => array('administer quiz configuration'),
    'type' => MENU_NORMAL_ITEM, // optional
    'file' => 'quiz.admin.inc',
  );

  $items['admin/quiz/settings/quiz_form'] = array(
    'title' => '@quiz form configuration',
    'title arguments' => array('@quiz' => QUIZ_NAME),
    'description' => 'Configure default values for the quiz creation form.',
    'page callback' => 'drupal_get_form',
    'page arguments' => array('quiz_admin_node_form'),
    'access arguments' => array('administer quiz configuration'),
    'type' => MENU_NORMAL_ITEM, // optional
    'file' => 'quiz.admin.inc',
  );

  $items['admin/quiz/reports'] = array(
    'title' => 'Quiz reports and scoring',
    'title arguments' => array('@quiz' => QUIZ_NAME),
    'description' => 'View reports and score answers.',
    'page callback' => 'system_admin_menu_block_page',
    'access arguments' => array('view any quiz results', 'view results for own quiz'),
    'access callback' => 'quiz_access_multi_or',
    'type' => MENU_NORMAL_ITEM,
    'file' => 'system.admin.inc',
    'file path' => drupal_get_path('module', 'system'),
  );

  $items['node/%quiz_menu/quiz-results/%quiz_rid/view'] = array(
    'title' => 'User results',
    'page callback' => 'quiz_user_results',
    'page arguments' => array(3),
    'access callback' => 'quiz_access_my_result',
    'access arguments' => array(3),
    'file' => 'quiz.pages.inc',
  );

  $items['node/%quiz_menu/quiz/results/%quiz_rid/view'] = array(
    'title' => 'Results',
    'page callback' => 'quiz_admin_results',
    'page arguments' => array(1, 4),
    'access callback' => 'quiz_access_results',
    'access arguments' => array(1, 4),
    'file' => 'quiz.admin.inc',
  );

  // Add questions to quiz.
  $items['node/%quiz_menu/quiz/questions'] = array(
    'title' => 'Manage questions',
    'page callback' => 'quiz_questions',
    'page arguments' => array(1),
    'access callback' => 'quiz_type_confirm',
    'access arguments' => array(1, 'update'),
    'type' => MENU_LOCAL_TASK,
    'file' => 'quiz.admin.inc',
    'weight' => 2,
  );

  // User pages.
  $items['user/%/quiz-results/%/view'] = array(
    'title' => 'User results',
    'page callback' => 'quiz_user_results',
    'page arguments' => array(3),
    'access arguments' => array(3),
    'access callback' => 'quiz_access_my_result',
    'type' => MENU_CALLBACK,
    'file' => 'quiz.pages.inc',
  );

  $items['node/%quiz_menu/questions/term_ahah'] = array(//TODO: Add node access instead of user access...
    'page callback' => 'quiz_categorized_term_ahah',
    'type' => MENU_CALLBACK,
    'access callback' => 'node_access',
    'access arguments' => array('create', 'quiz'),
    'file' => 'quiz.admin.inc',
  );

  return $items;
}

/**
 * Implements hook_theme().
 */
function quiz_theme($existing, $type, $theme, $path) {
  return array(
    'quiz_view_stats' => array(
      'variables' => array('node' => NULL),
      'file' => 'quiz.pages.inc',
    ),
    'quiz_categorized_form' => array(
      'render element' => 'form',
      'file' => 'quiz.admin.inc'
    ),
    'quiz_result' => array(
      'variables' => array('quiz' => NULL, 'questions' => NULL, 'score' => NULL, 'summary' => NULL, 'result_id' => NULL),
      'file' => 'quiz.pages.inc',
    ),
    'quiz_progress' => array(
      'variables' => array('question_number' => NULL, 'num_questions' => NULL, 'allow_jumping' => NULL, 'time_limit' => NULL),
      'file' => 'quiz.pages.inc',
    ),
    'quiz_no_feedback' => array(
      'file' => 'quiz.pages.inc',
      'variables' => array(),
    ),
    'question_selection_table' => array(
      'file' => 'quiz.admin.inc',
      'render element' => 'form',
    ),
    'quiz_answer_result' => array(
      'file' => 'quiz.pages.inc',
      'variables' => array(),
    ),
    'quiz_report_form' => array(
      'render element' => 'form',
      'path' => $path . '/theme',
      'template' => 'quiz-report-form',
    ),
    'quiz_node_form' => array(
      'render element' => 'form',
      'file' => 'quiz.admin.inc',
    ),
    'quiz_browser' => array(
      'render element' => 'form',
      'file' => 'quiz.admin.inc',
    ),
    'quiz_jumper' => array(
      'variables' => array('current' => 0, 'num_questions' => 0),
      'file' => 'quiz.admin.inc',
    ),
    'quiz_question_score' => array(
      'variables' => array('score' => NULL, 'max_score' => NULL),
      'path' => $path . '/theme',
      'template' => 'quiz-question-score',
    ),
  );
}

/**
 * Implements hook_form_alter().
 *
 * Override settings in some existing forms. For example, we remove the
 * preview button on a quiz.
 */
function quiz_form_alter(&$form, $form_state, $form_id) {
  // Set taking options the first vertical tab item
  if ('quiz_node_form' == $form_id && isset($form['menu']['#weight'])) {
    $form['menu']['#weight'] = 1;
  }
  // Remove revision fieldset if user don't have access to revise quiz manually.
  if (isset($form['#quiz_check_revision_access'])) {
    if (!user_access('manual quiz revisioning') || variable_get('quiz_auto_revisioning', 1)) {
      $form['revision_information']['revision']['#type'] = 'value';
      $form['revision_information']['revision']['#value'] = $form['revision_information']['revision']['#default_value'];
      $form['revision_information']['log']['#type'] = 'value';
      $form['revision_information']['log']['#value'] = $form['revision_information']['log']['#default_value'];
      $form['revision_information']['#access'] = FALSE;
    }
    unset($form['actions']['preview'], $form['actions']['preview_changes']);
    $form['actions']['submit']['#access'] = TRUE;
    // Quiz questions might want to add a cancel button.
    if (isset($form['#cancel_button'])) {
      $form['actions']['cancel'] = array(
        '#markup' => l(t('Cancel'), $form_state['redirect']),
        '#weight' => 6,
      );
    }
  }
}

//function quiz_element_info() {
//  $types['advancedtable'] = array(
//    '#process' => array('form_process_fieldset', 'ajax_process_form'),
//    '#theme_wrappers' => array('quiz_table'),
//  );
//  $types['advancedtable_header'] = array(
//    '#process' => array('form_process_fieldset', 'ajax_process_form'),
//    '#theme_wrappers' => array('quiz_table_header'),
//  );
//  $types['advancedtable_body'] = array(
//    '#process' => array('form_process_fieldset', 'ajax_process_form'),
//    '#theme_wrappers' => array('quiz_table_body'),
//  );
//  return $types;
//}

/**
 * Implements hook_form_FORM_ID_alter() for the quiz node form.
 */
function quiz_form_quiz_node_form_alter(&$form, $form_state) {
  $form['actions']['submit']['#submit'][] = 'quiz_form_quiz_node_form_submit';
}

/**
 * Implements hook_delete().
 */
function quiz_delete($node) {
  $res = db_query('SELECT result_id FROM {quiz_node_results}
          WHERE nid = :nid', array(':nid' => $node->nid));
  $result_ids = array();
  while ($result_id = $res->fetchField()) {
    $result_ids[] = $result_id;
  }
  quiz_delete_results($result_ids);

  // Remove quiz node records from table quiz_node_properties
  db_delete('quiz_node_properties')
    ->condition('nid', $node->nid)
    ->execute();
  // Remove quiz node records from table quiz_node_relationship
  db_delete('quiz_node_relationship')
    ->condition('parent_nid', $node->nid)
    ->execute();
  // Remove quiz node records from table quiz_node_results
  db_delete('quiz_node_results')
    ->condition('nid', $node->nid)
    ->execute();
  // Remove quiz node records from table quiz_node_result_options
  db_delete('quiz_node_result_options')
    ->condition('nid', $node->nid)
    ->execute();
}

/**
 * Implements hook_validate().
 */
function quiz_validate($node) {
  // Don't check dates if the quiz is always available.
  if (!$node->quiz_always) {
    if (mktime(0, 0, 0, $node->quiz_open['month'], $node->quiz_open['day'], $node->quiz_open['year']) > mktime(0, 0, 0, $node->quiz_close['month'], $node->quiz_close['day'], $node->quiz_close['year'])) {
      form_set_error('quiz_close', t('Please make sure the close date is after the open date.'));
    }
  }

  if (!empty($node->pass_rate)) {
    if (!_quiz_is_int($node->pass_rate, 0, 100)) {
      form_set_error('pass_rate', t('The pass rate value must be a number between 0 and 100.'));
    }
  }

  if (isset($node->time_limit)) {
    if (!_quiz_is_int($node->time_limit, 0)) {
      form_set_error('time_limit', t('Time limit must be a non negative interger'));
    }
  }

  if (isset($node->resultoptions) && count($node->resultoptions) > 0) {
    $taken_values = array();
    $num_options = 0;
    foreach ($node->resultoptions as $option) {
      if (!empty($option['option_name'])) {
        $num_options++;
        if (empty($option['option_summary'])) {
          form_set_error('option_summary', t('Option has no summary text.'));
        }
        if ($node->pass_rate && (isset($option['option_start']) || isset($option['option_end']))) {

          // Check for a number between 0-100.
          foreach (array('option_start' => 'start', 'option_end' => 'end') as $bound => $bound_text) {
            if (!_quiz_is_int($option[$bound], 0, 100)) {
              form_set_error($bound, t('The range %start value must be a number between 0 and 100.', array('%start' => $bound_text)));
            }
          }

          // Check that range end >= start.
          if ($option['option_start'] > $option['option_end']) {
            form_set_error('option_start', t('The start must be less than the end of the range.'));
          }

          // Check that range doesn't collide with any other range.
          $option_range = range($option['option_start'], $option['option_end']);
          if ($intersect = array_intersect($taken_values, $option_range)) {
            form_set_error('option_start', t('The ranges must not overlap each other. (%intersect)', array('%intersect' => implode(',', $intersect))));
          }
          else {
            $taken_values = array_merge($taken_values, $option_range);
          }
        }
      }
      elseif (!_quiz_is_empty_html($option['option_summary']['value'])) {
        form_set_error('option_summary', t('Option has a summary, but no name.'));
      }
    }
  }

  if ($node->allow_jumping && !$node->allow_skipping) {
    // @todo when we have pages of questions, we have to check that jumping is
    // not enabled, and randomization is not enabled unless there is only 1 page
    form_set_error('allow_skipping', t('If jumping is allowed skipping also has to be allowed.'));
  }
}

/**
 * Implements hook_insert().
 */
function quiz_insert($node) {
  global $user;

  // Need to set max_score if this is a cloned node
  $max_score = 0;

  // Copy all the questions belonging to the quiz if this is a new translation.
  if ($node->is_new && isset($node->translation_source)) {
    quiz_copy_questions($node);
  }

  // Add references to all the questions belonging to the quiz if this is a cloned quiz (node_clone compatibility)
  if ($node->is_new && isset($node->clone_from_original_nid)) {
    $old_quiz = node_load($node->clone_from_original_nid, NULL, TRUE);

    $max_score = $old_quiz->max_score;

    $questions = quiz_get_questions($old_quiz->nid, $old_quiz->vid);

    // Format the current questions for referencing
    foreach ($questions as $question) {
      $nid = $questions['nid'];
      $questions[$nid]->state = $question->question_status;
      $questions[$nid]->refresh = 0;
    }

    quiz_set_questions($node, $questions);
  }

  _quiz_common_presave_actions($node);

  // If the quiz is saved as not randomized we have to make sure that questions belonging to the quiz are saved as not random
  _quiz_check_num_random($node);
  _quiz_check_num_always($node);

  quiz_update_defaults($node);
  _quiz_insert_resultoptions($node);
}

/**
 * Implements hook_update().
 */
function quiz_update($node) {
  // Quiz node vid (revision) was updated.
  if (isset($node->revision) && $node->revision) {
    // Create new quiz-question relation entries in the quiz_node_relationship
    // table.
    quiz_update_quiz_question_relationship($node->old_vid, $node->vid, $node->nid);
  }

  // Update an existing row in the quiz_node_properties table.
  _quiz_common_presave_actions($node);

  quiz_update_defaults($node);
  _quiz_update_resultoptions($node);

  _quiz_check_num_random($node);
  _quiz_check_num_always($node);
  quiz_update_max_score_properties(array($node->vid));
  drupal_set_message(t('Some of the updated settings may not apply to quiz being taken already. To see all changes in action you need to start again.'), 'warning');
}

/**
 * Implementation of hook_node_presave().
 */
function quiz_node_presave($node) {
  if ($node->type == 'quiz') {
    // Convert the action id to the actual id from the MD5 hash.
    // Why the actions module does this I do not know? Maybe to prevent
    // invalid values put into the options value="" field.
    if (!empty($node->aid) && $aid = actions_function_lookup($node->aid)) {
      $node->aid = $aid;
    }

    if (variable_get('quiz_auto_revisioning', 1)) {
      $node->revision = (quiz_has_been_answered($node)) ? 1 : 0;
    }

    // If this is a programmatic save, ensure we use the defaults.
    $defaults = quiz_get_defaults();
    foreach ($defaults as $property => $value) {
      if (!isset($node->$property)) {
        $node->$property = $defaults->$property;
      }
    }
  }
  if (isset($node->is_quiz_question) && variable_get('quiz_auto_revisioning', 1)) {
    $node->revision = (quiz_question_has_been_answered($node)) ? 1 : 0;
  }
}

/**
 * Implementation of hook_node_prepare().
 */
function quiz_node_prepare($node) {
  if ($node->type == 'quiz' && !isset($node->nid)) {
    // If this is a new node we apply the user defaults for the quiz settings.
    if (arg(0) == 'node') {
      drupal_set_message(t('You are making your first @quiz. On this page you set the attributes, most of which you may tell the system to remember as defaults for the future. On the next screen you can add questions.', array('@quiz' => QUIZ_NAME)));
    }
    $settings = quiz_get_defaults();
    foreach ($settings as $key => $value) {
      if (!isset($node->$key)) {
        $node->$key = $value;
      }
    }
  }

  if (isset($node->is_quiz_question)) {
    if (variable_get('quiz_auto_revisioning', 1)) {
      $node->revision = (quiz_question_has_been_answered($node)) ? 1 : 0;
    }
  }
}

/**
 * Implements hook_load().
 *
 * Load up quiz properties onto quiz nodes.
 */
function quiz_load($nodes) {
  foreach ($nodes as $node) {
    $vids[] = $node->vid;
  }

  $quiz_properties = entity_load('quiz', FALSE, array('vid' => $vids));

  foreach ($quiz_properties as $qnp_id => $props) {
    foreach ($props as $name => $value) {
      if (!in_array($name, array('uid', 'nid', 'vid'))) {
        $nodes[$props->nid]->$name = $value;
      }
    }
  }

  // Fetching result options.
  $query = db_select('quiz_node_result_options', 'qnro');
  $query->fields('qnro');
  $query->condition('vid', $vids);
  $options = $query->execute();
  foreach ($options as $option) {
    $nodes[$option->nid]->resultoptions[$option->option_id] = (array) $option;
  }
}

/**
 * Implements hook_view().
 */
function quiz_view($node, $view_mode) {
  drupal_alter('quiz_view', $node, $view_mode);
  node_invoke($node, 'prepare');

  // Number of questions is needed on the statistics page.
  $node->number_of_questions = $node->number_of_random_questions + _quiz_get_num_always_questions($node->vid);

  $node->content['stats'] = array(
    '#markup' => theme('quiz_view_stats', array('node' => $node)),
    '#weight' => -1,
  );

  $available = quiz_availability($node);
  if ($available === TRUE) {
    // Check the permission before displaying start button.
    if (user_access('access quiz')) {
      // Add a link to the take tab as a button if this isn't a teaser view.
      if ($view_mode !== 'teaser') {
        $quiz_form = drupal_get_form('quiz_start_quiz_button_form', $node);
        $node->content['take'] = array(
          '#markup' => drupal_render($quiz_form),
          '#weight' => 2,
        );
      }
      // Add a link to the take tab if this is a teaser view.
      else {
        $node->content['take'] = array(
          '#markup' => l(t('Start quiz'), 'node/' . $node->nid . '/take'),
          '#weight' => 2,
        );
      }
    }
  }
  else {
    $node->content['take'] = array(
      '#markup' => '<div class="quiz-not-available">' . $available . '</div>',
      '#weight' => 2,
    );
  }

  return $node;
}

/**
 * Implements hook_user_cancel().
 */
function quiz_user_cancel($edit, $account, $method) {
  if (variable_get('quiz_durod', 0)) {
    _quiz_delete_users_results($account->uid);
  }
}

/**
 * Implements hook_form().
 *
 * This is an admin form used to build a new quiz. It is called as part of the
 * node edit form.
 */
function quiz_form(&$node, &$form_state) {
  $form = array();

  // We tell quiz_form_alter to check for the manual revisioning permission.
  $form['#quiz_check_revision_access'] = TRUE;

  $form['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Title'),
    '#default_value' => isset($node->title) ? $node->title : '',
    '#description' => t('The name of the @quiz.', array('@quiz' => QUIZ_NAME)),
    '#required' => TRUE,
  );

  $form['taking'] = array(
    '#type' => 'fieldset',
    '#title' => t('Taking options'),
    '#collapsed' => isset($settings_loaded) ? $settings_loaded : FALSE,
    '#collapsible' => TRUE,
    '#attributes' => array('id' => 'taking-fieldset'),
    '#group' => 'additional_settings',
    '#weight' => -2,
  );
  $form['taking']['allow_resume'] = array(
    '#type' => 'checkbox',
    '#title' => t('Allow Resume'),
    '#default_value' => $node->allow_resume,
    '#description' => t('Whether to allow users to leave the @quiz incomplete and then resume it from where they left off.', array('@quiz' => QUIZ_NAME)),
  );
  $form['taking']['allow_skipping'] = array(
    '#type' => 'checkbox',
    '#title' => t('Allow Skipping questions'),
    '#default_value' => $node->allow_skipping,
    '#description' => t('Whether to allow users to skip questions in the @quiz', array('@quiz' => QUIZ_NAME)),
  );
  $form['taking']['allow_jumping'] = array(
    '#type' => 'checkbox',
    '#title' => t('Allow jumping'),
    '#default_value' => $node->allow_jumping,
    '#description' => t('Whether to allow users to jump between questions using a menu in the @quiz', array('@quiz' => QUIZ_NAME)),
  );
  $form['taking']['backwards_navigation'] = array(
    '#type' => 'checkbox',
    '#title' => t('Backwards navigation'),
    '#default_value' => $node->backwards_navigation,
    '#description' => t('Whether to allow user to go back and revisit their answers'),
  );
  $form['taking']['repeat_until_correct'] = array(
    '#type' => 'checkbox',
    '#title' => t('Repeat until correct'),
    '#default_value' => $node->repeat_until_correct,
    '#description' => t('Require the user to re-try the question until they answer it correctly.'),
  );
  $form['taking']['mark_doubtful'] = array(
    '#type' => 'checkbox',
    '#title' => t('Mark Doubtful'),
    '#default_value' => $node->mark_doubtful,
    '#description' => t('Allow user to mention if they are not sure about the answer'),
  );
  $form['taking']['show_passed'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show passed status'),
    '#default_value' => $node->show_passed,
    '#description' => t('Show the status, if the user has previously passed'),
  );

  $form['taking']['randomization'] = array(
    '#type' => 'radios',
    '#title' => t('Randomize questions'),
    '#options' => array(
      t('No randomization'),
      t('Random order'),
      t('Random questions'),
      t('Categorized random questions'),
    ),
    '#description' => t('The difference between "random order" and "random questions" is that with "random questions" questions are drawn randomly from a pool of questions. With "random order" the quiz will always consist of the same questions. With "Categorized random questions" you can choose several terms questions should be drawn from, and you can also choose how many questions that should be drawn from each, and max score for each term.'),
    '#default_value' => $node->randomization,
  );
  $form['taking']['review_options'] = array(
    '#type' => 'fieldset',
    '#title' => t('Review options'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
    '#tree' => TRUE,
  );

  $review_options = quiz_get_feedback_options();

  foreach (array('question' => 'After the question', 'end' => 'After the quiz') as $key => $when) {
    $form['taking']['review_options'][$key] = array(
      '#title' => $when,
      '#type' => 'checkboxes',
      '#options' => $review_options,
      '#default_value' => isset($node->review_options[$key]) ? $node->review_options[$key] : array(),
    );
  }
  $options = array(t('Unlimited'));
  for ($i = 1; $i < 10; $i++) {
    $options[$i] = $i;
  }
  $form['taking']['multiple_takes'] = array(
    '#type' => 'fieldset',
    '#title' => t('Multiple takes'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
    '#attributes' => array('id' => 'multiple-takes-fieldset'),
  );
  $form['taking']['multiple_takes']['takes'] = array(
    '#type' => 'select',
    '#title' => t('Allowed number of attempts'),
    '#default_value' => $node->takes,
    '#options' => $options,
    '#description' => t('The number of times a user is allowed to take the @quiz. <strong>Anonymous users are only allowed to take quizzes that allow an unlimited number of attempts.</strong>', array('@quiz' => QUIZ_NAME)),
  );
  $form['taking']['multiple_takes']['show_attempt_stats'] = array(
    '#type' => 'checkbox',
    '#title' => t('Display allowed number of attempts'),
    '#default_value' => $node->show_attempt_stats,
    '#description' => t('Display the allowed number of attempts on the starting page for this quiz.'),
  );

  if (user_access('delete any quiz results') || user_access('delete results for own quiz')) {
    $form['taking']['multiple_takes']['keep_results'] = array(
      '#type' => 'radios',
      '#title' => t('These results should be stored for each user'),
      '#options' => array(
        t('The best'),
        t('The newest'),
        t('All'),
      ),
      '#default_value' => $node->keep_results,
    );
  }
  else {
    $form['taking']['multiple_takes']['keep_results'] = array(
      '#type' => 'value',
      '#value' => $node->keep_results,
    );
  }

  if (function_exists('jquery_countdown_add') && variable_get('quiz_has_timer', 0)) {
    $form['taking']['addons'] = array(
      '#type' => 'fieldset',
      '#title' => t('Quiz Addons Properties'),
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
    );
    $form['taking']['addons']['time_limit'] = array(
      '#type' => 'textfield',
      '#title' => t(' Time Limit'),
      '#default_value' => isset($node->time_limit) ? $node->time_limit : 0,
      '#description' => t('Set the maximum allowed time in seconds for this @quiz. Use 0 for no limit.', array('@quiz' => QUIZ_NAME)),
    );
  }
  else {
    $form['taking']['addons']['time_limit'] = array(
      '#type' => 'value',
      '#value' => 0,
    );
  }

  if (function_exists('userpoints_userpointsapi') && variable_get('quiz_has_userpoints', 1)) {
    $form['userpoints'] = array(
      '#type' => 'fieldset',
      '#title' => t('Userpoints'),
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
      '#group' => 'additional_settings',
    );
    $form['userpoints']['has_userpoints'] = array(
      '#type' => 'checkbox',
      '#default_value' => (isset($node->has_userpoints) ? $node->has_userpoints : 1),
      '#title' => t('Enable UserPoints Module Integration'),
      '#description' => t('If checked, marks scored in this @quiz will be credited to userpoints. For each correct answer 1 point will be added to user\'s point.', array('@quiz' => QUIZ_NAME)),
    );
    $form['userpoints']['userpoints_tid'] = array(
      '#type' => 'select',
      '#options' => _quiz_userpoints_type(),
      '#title' => t('Userpoints Category'),
      '#states' => array(
        'visible' => array(
          ':input[name=has_userpoints]' => array('checked' => TRUE),
        ),
      ),
      '#default_value' => isset($node->userpoints_tid) ? $node->userpoints_tid : 0,
      '#description' => t('Select the category to which user points to be added. To add new category see <a href="!url">admin/structure/taxonomy/userpoints</a>', array('!url' => url('admin/structure/taxonomy/userpoints'))),
    );
  }

  // Set up the availability options.
  $form['quiz_availability'] = array(
    '#type' => 'fieldset',
    '#title' => t('Availability options'),
    '#collapsed' => TRUE,
    '#collapsible' => TRUE,
    '#attributes' => array('id' => 'availability-fieldset'),
    '#group' => 'additional_settings',
  );
  $form['quiz_availability']['quiz_always'] = array(
    '#type' => 'checkbox',
    '#title' => t('Always Available'),
    '#default_value' => $node->quiz_always,
    '#description' => t('Click this option to ignore the open and close dates.'),
  );
  $form['quiz_availability']['quiz_open'] = array(
    '#type' => 'date',
    '#title' => t('Open Date'),
    '#default_value' => _quiz_form_prepare_date($node->quiz_open),
    '#description' => t('The date this @quiz will become available.', array('@quiz' => QUIZ_NAME)),
    '#after_build' => array('_quiz_limit_year_options'),
  );
  $form['quiz_availability']['quiz_close'] = array(
    '#type' => 'date',
    '#title' => t('Close Date'),
    '#default_value' => _quiz_form_prepare_date($node->quiz_close, variable_get('quiz_default_close', 30)),
    '#description' => t('The date this @quiz will cease to be available.', array('@quiz' => QUIZ_NAME)),
    '#after_build' => array('_quiz_limit_year_options'),
  );

  // Quiz summary options.
  $form['summaryoptions'] = array(
    '#type' => 'fieldset',
    '#title' => t('Pass/fail options'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#attributes' => array('id' => 'summaryoptions-fieldset'),
    '#group' => 'additional_settings',
  );
  // If pass/fail option is checked, present the form elements.
  if (variable_get('quiz_use_passfail', 1)) {
    $form['summaryoptions']['pass_rate'] = array(
      '#type' => 'textfield',
      '#title' => t('Pass rate for @quiz (%)', array('@quiz' => QUIZ_NAME)),
      '#default_value' => $node->pass_rate,
      '#description' => t('Pass rate for the @quiz as a percentage score.', array('@quiz' => QUIZ_NAME)),
      '#required' => FALSE,
    );
    $form['summaryoptions']['summary_pass'] = array(
      '#type' => 'text_format',
      '#base_type' => 'textarea',
      '#title' => t('Summary text if passed'),
      '#default_value' => $node->summary_pass,
      '#cols' => 60,
      '#description' => t("Summary for when the user gets enough correct answers to pass the @quiz. Leave blank if you don't want to give different summary text if they passed or if you are not using the 'percent to pass' option above. If you don't use the 'Percentage needed to pass' field above, this text will not be used.", array('@quiz' => QUIZ_NAME)),
      '#format' => isset($node->summary_pass_format) && !empty($node->summary_pass_format) ? $node->summary_pass_format : NULL,
    );
  }
  // If the pass/fail option is unchecked, use the default and hide it.
  else {
    $form['summaryoptions']['pass_rate'] = array(
      '#type' => 'hidden',
      '#value' => $node->pass_rate,
      '#required' => FALSE,
    );
  }
  // We use a helper to enable the wysiwyg module to add an editor to the
  // textarea.
  $form['summaryoptions']['helper']['summary_default'] = array(
    '#type' => 'text_format',
    '#base_type' => 'textarea',
    '#title' => t('Default summary text'),
    '#default_value' => $node->summary_default,
    '#cols' => 60,
    '#description' => t("Default summary. Leave blank if you don't want to give a summary."),
    '#format' => isset($node->summary_default_format) && !empty($node->summary_default_format) ? $node->summary_default_format : NULL,
  );

  // Number of random questions, max score and tid for random questions are set on
  // the manage questions tab. We repeat them here so that they're not removed
  // if the quiz is being updated.
  $num_rand = (isset($node->number_of_random_questions)) ? $node->number_of_random_questions : 0;
  $form['number_of_random_questions'] = array(
    '#type' => 'value',
    '#value' => $num_rand,
  );
  $max_score_for_random = (isset($node->max_score_for_random)) ? $node->max_score_for_random : 0;
  $form['max_score_for_random'] = array(
    '#type' => 'value',
    '#value' => $max_score_for_random,
  );
  $tid = (isset($node->tid)) ? $node->tid : 0;
  $form['tid'] = array(
    '#type' => 'value',
    '#value' => $tid,
  );

  $options = !empty($node->resultoptions) ? $node->resultoptions : array();
  $num_options = max(count($options), variable_get('quiz_max_result_options', 5));

  if ($num_options > 0) {
    $form['resultoptions'] = array(
      '#type' => 'fieldset',
      '#title' => t('Result Comments'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
      '#tree' => TRUE,
      '#attributes' => array('id' => 'resultoptions-fieldset'),
      '#group' => 'additional_settings',
    );

    for ($i = 0; $i < $num_options; $i++) {
      $option = (count($options) > 0) ? array_shift($options) : NULL; // grab each option in the array
      $form['resultoptions'][$i] = array(
        '#type' => 'fieldset',
        '#title' => t('Result Option ') . ($i + 1),
        '#collapsible' => TRUE,
        '#collapsed' => FALSE,
      );
      $form['resultoptions'][$i]['option_name'] = array(
        '#type' => 'textfield',
        '#title' => t('The name of the result'),
        '#default_value' => isset($option['option_name']) ? $option['option_name'] : '',
        '#maxlength' => 40,
        '#size' => 40,
      );
      $form['resultoptions'][$i]['option_start'] = array(
        '#type' => 'textfield',
        '#title' => t('Percentage Start Range'),
        '#description' => t('Show this result for scored quizzes in this range (0-100).'),
        '#default_value' => isset($option['option_start']) ? $option['option_start'] : '',
        '#size' => 5,
      );
      $form['resultoptions'][$i]['option_end'] = array(
        '#type' => 'textfield',
        '#title' => t('Percentage End Range'),
        '#description' => t('Show this result for scored quizzes in this range (0-100).'),
        '#default_value' => isset($option['option_end']) ? $option['option_end'] : '',
        '#size' => 5,
      );
      $form['resultoptions'][$i]['option_summary'] = array(
        '#type' => 'text_format',
        '#base_type' => 'textarea',
        '#title' => t('Display text for the result'),
        '#default_value' => isset($option['option_summary']) ? $option['option_summary'] : '',
        '#description' => t('Result summary. This is the summary that is displayed when the user falls in this result set determined by his/her responses.'),
        '#format' => isset($option['option_summary_format']) ? $option['option_summary_format'] : NULL,
      );
      if (isset($option['option_id'])) {
        $form['resultoptions'][$i]['option_id'] = array(
          '#type' => 'hidden',
          '#value' => isset($option['option_id']) ? $option['option_id'] : '',
        );
      }
    }
  }

  $form['remember_settings'] = array(
    '#type' => 'checkbox',
    '#title' => t('Remember my settings'),
    '#description' => t('If this box is checked most of the quiz specific settings you have made will be remembered and will be your default settings next time you create a quiz.'),
    '#weight' => 49,
  );

  $form['remember_global'] = array(
    '#type' => 'checkbox',
    '#title' => t('Remember as global'),
    '#description' => t('If this box is checked most of the quiz specific settings you have made will be remembered and will be everyone\'s default settings next time you create a quiz.'),
    '#weight' => 49,
    '#access' => user_access('administer quiz configuration'),
  );

  if (quiz_has_been_answered($node) && (!user_access('manual quiz revisioning') || variable_get('quiz_auto_revisioning', 1))) {
    $node->revision = 1;
    $node->log = t('The current revision has been answered. We create a new revision so that the reports from the existing answers stays correct.');
  }
  return $form;
}

/**
 * Implements hook_mail().
 */
function quiz_mail($key, &$message, $params) {
  global $user;

  switch ($key) {
    case 'notice':
      list($quiz, $score, $result_id, $target) = $params;
      $quiz_body = field_get_items('node', $quiz, 'body');
      $substitutions = array(
        '!title' => $quiz->title,
        '!sitename' => variable_get('site_name', 'Quiz'),
        '!taker' => $user->name,
        '!author' => $quiz->name,
        '!title' => check_plain($quiz->title),
        '!date' => format_date(REQUEST_TIME),
        '!desc' => $quiz_body ? $quiz_body[0]['value'] : '',
        '!correct' => isset($score['numeric_score']) ? $score['numeric_score'] : 0,
        '!total' => $score['possible_score'],
        '!percentage' => $score['percentage_score'],
        '!url' => url("user/{$user->uid}/quiz-results/{$result_id}/view", array('absolute' => TRUE)),
        '!minutes' => db_query("SELECT CEIL((time_end - time_start)/60) FROM {quiz_node_results} WHERE result_id = :result_id AND time_end", array(':result_id' => $result_id))->fetchField()
      );
      $type = $target != 'author' ? '_taker' : '';
      $test = variable_get('quiz_email_results_body' . $type, quiz_email_results_format('body', $target));
      $message['subject'] = t(variable_get('quiz_email_results_subject' . $type, quiz_email_results_format('subject', $target)), $substitutions, array('langcode' => $user->language));
      $message['body'][] = t($test, $substitutions, array('langcode' => $user->language));
  }
}
