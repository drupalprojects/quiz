<?php
class QuizQuestionH5P extends QuizQuestion {

  public function getCreationForm(array &$form_state = NULL) {
    return array();
  }

  public function getMaximumScore() {
    return variable_get('h5p_quiz_question_max_score', 10);
  }

  public function saveNodeProperties($is_new = FALSE) {

  }

  public function validateNode(array &$form) {

  }

  public function getAnsweringForm(array $form_state = NULL, $result_id) {
     return array(
      'tries' => array(
        '#type' => 'hidden',
        '#default_value' => 1,
      ),
    );
  }
}
