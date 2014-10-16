<?php

namespace Drupal\quiz\Form;

use Drupal\quiz\Entity\QuizEntity;
use Drupal\quiz\Form\QuizEntityForm\FormDefinition;

class QuizEntityForm {

  /** @var QuizEntity */
  private $quiz;

  public function __construct($quiz) {
    $this->quiz = $quiz;
  }

  public function get($form, &$form_state, $op) {
    $def = new FormDefinition($this->quiz);
    return $def->get($form, $form_state, $op);
  }

  public function validate($form, &$form_state) {
    form_set_error('title', 'working…');
  }

  public function submit($form, &$form_state) {
    $values = &$form_state['values'];
  }

}
