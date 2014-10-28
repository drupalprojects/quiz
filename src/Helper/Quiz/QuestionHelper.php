<?php

namespace Drupal\quiz\Helper\Quiz;

use Drupal\quiz\Entity\QuizEntity;

class QuestionHelper {

  /**
   * Update the session for this quiz to the active question.
   *
   * @param QuizEntity $quiz
   * @param int $page_number
   *   Question number starting at 1.
   */
  public function redirect(QuizEntity $quiz, $page_number) {
    $_SESSION['quiz'][$quiz->qid]['current'] = $page_number;
  }

  public function getCurrentPageNumber(QuizEntity $quiz) {
    $id = $quiz->qid;
    return isset($_SESSION['quiz'][$id]['current']) ? $_SESSION['quiz'][$id]['current'] : 1;
  }

}
