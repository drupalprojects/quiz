<?php
/**
 * @file
 * 
 * The main classes for the short answer response.
 *
 * Contains \Drupal\short_answer\ShortAnswerResponse.
 */

namespace Drupal\scale;

use Drupal\quiz_question\QuizQuestionResponse;
use Drupal\scale\ScaleQuestion;


/**
 * Extension of QuizQuestionResponse
 */
class ScaleResponse extends QuizQuestionResponse {
  /**
   * ID of the answer.
   */
  protected $answer_id = 0;

  /**
   * Constructor
   */
  public function __construct($result_id, $question_node, $answer = NULL) {
    parent::__construct($result_id, $question_node, $answer);

    if (isset($answer)) {
      $this->answer_id = intval($answer);
    }
    else {
      $this->answer_id = db_query('SELECT answer_id FROM {quiz_scale_user_answers} WHERE result_id = :rid AND question_nid = :qnid AND question_vid = :qvid', array(':rid' => $result_id, ':qnid' => $this->question->id(), ':qvid' => $this->question->getRevisionId()))->fetchField();
    }
    $answer = db_query('SELECT answer FROM {quiz_scale_answer} WHERE id = :id', array(':id' => $this->answer_id))->fetchField();
    $this->answer = check_plain($answer);
  }


  public function isValid() {
    if (empty($this->answer_id)) {
      return t('You must provide an answer');
    }
    return TRUE;
  }

  /**
   * Implementation of save
   *
   * @see QuizQuestionResponse#save()
   */
  public function save() {
    $id = db_insert('quiz_scale_user_answers')
      ->fields(array(
        'answer_id' => $this->answer_id,
        'result_id' => $this->rid,
        'question_vid' => $this->question->getRevisionId(),
        'question_nid' => $this->question->id(),
      ))
      ->execute();
  }

  /**
   * Implementation of delete
   *
   * @see QuizQuestionResponse#delete()
   */
  public function delete() {
    db_delete('quiz_scale_user_answers')
      ->condition('result_id', $this->rid)
      ->condition('question_nid', $this->question->id())
      ->condition('question_vid', $this->question->getRevisionId())
      ->execute();
  }


  /**
   * Implementation of score
   *
   * @see QuizQuestionResponse#score()
   */
  public function score() {
    return $this->isValid() ? 1 : 0;
  }

  /**
   * Implementation of getResponse
   *
   * @see QuizQuestionResponse#getResponse()
   */
  public function getResponse() {
    return $this->answer_id;
  }

  /**
   * Implementation of getReportFormResponse
   *
   * @see getReportFormResponse($showpoints, $showfeedback, $allow_scoring)
   */
  public function getReportFormResponse($showpoints = TRUE, $showfeedback = TRUE, $allow_scoring = FALSE) {
    $form = array();
    $form['#theme'] = 'scale_response_form';
    $form['answer'] = array('#markup' => check_plain($this->answer));
    return $form;
  }
}
