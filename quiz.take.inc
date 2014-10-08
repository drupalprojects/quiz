<?php

/**
 * Does the current user have access to take the quiz?
 *
 * @param $quiz
 *  The quiz node
 */
function quiz_take_access($quiz) {
  global $user;
  return quiz()->getQuizHelper()->getAccessHelper()->canTakeQuiz($quiz, $user);
}

/**
 * Update the session for this quiz to the active question.
 *
 * @param type $quiz
 *   A Quiz node.
 * @param type $question_number
 *   Question number starting at 1.
 */
function quiz_question_goto($quiz, $question_number) {
  $_SESSION['quiz'][$quiz->nid]['current'] = $question_number;
}

/**
 * Primary quiz-taking view on 'Take' tab.
 */
function quiz_take($quiz) {
  global $user;
  return quiz()->getQuizHelper()->getTakeHelper($quiz)->getMasterRender()->render($user);
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
  return quiz()->getQuizHelper()->getTakeHelper($quiz)->getQuestionRender()->render($question_number);
}

/**
 * Meun check to see if user has access to take this question.
 */
function quiz_take_question_access($quiz, $question_number) {
  return quiz()->getQuizHelper()->getAccessHelper()->canAccessQuestion($quiz, $question_number);
}

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
 * Show feedback for a question response.
 */
function quiz_take_question_feedback($quiz, $question_number) {
  return quiz()->getQuizHelper()->getTakeHelper($quiz)->getQuestionFeedBackRender()->render($question_number);
}

/**
 * Return a form with question feedback.
 */
function quiz_question_feedback($quiz, $question) {
  return quiz()
      ->getQuizHelper()
      ->getTakeHelper($quiz)
      ->getQuestionFeedBackRender()
      ->buildRenderArray($question);
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
 * Get the form to show to the quiz taker.
 *
 * @param $nodes
 *   A list of question nodes to get answers from.
 * @param $result_id
 *   The result ID for this attempt.
 */
function quiz_question_answering_form($form, $form_state, $nodes, $result_id) {
  $quiz_result = quiz_result_load($result_id);

  if (!is_array($nodes)) {
    // One single question (or page?)
    if ($nodes->type == 'quiz_page') {
      foreach ($quiz_result->layout as $question) {
        if ($question['nid'] == $nodes->nid) {
          // Found a page
          $nodes = array(node_load($nodes->nid));
          foreach ($quiz_result->layout as $question2) {
            if ($question2['qnr_pid'] == $question['qnr_id']) {
              // This question belongs in the requested page.
              $nodes[] = node_load($question2['nid']);
            }
          }
          break;
        }
      }
    }
    else {
      $nodes = array($nodes->nid => $nodes);
    }
  }

  $form['#attributes']['class'] = array('answering-form');

  foreach ($nodes as $node) {
    $question = _quiz_question_get_instance($node);
    $class = drupal_html_class('quiz-question-' . $node->type);
    // Element for a single question
    $element = $question->getAnsweringForm($form_state, $result_id);
    $quiz = node_load(arg(1));

    // Hide the label of the body field if this is an informative question.
    $options = !$question->isGraded() ? array('label' => 'hidden') : array();

    node_build_content($node, 'question');
    unset($node->content['answers']);
    $form['questions'][$node->nid] = array(
      '#attributes' => array('class' => array($class)),
      '#type' => 'container',
      'header' => $node->content,
      'question' => array('#tree' => TRUE, $node->nid => $element),
    );

    if ($quiz->mark_doubtful) {
      $form['is_doubtful'] = array(
        '#type' => 'checkbox',
        '#title' => t('doubtful'),
        '#weight' => 1,
        '#prefix' => '<div class="mark-doubtful checkbox enabled"><div class="toggle"><div></div></div>',
        '#suffix' => '</div>',
        '#default_value' => 0,
        '#attached' => array(
          'js' => array(drupal_get_path('module', 'quiz') . '/theme/quiz_take.js'),
        ),
      );
      if (isset($node->result_id)) {
        $form['is_doubtful']['#default_value'] = db_query('SELECT is_doubtful FROM {quiz_node_results_answers} WHERE result_id = :result_id AND question_nid = :question_nid AND question_vid = :question_vid', array(':result_id' => $node->result_id, ':question_nid' => $node->nid, ':question_vid' => $node->vid))->fetchField();
      }
    }
  }

  $is_last = _quiz_show_finish_button($quiz);

  $form['navigation']['#type'] = 'actions';
  $form['navigation']['#theme'] = 'quiz_question_navigation_form';

  if (!empty($quiz->backwards_navigation) && (arg(3) != 1)) {
    // Backwards navigation enabled, and we are looking at not the first
    // question. @todo detect when on the first page.
    $form['navigation']['back'] = array(
      '#weight' => 10,
      '#type' => 'submit',
      '#value' => t('Back'),
      '#submit' => array('quiz_question_answering_form_submit_back'),
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
    '#type' => 'submit',
    '#value' => $is_last ? t('Finish') : t('Next'),
  );

  if ($is_last && $quiz->backwards_navigation && !$quiz->repeat_until_correct) {
    // Display a confirmation dialogue if this is the last question and a user
    // is able to navigate backwards but not forced to answer correctly.
    $form['#attributes']['class'][] = 'quiz-answer-confirm';
    $form['#attributes']['data-confirm-message'] = t("By proceeding you won't be able to go back and edit your answers.");
    $form['#attached'] = array(
      'js' => array(drupal_get_path('module', 'quiz') . '/theme/quiz_confirm.js'),
    );
  }
  if ($quiz->allow_skipping) {
    $form['navigation']['skip'] = array(
      '#weight' => 20,
      '#type' => 'submit',
      '#value' => $is_last ? t('Leave blank and finish') : t('Leave blank'),
      '#access' => ($node->type == 'quiz_directions') ? FALSE : TRUE,
      '#submit' => $is_last ? array('quiz_question_answering_form_submit') : array('quiz_question_answering_form_submit_blank'),
      '#limit_validation_errors' => array(),
    );
  }

  return $form;
}

/**
 * Submit action for "leave blank".
 */
function quiz_question_answering_form_submit_blank($form, &$form_state) {
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
    $qi_instance->saveResult();
    $bare_object = $qi_instance->toBareObject();
    quiz_store_question_result($quiz, $bare_object, array('set_msg' => TRUE, 'question_data' => $question_array));
  }

  // Advance to next question.
  quiz_question_goto($quiz, $_SESSION['quiz'][$quiz->nid]['current'] + 1);
  $form_state['redirect'] = "node/{$quiz->nid}/take/" . $_SESSION['quiz'][$quiz->nid]['current'];
}

/**
 * Submit handler for the question answering form.
 *
 * There is no validation code here, but there may be feedback code for
 * correct feedback.
 */
function quiz_question_answering_form_submit(&$form, &$form_state) {
  $quiz = node_load(arg(1));
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
      quiz_store_question_result($quiz, $result, array('set_msg' => TRUE, 'question_data' => $question_array));

      // Increment the counter.
      quiz_question_goto($quiz, $_SESSION['quiz'][$quiz->nid]['current'] + 1);
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
    _quiz_maintain_results($quiz, $quiz_result->result_id);
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
function quiz_question_answering_form_submit_back(&$form, &$form_state) {
  // Back a question.
  $quiz = node_load(arg(1));
  quiz_question_goto($quiz, $_SESSION['quiz'][$quiz->nid]['current'] - 1);
  $quiz_result = quiz_result_load($_SESSION['quiz'][$quiz->nid]['result_id']);
  $question = $quiz_result->layout[$_SESSION['quiz'][$quiz->nid]['current']];
  if (!empty($question['qnr_pid'])) {
    foreach ($quiz_result->layout as $question2) {
      if ($question2['qnr_id'] == $question['qnr_pid']) {
        quiz_question_goto($quiz, $question2['number']);
      }
    }
  }

  $form_state['redirect'] = "node/$quiz->nid/take/" . ($_SESSION['quiz'][$quiz->nid]['current']);
}

/**
 * Validation callback for quiz question submit.
 */
function quiz_question_answering_form_validate(&$form, &$form_state) {
  $quiz = node_load(arg(1));
  foreach ($form_state['values']['question'] as $nid => $answer) {
    $current_question = node_load($nid);

    if ($current_question) {
      // There was an answer submitted.
      $quiz_question = _quiz_question_get_instance($current_question);
      $quiz_question->getAnsweringFormValidate($form, $form_state);
    }
  }
}

/**
 * Element validator (for repeat until correct).
 */
function quiz_question_element_validate(&$element, &$form_state) {
  $quiz = node_load(arg(1));
  $question_nid = $element['#array_parents'][1];
  $answer = $form_state['values']['question'][$question_nid];
  $current_question = node_load($question_nid);

  // There was an answer submitted.
  $result = _quiz_question_response_get_instance($_SESSION['quiz'][$quiz->nid]['result_id'], $current_question, $answer);
  if ($quiz->repeat_until_correct && !$result->isCorrect()) {
    form_set_error('', t('The answer was incorrect. Please try again.'));

    $feedback = quiz_question_feedback($quiz, $current_question);
    $element['feedback'] = array(
      '#weight' => 100,
      '#markup' => drupal_render($feedback),
    );
  }
}
