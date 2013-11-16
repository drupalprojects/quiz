<?php

/* @file
 * Contains \Drupal\matching\MatchingResponse.
 */

namespace Drupal\matching;

use Drupal\quiz_question\QuizQuestionResponse;

/**
 * Extension of QuizQuestionResponse
 */
class MatchingResponse extends QuizQuestionResponse {

  /**
   * Constructor
   */
  public function __construct($result_id, $question_node, $answer = NULL) {
    parent::__construct($result_id, $question_node, $answer);
    if (!isset($answer)) {
      $res = db_query('SELECT ua.answer, score, ua.match_id FROM {quiz_matching_user_answers} ua
              JOIN {quiz_matching_node} n ON n.match_id = ua.match_id
              WHERE n.nid = :nid AND n.vid = :vid AND ua.result_id = :result_id', array(':nid' => $question_node->id(), ':vid' => $question_node->getRevisionId(), ':result_id' => $result_id));
      $this->answer = array();
      while ($obj = $res->fetch()) {
        $this->answer[$obj->match_id] = $obj->answer;
      }
    }
    $this->is_correct = $this->isCorrect();
  }

  /**
   * Implementation of isValid
   *
   * @see QuizQuestionResponse#isValid()
   */
  public function isValid() {
    foreach ($this->answer as $value) {
      if ($value != 'def') {
        return TRUE;
      }
    }
    return t('You need to match at least one of the items.');
  }

  /**
   * Implementation of save
   *
   * @see QuizQuestionResponse#save()
   */
  public function save() {
    if (!isset($this->answer) || !is_array($this->answer)) {
      return;
    }
    $insert = db_insert('quiz_matching_user_answers')->fields(array('match_id', 'result_id', 'answer', 'score'));
    foreach ($this->answer as $key => $value) {
      $insert->values(array(
        'match_id' => $key,
        'result_id' => $this->rid,
        'answer' => (int) $value,
        'score' => ($key == $value) ? 1 : 0,
      ));
    }
    $insert->execute();
  }

  /**
   * Implementation of delete
   *
   * @see QuizQuestionResponse#delete()
   */
  public function delete() {
    $match_id = db_query('SELECT match_id FROM {quiz_matching_node} WHERE nid = :nid AND vid = :vid', array(':nid' => $this->question->id(), ':vid' => $this->question->getRevisionId()))->fetchCol();
    db_delete('quiz_matching_user_answers')
      ->condition('match_id', is_array($match_id) ? $match_id : array(0), 'IN')
      ->condition('result_id', $this->rid)
      ->execute();
  }

  /**
   * Implementation of score
   *
   * @see QuizQuestionResponse#score()
   */
  public function score() {
    $wrong_answer = 0;
    $correct_answer = 0;
    $user_answers = isset($this->answer['answer']) ? $this->answer['answer'] : $this->answer;
    foreach ((array) $user_answers as $key => $value) {
      if ($key == $value) {
        $correct_answer++;
      }
      elseif ($value == 0 || $value == 'def') {
      }
      else {
        $wrong_answer++;
      }
    }
    $score = $correct_answer - $wrong_answer;
    return $score < 0 ? 0 : $score;
  }

  /**
   * Implementation of getResponse
   *
   * @see QuizQuestionResponse#getResponse()
   */
  public function getResponse() {
    return $this->answer;
  }

  /**
   * Implementation of getReportFormResponse
   *
   * @see QuizQuestionResponse#getReportFormResponse($showpoints, $showfeedback, $allow_scoring)
   */
  public function getReportFormResponse($showpoints = TRUE, $showfeedback = TRUE, $allow_scoring = FALSE) {
    $data = $metadata = array();
    // Build the question answers header (add blank space for IE).
    $metadata[] = t('Match');
    if ($showpoints) {
      $metadata[] = t('Correct Answer');
    }
    $metadata[] = t('User answer');

    $MatchingQuestion = new MatchingQuestion($this->question);
    $correct_answers = $MatchingQuestion->getCorrectAnswer();
    $user_answers = isset($this->answer['answer']) ? $this->answer['answer'] : $this->answer;
    $has_feedback = TRUE;
    foreach ($correct_answers as $correct_answer) {
      $answer_data = array();
      $correct = FALSE;
      $id = NULL;
      $answer_data['question'] = check_markup($correct_answer['question'], $this->getFormat());
      if ( isset($user_answers[$correct_answer['match_id']])) {
        $id = $user_answers[$correct_answer['match_id']];
        $correct = isset($correct_answers[$id]) && $correct_answer['answer'] == $correct_answers[$id]['answer'];
        $answer_data['user_answer'] = isset($correct_answers[$id]) ? check_markup($correct_answers[$id]['answer'], $this->getFormat()) : NULL;
      }
      if ($showpoints) {
        $answer_data['correct_answer'] = $correct_answer['answer'];
      }
      if ($showfeedback && !empty($correct_answer['feedback'])) {
        $answer_data['feedback'] = check_markup($correct_answer['feedback'], $this->getFormat());
      }
      else {
        //$has_feedback = FALSE;
        if ($showfeedback)  {
          $answer_data['feedback'] = '';
        }
      }
      $answer_data['is_correct'] = $correct;
      $data[] = $answer_data;
    }

    if ($showfeedback && $has_feedback) {
      $metadata[] = t('Feedback');
    }

    return array(
      '#markup' => theme('matching_response', array('metadata' => $metadata, 'data' => $data)),
    );
  }
}
