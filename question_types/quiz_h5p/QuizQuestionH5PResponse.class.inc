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
    global $user;
    // TODO: Replace logged in user with the one who saved this shitt
    
    // 'answer' from the form is the content_id
    $points = db_select('h5p_points', 'p')
      ->fields('p', array('points'))
      ->condition('content_id', $this->getResponse())
      ->condition('uid', $user->uid)
      ->execute()->fetchField();
    return $points;
  }
}
