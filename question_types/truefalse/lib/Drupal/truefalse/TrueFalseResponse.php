<?php

/**
 * Extension of QuizQuestionResponse
 * Contains Drupal\truefalse\TrueFalseResponse.
 */
 
namespace Drupal\truefalse;

use Drupal\quiz_question\QuizQuestionResponse;
 
class TrueFalseResponse extends QuizQuestionResponse {

  /**
   * Constructor
   */
  public function __construct($result_id, $question_node, $answer = NULL) {
    parent::__construct($result_id, $question_node, $answer);
    if (!isset($answer)) {
      $r = $this->getCorrectAnswer();
      if (!empty($r)) {
        $this->answer = $r->answer;
        $this->score = $r->score;
      }
    }
    else {
      $this->answer = $answer;
    }
  }

  /**
   * Implementation of isValid
   *
   * @see QuizQuestionResponse#isValid()
   */
  public function isValid() {
    return ($this->answer === NULL) ? t('You must provide an answer') : TRUE;
  }

  /**
   * Implementation of save
   *
   * @see QuizQuestionResponse#save()
   */
  public function save() {
    db_insert('quiz_truefalse_user_answers')
      ->fields(array(
        'question_nid' => $this->question->id(),
        'question_vid' => $this->question->getRevisionId(),
        'result_id' => $this->rid,
        'answer' => (int) $this->answer,
        'score' => (int) $this->getScore(),
      ))
      ->execute();
  }

  /**
   * Implementation of delete
   *
   * @see QuizQuestionResponse#delete()
   */
  public function delete() {
    db_delete('quiz_truefalse_user_answers')
      ->condition('question_nid', $this->question->id())
      ->condition('question_vid', $this->question->getRevisionId())
      ->condition('result_id', $this->rid)
      ->execute();
  }

  /**
   * Implementation of score
   *
   * @see QuizQuestionResponse#score()
   */
  public function score() {
    $tfQuestion = new TrueFalseQuestion($this->question);
    return ($this->getResponse() == $tfQuestion->getCorrectAnswer()) ? 1 : 0;
  }

  /**
   * Implementation of getResponse
   *
   * @see QuizQuestionResponse#getResponse()
   */
  public function getResponse() {
    if (!isset($this->answer)) {
      $correct_answer = $this->getCorrectAnswer();
      $this->answer = isset($correct_answer->answer) ? $correct_answer->answer : NULL;
    }
    return $this->answer;
  }

  /**
   * Implementation of getCorrectAnswer
   */
  public function getCorrectAnswer() {
    if ($this->question instanceof \stdClass) {
      $params = array(':qvid' => $this->question->vid, ':rid' => $this->rid);
    }
    else {
      $params = array(':qvid' => $this->question->getRevisionId(), ':rid' => $this->rid);
    }
    return db_query('SELECT answer, score FROM {quiz_truefalse_user_answers} WHERE question_vid = :qvid AND result_id = :rid', $params)->fetch();
  }

  /**
   * Implementation of getReportFormResponse
   *
   * @see QuizQuestionResponse#getReportFormResponse($showpoints, $showfeedback, $allow_scoring)
   */
  public function getReportFormResponse($showpoints = TRUE, $showfeedback = TRUE, $allow_scoring = FALSE) {
    if (empty($this->question->answers)) {
      return array(
        '#markup' => t('Missing question.'),
      );
    }
    $metadata = array();
    $data = array();
    // Build the question answers header (add blank space for IE).
    if ($showpoints) {
      $metadata[] = t('Correct Answer');
    }
    $metadata[] = t('User answer');
    if ($showfeedback && !empty($this->question->feedback)) {
      $metadata[] = 'Feedback';
    }

    $answer = $this->question->answers[0];
    $correct_answer = $answer['is_correct'] ? $answer['answer'] : !$answer['answer'];
    $user_answer = $answer['answer'];

    if ($showpoints) {
      $data[0]['correct_answer'] = ($correct_answer ? t('True') : t('False'));
    }
    $data[0]['user_answer'] = (($user_answer === NULL) ? '' : ($user_answer ? t('True') : t('False')));

    if ($showfeedback && !empty($this->question->feedback)) {
      $data[0]['feedback'] = check_markup($this->question->feedback, $this->getFormat());
    }

    // Return themed output
    return array(
      '#markup' => theme('truefalse_response', array('metadata' => $metadata, 'data' => $data)),
    );
  }
}
