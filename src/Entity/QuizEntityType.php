<?php

namespace Drupal\quiz\Entity;

use Entity;

class QuizEntityType extends Entity {

  public $type;
  public $label;
  public $weight = 0;

  public function __construct(array $values = array()) {
    parent::__construct($values, 'quiz_type');
  }

}
