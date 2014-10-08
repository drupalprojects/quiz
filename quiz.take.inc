<?php

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
  $form = array();
  $form['#action'] = url("node/$node->nid/take");
  $form['button'] = array(
    '#type' => 'submit',
    '#value' => t('Start quiz'),
  );
  return $form;
}

/**
 * Primary quiz-taking view on 'Take' tab.
 */
function quiz_take($quiz) {
  return quiz()->getQuizHelper()->getTakeHelper($quiz)->render();
}

/**
 * Does the current user have access to take the quiz?
 *
 * @param $node
 *  The quiz node
 */
function quiz_take_access($node) {
  if ($node->type != 'quiz') {
    return FALSE;
  }
  return node_access('view', $node) && user_access('access quiz') && quiz_availability($node) === TRUE;
}

/**
 * Handles quiz taking.
 *
 * This gets executed when the main quiz node is first loaded.
 *
 * @param $quiz
 *   The quiz node.
 *
 * @return
 *   Content array.
 */
function quiz_take_quiz($quiz) {
  global $user;
  return quiz()->getQuizHelper()->getTakeHelper($quiz)->buildRenderArray($user);
}

/**
 * Initialize a quiz attempt.
 *
 * @return QuizResult
 *   The quiz attempt.
 */
function _quiz_take_quiz_init($quiz) {
  // Create question list.
  $questions = quiz_build_question_list($quiz);
  if ($questions === FALSE) {
    drupal_set_message(t('Not enough random questions were found. Please add more questions before trying to take this @quiz.', array('@quiz' => QUIZ_NAME)), 'error');
    return FALSE;
  }

  if (count($questions) == 0) {
    drupal_set_message(t('No questions were found. Please !assign_questions before trying to take this @quiz.', array('@quiz' => QUIZ_NAME, '!assign_questions' => l(t('assign questions'), 'node/' . $quiz->nid . '/quiz/questions'))), 'error');
    return FALSE;
  }

  $quiz_result = quiz_result_load(quiz_create_rid($quiz));
  $quiz_result->layout = $questions;

  // Write the layout for this result.
  entity_save('quiz_result', $quiz_result);

  return $quiz_result;
}

/**
 * Take a quiz questions.
 *
 * @param type $quiz
 *   A quiz node.
 * @param type $question_number
 *   A question number, starting at 1. Pages do not have question numbers. Quiz
 *   directions are considered part of the numbering.
 */
function quiz_take_question($quiz, $question_number) {
  if (!empty($_SESSION['quiz'][$quiz->nid]['result_id'])) {
    $quiz_result = quiz_result_load($_SESSION['quiz'][$quiz->nid]['result_id']);
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
    unset($_SESSION['quiz'][$quiz->nid]);
    drupal_goto("node/{$quiz->nid}");
  }

  // Mark this as the current question.
  quiz_question_goto($quiz, $question_number);

  // Added the progress info to the view.
  $quiz_result = quiz_result_load($_SESSION['quiz'][$quiz->nid]['result_id']);
  $questions = array();
  foreach ($quiz_result->layout as $idx => $question) {
    if (!empty($question['number'])) {
      // Question has a number associated with it. Show it in the jumper.
      $questions[$idx] = $question['number'];
    }
  }
  $content['progress']['#markup'] = theme('quiz_progress', array(
    'questions' => $questions,
    'current' => arg(3),
    'allow_jumping' => $quiz->allow_jumping,
    'time_limit' => $quiz->time_limit,
  ));
  $content['progress']['#weight'] = -50;

  /**
   * @todo: wat do?
    if (count($_SESSION['quiz'][$quiz->nid]['quiz_questions']) + count($_SESSION['quiz'][$quiz->nid]['previous_quiz_questions']) > $number_of_questions) {
    drupal_set_message(t('At least one question have been deleted from the quiz after you started taking it. You will have to start over.'), 'warning', FALSE);
    unset($_SESSION['quiz'][$quiz->nid]);
    drupal_goto('node/' . $quiz->nid . '/take');
    }
   */
  if (isset($_SESSION['quiz'][$quiz->nid]['question_duration'])) {
    $time = $_SESSION['quiz'][$quiz->nid]['question_duration'];
    if ($time < 1) {
      // The page was probably submitted by the js, we allow the data to be stored
      $time = 1;
    }
    db_update('quiz_node_results')
      ->fields(array('time_left' => $time))
      ->condition('result_id', $_SESSION['quiz'][$quiz->nid]['result_id'])
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
    $_SESSION['quiz'][$quiz->nid]['question_start_time'] = REQUEST_TIME;
  }

  $question_form = drupal_get_form('quiz_question_answering_form', $question_node, $_SESSION['quiz'][arg(1)]['result_id']);
  $content['body']['question']['#markup'] = drupal_render($question_form);
  drupal_set_title($quiz->title);

  return $content;
}

/**
 * Meun check to see if user has access to take this question.
 */
function quiz_take_question_access($quiz, $question_number) {
  if (!$question_number) {
    return FALSE;
  }

  if ($quiz->allow_jumping) {
    // Access to go to any question. Yay.
    return TRUE;
  }

  $result_id = $_SESSION['quiz'][$quiz->nid]['result_id'];
  $quiz_result = quiz_result_load($result_id);
  $question_index = $question_number;
  $qinfo_last = $question_number == 1 ? NULL : $quiz_result->layout[$question_index - 1];
  $qinfo = $quiz_result->layout[$question_index];

  if (!$quiz->backwards_navigation) {
    // No backwards navigation.
    if ($qra = quiz_result_answer_load($result_id, $qinfo['nid'], $qinfo['vid'])) {
      // Already have an answer for the requested question.
      return FALSE;
    }
  }

  // Enforce normal navigation.
  if ($question_number == 1 || $qra = quiz_result_answer_load($result_id, $qinfo_last['nid'], $qinfo_last['vid'])) {
    //  Previous answer was submitted or this is the first question.
    return TRUE;
  }
}

/**
 * Show feedback for a question response.
 */
function quiz_take_question_feedback($quiz, $question_number) {
  if (empty($_SESSION['quiz'][$quiz->nid]['result_id'])) {
    $result_id = $_SESSION['quiz']['temp']['result_id'];
  }
  else {
    $result_id = $_SESSION['quiz'][$quiz->nid]['result_id'];
  }
  $quiz_result = quiz_result_load($result_id);
  $question_node = node_load($quiz_result->layout[$question_number]['nid']);
  $feedback = quiz_question_feedback(node_load(arg(1)), $question_node);
  return $feedback;
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

function quiz_jumper($form, $form_state, $questions, $current) {
  $form['question_number'] = array(
    '#type' => 'select',
    '#options' => $questions,
    '#default_value' => $current,
    '#prefix' => t('Question'),
    '#suffix' => t('of @num', array('@num' => count($questions))),
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Jump'),
    '#attributes' => array('class' => array('js-hide')),
  );
  return $form;
}

function quiz_jumper_submit($form, &$form_state) {
  $quiz = node_load(arg(1));
  quiz_question_goto($quiz, $form_state['values']['question_number'] - 1);
  $form_state['redirect'] = "node/{$quiz->nid}/take/" . $form_state['values']['question_number'];
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
      quiz_store_question_result($quiz, $bare_object, array('set_msg' => TRUE, 'question_data' => $question_array));
    }
  }

  $score = quiz_calculate_score($quiz, $result_id);
  if (!isset($score['percentage_score'])) {
    $score['percentage_score'] = 0;
  }
  $quiz_result->is_evaluated = $score['is_evaluated'];
  $quiz_result->score = $score['percentage_score'];
  $quiz_result->time_end = REQUEST_TIME;
  entity_save('quiz_result', $quiz_result);
  if ($user->uid) {
    $score['passing'] = quiz_is_passed($user->uid, $quiz->nid, $quiz->vid);
  }
  else {
    $score['passing'] = $score['percentage_score'] >= $quiz->pass_rate;
  }
  return $score;
}

/**
 * Creates a unique id to be used when storing results for a quiz taker.
 *
 * @param $quiz
 *   The quiz node.
 * @return $result_id
 *   The result id.
 */
function quiz_create_rid($quiz) {
  $result_id = db_insert('quiz_node_results')
    ->fields(array(
      'nid' => $quiz->nid,
      'vid' => $quiz->vid,
      'uid' => $GLOBALS['user']->uid,
      'time_start' => REQUEST_TIME,
    ))
    ->execute();
  if (!is_numeric($result_id)) {
    form_set_error(t('There was a problem starting the @quiz. Please try again later.', array('@quiz' => QUIZ_NAME), array('langcode' => 'error')));
    return FALSE;
  }
  return $result_id;
}
