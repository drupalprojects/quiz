<?php

namespace Drupal\quiz\Entity;

use Entity;

class QuizEntity extends Entity {

  /** @var int Quiz ID */
  public $qid;

  /** @var int Quiz Revision ID */
  public $vid;

  /** @var string The name of the quiz type. */
  public $type = 'quiz';

  /** @var string The quiz label. */
  public $title;

  /** @var integer The user id of the quiz owner. */
  public $uid;

  /** @var integer The Unix timestamp when the quiz was created. */
  public $created;

  /** @var integer The Unix timestamp when the quiz was most recently saved. */
  public $changed;

  /** @var bool Magic flag to create new revision on save */
  public $is_new_revision;

  /** @var string Revision log */
  public $log;

  public function __construct(array $values = array()) {
    // fill default value
    $values += (array) quiz()->getQuizHelper()->getSettingHelper()->getUserDefaultSettings($legacy = FALSE);
    parent::__construct($values, 'quiz_entity');
  }

  public function save() {
    global $user;

    // Entity datetime
    $this->changed = time();
    if ($this->is_new = isset($this->is_new) ? $this->is_new : 0) {
      $this->created = time();
      if (null === $this->uid) {
        $this->uid = $user->uid;
      }
    }

    // Default properties
    foreach ((array) quiz()->getQuizHelper()->getSettingHelper()->getQuizDefaultSettings() as $k => $v) {
      if (!isset($this->{$k})) {
        $this->{$k} = $v;
      }
    }

    return parent::save();
  }

  /**
   * Default quiz entity uri.
   */
  protected function defaultUri() {
    return array('path' => 'quiz/' . $this->identifier());
  }

}
