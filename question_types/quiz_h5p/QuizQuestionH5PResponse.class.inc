<?php

class QuizQuestionH5PResponse extends QuizQuestionResponse {

  public function delete() {

  }

  public function getResponse() {
    return 'In H5P';
  }

  /**
   * Save current score and xAPI response in db.
   */
  public function save() {
    global $user;

    // Decode xAPI answer data
    $data = new \H5P\XAPIData(json_decode($this->answer));

    // Parse data
    $this->saveXAPIData($data);
  }

  /**
   * Recusive save of xAPI data
   *
   * @param \H5P\XAPIData $data
   */
  public function saveXAPIData($data) {

    // Save statement data
    $dataID = db_insert('quiz_h5p_user_results')
      ->fields(array(
        'parent_id' => $data->getParentID(),
        'question_nid' => $this->question->nid,
        'question_vid' => $this->question->vid,
        'result_id' => $this->rid,
        'score_scaled' => $data->getScoreScaled(),
        'score_raw' => $data->getScoreRaw(),
        'score_min' => $data->getScoreMin(),
        'score_max' => $data->getScoreMax(),
        'interaction_type' => $data->getInteractionType(),
        'description' => $data->getDescription(),
        'correct_responses_pattern' => $data->getCorrectResponsesPattern(),
        'response' => $data->getResponse(),
        'additionals' => $data->getAdditionals(),
      ))
      ->execute();

    // Save sub content statements data
    if ($statement->isCompound()) {
      foreach ($statement->getChildren($dataID) as $subData) {
        $this->saveXAPIData($subData);
      }
    }
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

  /**
   * Implements getReportFormResponse of QuizQuestionResponse interface
   */
  public function getReportFormResponse($showpoints = TRUE, $showfeedback = TRUE, $allow_scoring = FALSE) {
    $questionResponse = db_query(
      "SELECT interaction_type, description, correct_responses_pattern,
      response, extras
      FROM {quiz_h5p_user_results}
      WHERE question_nid = :question_nid
      AND question_vid = :question_vid
      AND result_id = :result_id",
      array(
        'question_nid' => $this->question->nid,
        'question_vid' => $this->question->vid,
        'result_id' => $this->rid
      ))->fetchObject();

    // Return themed report
    if (isset($questionResponse->interaction_type) &&
        isset($questionResponse->description) &&
        isset($questionResponse->correct_responses_pattern) &&
        isset($questionResponse->response)
    ) {
      return array('#markup' => theme('quiz_h5p_response', (array)$questionResponse));
    }
    else {
      return array('#no-response' => TRUE);
    }
  }


  public function getReportFormAnswerFeedback($showpoints = TRUE, $showfeedback = TRUE, $allow_scoring = FALSE) {
    return FALSE;
  }
}
