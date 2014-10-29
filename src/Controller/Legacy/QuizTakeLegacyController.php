<?php

namespace Drupal\quiz\Controller\Legacy;

use Drupal\quiz\Entity\QuizEntity;

class QuizTakeLegacyController {

  /** @var int */
  protected $result_id;

  /** @var QuizEntity */
  protected $quiz;

  protected function getQuizId() {
    return $this->quiz->qid;
  }

  public function getResultId() {
    return $this->result_id;
  }

  public function getQuestionTakePath() {
    $id = $this->getQuizId();
    $current = $_SESSION['quiz'][$id]['current'];
    return "quiz/{$id}/take/{$current}";
  }

  /**
   * Returns the result ID for any current result set for the given quiz.
   *
   * @param int $uid
   * @param int $vid Quiz version ID
   * @param int $now
   *   Timestamp used to check whether the quiz is still open. Default: current
   *   time.
   *
   * @return int
   *   If a quiz is still open and the user has not finished the quiz,
   *   return the result set ID so that the user can continue. If no quiz is in
   *   progress, this will return 0.
   */
  protected function activeResultId($uid, $vid, $now = NULL) {
    $sql = 'SELECT qnr.result_id '
      . ' FROM {quiz_results} qnr '
      . '   INNER JOIN {quiz_entity_revision} quiz ON qnr.quiz_vid = quiz.vid'
      . ' WHERE '
      . '   (quiz.quiz_always = :quiz_always OR (:between BETWEEN quiz.quiz_open AND quiz.quiz_close)) '
      . '   AND qnr.quiz_vid = :vid '
      . '   AND qnr.uid = :uid '
      . '   AND qnr.time_end IS NULL';

    // Get any quiz that is open, for this user, and has not already been completed.
    return (int) db_query($sql, array(
          ':quiz_always' => 1,
          ':between'     => $now ? $now : REQUEST_TIME,
          ':vid'         => $vid,
          ':uid'         => $uid
      ))->fetchField();
  }

}
