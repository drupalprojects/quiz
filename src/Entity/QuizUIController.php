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
