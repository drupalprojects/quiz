<?php

namespace Drupal\quiz\Helper\HookImplementation;

class HookMenu {

  public function execute() {
    $items = array();

    $items += $this->getQuizAdminMenuItems();
    $items += $this->getQuizNodeMenuItems();
    $items += $this->getQuizUserMenuItems();

    return $items;
  }

  private function getQuizAdminMenuItems() {
    $items = array();

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

    return $items;
  }

  private function getQuizUserMenuItems() {
    $items = array();

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

    return $items;
  }

  private function getQuizNodeMenuItems() {
    $items = array();

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
    $items['node/%quiz_menu/questions/term_ahah'] = array(// @TODO: Add node access instead of user access...
      'page callback'    => 'Drupal\quiz\Controller\QuizQuestionManagementController::categorizedTermAhah',
      'type'             => MENU_CALLBACK,
      'access callback'  => 'node_access',
      'access arguments' => array('create', 'quiz'),
    );

    $items['node/%quiz_menu/quiz-results/%quiz_result/view'] = array(
      'title'  => 'View',
      'type'   => MENU_DEFAULT_LOCAL_TASK,
      'weight' => -10,
    );

    $items['node/%quiz_menu/quiz-results/%quiz_result'] = array(
      'title'            => 'User results',
      'page callback'    => 'Drupal\quiz\Controller\QuizUserResultController::staticCallback',
      'page arguments'   => array(3),
      'access callback'  => 'quiz_access_my_result',
      'access arguments' => array(3),
      'file'             => 'quiz.pages.inc',
    );

    $items['quiz-result/%quiz_result'] = array(
      'title'            => 'User results',
      'access callback'  => 'quiz_access_my_result',
      'access arguments' => array(1),
      'page callback'    => 'Drupal\quiz\Controller\QuizUserResultController::staticCallback',
      'page arguments'   => array(1),
      'file'             => 'quiz.pages.inc',
    );

    return $items;
  }

}
