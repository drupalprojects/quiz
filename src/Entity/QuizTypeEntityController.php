<?php

namespace Drupal\quiz\Entity;

use DatabaseTransaction;
use EntityAPIControllerExportable;

class QuizTypeEntityController extends EntityAPIControllerExportable {

  public function save($entity, DatabaseTransaction $transaction = NULL) {
    $return = parent::save($entity, $transaction);

    $this->addBodyField($entity->type);

    return $return;
  }

  /**
   * Add default body field to a quiz type
   */
  private function addBodyField($bundle) {
    if (!field_info_field('quiz_body')) {
      field_create_field(array(
          'field_name'   => 'quiz_body',
          'type'         => 'text_with_summary',
          'entity_types' => array('quiz_entity'),
      ));
    }

    if (!$instance = field_info_instance('quiz_entity', 'quiz_body', $bundle)) {
      $instance = field_create_instance(array(
          'field_name'  => 'quiz_body',
          'entity_type' => 'quiz_entity',
          'bundle'      => $bundle,
          'label'       => t('Body'),
          'widget'      => array('type' => 'text_textarea_with_summary'),
          'settings'    => array('display_summary' => TRUE),
          'display'     => array(
              'default' => array('label' => 'hidden', 'type' => 'text_default'),
          ),
      ));
    }

    return $instance;
  }

}
