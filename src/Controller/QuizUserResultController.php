<?php

namespace Drupal\quiz\Controller;

use UnexpectedValueException;

class QuizUserResultController {

  /**
   * Callback for:
   *
   *  - node/%quiz_menu/quiz-results/%quiz_result/view.
   *  - user/%/quiz-results/%quiz_result/view
   *
   * Show result page for a given result id
   *
   * @param $result_id
   *  Result id
   */
  public static function staticCallback($result) {
    if (!$result->nid) {
      throw new UnexpectedValueException('Invalid result.');
    }

    $result_id = $result->result_id;
    $quiz = node_load($result->nid, $result->vid);
    $current_quiz = node_load($result->nid);
    $questions = quiz()->getQuizHelper()->getResultHelper()->getAnswers($quiz, $result_id);
    $score = quiz()->getQuizHelper()->getResultHelper()->calculateScore($quiz, $result_id);
    $summary = _quiz_get_summary_text($quiz, $score);
    $data = array(
      'quiz'      => $quiz,
      'questions' => $questions,
      'score'     => $score,
      'summary'   => $summary,
      'result_id' => $result_id,
      'account'   => user_load($result->uid),
    );
    if (user_access('view own quiz results') || (!empty($current_quiz->review_options['end']) && array_filter($current_quiz->review_options['end']))) {
      // User can view own quiz results OR the current quiz has "display solution".
      return theme('quiz_result', $data);
    }
    else {
      // User cannot view own results or show solution. Show summary.
      return theme('quiz_result', $data);
    }
  }

}
