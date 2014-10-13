<?php

use Drupal\quiz\Controller\QuizQuestionFeedbackController;

/**
 * @file quiz.take.inc
 *
 * Functions to render and process quiz-taking pages.
 */

/**
 * Helper function for hook_view().
 *
 * Returns a button to use as a link to start taking the quiz.
 *
 * @param $form_state
 *   Form state array.
 * @param $node
 *   The quiz node.
 * @return
 *   Form with a button linking to the take tab.
 */
function quiz_start_quiz_button_form($form, &$form_state, $node) {
  $form['#action'] = url("node/$node->nid/take");
  $form['button'] = array(
    '#type'  => 'submit',
    '#value' => t('Start quiz'),
  );
  return $form;
}

/**
 * Return a form with question feedback.
 */
function quiz_question_feedback($quiz, $question) {
  $controller = new QuizQuestionFeedbackController($quiz);
  return $controller->buildRenderArray($question);
}

/**
 * Submit handler to go to the next question from the question feedback.
 */
function quiz_take_question_feedback_submit($form, &$form_state) {
  $quiz = node_load(arg(1));
  $form_state['redirect'] = "node/{$quiz->nid}/take/" . $_SESSION['quiz'][$quiz->nid]['current'];
}

/**
 * Submit handler to go to the quiz results from the last question's feedback.
 */
function quiz_take_question_feedback_end_submit($form, &$form_state) {
  $quiz = node_load(arg(1));
  $result_id = $_SESSION['quiz']['temp']['result_id'];
  $form_state['redirect'] = "node/{$quiz->nid}/quiz-results/$result_id/view";
}

/**
 * Actions to take at the end of a quiz
 *
 * @param $quiz
 *  The quiz node
 * @param $result_id
 *  Result id
 * @param $score
 *  Score as a number
 */
function quiz_end_actions($quiz, $score, $session_data) {
  // Call hook_quiz_finished().
  // @TODO consider hook_entity_update if we make quiz results rules capable
  module_invoke_all('quiz_finished', $quiz, $score, $session_data);

  // Lets piggy back here to perform the quiz defined action since were done
  // with this quiz.
  // We will verify that there is an associated action with this quiz and then
  // perform that action.
  if (!empty($quiz->aid)) {
    // @TODO get rid of this. Replace with rules. Make quiz results entities or
    // something
    // Some actions are reliant on objects and I am unsure which ones, for now I
    // have simply passed the actions_do() function an empty array. By passing
    // this function a single id then it will retrieve the callback, get the
    // parameters and perform that function (action) for you.
    actions_do($quiz->aid, $quiz, $score, $session_data);
  }

  return $score;
}

/**
 * Jumper form.
 */
function quiz_jumper($form, $form_state, $questions, $current) {
  $form['question_number'] = array(
    '#type'          => 'select',
    '#options'       => $questions,
    '#default_value' => $current,
    '#prefix'        => t('Question'),
    '#suffix'        => t('of @num', array('@num' => count($questions))),
  );
  $form['submit'] = array(
    '#type'       => 'submit',
    '#value'      => t('Jump'),
    '#attributes' => array('class' => array('js-hide')),
    '#submit'     => array(function($form, &$form_state) {
        $quiz = node_load(arg(1));
        quiz()->getQuizHelper()->getQuestionHelper()->redirect($quiz, $form_state['values']['question_number'] - 1);
        $form_state['redirect'] = "node/{$quiz->nid}/take/" . $form_state['values']['question_number'];
      }),
  );
  return $form;
}

/**
 * Score a completed quiz.
 */
function quiz_end_scoring($result_id) {
  global $user;

  $quiz_result = quiz_result_load($result_id);
  $quiz = node_load($quiz_result->nid, $quiz_result->vid);
  $questions = $quiz_result->layout;

  // Mark all missing answers as blank. This is essential here for when we may
  // have pages of unanswered questions. Also kills a lot of the skip code that
  // was necessary before.
  foreach ($quiz_result->layout as $qinfo) {
    // Load the Quiz answer submission from the database.
    $qra = quiz_result_answer_load($result_id, $qinfo['nid'], $qinfo['vid']);
    $current_question = node_load($qinfo['nid'], $qinfo['vid']);

    foreach ($questions as $question) {
      if ($question['nid'] == $current_question->nid) {
        $question_array = $question;
      }
    }

    if (!$qra) {
      $qi_instance = _quiz_question_response_get_instance($result_id, $current_question, NULL);
      $qi_instance->delete();
      $qi_instance->saveResult();
      $bare_object = $qi_instance->toBareObject();
      quiz()->getQuizHelper()->saveQuestionResult($quiz, $bare_object, array('set_msg' => TRUE, 'question_data' => $question_array));
    }
  }

  $score = quiz()->getQuizHelper()->getResultHelper()->calculateScore($quiz, $result_id);
  if (!isset($score['percentage_score'])) {
    $score['percentage_score'] = 0;
  }
  $quiz_result->is_evaluated = $score['is_evaluated'];
  $quiz_result->score = $score['percentage_score'];
  $quiz_result->time_end = REQUEST_TIME;
  entity_save('quiz_result', $quiz_result);
  if ($user->uid) {
    $score['passing'] = quiz()->getQuizHelper()->isPassed($user->uid, $quiz->nid, $quiz->vid);
  }
  else {
    $score['passing'] = $score['percentage_score'] >= $quiz->pass_rate;
  }
  return $score;
}
