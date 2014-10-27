<?php

namespace Drupal\quiz\Helper\HookImplementation;

class HookMenu {

  public function execute() {
    $items = array();

    $items += $this->getQuizAdminMenuItems();
    $items += $this->getQuizNodeMenuItems();
    $items += $this->getQuizUserMenuItems();

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
    return array(
      'node/%quiz_menu/questions/term_ahah' => array(
        'page callback'    => 'Drupal\quiz\Controller\QuizQuestionManagementController::categorizedTermAhah',
        'type'             => MENU_CALLBACK,
        'access callback'  => 'node_access',
        'access arguments' => array('create', 'quiz'),
      ),
    );
  }

}
