<?php

namespace Drupal\quiz\Helper\HookImplementation;

class HookEntityInfo {

  public function execute() {
    $entity_types = array(
      'quiz_result'                => array(
        'label'                  => t('Quiz result'),
        'controller class'       => 'EntityAPIController',
        'base table'             => 'quiz_node_results',
        'entity keys'            => array('id' => 'result_id'),
        'views controller class' => 'EntityDefaultViewsController',
      ),
      'quiz_result_answer'         => array(
        'label'                  => t('Quiz result answer'),
        'controller class'       => 'EntityAPIController',
        'base table'             => 'quiz_node_results_answers',
        'entity keys'            => array('id' => 'result_answer_id'),
        'views controller class' => 'EntityDefaultViewsController',
      ),
      'quiz_question_relationship' => array(
        'label'                  => t('Quiz question relationship'),
        'controller class'       => 'EntityAPIController',
        'base table'             => 'quiz_node_relationship',
        'entity keys'            => array('id' => 'qnr_id'),
        'views controller class' => 'EntityDefaultViewsController',
      ),
    );

    $entity_types += $this->getQuizEntityInfo();
    $entity_types += $this->getDepratedEntityInfo();

    return $entity_types;
  }

  private function getQuizEntityInfo() {
    $entity_types = array();

    $entity_types['quiz_entity'] = array(
      'label'                     => t('Quiz properties'),
      'description'               => t('!quiz entity', array('!quiz' => QUIZ_NAME)),
      'entity class'              => 'Drupal\quiz\Entity\QuizEntity',
      'controller class'          => 'Drupal\quiz\Entity\QuizEntityController',
      'metadata controller class' => 'Drupal\quiz\Entity\QuizEntityMetadataController',
      'views controller class'    => 'Drupal\quiz\Entity\QuizViewsController',
      'base table'                => 'quiz_entity',
      'revision table'            => 'quiz_entity_revision',
      'fieldable'                 => TRUE,
      'entity keys'               => array('id' => 'qid', 'bundle' => 'type', 'revision' => 'vid', 'label' => 'title'),
      'bundle keys'               => array('bundle' => 'type'),
      'access callback'           => 'quiz_entity_access_callback',
      'label callback'            => 'entity_class_label',
      'uri callback'              => 'entity_class_uri',
      'module'                    => 'quiz',
      'bundles'                   => array(),
      'view modes'                => array(
        'question' => array('label' => t('Question'), 'custom settings' => TRUE),
      ),
      'admin ui'                  => array(// Enable the entity API's admin UI.
        'path'             => 'admin/content/quiz',
        'file'             => 'quiz.admin.inc',
        'controller class' => 'Drupal\quiz\Entity\QuizUIController',
      ),
    );

    $entity_types['quiz_type'] = array(
      'label'            => t('!quiz type', array('!quiz' => QUIZ_NAME)),
      'plural label'     => t('!quiz types', array('!quiz' => QUIZ_NAME)),
      'description'      => t('Types of !quiz.', array('!quiz' => QUIZ_NAME)),
      'entity class'     => 'Drupal\quiz\Entity\QuizEntityType',
      'controller class' => 'EntityAPIControllerExportable',
      'base table'       => 'quiz_type',
      'fieldable'        => FALSE,
      'bundle of'        => 'quiz_entity',
      'exportable'       => TRUE,
      'entity keys'      => array('id' => 'id', 'name' => 'type', 'label' => 'label'),
      'access callback'  => 'quiz_type_access',
      'module'           => 'quiz',
      'admin ui'         => array(// Enable the entity API's admin UI.
        'path'             => 'admin/structure/quiz',
        'file'             => 'quiz.admin.inc',
        'controller class' => 'Drupal\quiz\Entity\QuizTypeUIController',
      ),
    );

    // Add bundle info but bypass entity_load() as we cannot use it here.
    foreach (db_select('quiz_type', 'qt')->fields('qt')->execute()->fetchAllAssoc('type') as $type => $info) {
      $entity_types['quiz_entity']['bundles'][$type] = array(
        'label' => $info->label,
        'admin' => array(
          'path'             => 'admin/structure/quiz/manage/%quiz_type',
          'real path'        => 'admin/structure/quiz/manage/' . $type,
          'bundle argument'  => 4,
          'access arguments' => array('administer quiz'),
        ),
      );
    }

    // Support entity cache module.
    if (module_exists('entitycache')) {
      $entity_types['quiz_entity']['field cache'] = FALSE;
      $entity_types['quiz_entity']['entity cache'] = TRUE;
    }

    return $entity_types;
  }

  private function getDepratedEntityInfo() {
    return array(
      // @TODO: Once quiz entity is ready, remove this
      'quiz' => array(
        'label'                  => t('Quiz properties'),
        'controller class'       => 'EntityAPIController',
        'base table'             => 'quiz_node_properties',
        'entity keys'            => array('id' => 'qnp_id'),
        'views controller class' => 'EntityDefaultViewsController',
      ),
    );
  }

}
