<?php

namespace Drupal\quiz\Form;

use Drupal\quiz\Entity\QuizEntity;
use Drupal\quiz\Form\QuizEntityForm\FormDefinition;
use Drupal\quiz\Form\QuizEntityForm\FormValidation;

class QuizEntityForm {

  /** @var QuizEntity */
  private $quiz;

  public function __construct($quiz) {
    $this->quiz = $quiz;
  }

  public function get($form, &$form_state, $op) {
    $def = new FormDefinition($this->quiz);
    $return = $def->get($form, $form_state, $op);

    // Define callbacks
    $return['#validate'][] = array($this, 'validate');
    $return['#submit'][] = array($this, 'submit');

    return $return;
  }

  public function validate($form, &$form_state) {
    $validator = new FormValidation($form, $form_state);
    return $validator->validate();
  }

  public function submit($form, &$form_state) {
    $values = &$form_state['values'];
  }

}
