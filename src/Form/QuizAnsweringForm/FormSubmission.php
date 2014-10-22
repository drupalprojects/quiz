<?php

namespace Drupal\quiz\Form\QuizAnsweringForm;

class FormSubmission {

  private $quiz;
  private $result;

  public function __construct($quiz, $result) {
    $this->quiz = $quiz;
    $this->result = $result;
    $this->quiz_id = isset($quiz->nid) ? $quiz->nid : $quiz->qid;
  }

  /**
   * Submit handler for "back".
   */
  public function formBackSubmit(&$form, &$form_state) {
    // Back a question.
    $this->redirect($this->quiz, $_SESSION['quiz'][$this->quiz_id]['current'] - 1);
    $question = $this->result->layout[$_SESSION['quiz'][$this->quiz_id]['current']];
    if (!empty($question['qr_pid'])) {
      foreach ($this->result->layout as $item) {
        if ($item['qr_id'] == $question['qr_pid']) {
          $this->redirect($this->quiz, $item['number']);
        }
      }
    }
    $form_state['redirect'] = "node/$this->quiz_id/take/" . ($_SESSION['quiz'][$this->quiz_id]['current']);
  }

  /**
   * Submit action for "leave blank".
   */
  public function formBlankSubmit($form, &$form_state) {
    $result_id = $_SESSION['quiz'][$this->quiz_id]['result_id'];
    $quiz_result = quiz_result_load($result_id);
    $questions = $quiz_result->layout;
    foreach ($form_state['input']['question'] as $nid => $input) {
      // Loop over all question inputs provided, and record them as skipped.
      $question = node_load($nid);

      foreach ($questions as $question_item) {
        if ($question_item['nid'] == $question->nid) {
          $question_array = $question_item;
        }
      }

      $qi_instance = _quiz_question_response_get_instance($result_id, $question, NULL);
      $qi_instance->delete();
      $bare_object = $qi_instance->toBareObject();
      quiz()->getQuizHelper()->saveQuestionResult($this->quiz, $bare_object, array('set_msg' => TRUE, 'question_data' => $question_array));
    }

    // Advance to next question.
    $this->redirect($this->quiz, $_SESSION['quiz'][$this->quiz_id]['current'] + 1);
    $form_state['redirect'] = "node/{$this->quiz_id}/take/" . $_SESSION['quiz'][$this->quiz_id]['current'];
  }

  /**
   * Submit handler for the question answering form.
   *
   * There is no validation code here, but there may be feedback code for
   * correct feedback.
   */
  public function formSubmit(&$form, &$form_state) {
    global $user;

    if (!empty($form_state['values']['question'])) {
      foreach ($form_state['values']['question'] as $nid => $answer) {
        $current_question = node_load($nid);
        foreach ($this->result->layout as $item) {
          if ($item['nid'] == $current_question->nid) {
            $question_array = $item;
          }
        }
        $qi_instance = _quiz_question_response_get_instance($_SESSION['quiz'][$this->quiz_id]['result_id'], $current_question, $form_state['values']['question'][$current_question->nid]);
        $qi_instance->delete();
        $qi_instance->saveResult();
        $result = $qi_instance->toBareObject();
        quiz()->getQuizHelper()->saveQuestionResult($this->quiz, $result, array('set_msg' => TRUE, 'question_data' => $question_array));

        // Increment the counter.
        $this->redirect($this->quiz, $_SESSION['quiz'][$this->quiz_id]['current'] + 1);
      }
    }

    // Wat do?
    if (!empty($this->quiz->review_options['question']) && array_filter($this->quiz->review_options['question'])) {
      // We have question feedback.
      $form_state['redirect'] = "node/$this->quiz_id/take/" . ($_SESSION['quiz'][$this->quiz_id]['current'] - 1) . '/feedback';
    }
    else {
      // No question feedback. Go to next question.
      $form_state['redirect'] = "node/$this->quiz_id/take/" . ($_SESSION['quiz'][$this->quiz_id]['current']);
    }

    if (!isset($this->result->layout[$_SESSION['quiz'][$this->quiz_id]['current']])) {
      // No more questions. Score quiz.
      $score = quiz_end_scoring($_SESSION['quiz'][$this->quiz_id]['result_id']);

      // Delete old results if necessary.
      quiz()->getQuizHelper()->getResultHelper()->maintainResult($user, $this->quiz, $this->result->result_id);
      if (empty($this->quiz->review_options['question']) || !array_filter($this->quiz->review_options['question'])) {
        // Only redirect to question results if there is not question feedback.
        $form_state['redirect'] = "node/{$this->quiz_id}/quiz-results/{$this->result->result_id}/view";
      }

      quiz_end_actions($this->quiz, $score, $_SESSION['quiz'][$this->quiz_id]);

      // Remove all information about this quiz from the session.
      // @todo but for anon, we might have to keep some so they could access
      // results
      // When quiz is completed we need to make sure that even though the quiz has
      // been removed from the session, that the user can still access the
      // feedback for the last question, THEN go to the results page.
      $_SESSION['quiz']['temp']['result_id'] = $this->result->result_id;
      unset($_SESSION['quiz'][$this->quiz_id]);
    }
  }

}
