<?php
class QuizQuestionH5P extends QuizQuestion {

  public function getCreationForm(array &$form_state = NULL) {

  }

  public function getMaximumScore() {
    return 1;
  }

  public function saveNodeProperties($is_new = FALSE) {

  }

  public function validateNode(array &$form) {

  }

  public function getAnsweringForm(array $form_state = NULL, $result_id) {
    $element = parent::getAnsweringForm($form_state, $result_id);
    $element += array(
      '#type' => 'textfield',
      '#title' => t('Content ID'),
      '#default_value' => h5p_get_content_id($this->node),
    );
    return $element;
  }
}
