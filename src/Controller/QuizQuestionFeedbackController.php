<?php

namespace Drupal\quiz\Controller;

use Drupal\quiz\Entity\QuizEntity;
use Drupal\quiz\Entity\Result;

class QuizQuestionFeedbackController {

  /** @var QuizEntity */
  private $quiz;

  /** @var int */
  private $quiz_id;

  /** @var Result */
  private $result;

  public function __construct(QuizEntity $quiz, Result $result) {
    $this->quiz = $quiz;
    $this->quiz_id = $this->quiz->qid;
    $this->result = $result;
  }

  /**
   * Callback for quiz/%/take/%question_number/feedback. Show feedback for a
   * question response.
   */
  public static function staticCallback($quiz, $page_number) {
    $quiz_id = $quiz->qid;
    $result_id = empty($_SESSION['quiz'][$quiz_id]['result_id']) ? $_SESSION['quiz']['temp']['result_id'] : $_SESSION['quiz'][$quiz_id]['result_id'];
    $result = quiz_result_load($result_id);

    $controller = new static($quiz, $result);
    return $controller->render($page_number);
  }

  public function render($question_number) {
    $question = node_load($this->result->layout[$question_number]['nid']);
    return $this->buildRenderArray($question);
  }

  public function buildRenderArray($question) {
    require_once DRUPAL_ROOT . '/' . drupal_get_path('module', 'quiz') . '/quiz.pages.inc';

    $types = _quiz_get_question_types();
    $module = $types[$question->type]['module'];

    // Invoke hook_get_report().
    if ($report = module_invoke($module, 'get_report', $question->nid, $question->vid, $this->result->result_id)) {
      $report_form = @drupal_get_form('Drupal\quiz\Form\QuizReportForm::staticCallback', array($report));
      return $report_form;
    }
  }

}
