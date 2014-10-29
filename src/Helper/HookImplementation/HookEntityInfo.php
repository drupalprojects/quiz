<?php

namespace Drupal\quiz\Helper\HookImplementation;

class HookEntityInfo {

  public function execute() {
    return array(
        'quiz_type'                  => $this->getQuizEntityTypeInfo(),
        'quiz_entity'                => $this->getQuizEntityInfo(),
        'quiz_question_relationship' => $this->getQuizQuestionRelationshipInfo(),
        'quiz_result'                => $this->getQuizResultInfo(),
        'quiz_result_answer'         => $this->getQuizResultAnswerInfo(),
      ) + $this->getDepratedEntityInfo();
  }

  private function getQuizEntityTypeInfo() {
    return array(
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
  }

  private function getQuizEntityInfo() {
    $entity_info = array(
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

    // Add bundle info but bypass entity_load() as we cannot use it here.
    foreach (db_select('quiz_type', 'qt')->fields('qt')->execute()->fetchAllAssoc('type') as $type => $info) {
      $entity_info['bundles'][$type] = array(
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
      $entity_info['field cache'] = FALSE;
      $entity_info['entity cache'] = TRUE;
    }

    return $entity_info;
  }

  private function getQuizQuestionRelationshipInfo() {
    return array(
        'label'                     => t('Quiz question relationship'),
        'controller class'          => 'EntityAPIController',
        'base table'                => 'quiz_relationship',
        'entity keys'               => array('id' => 'qr_id'),
        'views controller class'    => 'EntityDefaultViewsController',
        'metadata controller class' => 'Drupal\quiz\Entity\RelationshipMetadataController',
    );
  }

  private function getQuizResultInfo() {
    return array(
        'label'                     => t('Quiz result'),
        'entity class'              => 'Drupal\quiz\Entity\Result',
        'controller class'          => 'Drupal\quiz\Entity\ResultController',
        'base table'                => 'quiz_results',
        'entity keys'               => array('id' => 'result_id'),
        'views controller class'    => 'EntityDefaultViewsController',
        'metadata controller class' => 'Drupal\quiz\Entity\ResultMetadataController',
    );
  }

  private function getQuizResultAnswerInfo() {
    return array(
        'label'                     => t('Quiz result answer'),
        'controller class'          => 'EntityAPIController',
        'base table'                => 'quiz_results_answers',
        'entity keys'               => array('id' => 'result_answer_id'),
        'views controller class'    => 'EntityDefaultViewsController',
        'metadata controller class' => 'Drupal\quiz\Entity\AnswerMetadataController',
    );
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
