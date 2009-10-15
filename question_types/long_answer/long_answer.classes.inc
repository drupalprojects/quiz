<?php
// $Id$

/**
 * Long answer classes.
 *
 * @file
 */


/**
 * Implementation of QuizQuestion.
 */
class LongAnswerQuestion extends QuizQuestion{

  public function save($is_new = FALSE) {
    parent::save($is_new);
    if (!isset($this->node->feedback)) {
      $this->node->feedback = '';
    }
  }

  public function validateNode(array &$form) {
    // Check to make sure that the maximum score is greater-than or equal-to 0.
    $max_score = $this->node->max_score;
    if (!ctype_digit($max_score) || intval($max_score) < 0) {
      form_set_error('max_score', t('Score must be a positive integer (0 or higher).'));
    }
  }

  // This is called whenever a question is rendered, either
  // to an administrator or to a quiz taker.
  public function getAnsweringForm(array $form_state = NULL) {
    $form['question'] = array(
       '#type' => 'markup',
       '#value' => check_markup($this->node->body, $this->node->format, FALSE),
     );
     $form['tries'] = array(
       '#type' => 'textarea',
       '#title' => t('Answer'),
       '#description' => t('Enter your answer here. If you need more space, click on the grey bar at the bottom of this area and drag it down.'),
       '#rows' => 15,
       '#cols' => 60,
       '#required' => FALSE,
     );
     return $form;
  }


  public function getAdminForm(array $form_state = NULL) {
    $form['settings'] = array(
      '#type' => 'markup',
      '#value' => t('There are no settings for this question type.'),
    );
    return $form;
  }

  public function getCreationForm(array $form_state = NULL) {
    $form['max_score'] = array(
      '#type' => 'textfield',
      '#title' => t('Maximum Possible Score'),
      '#description' => t('Long answer questions are scored manually. This field indicates to the person scoring what the maximum number of points per essay is. Multichoice questions have a score of 1.'),
      '#default_value' => isset($this->node->max_score) ? $this->node->max_score : variable_get('long_answer_default_maximum_score', 1),
      '#size' => 3,
      '#maxlength' => 3,
      '#weight' => -4,
      '#required' => TRUE,
    );
    return $form;
  }

  public function getMaximumScore() {
    return $this->node->max_score;
  }

}

class LongAnswerResponse extends AbstractQuizQuestionResponse {

  /**
   * Get all quiz scores that have not yet been evaluated.
   *
   * @param $count
   *  Number of items to return (default: 50).
   * @param $offset
   *  Where in the results we should start (default: 0).
   *
   * @return
   *  Array of objects describing unanswered questions. Each object will have result_id, question_nid, and question_vid.
   */
  public static function fetchAllUnscoredAnswers($count = 50, $offset = 0) {
    $sql = 'SELECT a.result_id, a.question_nid, a.question_vid, r.title, n.time_end, n.time_start, n.uid
      FROM {quiz_long_answer_user_answers} AS a
      INNER JOIN {node_revisions} AS r ON a.question_vid = r.vid
      INNER JOIN {quiz_node_results} AS n ON a.result_id = n.result_id
      WHERE is_evaluated = 0';
    $results = db_query_range($sql, $offset, $count);
    $unscored = array();

    if ($results) {
      while ($row = db_fetch_object($results)) {
        $unscored[] = $row;
      }
    }

    return $unscored;
  }
  /**
   * Given a quiz, return a list of all of the unscored answers.
   *
   * @param $nid
   *  Node ID for the quiz to check.
   * @param $vid
   *  Version ID for the quiz to check.
   * @param $count
   *  Number of items to return (default: 50).
   * @param $offset
   *  Where in the results we should start (default: 0).
   *
   * @return
   *  Indexed array of result IDs that need to be scored.
   */
  public static function fetchUnscoredAnswersByQuestion($nid, $vid, $count = 50, $offset = 0) {
    $results = db_query_range('SELECT result_id FROM {quiz_long_answer_user_answers} WHERE is_evaluated = 0 AND question_nid = %d AND question_vid = %d', $nid, $vid, $offset, $count);
    $unscored = array();
    foreach (db_fetch_object($results) as $row) {
      $unscored[] = $row->result_id;
    }
    return $unscored;
  }

  /**
   * ID of the answer.
   */
  protected $answer_id = 0;

  public function __construct($result_id, stdClass $question_node, $answer = NULL) {
    parent::__construct($result_id, $question_node, $answer);

    if (!isset($answer)) {
      $sql = "SELECT answer_id, answer, is_evaluated, score, question_vid, question_nid, result_id
        FROM {quiz_long_answer_user_answers}
        WHERE question_nid = %d AND question_vid = %d AND result_id = %d";
      $r = db_fetch_object(db_query($sql, $question_node->nid, $question_node->vid, $result_id));

      if (!empty($r)) {
        $this->answer = $r->answer;
        $this->score = $r->score;
        $this->evaluated = $r->is_evaluated;
        $this->answer_id = $r->answer_id;
      }
    }
    else {
      $this->answer = $answer;
    }
  }

  public function save() {
    $sql = "INSERT INTO {quiz_long_answer_user_answers} (answer, question_nid, question_vid, result_id) VALUES ('%s', %d, %d, %d)";
    db_query($sql, $this->answer, $this->question->nid, $this->question->vid, $this->rid);
    $this->answer_id = db_last_insert_id('quiz_long_answer_user_answers', 'answer_id');
  }

  public function delete() {
    $sql = 'DELETE FROM {quiz_long_answer_user_answers} WHERE question_nid = %d AND question_vid = %d AND result_id = %d';
    db_query($sql, $this->question->nid, $this->question->vid, $this->rid);
  }

  public function score() {
    $sql = "SELECT score FROM {quiz_long_answer_user_answers} WHERE result_id = %d AND question_vid = %d";
    $score = (int)db_result(db_query($sql, $this->rid, $this->question->vid));
    return $score;
  }

  public function isCorrect() {
    $possible = _quiz_question_get_instance($this->question)->getMaximumScore();
    $actual = $this->getScore();
    // To prevent Division by zero warning
    $possible = ($possible == 0) ? 1 : $possible;
    return (($actual / $possible) * 100 > 50);
  }

  public function getResponse() {
    return $this->answer;
  }
  
  public function getReportForm($showpoints = TRUE, $showfeedback = TRUE) {
    $form = array();
    if ($this->question && !empty($this->question->answers)) 
      $answer = (object) current($this->question->answers);
    else
      return $form;
    if (user_access('score long answer')) {
      $form['score'] = array(
        '#type' => 'textfield',
        '#default_value' => $this->getScore(),
        '#size' => 3,
        '#maxlength' => 3,
        '#attributes' => array('class' => 'answer-scoring'),
      );
      $form['submit'] = array(
        '#type' => 'value',
        '#value' => 'long_answer_report_submit',
      );
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
    }
    else {
      $form['score'] = array(
        '#type' => 'markup',
        '#value' => $answer->score,
      );
    }
    $form['max_score'] = array(
      '#type' => 'markup',
      '#value' => $this->question->max_score,
    );
    $form['question'] = array(
      '#type' => 'markup',
      '#value' => check_markup($this->question->body, $this->question->format, FALSE),
    );
    
    $form['answer'] = array(
      '#type' => 'markup',
      '#value' => check_markup($answer->answer),
    );
    if ($answer->is_evaluated == 1) {
      // Show feedback, if any.
      if ($showfeedback && !empty($answer->feedback)) {
        $feedback = check_markup($answer->feedback);
      }
    }
    else {
      $feedback = t('This answer has not yet been scored.') .
        '<br/>' .
        t('Until the answer is scored, the total score will not be correct.');
    }
    if (!empty($feedback)) {
      $form['feedback'] = array(
        '#type' => 'markup',
        '#value' => $feedback,
      );
    }
    
    $form['#is_correct'] = $this->isCorrect();
    $form['#theme'] = 'long_answer_report'; 
    return $form;
  }
  
  public function formatReport($showfeedback = TRUE, $showpoints = TRUE) {
    return '';
  }
}

