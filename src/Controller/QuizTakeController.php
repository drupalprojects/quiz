<?php

namespace Drupal\quiz\Controller;

use Drupal\quiz\Controller\Legacy\QuizTakeLegacyController;
use RuntimeException;
use stdClass;

class QuizTakeController extends QuizTakeLegacyController {

  /** @var stdClass */
  private $result;

  /** @var stdClass */
  private $account;

  /**
   * Callback for node/%quiz_menu/take
   */
  public static function staticCallback($quiz) {
    global $user;

    try {
      if (isset($quiz->rendered_content)) {
        return $quiz->rendered_content;
      }

      $controller = new static($quiz, $user);
      if ($controller->getResultId()) {
        drupal_goto($controller->getQuestionTakePath());
      }
    }
    catch (RuntimeException $e) {
      return array('body' => ['#markup' => $e->getMessage()]);
    }
  }

  public function __construct($quiz, $account) {
    parent::__construct(isset($quiz->nid) ? 'node' : 'quiz_entity');
    $this->quiz = $quiz;
    $this->account = $account;
    $this->initQuizResult();
  }

  private function initQuizResult() {
    // Inject result from user's session
    if (!empty($_SESSION['quiz'][$this->getQuizId()]['result_id'])) {
      $this->result_id = $_SESSION['quiz'][$this->getQuizId()]['result_id'];
      $this->result = quiz_result_load($this->result_id);
    }

    // Enforce that we have the same quiz version.
    if ((null !== $this->result) && ($this->quiz->vid != $this->result->vid)) {
      $this->quiz = $this->loadQuiz($this->getQuizId(), $this->quiz->vid);
    }

    // Resume quiz progress
    if (!$this->result && $this->quiz->allow_resume) {
      $this->initQuizResume();
    }

    // Start new quiz progress
    if (!$this->result) {
      if (!$this->checkAvailability($this->account)) {
        throw new RuntimeException(t('This quiz is closed.'));
      }

      $this->quiz_result = $this->createQuizResultObject();
      $_SESSION['quiz'][$this->getQuizId()]['result_id'] = $this->quiz_result->result_id;
      $_SESSION['quiz'][$this->getQuizId()]['current'] = 1;

      // Call hook_quiz_begin().
      module_invoke_all('quiz_begin', $this->quiz, $this->quiz_result->result_id);
    }

    if (!quiz()->getQuizHelper()->isAvailable($this->quiz)) {
      throw new RuntimeException(t('This quiz is not available.'));
    }
  }

  /**
   * If we allow resuming we can load it from the database.
   */
  private function initQuizResume() {
    if (!$result_id = $this->activeResultId($this->account->uid, $this->quiz->vid)) {
      return FALSE;
    }

    $this->result_id = $result_id;

    $_SESSION['quiz'][$this->getQuizId()]['result_id'] = $this->result_id;
    $_SESSION['quiz'][$this->getQuizId()]['current'] = 1;
    $quiz_result = quiz_result_load($result_id);
    $this->quiz = $this->loadQuiz($quiz_result->nid, $quiz_result->vid);

    // Resume a quiz from the database.
    drupal_set_message(t('Resuming a previous quiz in-progress.'), 'status');
  }

  /**
   * Returns the result ID for any current result set for the given quiz.
   *
   * @param int $uid
   * @param int $vid Quiz version ID
   * @param int $now
   *   Timestamp used to check whether the quiz is still open. Default: current
   *   time.
   *
   * @return int
   *   If a quiz is still open and the user has not finished the quiz,
   *   return the result set ID so that the user can continue. If no quiz is in
   *   progress, this will return 0.
   */
  protected function activeResultId($uid, $vid, $now = NULL) {
    if (!isset($now)) {
      $now = REQUEST_TIME;
    }

    // Get any quiz that is open, for this user, and has not already
    // been completed.
    $result_id = db_query('SELECT result_id FROM {quiz_results} qnr
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
   * @return
   *   Return quiz_results result_id, or FALSE if there is an error.
   */
  private function checkAvailability($account) {
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
      $taken = db_query("SELECT COUNT(*) AS takes FROM {quiz_results} WHERE uid = :uid AND nid = :nid", array(':uid' => $account->uid, ':nid' => $this->getQuizId()))->fetchField();
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
    if ($this->quiz->show_passed && $account->uid && quiz()->getQuizHelper()->isPassed($account->uid, $this->getQuizId(), $this->quiz->vid)) {
      drupal_set_message(t('You have already passed this @quiz.', array('@quiz' => QUIZ_NAME)), 'status');
    }

    return TRUE;
  }

  /**
   * Initialize a quiz attempt.
   *
   * @return QuizResult
   *   The quiz attempt.
   */
  private function createQuizResultObject() {
    // Create question list.
    $questions = quiz()->getQuizHelper()->getQuestionList($this->quiz);
    if ($questions === FALSE) {
      $msg = t('Not enough random questions were found. Please add more questions before trying to take this @quiz.', array('@quiz' => QUIZ_NAME));
      throw new RuntimeException($msg);
    }

    if (!count($questions)) {
      $msg = t('No questions were found. Please !assign_questions before trying to take this @quiz.', array('@quiz' => QUIZ_NAME, '!assign_questions' => l(t('assign questions'), 'node/' . $this->getQuizId() . '/quiz/questions')));
      throw new RuntimeException($msg);
    }

    $quiz_result = entity_create('quiz_result', array(
      'nid'        => $this->getQuizId(),
      'vid'        => $this->quiz->vid,
      'uid'        => $this->account->uid,
      'time_start' => REQUEST_TIME,
      'layout'     => $questions,
    ));

    // Write the layout for this result.
    entity_save('quiz_result', $quiz_result);

    return $quiz_result;
  }

}
