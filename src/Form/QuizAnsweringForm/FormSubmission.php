<?php

namespace Drupal\quiz\Form\QuizAnsweringForm;

use Drupal\quiz\Entity\QuizEntity;
use Drupal\quiz\Entity\Result;
use Drupal\quiz\Helper\Quiz\QuestionHelper;
use stdClass;

class FormSubmission extends QuestionHelper {

  private $quiz;
  private $quiz_id;
  private $quiz_uri;

  /** @var Result */
  private $result;
  private $page_number;

  /**
   * @param QuizEntity $quiz
   * @param stdClass $result
   * @param int $page_number
   */
  public function __construct($quiz, $result, $page_number) {
    $this->quiz = $quiz;
    $this->quiz_id = $quiz->qid;
    $this->quiz_uri = isset($quiz->nid) ? 'node/' . $quiz->nid : 'quiz/' . $quiz->qid;
    $this->result = $result;
    $this->page_number = $page_number;
  }

  /**
   * Submit handler for "back".
   */
  public function formBackSubmit(&$form, &$form_state) {
    $this->redirect($this->quiz, $this->page_number - 1);
    $item = $this->result->layout[$this->page_number];
    if (!empty($item['qr_pid'])) {
      foreach ($this->result->layout as $item) {
        if ($item['qr_id'] == $item['qr_pid']) {
          $this->redirect($this->quiz, $item['number']);
        }
      }
    }
    $form_state['redirect'] = $this->quiz_uri . '/take/' . $this->getCurrentPageNumber($this->quiz);
  }

  /**
   * Submit action for "leave blank".
   */
  public function formBlankSubmit($form, &$form_state) {
    foreach (array_keys($form_state['input']['question']) as $question_id) {
      // Loop over all question inputs provided, and record them as skipped.
      $question = node_load($question_id);

      foreach ($this->result->layout as $question_item) {
        if ($question_item['nid'] == $question->nid) {
          $question_array = $question_item;
        }
      }

      $qi_instance = _quiz_question_response_get_instance($this->result->result_id, $question, NULL);
      $qi_instance->delete();
      $bare_object = $qi_instance->toBareObject();
      quiz()
        ->getQuizHelper()
        ->saveQuestionResult($this->quiz, $bare_object, array('set_msg' => TRUE, 'question_data' => $question_array));
    }

    // Advance to next question.
    $this->redirect($this->quiz, $this->result->getNextPageNumber($this->page_number));
    $form_state['redirect'] = $this->quiz_uri . '/take/' . $this->getCurrentPageNumber($this->quiz);
  }

  /**
   * Submit handler for the question answering form.
   *
   * There is no validation code here, but there may be feedback code for
   * correct feedback.
   */
  public function formSubmit(&$form, &$form_state) {
    if (!empty($form_state['values']['question'])) {
      foreach (array_keys($form_state['values']['question']) as $question_id) {
        foreach ($this->result->layout as $item) {
          if ($item['nid'] == $question_id) {
            $question_array = $item;
          }
        }
        $_question = node_load($question_id);
        $_answer = $form_state['values']['question'][$question_id];
        $qi_instance = _quiz_question_response_get_instance($this->result->result_id, $_question, $_answer);
        $qi_instance->delete();
        $qi_instance->saveResult();
        $result = $qi_instance->toBareObject();
        quiz()
          ->getQuizHelper()
          ->saveQuestionResult($this->quiz, $result, array('set_msg' => TRUE, 'question_data' => $question_array));

        // Increment the counter.
        $this->redirect($this->quiz, $this->result->getNextPageNumber($this->page_number));
      }
    }

    // In case we have question feedback, redirect to feedback form.
    $form_state['redirect'] = $this->quiz_uri . '/take/' . $this->getCurrentPageNumber($this->quiz);
    if (!empty($this->quiz->review_options['question']) && array_filter($this->quiz->review_options['question'])) {
      $form_state['redirect'] = $this->quiz_uri . '/take/' . ($this->getCurrentPageNumber($this->quiz) - 1) . '/feedback';
    }

    if ($this->result->isLastPage($this->page_number)) {
      $this->formSubmitLastPage($form_state);
    }
  }

  private function formSubmitLastPage(&$form_state) {
    global $user;

    // No more questions. Score quiz.
    $score = quiz_end_scoring($this->result->result_id);

    // Delete old results if necessary.
    quiz()->getQuizHelper()->getResultHelper()->maintainResult($user, $this->quiz, $this->result->result_id);

    // Only redirect to question results if there is not question feedback.
    if (empty($this->quiz->review_options['question']) || !array_filter($this->quiz->review_options['question'])) {
      $form_state['redirect'] = "quiz-result/{$this->result->result_id}";
    }

    quiz_end_actions($this->quiz, $score, $this->quiz_id);

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
