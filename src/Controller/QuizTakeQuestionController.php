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
  private $is_quiz_node;
  private $quiz_uri;
  private $quiz_id;

  /**
   * Callback for node/%quiz_menu/take/%question_number. Take a quiz questions.
   *
   * @param QuizEntity $quiz A quiz entity
   * @param int $page_number
   *   A question number, starting at 1. Pages do not have question numbers. Quiz
   *   directions are considered part of the numbering.
   */
  public static function staticCallback($quiz, $page_number) {
    $result = $layout_item = NULL;
    $quiz_id = __quiz_entity_id($quiz);

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
    $this->is_quiz_node = isset($quiz->nid);
    $this->quiz_uri = isset($quiz->nid) ? 'node/' . $quiz->nid : 'quiz/' . $quiz->qid;
    $this->quiz_id = __quiz_entity_id($quiz);

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

    if (isset($_SESSION['quiz'][$this->quiz_id]['question_duration'])) {
      $this->updateQuestionDuration();
    }

    $form_id = 'Drupal\quiz\Form\QuizAnsweringForm::staticCallback';
    $question_form = @drupal_get_form($form_id, $this->quiz, $this->question, $this->page_number, $this->result);
    $content['body']['question']['#markup'] = drupal_render($question_form);

    return $content;
  }

  private function updateQuestionDuration() {
    $time = $_SESSION['quiz'][$this->quiz_id]['question_duration'];

    if ($time < 1) {
      // The page was probably submitted by the js, we allow the data to be stored
      $time = 1;
    }

    db_update('quiz_results')
      ->fields(array('time_left' => $time))
      ->condition('result_id', $this->result->result_id)
      ->execute();

    // Quiz has been timed out, run a loop to mark the remaining questions as skipped.
    // @todo we just need to run quiz_end_score here, I think
    if ($time <= 1) {
      throw new RuntimeException(t('You have run out of time.'));
    }
    // There is still time left, so let's go ahead and insert the countdown js.
    elseif (function_exists('jquery_countdown_add') && variable_get('quiz_has_timer', 1)) {
      $this->attachJs($time);
    }

    // Update start time in session
    $_SESSION['quiz'][$this->quiz_id]['question_start_time'] = REQUEST_TIME;
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
  }

}
