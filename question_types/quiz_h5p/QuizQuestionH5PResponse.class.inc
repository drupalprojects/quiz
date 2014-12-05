<?php

class QuizQuestionH5PResponse extends QuizQuestionResponse {

  public function delete() {

  }

  public function getResponse() {
    return h5p_get_content_id($this->question);
  }

  public function save() {
    // We have no save behavior for H5P, the storage is in the H5P module.
  }

  public function score() {
    $quiz_result = quiz_result_load($this->result_id);
    // 'answer' from the form is the content_id
    $points = db_select('h5p_points', 'p')
      ->fields('p', array('points'))
      ->condition('content_id', $this->getResponse())
      ->condition('uid', $quiz_result->uid)
      ->execute()->fetchField();
    return $points;
  }
}
