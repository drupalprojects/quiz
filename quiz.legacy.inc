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
 * Insert or update the quiz node properties accordingly.
 */
function quiz_save_properties($entity) {
  return quiz()->getQuizHelper()->getSettingHelper()->saveQuizSettings($entity);
}

/**
 * Returns default values for all quiz settings.
 *
 * @todo also store this in the quiz_node_properties table
 *
 * @return
 *   Array of default values.
 */
function _quiz_get_node_defaults() {
  return quiz()->getQuizHelper()->getSettingHelper()->getNodeDefaultSettings();
}

/**
 * Returns the titles for all quizzes the user has access to.
 *
 * @return quizzes
 *   Array with nids as keys and titles as values.
 */
function quiz_get_all_titles() {
  return quiz()->getAllTitles();
}

/**
 * Returns the titles for all quizzes the user has access to.
 *
 * @return quizzes
 *   Array with nids as keys and (array with vid as key and title as value) as values.
 *   Like this: array($nid => array($vid => $title))
 */
function quiz_get_all_version_titles() {
  return quiz()->getAllRevisionTitles();
}

/**
 * Return highest score data for given quizzes.
 *
 * @param $nids
 *   nids for the quizzes we want to collect scores from.
 * @param $uid
 *   uid for the user we want to collect score for.
 * @param $include_num_questions
 *   Do we want to collect information about the number of questions in a quiz?
 *   This adds a performance hit.
 * @return
 *   Array of score data.
 *   For several takes on the same quiz, only returns highest score.
 */
function quiz_get_score_data($nids, $uid, $include_num_questions = FALSE) {
  return quiz()->getQuizHelper()->getScoreData($nids, $uid, $include_num_questions);
}

/**
 * Finds out the number of questions for the quiz.
 *
 * Good example of usage could be to calculate the % of score.
 *
 * @param $nid
 *   Quiz ID.
 * @return
 *   Returns the number of quiz questions.
 */
function quiz_get_number_of_questions($vid) {
  return quiz()->getQuizHelper()->countQuestion($vid);
}

/**
 * Store a quiz question result.
 *
 * @param $quiz
 *  The quiz node
 * @param $result
 *  Object with data about the result for a question.
 * @param $options
 *  Array with options that affect the behavior of this function.
 *  ['set_msg'] - Sets a message if the last question was skipped.
 */
function quiz_store_question_result($quiz, $result, $options) {
  return quiz()->getQuizHelper()->saveQuestionResult($quiz, $result, $options);
}

/**
 * Check a user/quiz combo to see if the user passed the given quiz.
 *
 * This will return TRUE if the user has passed the quiz at least once, and
 * FALSE otherwise. Note that a FALSE may simply indicate that the user has not
 * taken the quiz.
 *
 * @param $uid
 *   The user ID.
 * @param $nid
 *   The node ID.
 * @param $vid
 *   The version ID.
 */
function quiz_is_passed($uid, $nid, $vid) {
  return quiz()->getQuizHelper()->isPassed($uid, $nid, $vid);
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
 * Sets the questions that are assigned to a quiz.
 *
 * @param $quiz
 *   The quiz(node) to modify.
 * @param $questions
 *   An array of questions.
 * @param $set_new_revision
 *   If TRUE, a new revision will be generated. Note that saving
 *   quiz questions unmodified will still generate a new revision of the quiz if
 *   this is set to TRUE. Why? For a few reasons:
 *   - All of the questions are updated to their latest VID. That is supposed to
 *     be a feature.
 *   - All weights are updated.
 *   - All status flags are updated.
 *
 * @return
 *   Boolean TRUE if update was successful, FALSE otherwise.
 */
function quiz_set_questions(&$quiz, $questions, $set_new_revision = FALSE) {
  return quiz()->getQuizHelper()->setQuestions($quiz, $questions, $set_new_revision);
}

/**
 * @} End of "defgroup quiz_public".
 */

/**
 * Delete quiz responses for quizzes that haven't been finished.
 *
 * @param $quiz
 *   A quiz node where old in progress results shall be deleted.
 * @param $uid
 *   The userid of the user the old in progress results belong to.
 */
function _quiz_delete_old_in_progress($quiz, $uid) {
  return quiz()->getQuizHelper()->getResultHelper()->deleteIncompletedResultsByUserId($quiz, $uid);
}

/**
 * Get an array list of random questions for a quiz.
 *
 * @param $quiz
 *   The quiz node.
 *
 * @return
 *   Array of nid/vid combos for quiz questions.
 */
function _quiz_get_random_questions($quiz) {
  return quiz()->getQuizHelper()->getRandomQuestions($quiz);
}

/**
 * Retrieve list of published questions assigned to quiz.
 *
 * This function should be used for question browsers and similiar... It should not be used to decide what questions
 * a user should answer when taking a quiz. quiz_build_question_list is written for that purpose.
 *
 * @param $quiz_nid
 *   Quiz node id.
 * @param $quiz_vid
 *   Quiz node version id.
 *
 * @return
 *   An array of questions.
 */
function quiz_get_questions($quiz_nid, $quiz_vid = NULL) {
  return quiz()->getQuizHelper()->getQuestions($quiz_nid, $quiz_vid);
}

/**
 * Get sub-questions.
 */
function quiz_get_sub_questions($qnr_pid, &$questions) {
  return quiz()->getQuizHelper()->getSubQuestions($qnr_pid, $questions);
}

/**
 * Get a list of all available quizzes.
 *
 * @param $uid
 *   An optional user ID. If supplied, only quizzes created by that user will be
 *   returned.
 *
 * @return
 *   A list of quizzes.
 */
function _quiz_get_quizzes($uid = 0) {
  return quiz()->getQuizHelper()->getQuizzesByUserId($uid);
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
 * Copies questions when a quiz is translated.
 *
 * @param $node
 *   The new translated quiz node.
 */
function quiz_copy_questions($node) {
  return quiz()->getQuizHelper()->copyQuestions($node);
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
 * Delete quiz results.
 *
 * @param $result_ids
 *   Result ids for the results to be deleted.
 */
function quiz_delete_results($result_ids) {
  return quiz()->getQuizHelper()->getResultHelper()->deleteByIds($result_ids);
}

/**
 * Returns the users default settings.
 *
 * @param $node
 *   Quiz node.
 * @param $uid
 *   (optional) The uid of the user to get the settings for. Defaults to the
 *   current user (NULL).
 *
 * @return
 *   An array of settings. The array is empty in case no settings are available.
 */
function quiz_get_defaults() {
  return quiz()->getQuizHelper()->getSettingHelper()->getUserDefaultSettings();
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
  $quiz = node_load(arg(1));
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
 * Get the feedback options for Quizzes.
 */
function quiz_get_feedback_options() {
  return quiz()->getQuizHelper()->getFeedbackHelper()->getOptions();
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
 * Update a score for a quiz.
 *
 * This updates the quiz node results table.
 *
 * It is used in cases where a quiz score is changed after the quiz has been
 * taken. For example, if a long answer question is scored later by a human,
 * then the quiz should be updated when that answer is scored.
 *
 * Important: The value stored in the table is the *percentage* score.
 *
 * @param $quiz
 *   The quiz node for the quiz that is being scored.
 * @param $result_id
 *   The result ID to update.
 * @return
 *   The score as an integer representing percentage. E.g. 55 is 55%.
 */
function quiz_update_total_score($quiz, $result_id) {
  return quiz()->getQuizHelper()->getResultHelper()->updateTotalScore($quiz, $result_id);
}

/**
 * Map node properties to a question object.
 *
 * @param $node
 *  The question node.
 *
 * @return
 *  Question object.
 */
function quiz_node_map($node) {
  return quiz()->getQuizHelper()->reloadQuestion($node);
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
