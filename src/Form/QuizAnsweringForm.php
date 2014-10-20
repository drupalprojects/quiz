<?php

namespace Drupal\quiz\Form;

use Drupal\quiz\Helper\Quiz\QuestionHelper;

class QuizAnsweringForm extends QuestionHelper {

  public static function staticCallback($form, &$form_state, $quizzes, $result_id) {
    $controller = new static();

    if (is_array($quizzes)) {
      return $controller->getForm($form, $form_state, $quizzes, $result_id);
    }

    // Not One single question (or page?)
    if ($quizzes->type !== 'quiz_page') {
      $quizzes = array($quizzes->nid => $quizzes);
      return $controller->getForm($form, $form_state, $quizzes, $result_id);
    }

    // One single question (or page?)
    $quiz_result = quiz_result_load($result_id);
    foreach ($quiz_result->layout as $question) {
      // Still not found a page
      if ($question['nid'] != $quizzes->nid) {
        continue;
      }

      $quizzes = array(node_load($quizzes->nid));
      foreach ($quiz_result->layout as $question2) {
        if ($question2['qnr_pid'] != $question['qnr_id']) {
          continue;
        }

        // This question belongs in the requested page.
        $quizzes[] = node_load($question2['nid']);
      }
      break;
    }

    return $controller->getForm($form, $form_state, $quizzes, $result_id);
  }

  /**
   * Get the form to show to the quiz taker.
   *
   * @param $nodes
   *   A list of question nodes to get answers from.
   * @param $result_id
   *   The result ID for this attempt.
   */
  public function getForm($form, &$form_state, $nodes, $result_id) {
    // set validate callback
    $form['#validate'][] = array($this, 'formValidate');

    $form['#attributes']['class'] = array('answering-form');

    foreach ($nodes as $node) {
      $question = _quiz_question_get_instance($node);
      $class = drupal_html_class('quiz-question-' . $node->type);
      // Element for a single question
      $element = $question->getAnsweringForm($form_state, $result_id);
      $quiz = node_load(arg(1));

      node_build_content($node, 'question');
      unset($node->content['answers']);
      $form['questions'][$node->nid] = array(
        '#attributes' => array('class' => array($class)),
        '#type'       => 'container',
        'header'      => $node->content,
        'question'    => array('#tree' => TRUE, $node->nid => $element),
      );

      // Should we disable this question?
      if (empty($quiz->allow_change) && ($qras = quiz_result_answer_load($result_id, $node->nid, $node->vid))) {
        if (($qra = reset($qras)) && empty($qra->is_skipped)) {
          // This question was already answered, and not skipped.
          $form['questions'][$node->nid]['#disabled'] = TRUE;
        }
      }

      if ($quiz->mark_doubtful) {
        $form['is_doubtful'] = array(
          '#type'          => 'checkbox',
          '#title'         => t('doubtful'),
          '#weight'        => 1,
          '#prefix'        => '<div class="mark-doubtful checkbox enabled"><div class="toggle"><div></div></div>',
          '#suffix'        => '</div>',
          '#default_value' => 0,
          '#attached'      => array(
            'js' => array(drupal_get_path('module', 'quiz') . '/js/quiz_take.js'),
          ),
        );
        if (isset($node->result_id)) {
          $form['is_doubtful']['#default_value'] = db_query('SELECT is_doubtful FROM {quiz_results_answers} WHERE result_id = :result_id AND question_nid = :question_nid AND question_vid = :question_vid', array(':result_id' => $node->result_id, ':question_nid' => $node->nid, ':question_vid' => $node->vid))->fetchField();
        }
      }
    }

    $is_last = $this->showFinishButton($quiz);

    $form['navigation']['#type'] = 'actions';

    if (!empty($quiz->backwards_navigation) && (arg(3) != 1)) {
      // Backwards navigation enabled, and we are looking at not the first
      // question. @todo detect when on the first page.
      $form['navigation']['back'] = array(
        '#weight'                  => 10,
        '#type'                    => 'submit',
        '#value'                   => t('Back'),
        '#submit'                  => array(array($this, 'formBackSubmit')),
        '#limit_validation_errors' => array(),
      );
      if ($is_last) {
        $form['navigation']['#last'] = TRUE;
        $form['navigation']['last_text'] = array(
          '#weight' => 0,
          '#markup' => '<p><em>' . t('This is the last question. Press Finish to deliver your answers') . '</em></p>',
        );
      }
    }

    $form['navigation']['submit'] = array(
      '#weight' => 30,
      '#type'   => 'submit',
      '#value'  => $is_last ? t('Finish') : t('Next'),
      '#submit' => array(array($this, 'formSubmit')),
    );

    if ($is_last && $quiz->backwards_navigation && !$quiz->repeat_until_correct) {
      // Display a confirmation dialogue if this is the last question and a user
      // is able to navigate backwards but not forced to answer correctly.
      $form['#attributes']['class'][] = 'quiz-answer-confirm';
      $form['#attributes']['data-confirm-message'] = t("By proceeding you won't be able to go back and edit your answers.");
      $form['#attached'] = array(
        'js' => array(drupal_get_path('module', 'quiz') . '/js/quiz_confirm.js'),
      );
    }

    if (!$quiz->allow_skipping) {
      return $form;
    }

    $form['navigation']['skip'] = array(
      '#weight'                  => 20,
      '#type'                    => 'submit',
      '#value'                   => $is_last ? t('Leave blank and finish') : t('Leave blank'),
      '#access'                  => ($node->type == 'quiz_directions') ? FALSE : TRUE,
      '#submit'                  => array(array($this, $is_last ? 'formSubmit' : 'formBlankSubmit')),
      '#limit_validation_errors' => array(),
    );

    return $form;
  }

  /**
   * Show the finish button?
   */
  private function showFinishButton($quiz) {
    $quiz_result = quiz_result_load($_SESSION['quiz'][$quiz->nid]['result_id']);
    $current = $_SESSION['quiz'][$quiz->nid]['current'];
    foreach ($quiz_result->layout as $idx => $question) {
      if ($question['type'] == 'quiz_page') {
        if ($current == $idx) {
          // Found a page that we are on
          $in_page = TRUE;
          $last_page = TRUE;
        }
        else {
          // Found a quiz page that we are not on.
          $last_page = FALSE;
        }
      }
      else if (empty($question['qnr_pid'])) {
        // A question without a parent showed up.
        $in_page = FALSE;
        $last_page = FALSE;
      }
    }

    return $last_page || !isset($quiz_result->layout[$_SESSION['quiz'][$quiz->nid]['current'] + 1]);
  }

  /**
   * Submit action for "leave blank".
   */
  public function formBlankSubmit($form, &$form_state) {
    $quiz = node_load(arg(1));
    $result_id = $_SESSION['quiz'][$quiz->nid]['result_id'];
    $quiz_result = quiz_result_load($result_id);
    $questions = $quiz_result->layout;
    foreach ($form_state['input']['question'] as $nid => $input) {
      // Loop over all question inputs provided, and record them as skipped.
      $question = node_load($nid);

      foreach ($questions as $question_item) {
        if ($question_item['nid'] == $question->nid) {
          $question_array = $question_item;
        }
      }

      $qi_instance = _quiz_question_response_get_instance($result_id, $question, NULL);
      $qi_instance->delete();
      $bare_object = $qi_instance->toBareObject();
      quiz()->getQuizHelper()->saveQuestionResult($quiz, $bare_object, array('set_msg' => TRUE, 'question_data' => $question_array));
    }

    // Advance to next question.
    $this->redirect($quiz, $_SESSION['quiz'][$quiz->nid]['current'] + 1);
    $form_state['redirect'] = "node/{$quiz->nid}/take/" . $_SESSION['quiz'][$quiz->nid]['current'];
  }

  /**
   * Submit handler for the question answering form.
   *
   * There is no validation code here, but there may be feedback code for
   * correct feedback.
   */
  public function formSubmit(&$form, &$form_state) {
    global $user;

    $quiz = node_load(arg(1)); // @todo: Avoid this
    $quiz_result = quiz_result_load($_SESSION['quiz'][$quiz->nid]['result_id']);
    $questions = $quiz_result->layout;

    if (!empty($form_state['values']['question'])) {
      foreach ($form_state['values']['question'] as $nid => $answer) {
        $current_question = node_load($nid);
        foreach ($questions as $question) {
          if ($question['nid'] == $current_question->nid) {
            $question_array = $question;
          }
        }
        $qi_instance = _quiz_question_response_get_instance($_SESSION['quiz'][$quiz->nid]['result_id'], $current_question, $form_state['values']['question'][$current_question->nid]);
        $qi_instance->delete();
        $qi_instance->saveResult();
        $result = $qi_instance->toBareObject();
        quiz()->getQuizHelper()->saveQuestionResult($quiz, $result, array('set_msg' => TRUE, 'question_data' => $question_array));

        // Increment the counter.
        $this->redirect($quiz, $_SESSION['quiz'][$quiz->nid]['current'] + 1);
      }
    }

    // Wat do?
    if (!empty($quiz->review_options['question']) && array_filter($quiz->review_options['question'])) {
      // We have question feedback.
      $form_state['redirect'] = "node/$quiz->nid/take/" . ($_SESSION['quiz'][$quiz->nid]['current'] - 1) . '/feedback';
    }
    else {
      // No question feedback. Go to next question.
      $form_state['redirect'] = "node/$quiz->nid/take/" . ($_SESSION['quiz'][$quiz->nid]['current']);
    }

    if (!isset($quiz_result->layout[$_SESSION['quiz'][$quiz->nid]['current']])) {
      // No more questions. Score quiz.
      $score = quiz_end_scoring($_SESSION['quiz'][$quiz->nid]['result_id']);

      // Delete old results if necessary.
      quiz()->getQuizHelper()->getResultHelper()->maintainResult($user, $quiz, $quiz_result->result_id);
      if (empty($quiz->review_options['question']) || !array_filter($quiz->review_options['question'])) {
        // Only redirect to question results if there is not question feedback.
        $form_state['redirect'] = "node/{$quiz->nid}/quiz-results/{$quiz_result->result_id}/view";
      }

      quiz_end_actions($quiz, $score, $_SESSION['quiz'][$quiz->nid]);

      // Remove all information about this quiz from the session.
      // @todo but for anon, we might have to keep some so they could access
      // results
      // When quiz is completed we need to make sure that even though the quiz has
      // been removed from the session, that the user can still access the
      // feedback for the last question, THEN go to the results page.
      $_SESSION['quiz']['temp']['result_id'] = $quiz_result->result_id;
      unset($_SESSION['quiz'][$quiz->nid]);
    }
  }

  /**
   * Submit handler for "back".
   */
  function formBackSubmit(&$form, &$form_state) {
    // Back a question.
    $quiz = node_load(arg(1));
    $this->redirect($quiz, $_SESSION['quiz'][$quiz->nid]['current'] - 1);
    $quiz_result = quiz_result_load($_SESSION['quiz'][$quiz->nid]['result_id']);
    $question = $quiz_result->layout[$_SESSION['quiz'][$quiz->nid]['current']];
    if (!empty($question['qnr_pid'])) {
      foreach ($quiz_result->layout as $question2) {
        if ($question2['qnr_id'] == $question['qnr_pid']) {
          $this->redirect($quiz, $question2['number']);
        }
      }
    }

    $form_state['redirect'] = "node/$quiz->nid/take/" . ($_SESSION['quiz'][$quiz->nid]['current']);
  }

  /**
   * Validation callback for quiz question submit.
   */
  function formValidate(&$form, &$form_state) {
    $quiz = node_load(arg(1)); // @todo: Avoid this.
    foreach ($form_state['values']['question'] as $nid => $answer) {
      $current_question = node_load($nid);

      if ($current_question) {
        // There was an answer submitted.
        $quiz_question = _quiz_question_get_instance($current_question);
        $quiz_question->getAnsweringFormValidate($form, $form_state);
      }
    }
  }

}
