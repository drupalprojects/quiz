<?php

namespace Drupal\quiz\Helper\Quiz;

class AccessHelper {

  public function userHasResult($quiz, $uid) {
    $sql = 'SELECT 1 FROM {quiz_node_results} WHERE nid = :nid AND uid = :uid AND is_evaluated = :is_evaluated';
    return db_query($sql, array(':nid' => $quiz->nid, ':uid' => $uid, ':is_evaluated' => 1))
        ->fetchField();
  }

  /**
   * Helper function to determine if a user has access to view his quiz results
   *
   * @param object $quiz
   *  The Quiz node
   */
  public function canAccessMyResults($quiz, $account) {
    if ($quiz->type !== 'quiz') {
      return false;
    }
    return $this->userHasResult($quiz, $account->uid);
  }

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
  public function canAccessResults($account, $quiz, $result_id = NULL) {
    if ($quiz->type !== 'quiz') {
      return FALSE;
    }
    // If rid is set we must make sure the result belongs to the quiz we are
    // viewing results for.
    if (isset($result_id)) {
      $res = db_query('SELECT qnr.nid, qnr.uid FROM {quiz_node_results} qnr WHERE result_id = :result_id', array(':result_id' => $result_id))->fetch();
      if ($res && $res->nid != $quiz->nid) {
        return FALSE;
      }
    }
    if (user_access('view any quiz results')) {
      return TRUE;
    }
    if (user_access('view results for own quiz') && $account->uid == $quiz->uid) {
      return TRUE;
    }
    if (user_access('score taken quiz answer')) {
      //check if the taken user is seeing his result
      if (isset($result_id) && $res && $res->uid == $account->uid) {
        return TRUE;
      }
    }
  }

  /**
   * Helper function to determine if a user has access to score a quiz.
   *
   * @param $quiz_creator
   *   uid of the quiz creator.
   */
  public function canAccessQuizScore($account, $quiz_creator = NULL) {
    if ($quiz_creator == NULL && ($quiz = quiz_get_quiz_from_menu())) {
      $quiz_creator = $quiz->uid;
    }
    if (user_access('score any quiz')) {
      return TRUE;
    }
    if (user_access('score own quiz') && $account->uid == $quiz_creator) {
      return TRUE;
    }
    if (user_access('score taken quiz answer')) {
      return TRUE;
    }
  }

  /**
   * Checks if the user has access to save score for his quiz.
   */
  public function canAccessScore($quiz, $account) {
    if (user_access('score any quiz', $account)) {
      return true;
    }
    return user_access('score own quiz', $account) && ($quiz->uid == $account->uid);
  }

  public function canAccessQuestion($quiz, $question_number) {
    if (!$question_number) {
      return FALSE;
    }

    if ($quiz->allow_jumping) { // Access to go to any question. Yay.
      return TRUE;
    }

    $result_id = $_SESSION['quiz'][$quiz->nid]['result_id'];
    $quiz_result = quiz_result_load($result_id);
    $question_index = $question_number;
    $qinfo_last = $question_number == 1 ? NULL : $quiz_result->layout[$question_index - 1];
    $qinfo = $quiz_result->layout[$question_index];

    if (!$quiz->backwards_navigation) { // No backwards navigation.
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

  public function canTakeQuiz($quiz, $account) {
    if ($quiz->type != 'quiz') {
      return FALSE;
    }

    if (!quiz_availability($quiz)) {
      return FALSE;
    }

    return node_access('view', $quiz, $account) && user_access('access quiz', $account);
  }

}
