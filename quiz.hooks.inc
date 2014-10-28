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
      return _filter_autop(t('The quiz module allows users to administer a quiz, as a sequence of questions, and track the answers given. It allows for the creation of questions (and their answers), and organizes these questions into a quiz. Its target audience includes educational institutions, online training programs, employers, and people who just want to add a fun activity for their visitors to their Drupal site.

The quiz module has a plethora of permission options. Unless you take care setting your permissions, the quiz module might not do everything you want it to do.

For more information about quiz, and resources on how to use quiz, see the <a href="http://drupal.org/project/quiz">Quiz project website</a>'));
    case 'quiz/add#quiz':
      return t('A collection of questions designed to create interactive tests');
  }
}

/**
 * Implements hook_views_api().
 */
function quiz_views_api() {
  return array('api' => '3.0', 'path' => QUIZ_VIEWS_DIR);
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
 * Implements hook_entity_info().
 */
function quiz_entity_info() {
  return quiz()->getHookImplementation()->getHookEntityInfo()->execute();
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
 * Implements hook_field_extra_fields().
 */
function quiz_field_extra_fields() {
  return quiz()->getHookImplementation()->getHookFieldExtraFields()->execute();
}

/**
 * Implements hook_permission().
 */
function quiz_permission() {
  return array(
    // Configure quiz
    'administer quiz configuration'           => array(
      'title'           => t('Administer quiz configuration'),
      'description'     => t('Control the various settings and behaviours of quiz'),
      'restrict access' => TRUE,
    ),
    // Managing quizzes
    'access quiz'                             => array(
      'title'       => t('Take quiz'),
      'description' => t('Can access (take) all quizzes.'),
    ),
    'create quiz content'                     => array(
      'title' => t('Create quiz content'),
    ),
    'edit any quiz content'                   => array(
      'title' => t('Edit any quiz content'),
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
  $items = array();

  if (variable_get('node_admin_theme', TRUE)) {
    $items['quiz/add'] = TRUE;
    $items['quiz/add/*'] = TRUE;
    $items['quiz/*/edit'] = TRUE;
    $items['quiz/*/edit/*'] = TRUE;
    $items['quiz/*/delete'] = TRUE;
  }

  return $items;
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
  if ($rm_time = variable_get('quiz_remove_partial_quiz_record', '0')) { // $time = 0 for never.
    db_delete('quiz_results')
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
  return quiz()->getHookImplementation()->getHookMenu()->execute();
}

/**
 * Implements hook_menu_local_tasks_alter().
 *
 * Add action link to 'quiz/add' on 'admin/content/quiz' page.
 */
function quiz_menu_local_tasks_alter(&$data, $router_item, $root_path) {
  if ($root_path === 'admin/content/quiz') {
    if (($item = menu_get_item('quiz/add')) && $item['access']) {
      $data['actions']['output'][] = array('#theme' => 'menu_local_action', '#link' => $item);
    }
  }
}

/**
 * Implements hook_theme().
 */
function quiz_theme($existing, $type, $theme, $path) {
  return array(
    'quiz_view_stats'          => array(
      'variables' => array('quiz' => NULL),
      'file'      => 'quiz.pages.inc'
    ),
    'quiz_categorized_form'    => array(
      'render element' => 'form',
      'file'           => 'quiz.admin.inc'
    ),
    'quiz_result'              => array(
      'variables' => array(
        'quiz'      => NULL,
        'questions' => NULL,
        'score'     => NULL,
        'summary'   => NULL,
        'result_id' => NULL),
      'file'      => 'quiz.pages.inc',
    ),
    'quiz_progress'            => array(
      'variables' => array(
        'quiz'            => null,
        'question_number' => NULL,
        'num_questions'   => NULL,
        'allow_jumping'   => NULL,
        'time_limit'      => NULL),
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
    'quiz_browser'             => array(
      'render element' => 'form',
      'file'           => 'quiz.admin.inc',
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
 */
function quiz_form_alter(&$form, $form_state, $form_id) {
  if (!isset($form['#quiz_check_revision_access'])) {
    return;
  }

  // Remove revision fieldset if user don't have access to revise quiz manually.
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
    $form['actions']['cancel'] = array('#weight' => 6, '#markup' => l(t('Cancel'), $form_state['redirect']));
  }
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
  // Convert the action id to the actual id from the MD5 hash.
  // Why the actions module does this I do not know? Maybe to prevent
  // invalid values put into the options value="" field.
  if (($node->type === 'quiz') && !empty($node->aid) && $aid = actions_function_lookup($node->aid)) {
    $node->aid = $aid;
  }

  if (isset($node->is_quiz_question) && variable_get('quiz_auto_revisioning', 1)) {
    $node->revision = (quiz_question_has_been_answered($node)) ? 1 : 0;
  }
}

/**
 * Implementation of hook_node_prepare().
 */
function quiz_node_prepare($node) {
  if (isset($node->is_quiz_question) && variable_get('quiz_auto_revisioning', 1)) {
    $node->revision = (quiz_question_has_been_answered($node)) ? 1 : 0;
  }
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
  return quiz()->getHookImplementation()->getHookQuizFinished($quiz, $score, $session_data)->execute();
}

/**
 * Implements hook_quiz_scored().
 *
 * Performs actions like sending quiz results over email at the end of quiz.
 */
function quiz_quiz_scored($quiz, $score, $result_id) {
  return quiz()->getHookImplementation()->getHookQuizScored($quiz, $score, $result_id)->execute();
}

// ---------------------
// Helper functions
// ---------------------

/**
 * Shortcut to entity_load_single().
 *
 * @return \Drupal\quiz\Entity\Result
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
 * @param \Drupal\quiz\Entity\Result|int $result
 *  Result id
 * @return boolean
 *  True if access, false otherwise
 */
function quiz_access_my_result($result) {
  if (is_numeric($result) && !$result = quiz_result_load($result)) {
    throw new RuntimeException('Invalid result ID.');
  }
  return $result->time_end > 0;
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

/**
 * Access callback for quiz-type.
 *
 * @param string $op
 * @param string $type
 * @param stdClass $account
 */
function quiz_type_access($op, $type = NULL, $account = NULL) {
  return user_access('administer quiz configuration', $account);
}

/**
 * Access callback for quiz-entity.
 * @param string $op
 * @param string $type
 * @param stdClass $account
 */
function quiz_entity_access_callback($op, $type = NULL, $account = NULL) {
  return TRUE;
}

/**
 * @TODO: Access callback for /quiz/add.
 *
 * @return bool
 *  TRUE only if user can create at least one quiz type.
 */
function quiz_can_create_quiz_entity() {
  return TRUE;
}
