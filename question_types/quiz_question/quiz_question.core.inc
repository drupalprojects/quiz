<?php

// $Id$

/**
 * Classes used in the Quiz Question module.
 *
 * The core of the Quiz Question module is a set of interfaces and abstract
 * classes that can be used to quickly and efficiently create new question
 * types.
 *
 * Why OO?
 * Drupal has a long history of avoiding many of the traditional OO structures
 * and metaphors. However, with PHP 5, there are many good reasons to use OO
 * principles more broadly.
 *
 * The case for Quiz question types is that question types all share common
 * structure and logic. Using the standard hook-only Drupal metaphor, we are
 * forced to copy and paste large amounts of repetitive code from question
 * type to question type. By using OO principles and construction, we can
 * easily encapsulate much of that logic, while still making it easy to
 * extend the existing content.
 *
 * Where do I start?
 * To create a new question type, check out the True/False type that is included
 * in this module.
 *
 * @file
 */


/**
 * A base implementation of a quiz_question, adding a layer of abstraction between the
 * node API, quiz API and the question types.
 *
 * It is required that Question types extend this abstract class.
 * 
 * This class has default behaviour that all question types must have. It also handles the node API, but
 * gives the question types oppurtunity to save, delete and provide data specific to the question types.
 * This is done by declaring helper functions as abstract functions instead of relying on the question
 * types to call their parent methods for all node API methods.
 * 
 * This abstract class also declares several abstract functions forcing question-types to implement required 
 * methods.
 */
abstract class QuizQuestion {
  /*
   * ABSTRACT FUNCTIONS
   * This part acts as a contract(/interface) between the question-types and the rest of the system.
   * 
   * All question types needs to implement the abstract functions.
   */
  
   /**
   * Retrieve information relevant for viewing the node.
   * 
   * (This data is generally added to the node's extra field.)
   * 
   * @return
   *  Content array
   */
  abstract public function getNodeView();
  
  /**
   * Getter function returning properties to be loaded when the node is loaded. 
   * 
   * @return array or object
   */
  abstract public function getNodeProperties();
  
  /**
   * Responsible for handling insert/update of question-specific data.
   * This is typically called from within the Node API, so there is no need
   * to save the node. This function is only responsible for saving data
   * specific to the implement ation.
   *
   * The $is_new flag is set to TRUE whenever the node is being initially
   * created.
   *
   * A save function is required to handle the following three situations:
   * - A new node is created ($is_new is TRUE)
   * - A new node *revision* is created ($is_new is NOT set, because the
   *   node itself is not new).
   * - An existing node revision is modified.
   *
   * @param $is_new
   *  TRUE when the node is initially created.
   */
  abstract public function save($is_new = FALSE);
  
  /**
   * Delete question data from the database
   * 
   * @param $only_this_version
   *   If only this version(vid) should be deleted, or all versions.
   */
  abstract public function delete($only_this_version = FALSE);
  
  /**
   * Provides validation for question before it is created.
   *
   * When a new question is created and initially submited, this is
   * called to validate that the settings are acceptible.
   *
   * @param $form
   *  The processed form.
   */
  abstract public function validateNode(array &$form);
  
  /**
   * Get the form through which the user will answer the question.
   *
   * @param $context
   *  The form context.
   * @return
   *  Must return a FAPI array.
   */
  abstract public function getAnsweringForm(array $form_state = NULL);
  
  /**
   * Get the form that contains admin settings for this question type.
   *
   * @param
   *  FAPI form state
   * @return
   *  Must return a FAPI array.
   */
  abstract public function getAdminForm(array $form_state = NULL);
  
  /**
   * Get the form used to create a new question.
   * 
   * @param
   *  FAPI form state
   * @return
   *  Must return a FAPI array.
   */
  abstract public function getCreationForm(array $form_state = NULL);
  
  /**
   * Get the maximum possible score for this question.
   */
  abstract public function getMaximumScore();
  
  
  
  
  /*
   * DEFAULT BEHAVIOR
   * Behavior default for all question types should be written below
   */
  
  /**
   * The current node for this question.
   */
  protected $node = NULL;
  
  /**
   * QuizQuestion constructor stores the node object.
   * 
   * @param $node
   *   The node object
   */
  public function __construct(stdClass $node) {
    $this->node = $node;
  }

  public function getNodeForm(array $form_state = NULL) {
    global $user;
    $form = array();
  
    $form['body'] = array(
      '#type' => 'textarea',
      '#title' => t('Question'),
      '#description' => t('Enter the full text of the question that will be shown to the user. Include any special instructions on how to answer.'),
      '#default_value' => $node->body,
      '#required' => TRUE,
      '#weight' => -15,
    );
  
    // process up the filter format
    $form['format'] = filter_form($node->format);
    $form['format']['#weight'] = -14;
  
    // First we do the basic title and text fields.
    // XXX: Should there be a way for question types to modify these (other than form alter hooks)?
  
    // Allow user to set title?
    if (user_access('allow user titles')) {
      $form['helper']['#theme'] = 'quiz_question_creation_form';
      $form['title'] = array(
        '#type' => 'textfield',
        '#title' => t('Title'),
        '#default_value' => $node->title,
        '#required' => FALSE,
        '#description' => t('Add a title that will help distinguish this question from other questions. This will not be seen during the quiz.'),
      );
    }
    else {
      $form['title'] = array(
        '#type' => 'value',
        '#value' => $node->title,
      );
    }
  
    // Collection ID used here to tie creation of a question to a specific quiz or qcollection
    $collection_id = arg(3);
    if (is_numeric($collection_id)) {
      $vid = (is_numeric(arg(4) && arg(4) > 0)) ? arg(4) : NULL;
      $collection = node_load((int)$collection_id, $vid);
      $form['collection_nid'] = array(
        '#type' => 'value',
        '#value' => $collection->nid,
      );
      $form['collection_vid'] = array(
        '#type' => 'value',
        '#value' => $collection->vid,
      );
      
      // If coming from collection view, go back there on submit.
  
      // TODO ask the node itself where to go after authoring a question
      switch ($collection->type) {
        case 'quiz':
          $form['#redirect'] = 'node/'. $collection->nid .'/questions';
          break;
        case 'qcollection':
          $form['#redirect'] = 'node/'. $collection->nid .'/items';
          break;
      }
    }
    
    //Add question specific content
    $form = array_merge($form, $this->getCreationForm($form_state));
    
    // Fieldset allowing question makers to add questions to multiple quizzes when creating quizzes
    $already = array();
    $already_nids = array();
    if (is_numeric($node->nid)) {
      $sql = 'SELECT n.nid, r.parent_vid AS vid, n.title
              FROM {quiz_node_relationship} r
              JOIN {node} n
              ON n.nid = r.parent_nid
              WHERE r.child_nid = %d AND r.child_vid = %d';
      $res = db_query($sql, $node->nid, $node->vid);
      while ($res_o = db_fetch_object($res)) {
        $already[$res_o->nid .'-'. $res_o->vid] = $res_o->title;
        $already_nids[] = $res_o->nid;
      }
    }
    $found = implode(', ', $already_nids);
    $latest = array();
    $latest_nids = array();
    $sql = "SELECT quiz_nid, n.vid, title
            FROM {quiz_question_latest_quizzes} lq
            JOIN {node} n
            ON n.nid = lq.quiz_nid
            WHERE lq.uid = %d"; 
    if (drupal_strlen($found) > 0) $sql .= " AND quiz_nid NOT IN ($found)";
    $sql .= " ORDER BY lq.id DESC";
    $res = db_query($sql, $user->uid);
    while ($res_o = db_fetch_object($res)) {
      $latest[$res_o->quiz_nid .'-'. $res_o->vid] = $res_o->title;
      $latest_nids[] = $res_o->quiz_nid;
    }
    $found = implode(', ', array_merge($already_nids, $latest_nids));
    if (count($latest) < QUIZ_QUESTION_N_O_LATEST) {
      $sql = "SELECT nid, vid, title
            FROM {node} n
            WHERE n.uid = %d AND type = 'quiz'"; 
      if (drupal_strlen($found) > 0) $sql .= " AND nid NOT IN ($found)";
      $sql .= " ORDER BY changed LIMIT %d";
      $res = db_query(db_rewrite_sql($sql), $user->uid, QUIZ_QUESTION_N_O_LATEST - count($latest));
      while ($res_o = db_fetch_object($res)) {
        $latest[$res_o->nid .'-'. $res_o->vid] = $res_o->title;
      } 
    }
    
    $form['add_directly'] = array(
      '#type' => 'fieldset',
      '#title' => t('Add to quiz'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
      '#weight' => -3,
      '#tree' => TRUE,
    );
    if (count($already) > 0) {
      $form['add_directly']['already'] = array(
        '#type' => 'checkboxes',
        '#title' => t('Quizzes this question already is a member of'),
        '#description' => t('If you uncheck any of the checkboxes this question will be removed from the corresponding quiz.'),
        '#options' => $already,
        '#default_value' => array_keys($already),
      );
    }
    if (count($latest) > 0) {
      $form['add_directly']['latest'] = array(
        '#type' => 'checkboxes',
        '#title' => t('The @latest latest quizzes this question isn\'t a member of', array('@latest' => count($latest))),
        '#description' => t('If you check any of the checkboxes this question will be added to the corresponding quiz.'),
        '#options' => $latest,
      );
    }
    $form['add_directly']['new'] = array(
      '#type' => 'textfield',
      '#title' => t('Title for new quiz'),
      '#description' => t('Write in the name of the new quiz you want to create and add this question to.'),
    );
    return $form;
  }
  
  /**
   * hook_view OO counterpart
   * 
   * Calls the abstract function getNodeView() to get question type specific data,
   * adds common data for all question types and returns the data to the node API.
   * 
   * @return array
   */
  public function viewNode() {
    $type = node_get_types('type', $this->node);
    $content['question_type'] = array(
      '#type'  => 'markup',
      '#value' => '<div class="question_type_name">' . $type->name . '</div>',
      '#weight' => -2,
    );
    $content['question'] = array(
      '#type'  => 'markup',
      '#value' => check_markup($this->node->body, $this->node->format, FALSE),
      '#weight' => -1,
    );
    $question_specific = $this->getNodeView();
    if (is_array($question_specific)) $content = array_merge($content, $question_specific);
    return $content;
  }
    
  /**
   * hook_load OO counterpart
   * 
   * Calls the abstract function getNodeProperties() to get question type specific data,
   * adds common data for all question types and returns the data to the node API.
   * 
   * @return array or object
   */
  public function loadNode() {
    $sql = 'SELECT max_score
            FROM {quiz_question_properties}
            WHERE nid = %d AND vid = %d';
    $max_score = db_result(db_query($sql, $this->node->nid, $this->node->vid));
    $to_return = $this->getNodeProperties();
    if (empty($to_return)) $to_return = array();
    if (is_array($to_return)) {
      $to_return['max_score'] = $max_score;
    } 
    elseif (is_object($to_return)) {
      $to_return->max_score = $max_score;
    }
    return $to_return;
  }
  
  /**
   * hook_update and hook_insert OO counterpart
   * 
   * Saves data common for all question types to the database and call the abstract function save()
   * to let the question-type save its specific data
   */
  public function saveNode($is_new = FALSE) {
    $is_new_node = $is_new || $this->node->revision == 1;
    if ($is_new_node) {
      $sql = 'INSERT INTO {quiz_question_properties}
              (nid, vid, max_score)
              VALUES(%d, %d, %d)';
      db_query($sql, $this->node->nid, $this->node->vid, $this->getMaximumScore());
    } else {
      $sql = 'UPDATE {quiz_question_properties}
              SET max_score = %d
              WHERE nid = %d AND vid = %d';
      db_query($sql, $this->getMaximumScore(), $this->node->nid, $this->node->vid);
    }
    $this->save($is_new);
  }
  
  /**
   * hook_delete OO counterpart
   * 
   * Deletes data common for all questiontypes and call the abstract function delete() 
   * to let the question-type delete its specific data
   *
   * @param $only_this_version
   *  If the $only_this_version flag is TRUE, then only the particular
   *  nid/vid combo should be deleted. Otherwise, all questions with the
   *  current nid can be deleted.
   */
  public function deleteNode($only_this_version = FALSE) {
    $sql = 'DELETE FROM {quiz_question_properties}
            WHERE nid = %d';
    if ($only_this_version) $sql .= ' AND vid = %d';
    db_query($sql, $this->node->nid, $this->node->vid);
    $this->delete();
  }
  
  /**
   * Finds out if a question has been answered or not...
   * 
   * @return
   *   true if there exists answers to the current question
   */
  public function hasBeenAnswered() {
    $sql = 'SELECT *
            FROM {quiz_node_results_answers}
            WHERE question_nid = %d AND question_vid = %d
            LIMIT 1';
    $res = db_query($sql, $this->node->nid, $this->node->vid);
    return db_fetch_object($res) ? TRUE : FALSE;
  }
}

/**
 * Each question type must store its own response data and be able to calculate a score for
 * that data.
 */
interface QuizQuestionResponse {

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
  public function __construct($result_id, stdClass $question_node, $tries = NULL);

  /**
   * Save the current response.
   */
  public function save();
  /**
   * Delete the response.
   */
  public function delete();
  
  /**
   * Calculate the score for the response.
   */
  public function score();
  
  /**
   * Determine whether the score is a passing score.
   */
  public function isCorrect();
  
  /**
   * Indicate whether the response has been evaluated (scored) yet.
   * Questions that require human scoring (e.g. essays) may need to manually
   * toggle this.
   */
  public function isEvaluated();
  
  /**
   * Get the user's response.
   */
  public function getResponse();
  
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
  public function getReport();

  /**
   * Return an HTML marked-up report for displaying the results of this question.
   *
   * @return
   *  An HTML string.
   */
  public function formatReport($showpoints = TRUE, $showfeedback = TRUE);

  /**
   * Get the integer score.
   */
  public function getScore();

  /**
   * Repesent the response as a stdClass object.
   *
   * Convert data to an object that has the following properties:
   * - $score
   * - $rid
   * - $nid
   * - $vid
   * - $is_correct
   */
  public function toBareObject();
}

/**
 * A base implementation of QuizQuestionResponse.
 *
 */
abstract class AbstractQuizQuestionResponse implements QuizQuestionResponse {
  protected $rid = 0;
  protected $is_correct = FALSE;

  protected $evaluated = TRUE;
  protected $question = NULL;
  protected $answer = NULL;
  protected $score;

  public function __construct($result_id, stdClass $question_node, $answer = NULL) {
    $this->rid = $result_id;
    $this->question = $question_node;
    $this->answer = $answer;
  }

  public function isEvaluated() {
    return $this->evaluated;
  }

  /**
   * Check to see if the answer is marked as correct.
   *
   * This default version returns TRUE iff the score is equal to the maximum possible score.
   */
  function isCorrect() {
    $possible = _quiz_question_get_instance($this->question)->getMaximumScore();
    $actual = $this->getScore();

    assert($action <= $possible);

    return ($possible == $actual);
  }

  function getScore() {
    if (!isset($this->score)) $this->score = $this->score();
    return $this->score;
  }

  function toBareObject() {
    $obj = new stdClass();
    $obj->score = $this->getScore(); // This can be 0 for unscored.
    $obj->nid = $this->question->nid;
    $obj->vid = $this->question->vid;
    $obj->rid = $this->rid;
    $obj->is_correct = $this->isCorrect();
    $obj->is_evaluated = $this->isEvaluated();
    $obj->is_skipped = FALSE;
    return $obj;
  }

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
}

