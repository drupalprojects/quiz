<?php

namespace Drupal\quiz_question;

use stdClass;

/**
 * Classes used in the Quiz Question module.
 *
 * The core of the Quiz Question module is a set of abstract classes that
 * can be used to quickly and efficiently create new question types.
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
 * To create a new question type, check out the multichoice question type for instance.
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
 *
 * This abstract class also declares several abstract functions forcing question-types to implement required
 * methods.
 */
abstract class QuizQuestion {
  /*
   * QUESTION IMPLEMENTATION FUNCTIONS
   *
   * This part acts as a contract(/interface) between the question-types and the rest of the system.
   *
   * Question types are made by extending these generic methods and abstract methods.
   */

  /**
   * The current node for this question.
   */
  public $node = NULL;

  /**
   * Extra node properties
   */
  public $nodeProperties = NULL;

  /**
   * QuizQuestion constructor stores the node object.
   *
   * @param $node
   *   The node object
   */
  public function __construct(stdClass &$node) {
    $this->node = $node;
  }

  /**
   * Allow question types to override the body field title
   *
   * @return
   *  The title for the body field
   */
  public function getBodyFieldTitle() {
    return t('Question');
  }

  /**
   * Returns a node form to quiz_question_form
   *
   * Adds default form elements, and fetches question type specific elements from their
   * implementation of getCreationForm
   *
   * @param array $form_state
   * @return unknown_type
   */
  public function getNodeForm(array &$form_state = NULL) {
    global $user;
    $form = array();

    // mark this form to be processed by quiz_form_alter. quiz_form_alter will among other things
    // hide the revion fieldset if the user don't have permission to controll the revisioning manually.
    $form['#quiz_check_revision_access'] = TRUE;

    // Allow user to set title?
    if (user_access('edit question titles')) {
      $this->includeAutoTitleScript();

      $form['title'] = array(
        '#type'          => 'textfield',
        '#title'         => t('Title'),
        '#maxlength'     => 255,
        '#default_value' => $this->node->title,
        '#required'      => FALSE,
        '#description'   => t('Add a title that will help distinguish this question from other questions. This will not be seen during the quiz.'),
      );
    }
    else {
      $form['title'] = array('#type' => 'value', '#value' => $this->node->title);
    }

    // Store quiz id in the form
    $form['quiz_nid'] = array('#type' => 'hidden');
    $form['quiz_vid'] = array('#type' => 'hidden');

    if (isset($_GET['quiz_nid']) && isset($_GET['quiz_vid'])) {
      $form['quiz_nid']['#value'] = intval($_GET['quiz_nid']);
      $form['quiz_vid']['#value'] = intval($_GET['quiz_vid']);
    }

    // Identify this node as a quiz question type so that it can be recognized by other modules effectively.
    $form['is_quiz_question'] = array(
      '#type'  => 'value',
      '#value' => TRUE
    );

    //Add question type specific content
    $form = array_merge($form, $this->getCreationForm($form_state));

    if ($this->hasBeenAnswered()) {
      $log = t('The current revision has been answered. We create a new revision so that the reports from the existing answers stays correct.');
      $this->node->revision = 1;
      $this->node->log = $log;
    }
    return $form;
  }

  /**
   * Adds inline js to automatically set the question's node title.
   */
  private function includeAutoTitleScript() {
    $max_length = variable_get('quiz_autotitle_length', 50);
    drupal_add_js(array('quiz_max_length' => $max_length), array('type' => 'setting'));
    drupal_add_js(drupal_get_path('module', 'quiz') . '/js/quiz.auto-title.js');
  }

  /**
   * Retrieve information relevant for viewing the node.
   *
   * (This data is generally added to the node's extra field.)
   *
   * @return
   *  Content array
   */
  public function getNodeView() {
    $type = node_type_get_type($this->node);
    $content['question_type'] = array(
      '#markup' => '<div class="question_type_name">' . $type->name . '</div>',
      '#weight' => -2,
    );
    return $content;
  }

  /**
   * Getter function returning properties to be loaded when the node is loaded.
   *
   * @see load hook in quiz_question.module (quiz_question_load)
   *
   * @return array
   */
  public function getNodeProperties() {
    if (isset($this->nodeProperties)) {
      return $this->nodeProperties;
    }
    $props['max_score'] = db_query('SELECT max_score
            FROM {quiz_question_properties}
            WHERE nid = :nid AND vid = :vid', array(':nid' => $this->node->nid, ':vid' => $this->node->vid))->fetchField();
    $props['is_quiz_question'] = TRUE;
    $this->nodeProperties = $props;
    return $props;
  }

  /**
   * Responsible for handling insert/update of question-specific data.
   * This is typically called from within the Node API, so there is no need
   * to save the node.
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
   * @see hook_update and hook_insert in quiz_question.module
   *
   * @param $is_new
   *  TRUE when the node is initially created.
   */
  public function save($is_new = FALSE) {
    // We call the abstract function saveNodeProperties to save type specific data
    $this->saveNodeProperties($is_new);

    db_merge('quiz_question_properties')
      ->key(array(
        'nid' => $this->node->nid,
        'vid' => $this->node->vid,
      ))
      ->fields(array(
        'nid'       => $this->node->nid,
        'vid'       => $this->node->vid,
        'max_score' => $this->getMaximumScore(),
      ))
      ->execute();

    // Save what quizzes this question belongs to.
    $quizzes_kept = $this->saveRelationships();
    if ($quizzes_kept && $this->node->revision) {
      if (user_access('manual quiz revisioning') && !variable_get('quiz_auto_revisioning', 1)) {
        unset($_GET['destination']);
        unset($_REQUEST['edit']['destination']);
        drupal_goto('quiz_question/' . $this->node->nid . '/' . $this->node->vid . '/revision_actions');
      }
      // For users without the 'manual quiz revisioning' permission we submit the revision_actions form
      // silently with its default values set.
      else {
        $form_state = array();
        $form_state['values']['op'] = t('Submit');
        require_once DRUPAL_ROOT . '/' . drupal_get_path('module', 'quiz_question') . '/quiz_question.pages.inc';
        drupal_form_submit('quiz_question_revision_actions', $form_state, $this->node->nid, $this->node->vid);
      }
    }
  }

  /**
   * Delete question data from the database.
   *
   * Called by quiz_question_delete (hook_delete).
   * Child classes must call super
   *
   * @param $only_this_version
   *  If the $only_this_version flag is TRUE, then only the particular
   *  nid/vid combo should be deleted. Otherwise, all questions with the
   *  current nid can be deleted.
   */
  public function delete($only_this_version = FALSE) {
    // Delete answeres
    $delete = db_delete('quiz_node_results_answers')
      ->condition('question_nid', $this->node->nid);
    if ($only_this_version) {
      $delete->condition('question_vid', $this->node->vid);
    }
    $delete->execute();

    // Delete properties
    $delete = db_delete('quiz_question_properties')
      ->condition('nid', $this->node->nid);
    if ($only_this_version) {
      $delete->condition('vid', $this->node->vid);
    }
    $delete->execute();
  }

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
   * @param $form_state
   *  The FAPI form_state array
   * @param $result_id
   *  The result id.
   * @return
   *  Must return a FAPI array.
   */
  public function getAnsweringForm(array $form_state = NULL, $result_id) {
    $form = array();
    $form['#element_validate'] = array(array($this, 'elementValidate'));
    return $form;
  }

  /**
   * Element validator (for repeat until correct).
   */
  public function elementValidate(&$element, &$form_state) {
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

  /**
   * Get the form used to create a new question.
   *
   * @param
   *  FAPI form state
   * @return
   *  Must return a FAPI array.
   */
  abstract public function getCreationForm(array &$form_state = NULL);

  /**
   * Get the maximum possible score for this question.
   */
  abstract public function getMaximumScore();

  /**
   * Save question type specific node properties
   */
  abstract public function saveNodeProperties($is_new = FALSE);

  /**
   * Save this Question to the specified Quiz.
   */
  function saveRelationships() {
    if (!empty($this->node->quiz_nid) && !empty($this->node->quiz_vid)) {
      $quiz_node = node_load($this->node->quiz_nid, $this->node->quiz_vid);
      $nid_vid[0] = $quiz_node->nid;
      $nid_vid[1] = $quiz_node->vid;

      if (quiz_has_been_answered($quiz_node)) {
        // We need to revise the quiz node if it has been answered
        $quiz_node->revision = 1;
        $quiz_node->auto_created = TRUE;
        node_save($quiz_node);
        $nid_vid[0] = $quiz_node->nid;
        $nid_vid[1] = $quiz_node->vid;
        drupal_set_message(t('New revision has been created for the @quiz %n', array('%n' => $quiz_node->title, '@quiz' => QUIZ_NAME)));
      }

      $nid = $this->node->nid;

      $insert_values[$nid]['quiz_qid'] = $quiz_node->nid;
      $insert_values[$nid]['quiz_vid'] = $quiz_node->vid;
      $insert_values[$nid]['child_nid'] = $this->node->nid;
      $insert_values[$nid]['child_vid'] = $this->node->vid;
      $insert_values[$nid]['max_score'] = $this->getMaximumScore();
      $insert_values[$nid]['auto_update_max_score'] = $this->autoUpdateMaxScore() ? 1 : 0;
      $insert_values[$nid]['weight'] = 1 + db_query('SELECT MAX(weight) FROM {quiz_relationship} WHERE quiz_vid = :vid', array(':vid' => $nid_vid[1]))->fetchField();
      $randomization = db_query('SELECT randomization FROM {quiz_node_properties} WHERE nid = :nid AND vid = :vid', array(':nid' => $nid_vid[0], ':vid' => $nid_vid[1]))->fetchField();
      $insert_values[$nid]['question_status'] = $randomization == 2 ? QUESTION_RANDOM : QUESTION_ALWAYS;

      $insert_qnr = db_insert('quiz_relationship');
      $insert_qnr->fields(array('quiz_qid', 'quiz_vid', 'child_nid', 'child_vid', 'max_score', 'weight', 'question_status', 'auto_update_max_score'));
      foreach ($insert_values as $insert_value) {
        $insert_qnr->values($insert_value);
      }
      $insert_qnr->execute();

      // Update max_score for relationships if auto update max score is enabled
      // for question
      $quizzes_to_update = array();
      $result = db_query(
        'SELECT quiz_vid as vid from {quiz_relationship} where child_nid = :nid and child_vid = :vid and auto_update_max_score=1', array(':nid' => $this->node->nid, ':vid' => $this->node->vid));
      foreach ($result as $record) {
        $quizzes_to_update[] = $record->vid;
      }

      db_update('quiz_relationship')
        ->fields(array('max_score' => $this->getMaximumScore()))
        ->condition('child_nid', $this->node->nid)
        ->condition('child_vid', $this->node->vid)
        ->condition('auto_update_max_score', 1)
        ->execute();

      quiz_update_max_score_properties($quizzes_to_update);
      quiz_update_max_score_properties(array($quiz_node->vid));
    }
  }

  /**
   * Finds out if a question has been answered or not
   *
   * This function also returns TRUE if a quiz that this question belongs to have been answered.
   * Even if the question itself haven't been answered. This is because the question might have
   * been rendered and a user is about to answer it...
   *
   * @return
   *   true if question has been answered or is about to be answered...
   */
  public function hasBeenAnswered() {
    if (!isset($this->node->vid)) {
      return FALSE;
    }
    $answered = db_query_range('SELECT 1 FROM {quiz_node_results} qnres
            JOIN {quiz_relationship} qnrel ON (qnres.vid = qnrel.quiz_vid)
            WHERE qnrel.child_vid = :child_vid', 0, 1, array(':child_vid' => $this->node->vid))->fetch();
    return $answered ? TRUE : FALSE;
  }

  /**
   * Determines if the user can view the correct answers
   *
   * @todo grabbing the node context here probably isn't a great idea
   *
   * @return boolean
   *   true iff the view may include the correct answers to the question
   */
  public function viewCanRevealCorrect() {
    global $user;
    $quiz_node = node_load(arg(1));

    $reveal_correct[] = user_access('view any quiz question correct response');
    $reveal_correct[] = ($user->uid == $this->node->uid);
    if (array_filter($reveal_correct)) {
      return TRUE;
    }
  }

  /**
   * Utility function that returns the format of the node body
   */
  protected function getFormat() {
    $node = isset($this->node) ? $this->node : $this->question;
    $body = field_get_items('node', $node, 'body');
    return isset($body[0]['format']) ? $body[0]['format'] : NULL;
  }

  /**
   * This may be overridden in subclasses. If it returns true,
   * it means the max_score is updated for all occurrences of
   * this question in quizzes.
   */
  protected function autoUpdateMaxScore() {
    return false;
  }

  public function getAnsweringFormValidate(array &$form, array &$form_state = NULL) {

  }

  /**
   * Is this question graded?
   *
   * Questions like Quiz Directions, Quiz Page, and Scale are not.
   *
   * By default, questions are expected to be gradeable
   *
   * @return bool
   */
  public function isGraded() {
    return TRUE;
  }

}
