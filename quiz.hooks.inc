<?php

/**
 * @file
 * Quiz hook implementations and helper functions (not API).
 */

/**
 * Implements hook_help().
 */
function quiz_help($path, $arg) {
  switch ($path) {
    case 'admin/help#quiz':
      return t('

<p>The quiz module allows users to administer a quiz, as a sequence of questions, and track the answers given. It allows for the creation of questions (and their answers), and organizes these questions into a quiz. Its target audience includes educational institutions, online training programs, employers, and people who just want to add a fun activity for their visitors to their Drupal site.</p>

<p>The quiz module has a plethora of permission options. Unless you take care setting your permissions, the quiz module might not do everything you want it to do.</p>

<p>For more information about quiz, and resources on how to use quiz, see the <a href="http://drupal.org/project/quiz">Quiz project website</a></p>

    ', array('@quiz' => QUIZ_NAME, '@admin-access' => url('admin/user/permissions'), '@admin-modules' => url('admin/modules'), '@admin-taxonomy' => url('admin/structure/taxonomy'), '@create-content' => url('node/add'), '@multichoice' => url('node/add/multichoice'), '@create-quiz' => url('node/add/quiz')));
    case 'node/add#quiz':
      return t('A collection of questions designed to create interactive tests');
    default:
      break;
  }
}

/**
 * Implements hook_views_api().
 */
function quiz_views_api() {
  return array(
    'api'  => 2,
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
      'name'        => t('@quiz', array("@quiz" => QUIZ_NAME)),
      'base'        => 'quiz',
      'description' => t('Create interactive quizzes for site visitors'),
    )
  );
}

/**
 * Implements hook_entity_info().
 */
function quiz_entity_info() {
  return array(
    'quiz_result'                => array(
      'label'                  => t('Quiz result'),
      'controller class'       => 'EntityAPIController',
      'base table'             => 'quiz_node_results',
      'entity keys'            => array(
        'id' => 'result_id',
      ),
      'views controller class' => 'EntityDefaultViewsController',
    ),
    'quiz_result_answer'         => array(
      'label'                  => t('Quiz result answer'),
      'controller class'       => 'EntityAPIController',
      'base table'             => 'quiz_node_results_answers',
      'entity keys'            => array(
        'id' => 'result_answer_id',
      ),
      'views controller class' => 'EntityDefaultViewsController',
    ),
    'quiz_question_relationship' => array(
      'label'                  => t('Quiz question relationship'),
      'controller class'       => 'EntityAPIController',
      'base table'             => 'quiz_node_relationship',
      'entity keys'            => array(
        'id' => 'qnr_id',
      ),
      'views controller class' => 'EntityDefaultViewsController',
    ),
    'quiz'                       => array(
      'label'                  => t('Quiz properties'),
      'controller class'       => 'EntityAPIController',
      'base table'             => 'quiz_node_properties',
      'entity keys'            => array(
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
      'label'           => t('Question'),
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
      'take'  => array(
        'label'       => t('Take quiz button'),
        'description' => t('The take button.'),
        'weight'      => 10,
      ),
      'stats' => array(
        'label'       => t('Quiz summary'),
        'description' => t('Quiz summary'),
        'weight'      => 9,
      ),
    ),
    'form'    => array(
      'taking'            => array(
        'label'       => t('Taking options'),
        'description' => t('Fieldset for customizing how a quiz is taken'),
        'weight'      => 0,
      ),
      'quiz_availability' => array(
        'label'       => t('Availability options'),
        'description' => t('Fieldset for customizing when a quiz is available'),
        'weight'      => 0,
      ),
      'summaryoptions'    => array(
        'label'       => t('Summary options'),
        'description' => t('Fieldset for customizing summaries in the quiz reports'),
        'weight'      => 0,
      ),
      'resultoptions'     => array(
        'label'       => t('Result options'),
        'description' => t('Fieldset for customizing result comments in quiz reports'),
        'weight'      => 0,
      ),
      'remember_settings' => array(
        'label'       => t('Remember settings'),
        'description' => t('Checkbox for remembering quiz settings'),
        'weight'      => 0,
      ),
      'remember_global'   => array(
        'label'       => t('Remember as global'),
        'description' => t('Checkbox for remembering quiz settings'),
        'weight'      => 0,
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
    'administer quiz configuration'           => array(
      'title'           => t('Administer quiz configuration'),
      'description'     => t('Control the various settings and behaviours of quiz'),
      'restrict access' => TRUE,
    ),
    // Managing quizzes:
    'access quiz'                             => array(
      'title'       => t('Take quiz'),
      'description' => t('Can access (take) all quizzes.'),
    ),
    // viewing results:
    'view any quiz results'                   => array(
      'title'       => t('View any quiz results'),
      'description' => t('Can view results for all quizzes and users.'),
    ),
    'view own quiz results'                   => array(
      'title'       => t('View own quiz results'),
      'description' => t('Quiz takers can view their own results, also when quiz is not passed.'),
    ),
    'view results for own quiz'               => array(
      'title'       => t('View results for own quiz'),
      'description' => t('Quiz makers can view results for their own quizzes.'),
    ),
    // deleting results:
    'delete any quiz results'                 => array(
      'title' => t('Delete any quiz results'),
    ),
    'delete results for own quiz'             => array(
      'title' => t('Delete own quiz results'),
    ),
    // scoring:
    'score any quiz'                          => array(
      'title' => t('Score any quiz'),
    ),
    'score own quiz'                          => array(
      'title' => t('Score own quiz'),
    ),
    'score taken quiz answer'                 => array(
      'title'       => t('score taken quiz answer'),
      'description' => t('Allows attendee to score questions needing manual evaluation.'),
    ),
    // Allow a quiz question to be viewed outside of a test.
    'view quiz question outside of a quiz'    => array(
      'title'       => t('View quiz question outside of a quiz'),
      'description' => t('Questions can only be accessed through taking a quiz (not as individual nodes) unless this permission is given.'),
    ),
    // Allow the user to see the correct answer, when viewed outside a quiz
    'view any quiz question correct response' => array(
      'title'       => t('View any quiz question correct response'),
      'description' => t('Allow the user to see the correct answer, when viewed outside a quiz.'),
    ),
    // Allows users to pick a name for their questions. Otherwise this is auto
    // generated.
    'edit question titles'                    => array(
      'title'       => t('Edit question titles'),
      'description' => t('Questions automatically get a title based on the question text. This allows titles to be set manually.'),
    ),
    // Control revisioning, only assign this permission to users who understand
    // who permissions work. Note: If a quiz or question is changed and not
    // revisioned you will also change existing result reports.
    'manual quiz revisioning'                 => array(
      'title'       => t('Manual quiz revisioning'),
      'description' => t('Quizzes are revisioned automatically each time they are changed. This allows you to do revisions manually.'),
    ),
  );
}

/**
 * Implements hook_admin_paths().
 */
function quiz_admin_paths() {
  return array(
    'node/*/quiz'   => TRUE,
    'node/*/quiz/*' => TRUE,
  );
}

/**
 * Implements hook_action_info().
 */
function quiz_action_info() {
  $actions = array();

  $actions['quiz_add_question_to_quiz'] = array(
    'type'             => 'node',
    'label'            => t('Add questions to quiz'),
    'configurable'     => FALSE,
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
        ':remove_time'  => $rm_time
      ))
      ->execute();
  }
}

/**
 * Implements hook_menu().
 */
function quiz_menu() {
  // Admin pages.
  $items['admin/quiz'] = array(
    'title'            => '@quiz',
    'title arguments'  => array('@quiz' => QUIZ_NAME),
    'description'      => 'View results, score answers, run reports and edit configurations.',
    'page callback'    => 'system_admin_menu_block_page',
    'access arguments' => array('administer quiz configuration', 'score any quiz', 'score own quiz', 'view any quiz results', 'view results for own quiz'),
    'access callback'  => 'quiz_access_multi_or',
    'type'             => MENU_NORMAL_ITEM,
    'file'             => 'system.admin.inc',
    'file path'        => drupal_get_path('module', 'system'),
  );

  $items['admin/quiz/settings'] = array(
    'title'            => '@quiz settings',
    'title arguments'  => array('@quiz' => QUIZ_NAME),
    'description'      => 'Change settings for the all Quiz project modules.',
    'page callback'    => 'system_admin_menu_block_page',
    'access arguments' => array('administer quiz configuration'),
    'type'             => MENU_NORMAL_ITEM,
    'file'             => 'system.admin.inc',
    'file path'        => drupal_get_path('module', 'system'),
  );

  $items['admin/quiz/settings/config'] = array(
    'title'            => '@quiz configuration',
    'title arguments'  => array('@quiz' => QUIZ_NAME),
    'description'      => 'Configure the Quiz module.',
    'page callback'    => 'drupal_get_form',
    'page arguments'   => array('Drupal\quiz\Controller\Admin\QuizAdminSettingsController::staticFormCallback'),
    'access arguments' => array('administer quiz configuration'),
    'type'             => MENU_NORMAL_ITEM, // optional
    'file'             => 'quiz.admin.inc',
  );

  $items['admin/quiz/settings/quiz_form'] = array(
    'title'            => '@quiz form configuration',
    'title arguments'  => array('@quiz' => QUIZ_NAME),
    'description'      => 'Configure default values for the quiz creation form.',
    'page callback'    => 'drupal_get_form',
    'page arguments'   => array('Drupal\quiz\Controller\Admin\QuizAdminController::staticFormCallback'),
    'access arguments' => array('administer quiz configuration'),
    'type'             => MENU_NORMAL_ITEM, // optional
    'file'             => 'quiz.admin.inc',
  );

  $items['admin/quiz/reports'] = array(
    'title'            => 'Quiz reports and scoring',
    'title arguments'  => array('@quiz' => QUIZ_NAME),
    'description'      => 'View reports and score answers.',
    'page callback'    => 'system_admin_menu_block_page',
    'access arguments' => array('view any quiz results', 'view results for own quiz'),
    'access callback'  => 'quiz_access_multi_or',
    'type'             => MENU_NORMAL_ITEM,
    'file'             => 'system.admin.inc',
    'file path'        => drupal_get_path('module', 'system'),
  );

  // Take quiz.
  $items['node/%quiz_menu/take'] = array(
    'title'            => 'Take',
    'page callback'    => 'Drupal\quiz\Controller\QuizTakeController::staticCallback',
    'page arguments'   => array(1),
    'access callback'  => 'quiz_take_access',
    'access arguments' => array(1),
    'type'             => MENU_LOCAL_TASK,
  );

  // Take question.
  // @todo Thought - the 4th argument could be a "page" instead of a question
  // number
  $items['node/%quiz_menu/take/%question_number'] = array(
    'title'            => 'Take',
    'page callback'    => 'Drupal\quiz\Controller\QuizTakeQuestionController::staticCallback',
    'page arguments'   => array(1, 3),
    'access callback'  => 'quiz_take_question_access',
    'access arguments' => array(1, 3),
  );

  // Feedback
  $items['node/%quiz_menu/take/%question_number/feedback'] = array(
    'title'            => 'Feedback',
    'page callback'    => 'Drupal\quiz\Controller\QuizQuestionFeedbackController::staticCallback',
    'page arguments'   => array(1, 3),
    'access callback'  => 'quiz_question_feedback_access',
    'access arguments' => array(1, 3),
  );

  $items['node/%quiz_menu/quiz-results/%quiz_result/view'] = array(
    'title'            => 'User results',
    'page callback'    => 'Drupal\quiz\Controller\QuizUserResultController::staticCallback',
    'page arguments'   => array(3),
    'access callback'  => 'quiz_access_my_result',
    'access arguments' => array(3),
    'file'             => 'quiz.pages.inc',
  );

  $items['node/%quiz_menu/quiz/results/%quiz_rid/view'] = array(
    'title'            => 'Results',
    'page callback'    => 'Drupal\quiz\Controller\QuizResultController::staticCallback',
    'page arguments'   => array(1, 4),
    'access callback'  => 'quiz_access_results',
    'access arguments' => array(1, 4),
  );

  // Add questions to quiz.
  $items['node/%quiz_menu/quiz/questions'] = array(
    'title'            => 'Manage questions',
    'page callback'    => 'Drupal\quiz\Controller\QuizQuestionManagementController::staticCallback',
    'page arguments'   => array(1),
    'access callback'  => 'quiz_type_confirm',
    'access arguments' => array(1, 'update'),
    'type'             => MENU_LOCAL_TASK,
    'file'             => 'quiz.admin.inc',
    'weight'           => 2,
  );

  // User pages.
  $items['user/%/quiz-results/%quiz_result/view'] = array(
    'title'            => 'User results',
    'page callback'    => 'Drupal\quiz\Controller\QuizUserResultController::staticCallback',
    'page arguments'   => array(3),
    'access arguments' => array(3),
    'access callback'  => 'quiz_access_my_result',
    'type'             => MENU_CALLBACK,
    'file'             => 'quiz.pages.inc',
  );

  $items['node/%quiz_menu/questions/term_ahah'] = array(// @TODO: Add node access instead of user access...
    'page callback'    => 'Drupal\quiz\Controller\QuizQuestionManagementController::categorizedTermAhah',
    'type'             => MENU_CALLBACK,
    'access callback'  => 'node_access',
    'access arguments' => array('create', 'quiz'),
  );

  return $items;
}

/**
 * Implements hook_theme().
 */
function quiz_theme($existing, $type, $theme, $path) {
  return array(
    'quiz_view_stats'          => array(
      'variables' => array('node' => NULL),
      'file'      => 'quiz.pages.inc',
    ),
    'quiz_categorized_form'    => array(
      'render element' => 'form',
      'file'           => 'quiz.admin.inc'
    ),
    'quiz_result'              => array(
      'variables' => array('quiz' => NULL, 'questions' => NULL, 'score' => NULL, 'summary' => NULL, 'result_id' => NULL),
      'file'      => 'quiz.pages.inc',
    ),
    'quiz_progress'            => array(
      'variables' => array('quiz' => null, 'question_number' => NULL, 'num_questions' => NULL, 'allow_jumping' => NULL, 'time_limit' => NULL),
      'file'      => 'quiz.pages.inc',
    ),
    'quiz_no_feedback'         => array(
      'file'      => 'quiz.pages.inc',
      'variables' => array(),
    ),
    'question_selection_table' => array(
      'file'           => 'quiz.admin.inc',
      'render element' => 'form',
    ),
    'quiz_answer_result'       => array(
      'file'      => 'quiz.pages.inc',
      'variables' => array(),
    ),
    'quiz_report_form'         => array(
      'render element' => 'form',
      'path'           => $path . '/theme',
      'template'       => 'quiz-report-form',
    ),
    'quiz_node_form'           => array(
      'render element' => 'form',
      'file'           => 'quiz.admin.inc',
    ),
    'quiz_browser'             => array(
      'render element' => 'form',
      'file'           => 'quiz.admin.inc',
    ),
    'quiz_jumper'              => array(
      'variables' => array('current' => 0, 'num_questions' => 0),
      'file'      => 'quiz.admin.inc',
    ),
    'quiz_question_score'      => array(
      'variables' => array('score' => NULL, 'max_score' => NULL),
      'path'      => $path . '/theme',
      'template'  => 'quiz-question-score',
    ),
    'quiz_pager'               => array(
      'variables' => array('quiz' => null, 'total' => 0, 'current' => 0, 'siblings' => 0),
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

/**
 * Implements hook_form_FORM_ID_alter() for the quiz node form.
 */
function quiz_form_quiz_node_form_alter(&$form, $form_state) {
  $form['actions']['submit']['#submit'][] = function($form, &$form_state) {
    $node = $form_state['node'];

    if (empty($form_state['values']['nid'])) {
      drupal_set_message(t('You just created a new quiz. Now you have to add questions to it. This page is for adding and managing questions. Here you can create new questions or add some of your already created questions. If you want to change the quiz settings, you can use the "edit" tab.'));
      $form_state['redirect'] = "node/$node->nid/quiz/questions";
    }

    // If the quiz don't have any questions jump to the manage questions tab.
    $res = db_query('SELECT child_vid FROM {quiz_node_relationship} WHERE parent_vid = :vid', array(':vid' => $node->vid))->fetchField();
    if (!is_numeric($res)) {
      $form_state['redirect'] = 'node/' . $node->nid . '/quiz/questions';
    }
  };
}

/**
 * Implements hook_delete().
 */
function quiz_delete($node) {
  quiz()->getNodeHelper()->delete($node);
}

/**
 * Implements hook_validate().
 */
function quiz_validate($node) {
  quiz()->getNodeHelper()->validate($node);
}

/**
 * Implements hook_insert().
 */
function quiz_insert($node) {
  quiz()->getNodeHelper()->insert($node);
}

/**
 * Implements hook_update().
 */
function quiz_update($node) {
  quiz()->getNodeHelper()->update($node);
}

/**
 * Implementation of hook_node_presave().
 */
function quiz_node_presave($node) {
  quiz()->getNodeHelper()->presave($node);
}

/**
 * Implementation of hook_node_prepare().
 */
function quiz_node_prepare($node) {
  if ($node->type == 'quiz' && !isset($node->nid)) {
    // If this is a new node we apply the user defaults for the quiz settings.
    if (arg(0) === 'node') {
      drupal_set_message(t('You are making your first @quiz. On this page you set the attributes, most of which you may tell the system to remember as defaults for the future. On the next screen you can add questions.', array('@quiz' => QUIZ_NAME)));
    }
    $settings = quiz()->getQuizHelper()->getSettingHelper()->getUserDefaultSettings();
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
  return quiz()->getNodeHelper()->view($node, $view_mode);
}

/**
 * Implements hook_user_cancel().
 */
function quiz_user_cancel($edit, $account, $method) {
  if (variable_get('quiz_durod', 0)) {
    quiz()->getQuizHelper()->getResultHelper()->deleteByUserId($account->uid);
  }
}

/**
 * Implements hook_form().
 *
 * This is an admin form used to build a new quiz. It is called as part of the
 * node edit form.
 */
function quiz_form(&$node, &$form_state) {
  return quiz()->getNodeHelper()->getNodeFormHelper()->execute($node, $form_state);
}

/**
 * Implements hook_mail().
 */
function quiz_mail($key, &$message, $params) {
  global $user;

  switch ($key) {
    case 'notice':
      list($quiz, $score, $result_id, $target) = $params;
      list($subject, $body) = quiz()->getMailHelper()->notice($user, $quiz, $score, $result_id, $target);
      $message['subject'] = $subject;
      $message['body'][] = $body;
      break;
  }
}

/**
 * Implements hook_quiz_finished().
 *
 * Performs actions like sending quiz results over email at the end of quiz.
 */
function quiz_quiz_finished($quiz, $score, $session_data) {
  // @TODO convert to entity/rules
  $result_id = $session_data['result_id'];
  // Load data about the quiz taker
  $sql = 'SELECT u.uid, u.mail FROM {users} u JOIN {quiz_node_results} qnr ON u.uid = qnr.uid WHERE result_id = %d';
  $taker = db_query('SELECT u.uid, u.mail FROM {users} u JOIN {quiz_node_results} qnr ON u.uid = qnr.uid WHERE result_id = :result_id', array(':result_id' => $result_id))->fetch();

  if (variable_get('quiz_results_to_quiz_author', 0)) {
    $author_mail = db_query('SELECT mail FROM {users} WHERE uid = :uid', array(':uid' => $quiz->uid))->fetchField();
    drupal_mail('quiz', 'notice', $author_mail, NULL, array($quiz, $score, $result_id, 'author'));
  }

  if (variable_get('quiz_email_results', 0) && variable_get('quiz_use_passfail', 1) && $taker->uid != 0 && $score['is_evaluated']) {
    drupal_mail('quiz', 'notice', $taker->mail, NULL, array($quiz, $score, $result_id, 'taker'));
    drupal_set_message(t('Your results have been sent to your e-mail address.'));
  }

  // Calls userpoints functions to credit user point based on number of correct
  // answers.
  if ($quiz->has_userpoints && $taker->uid != 0 && $score['is_evaluated']) {

    //Looking up the tid of the selected Userpoint vocabulary
    $selected_tid = db_query("SELECT tid FROM {taxonomy_index}
                WHERE nid = :nid AND tid IN (
                  SELECT tid
                  FROM {taxonomy_term_data} t_t_d JOIN {taxonomy_vocabulary} t_v ON t_v.vid = t_t_d.vid
                  WHERE t_t_d.vid = :vid
                )", array(':nid' => $quiz->nid, ':vid' => $quiz->vid, ':vid' => userpoints_get_vid()))->fetchField();
    $variables = array(
      '@title' => $quiz->title,
      '@quiz'  => variable_get('quiz_name', QUIZ_NAME),
      '@time'  => date('l jS \of F Y h:i:s A'),
    );
    $params = array(
      'points'      => $score['numeric_score'],
      'description' => t('Attended @title @quiz on @time', $variables),
      'tid'         => $selected_tid,
      'uid'         => $taker->uid,
    );
    if ($quiz->userpoints_tid != 0) {
      $params['tid'] = $quiz->userpoints_tid;
    }
    userpoints_userpointsapi($params);
  }
}

/**
 * Implements hook_quiz_scored().
 *
 * Performs actions like sending quiz results over email at the end of quiz.
 */
function quiz_quiz_scored($quiz, $score, $result_id) {
  // @TODO Rules
  $taker = db_query('SELECT u.uid, u.mail FROM {users} u JOIN {quiz_node_results} qnr ON u.uid = qnr.uid WHERE result_id = :rid', array(':rid' => $result_id))->fetch();
  if (variable_get('quiz_email_results', 0) && $taker->uid != 0 && $score['is_evaluated']) {
    drupal_mail('quiz', 'notice', $taker->mail, NULL, array($quiz, $score, $result_id, 'taker'));
    drupal_set_message(t('The results has been sent to the users e-mail address.'));
  }

  // Calls userpoints functions to credit user point based on number of correct
  // answers.
  if ($quiz->has_userpoints && $taker->uid != 0 && $score['is_evaluated']) {

    //Looking up the tid of the selected Userpoint vocabulary
    $selected_tid = db_query("SELECT ti.tid
                FROM {taxonomy_index} ti
                JOIN {taxonomy_term_data} td
                  ON td.tid = ti.tid AND td.vid = :vid
                WHERE ti.nid = :nid", array(':nid' => $quiz->nid, ':vid' => userpoints_get_vid()))->fetchField();
    $variables = array(
      '@title' => $quiz->title,
      '@quiz'  => variable_get('quiz_name', QUIZ_NAME),
      '@time'  => date('l jS \of F Y h:i:s A'),
    );
    $params = array(
      'points'      => $score['numeric_score'],
      'description' => t('Attended @title @quiz on @time', $variables),
      'tid'         => $selected_tid,
    );
    if ($quiz->userpoints_tid != 0) {
      $params['tid'] = $quiz->userpoints_tid;
    }
    userpoints_userpointsapi($params);
  }
}

// ---------------------
// Helper functions
// ---------------------

/**
 * Shortcut to entity_load_single().
 */
function quiz_result_load($result_id) {
  return entity_load_single('quiz_result', $result_id);
}

/**
 * Workaround for menu_translate bug.
 */
function question_number_to_arg() {
  return 0;
}

/**
 * Result_id, menu argument loader.
 */
function quiz_rid_load($result_id) {
  if ($result_id > 0) {
    return $result_id;
  }
  return FALSE;
}

/**
 * Workaround for a _menu_translate() bug.
 */
function quiz_rid_to_arg() {
  return 0;
}

/**
 * Helper function to check if the user has any of a given list of permissions.
 *
 * @param args
 *   Any number of permissions.
 * @return
 *   TRUE if the user has access to any of the arguments given.
 */
function quiz_access_multi_or() {
  foreach (func_get_args() as $perm) {
    if (user_access($perm)) {
      return TRUE;
    }
  }
}

/**
 * Helper function to determine if a user has access to view a specific quiz result.
 *
 * @param int|stdClass $result
 *  Result id
 * @return boolean
 *  True if access, false otherwise
 */
function quiz_access_my_result($result) {
  $result_id = is_object($result) ? $result->result_id : $result;
  return quiz()->getQuizHelper()->getResultHelper()->isResultCompleted($result_id);
}

/**
 * Helper function to determine if a user has access to the different results
 * pages.
 *
 * @param $quiz
 *   The quiz node.
 * @param $result_id
 *   The result id of a result we are trying to access.
 * @return boolean
 *   TRUE if user has permission.
 */
function quiz_access_results($quiz, $result_id = NULL) {
  global $user;
  return quiz()->getQuizHelper()->getAccessHelper()->canAccessResults($user, $quiz, $result_id);
}

/**
 * Helper function to determine if a user has access to view his quiz results
 *
 * @param object $quiz
 *  The Quiz node
 */
function quiz_access_my_results($quiz) {
  global $user;
  return quiz()->getQuizHelper()->getAccessHelper()->canAccessMyResults($quiz, $user);
}

/**
 * Does the current user have access to take the quiz?
 *
 * @param $quiz
 *  The quiz node
 */
function quiz_take_access($quiz) {
  global $user;
  return quiz()->getQuizHelper()->getAccessHelper()->canTakeQuiz($quiz, $user);
}

/**
 * Meun check to see if user has access to take this question.
 */
function quiz_take_question_access($quiz, $question_number) {
  return quiz()->getQuizHelper()->getAccessHelper()->canAccessQuestion($quiz, $question_number);
}
