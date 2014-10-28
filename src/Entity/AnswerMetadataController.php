<?php

namespace Drupal\quiz\Entity;

use EntityDefaultMetadataController;

class AnswerMetadataController extends EntityDefaultMetadataController {

  public function entityPropertyInfo() {
    $info = parent::entityPropertyInfo();
    $properties = &$info[$this->type]['properties'];

    $properties['result_id']['type'] = 'quiz_result';
    $properties['question_nid']['type'] = 'node';
    $properties['is_correct']['type'] = 'boolean';
    $properties['is_skipped']['type'] = 'boolean';
    $properties['is_doubtful']['type'] = 'boolean';
    $properties['answer_timestamp']['type'] = 'date';

    return $info;
  }

}
