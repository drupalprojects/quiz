<?php

namespace Drupal\quiz\Controller;

use Drupal\quiz\Entity\QuizEntity;
use Drupal\quiz\Helper\Quiz\QuestionHelper;
use RuntimeException;

class QuizTakeQuestionController extends QuestionHelper {

  private $quiz;
  private $question;
  private $page_number;

  /** @var \Drupal\quiz\Entity\Result */
  private $result;
  private $quiz_uri;
  private $quiz_id;

  /**
   * Callback for quiz/%/take/%. Take a quiz questions.
   *
   * @param QuizEntity $quiz A quiz entity
   * @param int $page_number
   *   A question number, starting at 1. Pages do not have question numbers. Quiz
   *   directions are considered part of the numbering.
   */
  public static function staticCallback($quiz, $page_number) {
    $result = $layout_item = NULL;
    $quiz_id = $quiz->qid;

    if (isset($_SESSION['quiz'][$quiz_id]['result_id'])) {
      $result = quiz_result_load($_SESSION['quiz'][$quiz_id]['result_id']);
    }

    // Load the page that the requested question belongs to.
    if ($result && ($_layout_item = $result->getPageItem($page_number))) {
      $layout_item = node_load($_layout_item['nid']);
    }

    $controller = new static($quiz, $result, $page_number, $layout_item);
    return $controller->render();
  }

  public function __construct($quiz, $result, $question_number, $question) {
    drupal_set_title($quiz->title);

    $this->quiz = $quiz;
    $this->result = $result;
    $this->page_number = $question_number;
    $this->question = $question;

    // Legacy code
    $this->quiz_uri = 'quiz/' . $quiz->qid;
    $this->quiz_id = $quiz->qid;

    // Question disappeared or invalid session. Start over.
    if (!$question) {
      drupal_set_message(t('Invalid session.'), 'error');
      unset($_SESSION['quiz'][$this->quiz_id]);
      drupal_goto($this->quiz_uri);
    }
  }

  public function render() {
    $content = array();
    $questions = array();
    $i = 0;

    // Mark this as the current question.
    $this->redirect($this->quiz, $this->page_number);

    // Added the progress info to the view.
    foreach ($this->result->layout as $idx => $question) {
      if (empty($question['qr_pid'])) {
        $questions[$idx] = ++$i; // Question has no parent. Show it in the jumper.
      }
    }

    $content['progress']['#markup'] = theme('quiz_progress', array(
        'quiz'          => $this->quiz,
        'questions'     => $questions,
        'current'       => $this->page_number,
        'allow_jumping' => $this->quiz->allow_jumping,
        'pager'         => count($questions) >= variable_get('quiz_pager_start', 100),
        'time_limit'    => $this->quiz->time_limit,
    ));
    $content['progress']['#weight'] = -50;

    if (function_exists('jquery_countdown_add') && variable_get('quiz_has_timer', 0) && $this->quiz->time_limit) {
      $this->attachJs($this->result->time_start + $this->quiz->time_limit - REQUEST_TIME);
    }

    $form_id = 'Drupal\quiz\Form\QuizAnsweringForm::staticCallback';
    $question_form = @drupal_get_form($form_id, $this->quiz, $this->question, $this->page_number, $this->result);
    $content['body']['question']['#markup'] = drupal_render($question_form);

    return $content;
  }

  /**
   * @todo Get rid of inline Javascript.
   * @param int $time
   */
  private function attachJs($time) {
    jquery_countdown_add('.countdown', array(
        'until'    => $time,
        'onExpiry' => 'quiz_take_finished',
        'compact'  => TRUE,
        'layout'   => t('Time left') . ': {hnn}{sep}{mnn}{sep}{snn}'
    ));

    // These are the two button op values that are accepted for answering questions.
    $vars = array('quiz_button_1' => t('Finish'), 'quiz_button_2' => t('Next'));
    drupal_add_js($vars, array('type' => 'setting'));
    drupal_add_js(drupal_get_path('module', 'quiz') . '/js/quiz_take.count-down.js');
  }

}
