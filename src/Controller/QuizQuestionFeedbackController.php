<?php

namespace Drupal\quiz\Controller;

class QuizQuestionFeedbackController {

  private $quiz;

  public function __construct($quiz) {
    $this->quiz = $quiz;
  }

  /**
   * Callback for node/%quiz_menu/take/%question_number/feedback. Show feedback
   * for a question response.
   */
  public static function staticCallback($quiz, $question_number) {
    $controller = new static($quiz);
    return $controller->render($question_number);
  }

  public function render($question_number) {
    if (empty($_SESSION['quiz'][$this->quiz->nid]['result_id'])) {
      $result_id = $_SESSION['quiz']['temp']['result_id'];
    }
    else {
      $result_id = $_SESSION['quiz'][$this->quiz->nid]['result_id'];
    }
    $quiz_result = quiz_result_load($result_id);
    $question = node_load($quiz_result->layout[$question_number]['nid']);
    $feedback = $this->buildRenderArray($question);
    return $feedback;
  }

  public function buildRenderArray($question) {
    if (empty($_SESSION['quiz'][$this->quiz->nid]['result_id'])) {
      $result_id = $_SESSION['quiz']['temp']['result_id'];
    }
    else {
      $result_id = $_SESSION['quiz'][$this->quiz->nid]['result_id'];
    }

    $types = _quiz_get_question_types();
    $module = $types[$question->type]['module'];

    // Invoke hook_get_report().
    $report = module_invoke($module, 'get_report', $question->nid, $question->vid, $result_id);
    require_once DRUPAL_ROOT . '/' . drupal_get_path('module', 'quiz') . '/quiz.pages.inc';
    if ($report) {
      $report_form = drupal_get_form('quiz_report_form', array($report));
      return $report_form;
    }
  }

}
