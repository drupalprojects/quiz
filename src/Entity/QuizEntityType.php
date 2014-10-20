<?php

namespace Drupal\quiz\Entity;

use Entity;

class QuizEntityType extends Entity {

  public $type;
  public $label;
  public $description;
  public $help;
  public $weight = 0;

  public function __construct(array $values = array()) {
    parent::__construct($values, 'quiz_type');
  }

  /**
   * Returns whether the quiz type is locked, thus may not be deleted or renamed.
   *
   * Quiz types provided in code are automatically treated as locked, as well
   * as any fixed quiz type.
   */
  public function isLocked() {
    return isset($this->status) && empty($this->is_new) && (($this->status & ENTITY_IN_CODE) || ($this->status & ENTITY_FIXED));
  }

}
