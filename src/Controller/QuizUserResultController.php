<?php

namespace Drupal\quiz\Controller;

use Drupal\quiz\Entity\QuizEntity;
use Drupal\quiz\Entity\Result;

/**
 * Callback for:
 *
 *  - node/%quiz_menu/quiz-results/%quiz_result/view.
 *  - user/%/quiz-results/%quiz_result/view
 *
 * Show result page for a given result
 */
class QuizUserResultController {

  /** @var QuizEntity */
  private $quiz;

  /** @var QuizEntity */
  private $quiz_revision;

  /** @var Result */
  private $result;

  /** @var int */
  private $quiz_id;

  /**
   * @param Result $result
   */
  public static function staticCallback($result) {
    $quiz = quiz_entity_single_load($result->nid);
    $quiz_revision = quiz_entity_single_load($result->nid, $result->vid);
    $obj = new static($quiz, $quiz_revision, $result);
    return $obj->render();
  }

  public function __construct($quiz, $quiz_revision, $result) {
    $this->quiz = $quiz;
    $this->quiz_revision = $quiz_revision;
    $this->result = $result;
    $this->quiz_id = $this->result->nid;
    $this->score = quiz()
      ->getQuizHelper()
      ->getResultHelper()
      ->calculateScore($this->quiz_revision, $this->result->result_id);
  }

  /**
   * Render user's result.
   *
   * Check issue #2362097
   */
  public function render() {
    $this->setBreadcrumb();

    $data = array(
      'quiz'      => $this->quiz_revision,
      'questions' => quiz()->getQuizHelper()->getResultHelper()->getAnswers($this->quiz_revision, $this->result->result_id),
      'score'     => $this->score,
      'summary'   => quiz()->getQuizHelper()->getResultHelper()->getSummaryText($this->quiz_revision, $this->score),
      'result_id' => $this->result->result_id,
      'account'   => user_load($this->result->uid),
    );

    // User can view own quiz results OR the current quiz has "display solution".
    if (user_access('view own quiz results')) {
      return theme('quiz_result', $data);
    }

    // the current quiz has "display solution".
    if (!empty($this->quiz->review_options['end']) && array_filter($this->quiz->review_options['end'])) {
      return theme('quiz_result', $data);
    }

    // User cannot view own results or show solution. Show summary.
    return theme('quiz_result', $data);
  }

  private function setBreadcrumb() {
    $bc = drupal_get_breadcrumb();
    $bc[] = l($this->quiz->title, isset($this->quiz->nid) ? 'node/' . $this->quiz_id : 'quiz/' . $this->quiz_id);
    drupal_set_breadcrumb($bc);
  }

}
