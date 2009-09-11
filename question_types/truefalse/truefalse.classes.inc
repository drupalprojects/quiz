<?php

// $Id$

/**
 * Defines the classes necessary for a True/False quiz.
 *
 * @file
 */

/**
 * Implementation of QuizQuestion.
 */
class TrueFalseQuestion extends QuizQuestion {

  public function save($is_new = FALSE) {
    parent::save($is_new);

    if (!isset($this->node->feedback)) {
      $this->node->feedback = '';
    }

    if ($is_new || $this->node->revision == 1) {
      $sql = "INSERT INTO {quiz_truefalse_node} (nid, vid, correct_answer, feedback) VALUES (%d, %d, %d, '%s')";
      db_query($sql, $this->node->nid, $this->node->vid, (int)$this->node->correct_answer, $this->node->feedback);
    }
    else {
      drupal_set_message('Updating', 'status');
      $sql = "UPDATE {quiz_truefalse_node} SET correct_answer = %d, feedback = '%s' WHERE nid = %d AND vid = %d";
      db_query($sql, (int)$this->node->correct_answer, $this->node->feedback, $this->node->nid, $this->node->vid);
    }
  }

  public function validateNode(array &$form) {
    // This space intentionally left blank. :)
  }

  public function delete($only_this_version = FALSE) {
    parent::delete($only_this_version);

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

  public function getNodeProperties() {
    $sql = 'SELECT correct_answer, feedback FROM {quiz_truefalse_node} WHERE nid = %d AND vid = %d';
    $result = db_fetch_object(db_query($sql, $this->node->nid, $this->node->vid));
    return $result;
  }

  public function getNodeView() {
    $content = parent::getNodeView();

    $answer = ($this->node->correct_answer) ? t('True') : t('False');
    // leave other values of 'answers' element from parent
    $content['answers']['#type'] = 'markup';
    $content['answers']['#value'] = '<div class="quiz answers" style="background-color: lightgreen">'. $answer .'</div>';

    return $content;
  }


  // This is called whenever a question is rendered, either
  // to an administrator or to a quiz taker.
  public function getAnsweringForm(array $form_state = NULL) {

    // Question first
    $form['question'] = array(
      '#type' => 'markup',
      '#value' => check_markup($this->node->body, $this->node->format, FALSE)
    );

    // 'tries' is unfortunately required by quiz.module
    $form['tries'] = array(
      '#type' => 'radios',
      '#title' => t('Choose one'),
      '#options' => array(
    1 => t('True'),
    0 => t('False')
    ),
    //'#default_value' => 1,
      '#required' => TRUE,
    );

    return $form;
  }


  public function getAdminForm(array $form_state = NULL) {
    $form['settings'] = array(
      '#type' => 'markup',
      '#value' => t('There are no settings for this question type.'),
    );
    return $form;
  }


  public function getCreationForm(array $form_state = NULL) {
    $form['correct_answer'] = array(
      '#type' => 'radios',
      '#title' => t('Correct answer'),
      '#options' => array(
        1 => t('True'),
        0 => t('False'),
      ),
      '#default_value' => isset($this->node->correct_answer) ? $this->node->correct_answer : 1,
      '#required' => TRUE,
      '#weight' => -4,
    );
    $form['feedback_fields'] = array(
      '#type' => 'fieldset',
      '#title' => t('Feedback Settings'),
      '#description' => t('Settings pertaining to feedback given along with results.'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
      '#weight' => -3,
    );
    $form['feedback_fields']['feedback'] = array(
      '#type' => 'textarea',
      '#title' => t('Feedback Text'),
      '#description' => t('Text to be displayed when the results are displayed'),
      '#rows' => 5,
      '#cols' => 60,
      '#required' => FALSE,
      '#default_value' => isset($this->node->feedback) ? $this->node->feedback : '',
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

  public function __construct($result_id, stdClass $question_node, $answer = NULL) {
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

  public function save() {
    $sql = "INSERT INTO {quiz_truefalse_user_answers} (question_nid, question_vid, result_id, answer, score) VALUES (%d, %d, %d, %d, %d)";
    db_query($sql, $this->question->nid, $this->question->vid, $this->rid, (int)$this->answer, (int)$this->getScore());
  }

  public function delete() {
    $sql = "DELETE FROM {quiz_truefalse_user_answers} WHERE question_nid = %d AND question_vid = %d AND result_id = %d";
    db_query($sql, $this->question->nid, $this->question->vid, $this->rid);
  }

  public function score() {
    $tfQuestion = new TrueFalseQuestion($this->question);
    return ($this->getResponse() == $tfQuestion->getCorrectAnswer()) ? 1 : 0;
  }

  public function getResponse() {
    if (!isset($this->answer)) {
      $correct_answer = $this->getCorrectAnswer();
      $this->answer = $correct_answer->answer;
    }
    return $this->answer;
  }

  public function getCorrectAnswer() {
    $sql = "SELECT answer, score FROM {quiz_truefalse_user_answers} WHERE question_vid = %d AND result_id = %d";
    return db_fetch_object(db_query($sql, $this->question->vid, $this->rid));
  }

  public function formatReport($showpoints = TRUE, $showfeedback = TRUE) {

    // Build the question answers header (add blank space for IE).
    if ($showpoints) {
      $innerheader[] = t('Correct Answer');
    }
    $innerheader[] = t('User Answer');
    if ($showfeedback) {
      $innerheader[] = '&nbsp;';
    }

    if (empty($this->question->answers)) {
      return t('Missing question.');
    }

    $answer = $this->question->answers[0];
    $correct_answer = $answer['is_correct'] ? $answer['answer'] : !$answer['answer'];
    $user_answer = $answer['answer'];

    if ($showpoints) {
      $rows[0][] = ($correct_answer ? t('True') : t('False'));
    }
    $rows[0][] = ($user_answer ? t('True') : t('False'));
    if ($showfeedback && !empty($this->question->feedback)) {
      $rows[0][] = $this->question->feedback;
    }

    // Add the cell with the question and the answers.
    $q_output = '<div class="quiz_summary_question"><span class="quiz_question_bullet">Q:</span> '. check_markup($this->question->body, $this->question->format, FALSE) .'</div>';
    $q_output .= theme('table', $innerheader, $rows) .'<br />';
    return $q_output;
  }
}
