<?php

class QuizQuestionH5PResponse extends QuizQuestionResponse {

  public function delete() {

  }

  public function getResponse() {
    return 'In H5P';
  }

  public function save() {
    global $user;
    $content_id = h5p_get_content_id($this->question);
    $results = db_query(
      "SELECT points, max_points
      FROM {h5p_points}
      WHERE content_id = :content_id
      AND uid = :uid",
      array(
        ':content_id' => $content_id,
        ':uid' => $user->uid))->fetchAssoc();

    $percentScore = 1;
    if ($results['max_points'] > 0) {
      $percentScore = $results['points'] / $results['max_points'];
      if ($percentScore < 0) {
        $percentScore = 0;
      }
      elseif ($percentScore > 1) {
        $percentScore = 1;
      }
    }

    db_insert('quiz_h5p_user_results')
      ->fields(array(
        'question_nid' => $this->question->nid,
        'question_vid' => $this->question->vid,
        'result_id' => $this->rid,
        'percent_score' => $percentScore,
      ))
      ->execute();
  }

  public function score() {
    global $user;
    
    $percentScore = db_query(
      "SELECT percent_score
      FROM {quiz_h5p_user_results}
      WHERE question_nid = :question_nid
      AND question_vid = :question_vid
      AND result_id = :result_id",
      array(
        'question_nid' => $this->question->nid,
        'question_vid' => $this->question->vid,
        'result_id' => $this->rid
      ))->fetchField();
    
    return round($percentScore * $this->getMaxScore());
  }
  /**
   * Implementation of getReportFormQuestion
   *
   * @see QuizQuestionResponse#getReportFormQuestion($showpoints, $showfeedback, $allow_scoring)
   */
  public function getReportFormQuestion($showpoints = TRUE, $showfeedback = TRUE, $allow_scoring = FALSE) {
    $title = db_query('SELECT title FROM {node} WHERE nid = :nid', array(':nid' => $this->question->nid))->fetchField();
    return array('#markup' => check_plain($title));
  }
  
  public function getReportFormResponse($showpoints = TRUE, $showfeedback = TRUE, $allow_scoring = FALSE) {
    return array('#markup' => t("This question type doesn't provide a response for the results report"));
  }
	

  public function getReportFormAnswerFeedback($showpoints = TRUE, $showfeedback = TRUE, $allow_scoring = FALSE) {
    return FALSE;
  }
}
