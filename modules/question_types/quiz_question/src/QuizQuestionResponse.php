<?php

namespace Drupal\quiz_question;

use Drupal\quiz_question\QuizQuestion;
use stdClass;

/**
 * Each question type must store its own response data and be able to calculate a score for
 * that data.
 */
abstract class QuizQuestionResponse {

  // Result id
  protected $result_id = 0;
  protected $is_correct = FALSE;
  protected $evaluated = TRUE;
  // The question node(not a quiz question instance)
  public $question = NULL;
  public $quizQuestion = NULL;
  protected $answer = NULL;
  protected $score;
  public $is_skipped;
  public $is_doubtful;

  /**
   * Create a new user response.
   *
   * @param $result_id
   *  The result ID for the user's result set. There is one result ID per time
   *  the user takes a quiz.
   * @param $question_node
   *  The question node.
   * @param $answer
   *  The answer (dependent on question type).
   */
  public function __construct($result_id, stdClass $question_node, $answer = NULL) {
    $this->result_id = $result_id;
    $this->question = $question_node;
    $this->quizQuestion = _quiz_question_get_instance($question_node);
    $this->answer = $answer;
    $result = db_query('SELECT is_skipped, is_doubtful FROM {quiz_node_results_answers}
            WHERE result_id = :result_id AND question_nid = :question_nid AND question_vid = :question_vid', array(':result_id' => $result_id, ':question_nid' => $question_node->nid, ':question_vid' => $question_node->vid))->fetch();
    if (is_object($result)) {
      $this->is_doubtful = $result->is_doubtful;
      $this->is_skipped = $result->is_skipped;
    }
  }

  /**
   *
   * @return QuizQuestion
   */
  function getQuizQuestion() {
    return $this->quizQuestion;
  }

  /**
   * Used to refresh this instances question node in case drupal has changed it.
   *
   * @param $newNode
   *  Question node
   */
  public function refreshQuestionNode($newNode) {
    $this->question = $newNode;
  }

  /**
   * Indicate whether the response has been evaluated (scored) yet.
   * Questions that require human scoring (e.g. essays) may need to manually
   * toggle this.
   */
  public function isEvaluated() {
    return (bool) $this->evaluated;
  }

  /**
   * Check to see if the answer is marked as correct.
   *
   * This default version returns TRUE iff the score is equal to the maximum possible score.
   */
  function isCorrect() {
    return ($this->getMaxScore() == $this->getScore());
  }

  /**
   * Returns stored score if it exists, if not the score is calculated and returned.
   *
   * @param $weight_adjusted
   *  If the returned score shall be adjusted according to the max_score the question has in a quiz
   * @return
   *  Score(int)
   */
  function getScore($weight_adjusted = TRUE) {
    if ($this->is_skipped) {
      return 0;
    }
    if (!isset($this->score)) {
      $this->score = $this->score();
    }
    if (isset($this->question->score_weight) && $weight_adjusted) {
      return round($this->score * $this->question->score_weight);
    }
    return $this->score;
  }

  /**
   * Returns stored max score if it exists, if not the max score is calculated and returned.
   *
   * @param $weight_adjusted
   *  If the returned max score shall be adjusted according to the max_score the question has in a quiz
   * @return
   *  Max score(int)
   */
  public function getMaxScore($weight_adjusted = TRUE) {
    if (!isset($this->question->max_score)) {
      $this->question->max_score = $this->question->getMaximumScore();
    }
    if (isset($this->question->score_weight) && $weight_adjusted) {
      return round($this->question->max_score * $this->question->score_weight);
    }
    return $this->question->max_score;
  }

  /**
   * Represent the response as a stdClass object.
   *
   * Convert data to an object that has the following properties:
   * - $score
   * - $result_id
   * - $nid
   * - $vid
   * - $is_correct
   */
  function toBareObject() {
    $obj = new stdClass();
    $obj->score = $this->getScore(); // This can be 0 for unscored.
    $obj->nid = $this->question->nid;
    $obj->vid = $this->question->vid;
    $obj->result_id = $this->result_id;
    $obj->is_correct = (int) $this->isCorrect();
    $obj->is_evaluated = $this->isEvaluated();
    $obj->is_skipped = 0;
    $obj->is_doubtful = isset($_POST['is_doubtful']) ? $_POST['is_doubtful'] : 0;
    $obj->is_valid = $this->isValid();
    return $obj;
  }

  /**
   * Validates response from a quiz taker. If the response isn't valid the quiz taker won't be allowed to proceed.
   *
   * @return
   *  True if the response is valid.
   *  False otherwise
   */
  public function isValid() {
    return TRUE;
  }

  /**
   * Get data suitable for reporting a user's score on the question.
   * This expects an object with the following attributes:
   *
   *  answer_id; // The answer ID
   *  answer; // The full text of the answer
   *  is_evaluated; // 0 if the question has not been evaluated, 1 if it has
   *  score; // The score the evaluator gave the user; this should be 0 if is_evaluated is 0.
   *  question_vid
   *  question_nid
   *  result_id
   */
  public function getReport() {
    // Basically, we encode internal information in a
    // legacy array format for Quiz.
    $report = array(
      'answer_id'    => 0, // <-- Stupid vestige of multichoice.
      'answer'       => $this->answer,
      'is_evaluated' => $this->isEvaluated(),
      'is_correct'   => $this->isCorrect(),
      'score'        => $this->getScore(),
      'question_vid' => $this->question->vid,
      'question_nid' => $this->question->nid,
      'result_id'    => $this->result_id,
    );
    return $report;
  }

  /**
   * Creates the report form for the admin pages, and for when a user gets feedback after answering questions.
   *
   * The report is a form to allow editing scores and the likes while viewing the report form
   *
   * @return $form
   *  Drupal form array
   */
  public function getReportForm() {
    global $user;

    // Add general data, and data from the question type implementation
    $form = array();
    $form['nid'] = array(
      '#type'  => 'value',
      '#value' => $this->question->nid,
    );
    $form['vid'] = array(
      '#type'  => 'value',
      '#value' => $this->question->vid,
    );
    $form['result_id'] = array(
      '#type'  => 'value',
      '#value' => $this->result_id,
    );
    if ($submit = $this->getReportFormSubmit()) {
      $form['submit'] = array(
        '#type'  => 'value',
        '#value' => $submit,
      );
    }
    $form['question'] = $this->getReportFormQuestion();

    $form['answer_feedback'] = $this->getReportFormAnswerFeedback();

    $form['max_score'] = array(
      '#type'  => 'value',
      '#value' => ($this->canReview('score')) ? $this->getMaxScore() : '?',
    );

    $rows = array();

    $labels = array(
      'attempt'         => t('Your answer'),
      'choice'          => t('Choice'),
      'correct'         => t('Correct?'),
      'score'           => t('Score'),
      'answer_feedback' => t('Feedback'),
      'solution'        => t('Correct answer'),
    );
    drupal_alter('quiz_feedback_labels', $labels);

    foreach ($this->getReportFormResponse() as $idx => $row) {
      foreach ($labels as $reviewType => $label) {
        if (in_array($reviewType, array('choice')) || (isset($row[$reviewType]) && $this->canReview($reviewType))) {
          $rows[$idx][$reviewType] = $row[$reviewType];
        }
      }
    }

    if ($this->isEvaluated()) {
      $score = $this->getScore();
      if ($this->isCorrect()) {
        $class = 'q-correct';
      }
      else {
        $class = 'q-wrong';
      }
    }
    else {
      $score = t('?');
      $class = 'q-waiting';
    }

    if (quiz()->getQuizHelper()->getAccessHelper()->canAccessQuizScore($user) && $submit) {
      $form['score'] = $this->getReportFormScore();
    }

    if ($this->canReview('score') || quiz()->getQuizHelper()->getAccessHelper()->canAccessQuizScore($user)) {
      $form['score_display']['#markup'] = theme('quiz_question_score', array('score' => $score, 'max_score' => $this->getMaxScore(), 'class' => $class));
    }

    $headers = array_intersect_key($labels, $rows[0]);
    $type = $this->getQuizQuestion()->node->type;
    $form['response']['#markup'] = theme('quiz_question_feedback__' . $type, array('labels' => $headers, 'data' => $rows));

    $form['#theme'] = $this->getReportFormTheme();
    return $form;
  }

  /**
   * get the question part of the reportForm
   *
   * @return
   *  FAPI form array holding the question
   */
  public function getReportFormQuestion() {
    $node = node_load($this->question->nid);
    $node->no_answer_form = TRUE;
    node_build_content($node, 'question');
    return $node->content;
  }

  /**
   * Get the response part of the report form

   * @return
   *  Array of choices
   */
  public function getReportFormResponse() {
    $data = array();

    $data[] = array(
      'choice'            => 'True',
      'attempt'           => 'Did the user choose this?',
      'correct'           => 'Was their answer correct?',
      'score'             => 'Points earned for this answer',
      'answer_feedback'   => 'Feedback specific to the answer',
      'question_feedback' => 'General question feedback for any answer',
      'solution'          => 'Is this choice the correct solution?',
      'quiz_feedback'     => 'Quiz feedback at this time',
    );

    return $data;
  }

  public function getReportFormAnswerFeedback() {
    return FALSE;
  }

  /**
   * Get the submit function for the reportForm
   *
   * @return
   *  Submit function as a string, or FALSE if no submit function
   */
  public function getReportFormSubmit() {
    return FALSE;
  }

  /**
   * Get the validate function for the reportForm
   *
   * @return
   *  Validate function as a string, or FALSE if no validate function
   */
  public function getReportFormValidate(&$element, &$form_state) {
    return FALSE;
  }

  /**
   * Get the theme key for the reportForm
   *
   * @return
   *  Theme key as a string, or FALSE if no submit function
   */
  public function getReportFormTheme() {
    return FALSE;
  }

  /**
   * Saves the quiz result. This should only be called when an answer is
   * provided.
   */
  public function saveResult() {
    $this->save();
  }

  /**
   * Utility function that returns the format of the node body
   */
  protected function getFormat() {
    $body = field_get_items('node', $this->question, 'body');
    return ($body ? $body[0]['format'] : NULL);
  }

  /**
   * Save the current response.
   */
  abstract public function save();

  /**
   * Delete the response.
   */
  abstract public function delete();

  /**
   * Calculate the score for the response.
   */
  abstract public function score();

  /**
   * Get the user's response.
   */
  abstract public function getResponse();

  /**
   * Can the quiz taker view the requested review?
   */
  public function canReview($option) {
    $quiz_result = quiz_result_load($this->result_id);
    return quiz_feedback_can_review($option, $quiz_result);
  }

  /**
   * Implementation of getReportFormScore
   *
   * @see QuizQuestionResponse#getReportFormScore()
   */
  public function getReportFormScore() {
    $score = ($this->isEvaluated()) ? $this->getScore() : '';
    return array(
      '#title'            => 'Enter score',
      '#type'             => 'textfield',
      '#default_value'    => $score,
      '#size'             => 3,
      '#maxlength'        => 3,
      '#attributes'       => array('class' => array('quiz-report-score')),
      '#element_validate' => array('element_validate_integer'),
      '#required'         => TRUE,
      '#field_suffix'     => '/ ' . $this->getMaxScore(),
    );
  }

  /**
   * Set the target result ID for this Question response.
   *
   * Useful for cloning entire result sets.
   */
  public function setResultId($result_id) {
    $this->result_id = $result_id;
  }

}
