<?php

// $Id$
/**
 * quiz_directions.classes
 *
 * This module uses the question interface to define something which is actually not a question.
 *
 * A Quiz Directions node is a placeholder for adding directions to a quiz. It can be inserted any number
 * of times into a quiz. Example uses may include:
 *
 * - Initial quiz-wide directions
 * - Section directions, e.g. "The next five questions are multiple choice, please..."
 * - Final confirmation, e.g. "You have answered all questions. Click submit to submit this quiz."
 *
 * @file
 */

/**
 * Implementation of QuizQuestion.
 */
class QuizDirectionsQuestion extends QuizQuestion {

  public function __construct(stdClass $node) {
    parent::__construct($node);
    $this->node->no_skip_button = TRUE; // No skip button
    $this->node->not_a_question = TRUE;
  }

  public function validateNode(array &$form) {}
  public function saveNodeProperties($is_new = FALSE) {}

  // This is called whenever a question is rendered, either
  // to an administrator or to a quiz taker.
  public function getAnsweringForm(array $form_state = NULL, $rid) {
    $form['question'] = array(
       '#type' => 'markup',
       '#value' => check_markup($this->node->body, $this->node->format, FALSE),
     );
     $form['tries'] = array(
       '#type' => 'hidden',
       '#value' => 0,
     );

     $form['empty_space'] = array(
       '#type' => 'markup',
       '#value' => '<br/>',
     );

     return $form;
  }
  public function getBodyFieldTitle() {
    return t('Direction');
  }
  public function getCreationForm(array $form_state = NULL) {
    return array();
  }
  public function getMaximumScore() {
    return 0;
  }

}

/**
 * Class that describes a "Directions question response". For
 * the most part, no real scoring takes place for a direction
 * node. However, there are a few behind-the-scenes tricks that
 * are done here to make the quiz-taking process a little easier.
 */
class QuizDirectionsResponse extends QuizQuestionResponse {

  public function save() {}
  public function delete() {}
  public function score() {
    // First, due to popular demand, if the directions are at the beginning of
    // the quiz, we restart the timer after the user has read the question.
    $quiz_nid = arg(1);
    $quiz_key = 'quiz_' . $quiz_nid;
    if (isset($_SESSION[$quiz_key]['previous_quiz_questions']) && count($_SESSION[$quiz_key]['previous_quiz_questions']) === 1) {
      // reset the timer.
      $sql = 'UPDATE {quiz_node_results} SET time_start=%d WHERE result_id = %d';
      db_query($sql, time(), $this->rid);
    }

    // Set the score
    $this->score = 0;
    return 0;
  }
  public function isCorrect() {
    return TRUE;
  }
  public function getResponse() {
    return $this->answer;
  }
  public function getReportForm($showpoints = TRUE, $showfeedback = TRUE) {
    return array(
      '#no_report' => TRUE,
    );
  }
  public function getReportFormResponse($showpoints = TRUE, $showfeedback = TRUE, $allow_scoring = FALSE) {
    return '';
  }
}




