<?php

/* @file
 * Contains \Drupal\multichoice\MultichoiceResponse.
 */

namespace Drupal\cloze;

use Drupal\quiz_question\QuizQuestionResponse;

/**
 * Extension of QuizQuestionResponse
 */
class ClozeResponse extends QuizQuestionResponse {
  /**
   * ID of the answer.
   */
  protected $answer_id = 0;

  /**
   * Constructor
   */
  public function __construct($result_id, stdClass $question_node, $answer = NULL) {
    parent::__construct($result_id, $question_node, $answer);
    if (!isset($answer)) {
      $r = db_query("SELECT answer_id, answer, score, question_vid, question_nid, result_id FROM {quiz_cloze_user_answers} WHERE question_nid = :question_nid AND question_vid = :question_vid AND result_id = :result_id", array(':question_nid' => $question_node->id(), ':question_vid'=>$question_node->getRevisionId(), ':result_id' => $result_id))->fetch();
      if (!empty($r)) {
        $this->answer = unserialize($r->answer);
        $this->score = $r->score;
        $this->answer_id = $r->answer_id;
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
    return TRUE;
  }

  /**
   * Implementation of save
   *
   * @see QuizQuestionResponse#save()
   */
  public function save() {
    $this->answer_id = db_insert('quiz_cloze_user_answers')
      ->fields(array(
        'answer' => serialize($this->answer),
        'question_nid' => $this->question->id(),
        'question_vid' => $this->question->getRevisionId(),
        'result_id' => $this->rid,
        'score' => $this->getScore(FALSE),
      ))
      ->execute();
  }

  /**
   * Implementation of delete()
   *
   * @see QuizQuestionResponse#delete()
   */
  public function delete() {
    db_delete('quiz_cloze_user_answers')
      ->condition('question_nid', $this->question->id())
      ->condition('question_vid', $this->question->getRevisionId())
      ->condition('result_id', $this->rid)
      ->execute();
  }

  /**
   * Implementation of score()
   *
   * @see QuizQuestionResponse#score()
   */
  public function score() {
    $shortAnswer = new ClozeQuestion($this->question);
    $score = $shortAnswer->evaluateAnswer($this->answer);
    return $score;
  }

  /**
   * Implementation of getResponse()
   *
   * @see QuizQuestionResponse#getResponse()
   */
  public function getResponse() {
    return $this->answer;
  }

  /**
   * Implementation of getReportForm()
   *
   * @see QuizQuestionResponse#getReportForm($showpoints, $showfeedback, $allow_scoring)
   */
  public function getReportForm($showpoints = TRUE, $showfeedback = TRUE, $allow_scoring = FALSE) {
    $form = parent::getReportForm($showpoints, $showfeedback, $allow_scoring);
    $question = strip_tags($form['question']['#markup']);
    $question_form['open_wrapper'] = array(
      '#markup' => '<div class="cloze-question">',
    );
    foreach (_cloze_get_question_chunks($question) as $position => $chunk) {
      if (strpos($chunk, '[') === FALSE) {
        // this "tries[foobar]" hack is needed becaues response handler engine checks for input field
        // with name tries
        $question_form['tries['. $position .']'] = array(
          '#markup' => str_replace("\n", "<br/>", $chunk),
          '#prefix' => '<div class="form-item">',
          '#suffix' => '</div>',
        );
      }
      else {
        $chunk = str_replace(array('[', ']'), '', $chunk);
        $choices = explode(',', $chunk);
        if (count($choices) > 1) {
          $question_form['tries['. $position .']'] = array(
            '#type' => 'select',
            '#title' => '',
            '#options' => _cloze_shuffle_choices(drupal_map_assoc($choices)),
            '#required' => FALSE,
          );
        }
        else {
          $question_form['tries['. $position .']'] = array(
            '#type' => 'textfield',
            '#title' => '',
            '#size' => 32,
            '#required' => FALSE,
            '#attributes' => array(
              'autocomplete' => 'off',
            ),
          );
        }
      }
    }
    $question_form['close_wrapper'] = array(
      '#markup' => '</div>',
    );
    $form['question']['#markup'] = drupal_render($question_form);
    return $form;
  }

  /**
   * Implementation of getReportFormResponse()
   *
   * @see QuizQuestionResponse#getReportFormResponse($showpoints, $showfeedback, $allow_scoring)
   */
  public function getReportFormResponse($showpoints = TRUE, $showfeedback = TRUE, $allow_scoring = FALSE) {
    $form = array();
    $form['#theme'] = 'cloze_response_form';
    $form['#attached']['css'] = array(
      drupal_get_path('module', 'cloze') . '/css/cloze.css'
    );
    if (($this->question) && !empty($this->question->answers)) {
      $answer = (object) current($this->question->answers);
    }
    else {
      return $form;
    }
    $body = $this->question->body->getValue();
    $this->question = node_load($this->question->id());
    $question = $body[0]['value'];
    $correct_answer = _cloze_get_correct_answer($question);
    $user_answer = _cloze_get_user_answer($question, $this->answer);
    $form['answer'] = array(
      '#markup' => theme('cloze_user_answer', array('answer' => $user_answer, 'correct' => $correct_answer)),
    );
    return $form;
  }

  /**
   * Implementation of getReportFormScore()
   *
   * @see QuizQuestionResponse#getReportFormScore($showpoints, $showfeedback, $allow_scoring)
   */
  public function getReportFormScore($showfeedback = TRUE, $showpoints = TRUE, $allow_scoring = FALSE) {
    return array(
      '#markup' => $this->getScore(),
    );
  }
}

