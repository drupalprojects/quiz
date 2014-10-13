<?php

namespace Drupal\quiz\Entity;

use Entity;

class QuizEntity extends Entity {

  public $qid;

  /**
   * The name of the profile type.
   *
   * @var string
   */
  public $type;

  /**
   * The profile label.
   *
   * @var string
   */
  public $label;

  /**
   * The user id of the profile owner.
   *
   * @var integer
   */
  public $uid;

  /**
   * The Unix timestamp when the profile was created.
   *
   * @var integer
   */
  public $created;

  /**
   * The Unix timestamp when the profile was most recently saved.
   *
   * @var integer
   */
  public $changed;

  public function __construct(array $values = array()) {
    parent::__construct($values, 'quiz_entity');
  }

}
