<?php

namespace Drupal\quiz\Controller;

use Drupal\quiz\Helper\Quiz\QuestionHelper;

class QuizTakeQuestionController extends QuestionHelper {

  private $quiz;

  public function __construct($quiz) {
    $this->quiz = $quiz;
  }

  /**
   * Callback for node/%quiz_menu/take/%question_number. Take a quiz questions.
   *
   * @param type $quiz
   *   A quiz node.
   * @param type $question_number
   *   A question number, starting at 1. Pages do not have question numbers. Quiz
   *   directions are considered part of the numbering.
   */
  public static function staticCallback($quiz, $question_number) {
    $controller = new static($quiz);
    return $controller->render($question_number);
  }

  public function render($question_number) {
    if (!empty($_SESSION['quiz'][$this->quiz->nid]['result_id'])) {
      $quiz_result = quiz_result_load($_SESSION['quiz'][$this->quiz->nid]['result_id']);
      $question = $quiz_result->layout[$question_number];

      if (!empty($question['qnr_pid'])) {
        // Find the parent.
        foreach ($quiz_result->layout as $pquestion) {
          if ($pquestion['qnr_id'] == $question['qnr_pid']) {
            // Load the page that the requested question belongs to.
            $question_node = node_load($pquestion['nid']);
          }
        }
      }
      else {
        // Load the question.
        $question_node = node_load($question['nid']);
      }
    }

    if (!$question_node) {
      // Question disappeared or invalid session. Start over.
      unset($_SESSION['quiz'][$this->quiz->nid]);
      drupal_goto("node/{$this->quiz->nid}");
    }

    // Mark this as the current question.
    $this->redirect($this->quiz, $question_number);

    // Added the progress info to the view.
    $quiz_result = quiz_result_load($_SESSION['quiz'][$this->quiz->nid]['result_id']);
    $questions = array();
    foreach ($quiz_result->layout as $idx => $question) {
      if (!empty($question['number'])) {
        // Question has a number associated with it. Show it in the jumper.
        $questions[$idx] = $question['number'];
      }
    }
    $content['progress']['#markup'] = theme('quiz_progress', array(
      'quiz'          => $this->quiz,
      'questions'     => $questions,
      'current'       => arg(3),
      'allow_jumping' => $this->quiz->allow_jumping,
      'pager'         => count($questions) >= variable_get('quiz_pager_start', 100),
      'time_limit'    => $this->quiz->time_limit,
    ));
    $content['progress']['#weight'] = -50;

    if (isset($_SESSION['quiz'][$this->quiz->nid]['question_duration'])) {
      $time = $_SESSION['quiz'][$this->quiz->nid]['question_duration'];
      if ($time < 1) {
        // The page was probably submitted by the js, we allow the data to be stored
        $time = 1;
      }
      db_update('quiz_node_results')
        ->fields(array('time_left' => $time))
        ->condition('result_id', $_SESSION['quiz'][$this->quiz->nid]['result_id'])
        ->execute();

      if ($time <= 1) {
        // Quiz has been timed out, run a loop to mark the remaining questions
        // as skipped.
        // @todo we just need to run quiz_end_score here, I think
        drupal_set_message(t('You have run out of time.'), 'error');
      }
      else {
        // There is still time left, so let's go ahead and insert the countdown
        // javascript.
        if (function_exists('jquery_countdown_add') && variable_get('quiz_has_timer', 1)) {
          jquery_countdown_add('.countdown', array('until' => $time, 'onExpiry' => 'finished', 'compact' => TRUE, 'layout' => t('Time left') . ': {hnn}{sep}{mnn}{sep}{snn}'));
          // These are the two button op values that are accepted for answering
          // questions.
          $button_op1 = drupal_json_encode(t('Finish'));
          $button_op2 = drupal_json_encode(t('Next'));
          $js = "
            function finished() {
              // Find all buttons with a name of 'op'.
              var buttons = jQuery('input[type=submit][name=op], button[type=submit][name=op]');
              // Filter out the ones that don't have the right op value.
              buttons = buttons.filter(function() {
                return this.value == $button_op1 || this.value == $button_op2;
              });
              if (buttons.length == 1) {
                // Since only one button was found, this must be it.
                buttons.click();
              }
              else {
                // Zero, or more than one buttons were found; fall back on a page refresh.
                window.location = window.location.href;
              }
            }
          ";
          drupal_add_js($js, array('type' => 'inline', 'scope' => JS_DEFAULT));
        }
      }
      $_SESSION['quiz'][$this->quiz->nid]['question_start_time'] = REQUEST_TIME;
    }

    $question_form = @drupal_get_form('\Drupal\quiz\Form\QuizAnsweringForm::staticCallback', $question_node, $_SESSION['quiz'][arg(1)]['result_id']);
    $content['body']['question']['#markup'] = drupal_render($question_form);
    drupal_set_title($this->quiz->title);

    return $content;
  }

}
