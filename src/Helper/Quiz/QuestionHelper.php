<?php

namespace Drupal\quiz\Helper\Quiz;

class QuestionHelper {

  /**
   * Update the session for this quiz to the active question.
   *
   * @param \stdClass $quiz
   *   A Quiz node.
   * @param type $question_number
   *   Question number starting at 1.
   */
  public function redirect($quiz, $question_number) {
    $id = isset($quiz->nid) ? $quiz->nid : $quiz->qid;
    $_SESSION['quiz'][$id]['current'] = $question_number;
  }

}
