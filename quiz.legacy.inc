<?php

/**
 * Helper function to determine if a user has access to the different results
 * pages.
 *
 * @param $quiz
 *   The quiz node.
 * @param $result_id
 *   The result id of a result we are trying to access.
 * @return boolean
 *   TRUE if user has permission.
 */
function quiz_access_results($quiz, $result_id = NULL) {
  global $user;
  return quiz()->getQuizHelper()->getAccessHelper()->canAccessResults($user, $quiz, $result_id);
}

/**
 * Helper function to determine if a user has access to view his quiz results
 *
 * @param object $quiz
 *  The Quiz node
 */
function quiz_access_my_results($quiz) {
  global $user;
  return quiz()->getQuizHelper()->getAccessHelper()->canAccessMyResults($quiz, $user);
}

/**
 * Helper function to determine if a user has access to view a specific quiz result.
 *
 * @param int|stdClass $result
 *  Result id
 * @return boolean
 *  True if access, false otherwise
 */
function quiz_access_my_result($result) {
  $result_id = is_object($result) ? $result->result_id : $result;
  return quiz()->getQuizHelper()->getResultHelper()->isResultCompleted($result_id);
}

/**
 * Helper function to determine if a user has access to score a quiz.
 *
 * @param $quiz_creator
 *   uid of the quiz creator.
 */
function quiz_access_to_score($quiz_creator = NULL) {
  global $user;
  return quiz()->getQuizHelper()->getAccessHelper()->canAccessQuizScore($user, $quiz_creator);
}

function quiz_update_defaults($node) {
  return quiz()->getQuizHelper()->getSettingHelper()->updateUserDefaultSettings($node);
}

/**
 * Find out if a quiz is available for taking or not
 *
 * @param $quiz
 *  The quiz node
 * @return
 *  TRUE if available
 *  Error message(String) if not available
 */
function quiz_availability($quiz) {
  return quiz()->getQuizHelper()->isAvailable($quiz);
}

/**
 * Calculates the score user received on quiz.
 *
 * @param $quiz
 *   The quiz node.
 * @param $result_id
 *   Quiz result ID.
 *
 * @return array
 *   Contains three elements: question_count, num_correct and percentage_score.
 */
function quiz_calculate_score($quiz, $result_id) {
  return quiz()->getQuizHelper()->getResultHelper()->calculateScore($quiz, $result_id);
}

/**
 * Retrieves a list of questions (to be taken) for a given quiz.
 *
 * If the quiz has random questions this function only returns a random
 * selection of those questions. This function should be used to decide
 * what questions a quiz taker should answer.
 *
 * This question list is stored in the user's result, and may be different
 * when called multiple times. It should only be used to generate the layout
 * for a quiz attempt and NOT used to do operations on the questions inside of
 * a quiz.
 *
 * @param $quiz
 *   Quiz node.
 * @return
 *   Array of question node IDs.
 */
function quiz_build_question_list($quiz) {
  return quiz()->getQuizHelper()->getQuestionList($quiz);
}

/**
 * Get answer data for a specific result.
 *
 * @param $result_id
 *   Result id.
 *
 * @return
 *   Array of answers.
 */
function _quiz_get_answers($quiz, $result_id) {
  return quiz()->getQuizHelper()->getResultHelper()->getAnswers($quiz, $result_id);
}

/**
 * Get the quiz name variable and set it as a constant so we don't have to keep
 * calling it in every function.
 *
 * @return
 *   Quiz name variable.
 */
function _quiz_get_quiz_name() {
  // @todo: remove all the quiz_name stuff? Isn't there better ways to do this?
  return variable_get('quiz_name', 'Quiz');
}

/**
 * Returns an array with quiz titles keyed with quiz node ids.
 *
 * @return array
 *   Options suitable for a form, in which the value is nid.
 */
function quiz_get_quiz_options() {
  return quiz()->getQuizHelper()->getSettingHelper()->getQuizOptions();
}

/**
 * Finds out if a quiz has been answered or not.
 *
 * @return
 *   TRUE if there exists answers to the current question.
 */
function quiz_has_been_answered($node) {
  return quiz()->getQuizHelper()->isAnswered($node);
}

/**
 * Deletes results for a quiz according to the keep results setting
 *
 * @param $quiz
 *  The quiz node to be maintained
 * @param $result_id
 *  The result id of the latest result for the current user
 * @return
 *  TRUE if results where deleted.
 */
function _quiz_maintain_results($quiz, $result_id) {
  global $user;
  return quiz()->getQuizHelper()->getResultHelper()->maintainResult($user, $quiz, $result_id);
}

/**
 * Menu access check for question feedback.
 */
function quiz_question_feedback_access($quiz, $question_number) {
  return quiz()->getQuizHelper()->getFeedbackHelper()->canAccess($quiz, $question_number);
}

/**
 * Load a specific result answer.
 */
function quiz_result_answer_load($result_id, $nid, $vid) {
  return quiz()->getQuizHelper()->getResultHelper()->loadAnswerResult($result_id, $nid, $vid);
}

/**
 * Action to add questions to the current quiz.
 */
function quiz_add_question_to_quiz($question) {
  $quiz = node_load(arg(1)); // @TODO: Is this correct? We can load quiz from question thou!
  return quiz()->getQuizHelper()->addQuestion($quiz, $question);
}

/**
 * Can the quiz taker view the requested review?
 *
 * There's a workaround in here: @kludge
 *
 * When review for the question is enabled, and it is the last question,
 * technically it is the end of the quiz, and the "end of quiz" review settings
 * apply. So we check to make sure that we are in question taking and the
 * feedback is viewed within 5 seconds of completing the question/quiz.
 */
function quiz_feedback_can_review($option, $quiz_result) {
  return quiz()->getQuizHelper()->getFeedbackHelper()->canReview($option, $quiz_result);
}

/**
 * Updates the max_score property on the specified quizzes
 *
 * @param $vids
 *  Array with the vid's of the quizzes to update
 */
function quiz_update_max_score_properties($vids) {
  return quiz()->getQuizHelper()->updateMaxScoreProperties($vids);
}

/**
 * @param string $question_type
 *
 * @return string
 *   Name of module matching the question type, as given by quiz_question_info()
 *   hook.
 */
function quiz_question_module_for_type($question_type) {
  return quiz()->getQuizHelper()->getQuestionModuleFromType($question_type);
}

/**
 * Checks if the user has access to save score for his quiz.
 */
function quiz_allow_score_access($quiz, $account = NULL) {
  global $user;

  if ($account == NULL) {
    $account = $user;
  }

  return quiz()->getQuizHelper()->getAccessHelper()->canAccessScore($quiz, $account);
}
