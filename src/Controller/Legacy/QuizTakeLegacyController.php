<?php

namespace Drupal\quiz\Controller\Legacy;

use Drupal\quiz\Entity\QuizEntity;

class QuizTakeLegacyController {

  private $quiz_entity_type;

  /** @var int */
  protected $result_id;

  /** @var QuizEntity */
  protected $quiz;

  public function __construct($quiz_entity_type) {
    $this->quiz_entity_type = $quiz_entity_type;
  }

  protected function isNode() {
    return $this->quiz_entity_type === 'node';
  }

  protected function getQuizId() {
    return $this->isNode() ? $this->quiz->nid : $this->quiz->qid;
  }

  public function loadQuiz($id, $vid) {
    return $this->isNode() ? node_load($id, $vid) : quiz_entity_single_load($id, $vid);
  }

  public function getResultId() {
    return $this->result_id;
  }

  public function getQuestionTakePath() {
    $id = $this->getQuizId();
    $current = $_SESSION['quiz'][$id]['current'];
    return $this->isNode() ? "node/{$id}/take/{$current}" : "quiz/{$id}/take/{$current}";
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
    $sql = 'SELECT qnr.result_id'
      . ' FROM {quiz_results} qnr'
      . '   INNER JOIN {quiz_node_properties} qnp ON qnr.vid = qnp.vid '
      . ' WHERE '
      . '   (qnp.quiz_always = :quiz_always OR (:between BETWEEN qnp.quiz_open AND qnp.quiz_close)) '
      . '   AND qnr.vid = :vid '
      . '   AND qnr.uid = :uid '
      . '   AND qnr.time_end IS NULL';

    $params = array(
      ':quiz_always' => 1,
      ':between'     => $now ? $now : REQUEST_TIME,
      ':vid'         => $vid,
      ':uid'         => $uid
    );

    // Get any quiz that is open, for this user, and has not already been completed.
    return (int) db_query($sql, $params)->fetchField();
  }

}
