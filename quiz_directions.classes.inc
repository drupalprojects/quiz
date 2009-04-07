<?php

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
class QuizDirectionsQuestion implements QuizQuestion {
  
  /**
   * The current node for this question.
   */
  protected $node = NULL;
  
  public function __construct($node) {
    $this->node = $node;
    $this->node->no_skip_button = TRUE; // No skip button
    $this->node->not_a_question = TRUE;
  }
  
  public function save($is_new = FALSE) {}
  
  public function validate($node, &$form) {}

  public function delete($only_this_version = FALSE) {}
  
  public function load() {}
  
  public function view() {
    return $this->getQuestionForm($this->node);
  }
  
  // This is called whenever a question is rendered, either 
  // to an administrator or to a quiz taker.
  public function getQuestionForm($node, $context = NULL) {
    $form['question'] = array(
       '#type' => 'markup', 
       '#value' => $node->body
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
  public function getAdminForm($edit = NULL) {
    $form['settings'] = array(
      '#type' => 'markup',
      '#value' => t('There are no settings for this question type.'),
    );
    return $form;
  }
  public function getCreationForm($edit) {
    return array();
  }
  public function getMaximumScore() {
    return 0;
  }

}

class QuizDirectionsResponse extends AbstractQuizQuestionResponse {
  
  public function __construct($rid, $question, $answer = NULL) {
    $this->answer = $answer;
    $this->question = $question;
    $this->rid = $rid;
  }
  
  public function save() {}
  public function delete() {}
  public function score() {
    $this->score = 0;
    return 0;
  }
  public function isCorrect() {
    return TRUE;
  }
  public function getResponse() {
    return $answer;
  }
  public function formatReport($showpoints = TRUE, $showfeedback = TRUE) {
    return '';
  }
}




