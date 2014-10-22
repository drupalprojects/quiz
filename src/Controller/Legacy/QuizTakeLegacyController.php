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

}
