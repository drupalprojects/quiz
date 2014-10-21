<?php

namespace Drupal\quiz\Entity;

use EntityDefaultUIController;

class QuizUiController extends EntityDefaultUIController {

  public function hook_menu() {
    $items = parent::hook_menu();
    $items['admin/content/quiz']['type'] = MENU_LOCAL_TASK;

    // Change path from /admin/content/quiz/add -> /quizz/add
    $items['quiz/add'] = $items['admin/content/quiz/add'];
    unset($items['admin/content/quiz/add']);

    // Menu items for /quiz/add/*
    if (($types = quiz_get_types()) && (1 < count($types))) {
      $items['quiz/add'] = array(
        'title'           => 'Add ' . QUIZ_NAME,
        'access callback' => 'quiz_can_create_quiz_entity',
        'page callback'   => 'Drupal\quiz\Controller\QuizEntityAddController::staticCallback',
      );

      foreach (array_keys($types) as $name) {
        $items["quiz/add/{$name}"] = array(
          'title callback'   => 'entity_ui_get_action_title',
          'title arguments'  => array('add', 'quiz_entity'),
          'access callback'  => 'entity_access',
          'access arguments' => array('create', 'quiz_entity'),
          'page callback'    => 'Drupal\quiz\Form\QuizEntityForm::staticCallback',
          'page arguments'   => array('add', $name),
          'file path'        => drupal_get_path('module', 'quiz'),
          'file'             => 'quiz.admin.inc',
        );
      }
    }

    $items['quiz/%quiz_entity_single'] = array(
      'title callback'   => 'entity_class_label',
      'title arguments'  => array(1),
      'access callback'  => 'quiz_entity_access_callback',
      'access arguments' => array('view'),
      'page callback'    => 'Drupal\quiz\Controller\QuizEntityViewController::staticCallback',
      'page arguments'   => array(1),
    );

    // Define menu item structure for /quiz/%/edit
    $items['quiz/%entity_object/edit'] = $items['admin/content/quiz/manage/%entity_object'];
    $items['quiz/%entity_object/edit']['title arguments'][1] = 1;
    $items['quiz/%entity_object/edit']['page arguments'][1] = 1;
    $items['quiz/%entity_object/edit']['access arguments'][2] = 1;

    // Define menu item structure for /quiz/%/delete
    $items['quiz/%entity_object/delete'] = array(
      'load arguments'   => array('quiz_entity'),
      'page callback'    => 'drupal_get_form',
      'page arguments'   => array('quiz_entity_operation_form', 'quiz_entity', 1, 'delete'),
      'access callback'  => 'entity_access',
      'access arguments' => array('delete', 'quiz_entity', 1),
      'file'             => 'includes/entity.ui.inc',
    );

    return $items;
  }

}
