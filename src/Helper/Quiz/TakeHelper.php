<?php

namespace Drupal\quiz\Helper\Quiz;

class TakeHelper {

  private $quiz;

  public function setQuiz($quiz) {
    $this->quiz = $quiz;
  }

  /**
   * Primary quiz-taking view on 'Take' tab.
   */
  public function render() {
    global $user;

    if (isset($this->quiz->rendered_content)) {
      return $this->quiz->rendered_content;
    }

    $render_array = $this->buildRenderArray($user);
    return drupal_render($render_array);
  }

  public function buildRenderArray($account) {
    // Make sure we use the same revision of the quiz throughout the quiz taking
    // session.
    $result_id = !empty($_SESSION['quiz'][$this->quiz->nid]['result_id']) ? $_SESSION['quiz'][$this->quiz->nid]['result_id'] : NULL;
    if ($result_id && $quiz_result = quiz_result_load($result_id)) {
      // Enforce that we have the same quiz version.
      $this->quiz = node_load($quiz_result->nid, $quiz_result->vid);
    }
    else {
      // User doesn't have attempt in session. If we allow resuming we can load it
      // from the database.
      if ($this->quiz->allow_resume) {
        if ($result_id = $this->activeResultId($account->uid, $this->quiz->nid, $this->quiz->vid)) {
          $_SESSION['quiz'][$this->quiz->nid]['result_id'] = $result_id;
          $_SESSION['quiz'][$this->quiz->nid]['current'] = 1;
          $quiz_result = quiz_result_load($result_id);
          $this->quiz = node_load($quiz_result->nid, $quiz_result->vid);
          // Resume a quiz from the database.
          drupal_set_message(t('Resuming a previous quiz in-progress.'), 'status');
        }
      }
    }

    if (!$result_id) {
      // Can user start quiz?
      if ($this->startCheck($account)) {
        // Set up a new attempt.
        $quiz_result = _quiz_take_quiz_init($this->quiz);
        $_SESSION['quiz'][$this->quiz->nid]['result_id'] = $quiz_result->result_id;
        $_SESSION['quiz'][$this->quiz->nid]['current'] = 1;

        // Call hook_quiz_begin().
        module_invoke_all('quiz_begin', $this->quiz, $quiz_result->result_id);
      }
      else {
        return array('body' => array('#markup' => t('This quiz is closed.')));
      }
    }

    if (!quiz_availability($this->quiz)) {
      return array('body' => array('#markup' => t('This quiz is not available.')));
    }

    drupal_goto("node/{$this->quiz->nid}/take/" . ($_SESSION['quiz'][$this->quiz->nid]['current']));
  }

  /**
   * Returns the result ID for any current result set for the given quiz.
   *
   * @param $uid
   *   User ID
   * @param $nid
   *   Quiz node ID
   * @param $vid
   *   Quiz node version ID
   * @param $now
   *   Timestamp used to check whether the quiz is still open. Default: current
   *   time.
   *
   * @return
   *   If a quiz is still open and the user has not finished the quiz,
   *   return the result set ID so that the user can continue. If no quiz is in
   *   progress, this will return 0.
   */
  protected function activeResultId($uid, $nid, $vid, $now = NULL) {
    if (!isset($now)) {
      $now = REQUEST_TIME;
    }

    // Get any quiz that is open, for this user, and has not already
    // been completed.
    $result_id = db_query('SELECT result_id FROM {quiz_node_results} qnr
          INNER JOIN {quiz_node_properties} qnp ON qnr.vid = qnp.vid
          WHERE (qnp.quiz_always = :quiz_always OR (:between BETWEEN qnp.quiz_open AND qnp.quiz_close))
          AND qnr.vid = :vid
          AND qnr.uid = :uid
          AND qnr.time_end IS NULL', array(':quiz_always' => 1, ':between' => $now, ':vid' => $vid, ':uid' => $uid))->fetchField();
    return (int) $result_id;
  }

  /**
   * Actions to take place at the start of a quiz.
   *
   * This is called when the quiz node is viewed for the first time. It ensures
   * that the quiz can be taken at this time.
   *
   * @param $quiz
   *   The quiz node.
   *
   * @return
   *   Return quiz_node_results result_id, or FALSE if there is an error.
   */
  private function startCheck($account) {
    $user_is_admin = user_access('edit any quiz content') || (user_access('edit own quiz content') && $this->quiz->uid == $account->uid);

    // Make sure this is available.
    if ($this->quiz->quiz_always != 1) {
      // Compare current GMT time to the open and close dates (which should still
      // be in GMT time).
      $now = REQUEST_TIME;

      if ($now >= $this->quiz->quiz_close || $now < $this->quiz->quiz_open) {
        if ($user_is_admin) {
          drupal_set_message(t('You are marked as an administrator or owner for this quiz. While you can take this quiz, the open/close times prohibit other users from taking this quiz.'), 'status');
        }
        else {
          drupal_set_message(t('This @quiz is not currently available.', array('@quiz' => QUIZ_NAME)), 'status');
          // Can't take quiz.
          return FALSE;
        }
      }
    }

    // Check to see if this user is allowed to take the quiz again:
    if ($this->quiz->takes > 0) {
      $taken = db_query("SELECT COUNT(*) AS takes FROM {quiz_node_results} WHERE uid = :uid AND nid = :nid", array(':uid' => $account->uid, ':nid' => $this->quiz->nid))->fetchField();
      $allowed_times = format_plural($this->quiz->takes, '1 time', '@count times');
      $taken_times = format_plural($taken, '1 time', '@count times');

      // The user has already taken this quiz.
      if ($taken) {
        if ($user_is_admin) {

          drupal_set_message(t('You have taken this quiz already. You are marked as an owner or administrator for this quiz, so you can take this quiz as many times as you would like.'), 'status');
        }
        // If the user has already taken this quiz too many times, stop the user.
        elseif ($taken >= $this->quiz->takes) {
          drupal_set_message(t('You have already taken this quiz @really. You may not take it again.', array('@really' => $taken_times)), 'error');
          return FALSE;
        }
        // If the user has taken the quiz more than once, see if we should report
        // this.
        elseif ($this->quiz->show_attempt_stats) {
          drupal_set_message(t("You can only take this quiz @allowed. You have taken it @really.", array('@allowed' => $allowed_times, '@really' => $taken_times)), 'status');
        }
      }
    }

    // Check to see if the user is registered, and user alredy passed this quiz.
    if ($this->quiz->show_passed && $account->uid && quiz_is_passed($account->uid, $this->quiz->nid, $this->quiz->vid)) {
      drupal_set_message(t('You have already passed this @quiz.', array('@quiz' => QUIZ_NAME)), 'status');
    }

    return TRUE;
  }

}
