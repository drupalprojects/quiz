<?php

/**
 * Defines the classes necessary for a True/False quiz.
 *
 * @file
 */

/**
 * Implementation of QuizQuestion.
 */
class TrueFalseQuestion implements QuizQuestion {
  
  /**
   * The current node for this question.
   */
  protected $node = NULL;
  
  public function __construct($node) {
    $this->node = $node;
  }
  
  public function save($is_new = FALSE) {
    if ($is_new || $this->node->revision == 1) {
      $sql = "INSERT INTO {quiz_truefalse_node} (nid, vid, correct_answer) VALUES (%d, %d, %d)";
      db_query($sql, $this->node->nid, $this->node->vid, (int)$this->node->correct_answer);
    }
    else {
      $sql = "UPDATE {quiz_truefalse_node} SET correct_answer = %d WHERE nid = %d AND vid = %d";
      db_query($sql, (int)$this->node->correct_answer, $this->node->nid, $this->node->vid);
    }
  }
  
  public function validate($node, &$form) {
    // This space intentionally left blank. :)
  }

  public function delete($only_this_version = FALSE) {
    // Only delete a nid/vid.
    if ($only_this_version) {
      $sql = 'DELETE FROM {quiz_truefalse_node} WHERE nid = %d AND vid = %d';
      db_query($sql, $this->node->nid, $this->node->vid);
    }
    // Delete all versions of the quiz question.
    else {
      $sql = 'DELETE FROM {quiz_truefalse_node} WHERE nid = %d';
      db_query($sql, $this->node->nid, $this->node->vid);
    }
  }
  
  public function view() {
    return $this->getQuestionForm($this->node);
  }
  
  // This is called whenever a question is rendered, either 
  // to an administrator or to a quiz taker.
  public function getQuestionForm($node, $context = NULL) {
    
    // Question first
    $form['question'] = array(
      '#type' => 'markup', 
      '#value' => $node->body
    );
    
    // 'tries' is unfortunately required by quiz.module
    $form['tries'] = array(
      '#type' => 'radios',
      '#title' => t('Choose one'),
      '#options' => array(
        1 => t('True'),
        0 => t('False'),
      ),
      '#default_value' => 'true',
      '#required' => TRUE,
    );
    
    // Submit button
    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Submit'),
    );
    
    return $form;
  }
  public function getAdminForm() {
    $form['settings'] = array(
      '#type' => 'markup',
      '#value' => t('There are no settings for this question type.'),
    );
    return $form;
  }
  public function getCreationForm() {
    $form['correct_answer'] = array(
      '#type' => 'radios',
      '#title' => t('Correct answer'),
      '#options' => array(
        1 => t('True'),
        0 => t('False'),
      ),
      '#default_value' => 'true',
      '#required' => TRUE,
    );
    return $form;
  }
  public function getMaximumScore() {
    return 1;
  }
  
  /**
   * Get the answer to this question.
   *
   * This is a utility function. It is not defined in the interface.
   */
  public function getCorrectAnswer() {
    $sql = "SELECT correct_answer FROM {quiz_truefalse_node} WHERE nid = %d AND vid = %d";
    return db_result(db_query($sql, $this->node->nid, $this->node->vid));
  }
}

class TrueFalseResponse extends AbstractQuizQuestionResponse {
  
  public function __construct($rid, $question, $answer = NULL) {
    $this->rid = $rid;
    $this->question = $question;
    
    if (!isset($answer)) {
      $sql = "SELECT answer, score FROM {quiz_truefalse_user_answers} WHERE question_vid = %d AND result_id = %d";
      $r = db_fetch_object(db_query($sql, $question->vid, $rid));
      if (!empty($r)) {
        $this->answer = $r->answer;
        $this->score = $r->score;
      }
    }
    else {
      $this->answer = $answer;
    }
  }
  
  public function save() {
    $sql = "INSERT INTO {quiz_truefalse_user_answers} (question_nid, question_vid, result_id, answer, score) VALUES (%d, %d, %d, %d, %d)";
    db_query($sql, $this->question->nid, $this->question->vid, $this->rid, (int)$this->answer, (int)$this->score);
  }
  public function delete() {
    $sql = 'DELETE FROM {quiz_truefalse_user_answers} WHERE question_nid = %d AND question_vid = %d AND result_id = %d';
    db_query($sql, $this->question->nid, $this->question->vid, $this->rid);
  }
  public function score() {
    $tfQuestion = new TrueFalseQuestion($this->question);
    $this->score = (int)($this->getResponse() == $tfQuestion->getCorrectAnswer());
    return $this->score;
  }
  public function getResponse() {
    return $this->answer;
  }
}