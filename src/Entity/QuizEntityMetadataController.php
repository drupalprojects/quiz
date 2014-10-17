<?php

namespace Drupal\quiz\Entity;

use EntityDefaultMetadataController;

class QuizEntityMetadataController extends EntityDefaultMetadataController {

  public function entityPropertyInfo() {
    $info = parent::entityPropertyInfo();
    $properties = &$info[$this->type]['properties'];

    $properties['uid']['type'] = 'user';
    $properties['created']['type'] = 'date';
    $properties['created']['setter callback'] = 'entity_property_verbatim_set';
    $properties['created']['setter permission'] = 'administer quizzes';
    $properties['changed']['type'] = 'date';
    $properties['changed']['setter callback'] = 'entity_property_verbatim_set';
    $properties['changed']['setter permission'] = 'administer quizzes';

    return $info;
  }

}
