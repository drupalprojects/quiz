<?php
/**
 * Each question type must store its own response data and be able to calculate a score for
 * that data.
 * Contains Drupal\quiz_question\QuizQuestionResponse.
 */
 
namespace Drupal\quiz_question;
 
abstract class QuizQuestionResponse {
  // Result id
  protected $rid = 0;

  protected $is_correct = FALSE;
  protected $evaluated = TRUE;

  // The question node(not a quiz question instance)
  public $question = NULL;

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
  public function __construct($result_id, $question_node, $answer = NULL) {
    $this->rid = $result_id;
    $this->question = $question_node;
    $this->answer = $answer;
    $result = db_query('SELECT is_skipped, is_doubtful FROM {quiz_node_results_answers}
            WHERE result_id = :result_id AND question_nid = :question_nid AND question_vid = :question_vid', array(':result_id' => $result_id, ':question_nid' => $question_node->id(), ':question_vid' => $question_node->getRevisionId()))->fetch();
    if (is_object($result)) {
      $this->is_doubtful = $result->is_doubtful;
      $this->is_skipped = $result->is_skipped;
    }
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
   * - $rid
   * - $nid
   * - $vid
   * - $is_correct
   */
  function toBareObject() {
    $obj = new \stdClass();
    $obj->score = $this->getScore(); // This can be 0 for unscored.
    $obj->nid = $this->question->nid;
    $obj->vid = $this->question->vid;
    $obj->rid = $this->rid;
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
      'answer_id' => 0, // <-- Stupid vestige of multichoice.
      'answer' => $this->answer,
      'is_evaluated' => $this->isEvaluated(),
      'is_correct' => $this->isCorrect(),
      'score' => $this->getScore(),
      'question_vid' => $this->question->vid,
      'question_nid' => $this->question->nid,
      'result_id' => $this->rid,
    );
    return $report;
  }

  /**
   * Creates the report form for the admin pages, and for when a user gets feedback after answering questions.
   *
   * The report is a form to allow editing scores and the likes while viewing the report form
   *
   * @param $showpoints
   * @param $showfeedback
   * @param $allow_scoring
   * @return $form
   *  Drupal form array
   */
  public function getReportForm($showpoints = TRUE, $showfeedback = TRUE, $allow_scoring = FALSE) {
    /*
     * Add general data, and data from the question type implementation
     */
    $form = array();
    $form['nid'] = array(
      '#type' => 'value',
      '#value' => $this->question->nid,
    );
    $form['vid'] = array(
      '#type' => 'value',
      '#value' => $this->question->vid,
    );
    $form['rid'] = array(
      '#type' => 'value',
      '#value' => $this->rid,
    );
    if ($submit = $this->getReportFormSubmit($showpoints, $showfeedback, $allow_scoring)) {
      $form['submit'] = array(
        '#type' => 'value',
        '#value' => $submit,
      );
    }
    if ($validate = $this->getReportFormValidate($showpoints, $showfeedback, $allow_scoring)) {
      $form['validate'] = array(
        '#type' => 'value',
        '#value' => $validate,
      );
    }
    $form['question'] = $this->getReportFormQuestion($showpoints, $showfeedback);
    $form['score'] = $this->getReportFormScore($showpoints, $showfeedback, $allow_scoring);
    $form['answer_feedback'] = $this->getReportFormAnswerFeedback($showpoints, $showfeedback, $allow_scoring);
    $form['max_score'] = array(
      '#type' => 'value',
      '#value' => $this->getMaxScore(),
    );
    $form['response'] = $this->getReportFormResponse($showpoints, $showfeedback, $allow_scoring);

    $form['#theme'] = $this->getReportFormTheme($showpoints, $showfeedback);
    $form['#is_correct'] = $this->isCorrect();
    $form['#is_evaluated'] = $this->isEvaluated();
    $form['#is_skipped'] = $this->is_skipped;
    return $form;
  }

  /**
   * get the question part of the reportForm
   *
   * @param $showpoints
   * @param $showfeedback
   * @return
   *  FAPI form array holding the question
   */
  public function getReportFormQuestion($showpoints = TRUE, $showfeedback = TRUE) {
    $node = node_load($this->question->nid);
    $items = field_get_items($node, 'body');
    return field_view_value($node, 'body', $items[0]);
  }

  /**
   * Get the response part of the report form
   *
   * @param $showpoints
   * @param $showfeedback
   * @param $allow_scoring
   * @return
   *  FAPI form array holding the response part
   */
  public function getReportFormResponse($showpoints = TRUE, $showfeedback = TRUE, $allow_scoring = FALSE) {
    return array('#markup' => '');
  }

  /**
   * Get the score part of the report form
   *
   * @param $showpoints
   * @param $showfeedback
   * @param $allow_scoring
   * @return
   *  FAPI form array holding the score part
   */
  public function getReportFormScore($showpoints = TRUE, $showfeedback = TRUE, $allow_scoring = FALSE) {
    return array('#markup' => '<span class="quiz-report-score">' . $this->getScore() . '</span>');
  }


  public function getReportFormAnswerFeedback($showpoints, $showfeedback, $allow_scoring) {
    return array();
  }

  /**
   * Get the submit function for the reportForm
   *
   * @return
   *  Submit function as a string, or FALSE if no submit function
   */
  public function getReportFormSubmit($showfeedback = TRUE, $showpoints = TRUE, $allow_scoring = FALSE) {
    return FALSE;
  }

  /**
   * Get the validate function for the reportForm
   *
   * @return
   *  Validate function as a string, or FALSE if no validate function
   */
  public function getReportFormValidate($showfeedback = TRUE, $showpoints = TRUE, $allow_scoring = FALSE) {
    return FALSE;
  }

  /**
   * Get the theme key for the reportForm
   *
   * @return
   *  Theme key as a string, or FALSE if no submit function
   */
  public function getReportFormTheme($showfeedback = TRUE, $showpoints = TRUE) {
    return FALSE;
  }

  /**
   * Saves the quiz result. This is not used when a question is skipped!
   */
  public function saveResult() {
    $this->is_skipped = FALSE;
    $this->save();
  }

  /**
   * Utility function that returns the format of the node body
   */
  protected function getFormat() {
    $body = field_get_items($this->question, 'body');
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
}
