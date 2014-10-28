<?php

namespace Drupal\quiz\Helper\Quiz;

class FeedbackHelper {

  /**
   * Get the feedback options for Quizzes.
   */
  public function getOptions() {
    $feedback_options = array(
      'attempt'           => "Attempt",
      'correct'           => "Whether correct",
      'score'             => "Score",
      'answer_feedback'   => 'Answer feedback',
      'question_feedback' => 'Question feedback',
      'solution'          => "Correct answer",
      'quiz_feedback'     => "Quiz feedback",
    );

    drupal_alter('quiz_feedback_options', $feedback_options);

    return $feedback_options;
  }

  /**
   * Menu access check for question feedback.
   */
  public function canAccess($quiz, $question_number) {
    if ($question_number <= 0) {
      return FALSE;
    }

    if (array_filter($quiz->review_options['question'])) {
      $question_index = $question_number;
      if (empty($_SESSION['quiz'][$quiz->qid]['result_id'])) {
        $result_id = $_SESSION['quiz']['temp']['result_id'];
      }
      else {
        $result_id = $_SESSION['quiz'][$quiz->qid]['result_id'];
      }
      $quiz_result = quiz_result_load($result_id);
      $qinfo = $quiz_result->layout[$question_index];

      if ($qra = quiz_result_answer_load($result_id, $qinfo['nid'], $qinfo['vid'])) {
        return TRUE;
      }
    }
  }

  /**
   * Can the quiz taker view the requested review?
   *
   * There's a workaround in here: @kludge
   *
   * When review for the question is enabled, and it is the last question,
   * technically it is the end of the quiz, and the "end of quiz" review settings
   * apply. So we check to make sure that we are in question taking and the
   * feedback is viewed within 5 seconds of completing the question/quiz.
   */
  public function canReview($option, $result) {
    $quiz = __quiz_load_from_result($result);

    // Check what context the result is in.
    if ($result->time_end && arg(2) !== 'take') {
      // Quiz is over. Pull from the "at quiz end" settings.
      return !empty($quiz->review_options['end'][$option]);
    }

    // Quiz ongoing. Pull from the "after question" settings.
    if (!$result->time_end || $result->time_end >= REQUEST_TIME - 5) {
      return !empty($quiz->review_options['question'][$option]);
    }
  }

}
