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

    $percentScore = 0;
    $data = json_decode($this->answer);

    if ($data->answer !== 1 && is_numeric($data->answer)) {
      // Answer is the "encrypted" percentScore. Let's "decrypt" it...
      $percentScore = $data->answer / 1.234 - 32.17;
    }
    if ($percentScore < 0) {
      $percentScore = 0;
    }
    elseif ($percentScore > 1) {
      $percentScore = 0;
    }

    $interactionType = isset($data->interactionType) ?
      json_encode($data->interactionType) : null;
    $correctResponsesPattern = isset($data->correctResponsesPattern) ?
      json_encode($data->correctResponsesPattern) : null;
    $response = isset($data->response) ? json_encode($data->response) : null;
    $extras = isset($data->extras) ? json_encode($data->extras) : null;

    $description = null;
    if (isset($data->description)) {
      $convertedDescription = (array) $data->description;
      $description = json_encode($convertedDescription['en-US']);
    }

    db_merge('quiz_h5p_user_results')
      ->key(array(
        'question_nid' => $this->question->nid,
        'question_vid' => $this->question->vid,
        'result_id' => $this->rid,
      ))
      ->fields(array(
        'question_nid' => $this->question->nid,
        'question_vid' => $this->question->vid,
        'result_id' => $this->rid,
        'percent_score' => $percentScore,
        'interaction_type' => $interactionType,
        'description' => $description,
        'correct_responses_pattern' => $correctResponsesPattern,
        'response' => $response,
        'extras' => $extras
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

  /**
   * Implements getReportFormResponse of QuizQuestionResponse interface
   */
  public function getReportFormResponse($showpoints = TRUE, $showfeedback = TRUE, $allow_scoring = FALSE) {
    $questionResponse = db_query(
      "SELECT interaction_type, description, correct_responses_pattern, response
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
