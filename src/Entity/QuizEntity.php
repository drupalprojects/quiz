<?php

namespace Drupal\quiz\Entity;

use Entity;

class QuizEntity extends Entity {

  public $qid;

  /** @var string The name of the quiz type. */
  public $type;

  /** @var string The quiz label. */
  public $title;

  /** @var integer The user id of the quiz owner. */
  public $uid;

  /** @var integer The Unix timestamp when the quiz was created. */
  public $created;

  /** @var integer The Unix timestamp when the quiz was most recently saved. */
  public $changed;

  public function __construct(array $values = array()) {
    // fill default value
    $values += (array) quiz()->getQuizHelper()->getSettingHelper()->getUserDefaultSettings();
    parent::__construct($values, 'quiz_entity');
  }

}
