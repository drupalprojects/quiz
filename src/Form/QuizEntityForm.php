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

  public static function staticCallback($op, $quiz_type) {
    $quiz = $op === 'add' ? entity_create('quiz_entity', array('type' => $quiz_type)) : NULL;
    return entity_ui_get_form('quiz_entity', $quiz, $op);
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
    /* @var $quiz \Drupal\quiz\Entity\QuizEntity */
    $quiz = entity_ui_controller('quiz_entity')->entityFormSubmitBuildEntity($form, $form_state);

    // convert formatted text fields
    $quiz->summary_default_format = $quiz->summary_default['format'];
    $quiz->summary_default = $quiz->summary_default['value'];
    $quiz->summary_pass_format = $quiz->summary_pass['format'];
    $quiz->summary_pass = $quiz->summary_pass['value'];

    // convert value from date widget to timestamp
    $quiz->quiz_open = mktime(0, 0, 0, $quiz->quiz_open['month'], $quiz->quiz_open['day'], $quiz->quiz_open['year']);
    $quiz->quiz_close = mktime(0, 0, 0, $quiz->quiz_close['month'], $quiz->quiz_close['day'], $quiz->quiz_close['year']);

    // Add in created and changed times.
    $quiz->save();

    $form_state['redirect'] = 'admin/content/quiz';
  }

}
