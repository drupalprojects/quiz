<?php


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
 * Contains Drupal\quiz_question\QuizQuestion.
 */

namespace Drupal\quiz_question;

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

  // Extra node properties
  public $nodeProperties = NULL;

  /**
   * QuizQuestion constructor stores the node object.
   *
   * @param $node
   *   The node object
   */
  public function __construct($node) {
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
  public function getNodeForm(array $form_state = NULL) {
    $user = \Drupal::currentUser();
    $form = array();
    
    // mark this form to be processed by quiz_form_alter. quiz_form_alter will among other things
    // hide the revion fieldset if the user don't have permission to controll the revisioning manually.
    $form['#quiz_check_revision_access'] = TRUE;

    // Allow user to set title?
    if ($user->hasPermission('edit question titles')) {
      $form['helper']['#theme'] = 'quiz_question_creation_form';
      $form['title'] = array(
        '#type' => 'textfield',
        '#title' => t('Title'),
        '#maxlength' => 255,
        '#default_value' => $this->node->getTitle(),
        '#required' => FALSE,
        '#description' => t('Add a title that will help distinguish this question from other questions. This will not be seen during the quiz.'),
      );
    }
    else {
      $form['title'] = array(
        '#type' => 'value',
        '#value' => $this->node->getTitle(),
      );
    }

    // Quiz id used here to tie creation of a question to a specific quiz
    if (isset($_GET['quiz_nid']) && is_numeric($_GET['quiz_nid'])) {
      $vid = (_quiz_is_int($_GET['quiz_vid'], 0)) ? $_GET['quiz_vid'] : NULL;
      $quiz = node_load((int) $_GET['quiz_nid'], $vid);

      // Store quiz id in the form
      $form['quiz_nid'] = array(
        '#type' => 'value',
        '#value' => $quiz->id(),
      );
      $form['quiz_vid'] = array(
        '#type' => 'value',
        '#value' => $quiz->getRevisionId(),
      );

      // Identify this node as a quiz question type so that it can be recognized by other modules effectively.
      $form['is_quiz_question'] = array(
        '#type' => 'value',
        '#value' => TRUE
      );

      // If coming from quiz view, go back there on submit.
      if ($quiz->getType() == 'quiz') {
        $form_state['redirect'] = 'node/' . $quiz->id() . '/questions';
        $form['#cancel_button'] = TRUE;
      }
    }

    //Add question type specific content
    $form = array_merge($form, $this->getCreationForm($form_state));

    // If access to edit quizzes we add the add to quiz fieldset
    $edit_access = quiz_access_multi_or('edit any quiz content', 'edit own quiz content', 'administer nodes');
    if ($edit_access) {
      $own_filter = quiz_access_multi_or('edit any quiz', 'administer nodes') ? '' : 'AND n.uid = ' . intval($user->id());

      // Fieldset allowing question makers to add questions to multiple quizzes when creating or editing a question
      $already = array();
      $already_nids = array();
      if ($this->node->id() && is_numeric($this->node->id())) {
        // Finding quizzes this question already belongs to.
        $sql = 'SELECT n.nid, r.parent_vid AS vid, nd.title FROM {quiz_node_relationship} r
                JOIN {node} n ON n.nid = r.parent_nid
                JOIN {node_field_data} nd ON nd.nid = r.parent_nid
                WHERE r.child_vid = :child_vid
                ORDER BY r.parent_vid DESC';
        $res = db_query($sql, array(':child_vid' => $this->node->getRevisionId()));

        // Store the results
        while ($res_o = $res->fetch()) {
          if (in_array($res_o->nid, $already_nids)) {
            continue;
          }
          // Store in simple array to use in later querries
          $already_nids[] = $res_o->nid;

          // Store in array to use as #options
          $already[$res_o->nid .'-'. $res_o->vid] = check_plain($res_o->title);
        }
      }
      $found = implode(', ', $already_nids);
      $latest = array();
      $latest_nids = array();

      // Finding the last quizzes the current user has been using
      $sql = "SELECT lq.quiz_nid, n.vid, nd.title, lq.id FROM {quiz_question_latest_quizzes} lq
              JOIN {node} n ON n.nid = lq.quiz_nid
              JOIN {node_field_data} nd ON nd.nid = lq.quiz_nid AND n.vid = nd.vid
              JOIN {quiz_node_properties} qnp ON qnp.vid = n.vid
              WHERE lq.uid = :uid AND qnp.randomization < 3";
      if (drupal_strlen($found) > 0) {
        $sql .= " AND quiz_nid NOT IN ($found)";
      }
      $sql .= " ORDER BY lq.id DESC";
      // TODO Please convert this statement to the D7 database API syntax.
      $res = db_query($sql, array(':uid' => $user->id()));

      while ($res_o = $res->fetch()) {
        // Array to use as #option in form element
        $latest[$res_o->quiz_nid .'-'. $res_o->vid] = check_plain($res_o->title);
        // Array to use in later queries
        $latest_nids[] = $res_o->quiz_nid;
      }

      if (count($latest) < QUIZ_QUESTION_NUM_LATEST) {
        // Suplementing with other available quizzes...
        $found = implode(', ', array_merge($already_nids, $latest_nids));
        $sql = "SELECT n.nid, n.vid, nd.title, changed FROM {node} n
        		JOIN {quiz_node_properties} qnp ON qnp.vid = n.vid
        		JOIN {node_field_data} nd ON nd.vid = n.vid AND n.vid = nd.vid
                WHERE nd.type = 'quiz' AND qnp.randomization < 3";
        if (drupal_strlen($found) > 0) {
          $sql .= " AND n.nid NOT IN ($found)";
        }
        //$sql .= " ORDER BY changed LIMIT %d";
        // TODO Please convert this statement to the D7 database API syntax.
        //$res = db_query(db_rewrite_sql($sql), QUIZ_QUESTION_NUM_LATEST - count($latest));
        $res = db_query($sql);
        while ($res_o = $res->fetch()) {
          // array to be used as #options in form element
          $latest[$res_o->nid .'-'. $res_o->vid] = check_plain($res_o->title);
        }
      }

      // If we came from the manage questions tab we need to mark the quiz we came from as selected.
      $latest_default = array();
      if (isset($quiz)) {
        foreach ($latest as $key => $value) {
          $latest_nid = preg_match('/^[0-9]+/', $key);
          if ($latest_nid == $quiz->id()) {
            unset($latest[$key]);
            break;
          }
        }
        
        $res = db_query('SELECT title FROM {node_field_revision} WHERE vid = :vid', array(':vid' => $quiz->getRevisionId()));
        $latest[$quiz->id() . '-' . $quiz->getRevisionId()] = check_plain($res->fetchField());
        // $latest_default is to be used as #default_value in form item
        $latest_default[] = $quiz->id() . '-' . $quiz->getRevisionId();
      }
    }
    if ($edit_access || quiz_access_multi_or('create quiz content', 'administer nodes')) {
      $form['add_directly'] = array(
        '#type' => 'fieldset',
        '#title' => t('Add to @quiz', array('@quiz' => QUIZ_NAME)),
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
        '#weight' => -3,
        '#tree' => TRUE,
        '#group' => 'additional_settings',
      );
    }
    if ($edit_access) {
      if (count($already) > 0) {
        $form['add_directly']['already'] = array(
          '#type' => 'checkboxes',
          '#title' => t('This question is already a member of'),
          '#description' => t('If you uncheck any of the checkboxes this question will be removed from the corresponding @quiz. If the @quiz has been answered a new revision of the @quiz will be created automatically.', array('@quiz' => QUIZ_NAME)),
          '#options' => $already,
          '#default_value' => array_keys($already),
        );
      }
      if (count($latest) > 0) {
        $form['add_directly']['latest'] = array(
          '#type' => 'checkboxes',
          '#title' => t('The @latest latest @quiz nodes this question isn\'t a member of', array('@latest' => count($latest), '@quiz' => QUIZ_NAME)),
          '#description' => t('If you check any of the checkboxes this question will be added to the corresponding @quiz. If the @quiz has been answered a new revision will be created automatically.', array('@quiz' => QUIZ_NAME)),
          '#options' => $latest,
          '#default_value' => $latest_default,
        );
      }
    }
    if (quiz_access_multi_or('create quiz content', 'administer nodes')) {
      $form['add_directly']['new'] = array(
        '#type' => 'textfield',
        '#title' => t('Title for new @quiz', array('@quiz' => QUIZ_NAME)),
        '#description' => t('Write in the name of the new @quiz you want to create and add this question to.', array('@quiz' => QUIZ_NAME)),
      );
    }
    if ($this->hasBeenAnswered()) {
      $log = t('The current revision has been answered. We create a new revision so that the reports from the existing answers stays correct.');
      $this->node->revision = 1;
      $this->node->log = $log;
    }
    return $form;
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
    $type = node_type_load($this->node->getType());
    $content['question_type'] = array(
      '#markup' => '<div class="question_type_name">' . $type->name . '</div>',
      '#weight' => -2,
    );
    /*
    $question_body = field_get_items('node', $this->node, 'body');
    $content['question'] = array(
      '#markup' => '<div class="question-body">' . $question_body[0]['safe_value'] . '</div>',
      '#weight' => -1,
    );
    */
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

    //TODO: Find right way.
    if ($this->node instanceof \stdClass) {
      $params = array(':nid' => $this->node->nid, ':vid' => $this->node->vid);
    }
    else {
      $params = array(':nid' => $this->node->id(), ':vid' => $this->node->getRevisionId());
    }

    $sql = 'SELECT max_score
            FROM {quiz_question_properties}
            WHERE nid = %d AND vid = %d';
    $props['max_score'] = db_query('SELECT max_score
            FROM {quiz_question_properties}
            WHERE nid = :nid AND vid = :vid', $params)->fetchField();
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

    $is_new_node = $is_new || $this->node->isNewRevision() == 1;

    // Save general data
    if ($is_new_node) {
      db_query('INSERT INTO {quiz_question_properties} (nid, vid, max_score) VALUES(:nid, :vid, :max_score)', array(':nid' => $this->node->id(), ':vid' => $this->node->getRevisionId(), ':max_score' => $this->getMaximumScore()));
    }
    else {
      db_update('quiz_question_properties')
        ->fields(array(
          'max_score' => $this->getMaximumScore(),
        ))
        ->condition('nid', $this->node->id())
        ->condition('vid', $this->node->getRevisionId())
        ->execute();
    }

    // Save what quizzes this question belongs to.
    $quizzes_kept = $this->saveRelationships();
    if ($quizzes_kept && $this->node->isNewRevision()) {
      if (user_access('manual quiz revisioning') && !\Drupal::config('quiz_question.settings')->get('quiz_auto_revisioning')) {
        unset($_GET['destination']);
        unset($_REQUEST['edit']['destination']);
        drupal_goto('quiz_question/' . $this->node->id() . '/' . $this->node->getRevisionId() . '/revision_actions');
      }
      // For users without the 'manual quiz revisioning' permission we submit the revision_actions form
      // silently with its default values set.
      else {
        $form_state = array();
        $form_state['values']['op'] = t('Submit');
        require_once DRUPAL_ROOT . '/' . drupal_get_path('module', 'quiz_question') . '/quiz_question.pages.inc';
        drupal_form_submit('quiz_question_revision_actions', $form_state, $this->node->id(), $this->node->getRevisionId());
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
      ->condition('question_nid', $this->node->id());
    if ($only_this_version) {
      $delete->condition('question_vid', $this->node->getRevisionId());
    }
    $delete->execute();

    // Delete properties
    $delete = db_delete('quiz_question_properties')
      ->condition('nid', $this->node->id());
    if ($only_this_version) {
      $delete->condition('vid', $this->node->getRevisionId());
    }
    $delete->execute();
  }

  /**
   * Provides validation for question before it is created.
   *
   * When a new question is created and initially submited, this is
   * called to validate that the settings are acceptible.
   *
   * @param $form_state
   *  The processed $form_state.
   */
  abstract public function validateNode(array &$form_state);

  /**
   * Get the form through which the user will answer the question.
   *
   * @param $form_state
   *  The FAPI form_state array
   * @param $rid
   *  The result id.
   * @return
   *  Must return a FAPI array. At the moment all form elements that takes
   *  user response must have a key named "tries". (This is a Quiz 3.x legacy AFAIK. I'm
   *  not thrilled about it...)
   */
  public function getAnsweringForm(array $form_state = NULL, $rid) {
    //echo $this->node->getType();exit;
    $form = array();
    $form['question_nid'] = array(
      '#type' => 'hidden',
      '#value' => $this->node->id(),
    );
    $body = $this->node->{'body'}->getValue();

    $form['question'] = array(
      '#markup' => check_markup($body[0]['value'], $body[0]['format']),
      '#prefix' => '<div class="quiz-question-body">',
      '#suffix' => '</div>',
    );
    return $form;
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
   * Handle the add to quiz part of the quiz_question_form
   *
   * @return
   *  TRUE if at least one of the questions were kept.
   *  FALSE otherwise
   */
  function saveRelationships() {
    $quizzes_kept = FALSE;
    unset($_SESSION['quiz_question_kept']);
    /*
     * If the question already is part of quizzes we might have to remove some
     * relationships
     */
    $quizzes_to_update = array();
    if (isset($this->node->add_directly['already']) && is_array($this->node->add_directly['already'])) {
      foreach ($this->node->add_directly['already'] as $key => $checked) {
        if ($checked == 0) {
          $nid_vid = explode('-', $key);
          $dummy_node = new \stdClass();
          $dummy_node->nid = $nid_vid[0];
          $dummy_node->vid = $nid_vid[1];
          if (quiz_has_been_answered($dummy_node)) {
            // We need to revise the quiz node if it has been answered
            $temp_quiz_node = node_load($dummy_node->nid, $dummy_node->vid);
            $temp_quiz_node->revision = 1;
            $temp_quiz_node->auto_created = TRUE;
            $temp_quiz_node->save();
            $nid_vid[1] = $temp_quiz_node->getRevisionId();
            drupal_set_message(t('New revision has been created for the @quiz %n', array('%n' => $temp_quiz_node->getTitle(), '@quiz' => QUIZ_NAME)));
          }
          $quizzes_to_update[] = $nid_vid[1];
          db_delete('quiz_node_relationship')
            ->condition('parent_nid', $nid_vid[0])
            ->condition('parent_vid', $nid_vid[1])
            ->condition('child_nid', $this->node->id())
            ->condition('child_vid', $this->node->getRevisionId())
            ->execute();
        }
        else {
          $quizzes_kept = TRUE;
          $_SESSION['quiz_question_kept'][] = $key;
        }
      }
    }

    /*
     * The quiz question might have been added to new quizzes
     */
    if (isset($this->node->add_directly['latest']) && is_array($this->node->add_directly['latest'])) {
      $to_insert = 'VALUES';
      $insert_values = array();
      foreach ($this->node->add_directly['latest'] as $nid => $checked) {
        if ($checked != 0) {
          $nid_vid = explode('-', $checked);
          $dummy_node = new \stdClass();
          $dummy_node->nid = $nid_vid[0];
          $dummy_node->vid = $nid_vid[1];
          if (quiz_has_been_answered($dummy_node)) {
            $temp_quiz_node = node_load($dummy_node->nid, $dummy_node->vid);
            $temp_quiz_node->revision = 1;
            $temp_quiz_node->auto_created = TRUE;
            $temp_quiz_node->save();
            $nid_vid[1] = $temp_quiz_node->getRevisionId();
            drupal_set_message(t('New revision has been created for the @quiz %n', array('%n' => $temp_quiz_node->getTitle(), '@quiz' => QUIZ_NAME)));
          }
          $quizzes_to_update[] = $nid_vid[1];
          $insert_values[$nid]['parent_nid'] = $nid_vid[0];
          $insert_values[$nid]['parent_vid'] = $nid_vid[1];
          $insert_values[$nid]['child_nid'] = $this->node->id();
          $insert_values[$nid]['child_vid'] = $this->node->getRevisionId();
          $insert_values[$nid]['max_score'] = $this->getMaximumScore();
          $insert_values[$nid]['auto_update_max_score'] = $this->autoUpdateMaxScore() ? 1 : 0;
          $insert_values[$nid]['weight'] = 1 + db_query('SELECT MAX(weight) FROM {quiz_node_relationship} WHERE parent_vid = :vid', array(':vid' => $nid_vid[1]))->fetchField();
          $randomization = db_query('SELECT randomization FROM {quiz_node_properties} WHERE nid = :nid AND vid = :vid', array(':nid' => $nid_vid[0], ':vid' => $nid_vid[1]))->fetchField();
          $insert_values[$nid]['question_status'] = $randomization == 2 ? QUESTION_RANDOM : QUESTION_ALWAYS;

          $delete_values[] = array('vid' => $nid_vid[1], 'nid' => $this->node->id());
        }
      }
      if (count($insert_values) > 0) {
        foreach ($delete_values as $delete_value) {
          $delete_qnr = db_delete('quiz_node_relationship');
          $delete_qnr->condition('parent_vid', $delete_value['vid']);
          $delete_qnr->condition('child_nid', $delete_value['nid']);
          $delete_qnr->execute();
        }
        $insert_qnr = db_insert('quiz_node_relationship');
        $insert_qnr->fields(array('parent_nid', 'parent_vid', 'child_nid', 'child_vid', 'max_score', 'weight', 'question_status', 'auto_update_max_score'));
        foreach ($insert_values as $insert_value) {
          $insert_qnr->values($insert_value);
        }
        $insert_qnr->execute();
      }
    }
    if (drupal_strlen($this->node->add_directly['new']) > 0) {
      $new_node = quiz_make_new($this->node->add_directly['new']);
      
      $id = db_insert('quiz_node_relationship')
      ->fields(array(
        'parent_nid' => $new_node->id(),
        'parent_vid' => $new_node->getRevisionId(),
        'child_nid' => $this->node->id(),
        'child_vid' => $this->node->getRevisionId(),
        'max_score' => $this->getMaximumScore(),
        'auto_update_max_score' => $this->autoUpdateMaxScore() ? 1 : 0,
      ))
      ->execute();
      $quizzes_to_update[] = $new_node->getRevisionId();
    }
    
    // Update max_score for relationships if auto update max score is enabled
    // for question
    if($this->node->id()) {
      $result = db_query(
          'SELECT parent_vid as vid from {quiz_node_relationship} where child_nid = :nid and child_vid = :vid and auto_update_max_score=1', 
          array(':nid' => $this->node->id(), ':vid' => $this->node->getRevisionId()));
      foreach ($result as $record) {
        $quizzes_to_update[] = $record->vid;
      }
      
      db_update('quiz_node_relationship')
        ->fields(array('max_score' => $this->getMaximumScore()))
        ->condition('child_nid', $this->node->id())
        ->condition('child_vid', $this->node->getRevisionId())
        ->condition('auto_update_max_score', 1)
        ->execute();
    }
    
    quiz_update_max_score_properties($quizzes_to_update);
    return $quizzes_kept;
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
    if (!$this->node->getRevisionId()) {
      return FALSE;
    }
    $answered = db_query_range('SELECT 1 FROM {quiz_node_results} qnres
            JOIN {quiz_node_relationship} qnrel ON (qnres.vid = qnrel.parent_vid)
            WHERE qnrel.child_vid = :child_vid', 0, 1, array(':child_vid' => $this->node->getRevisionId()))->fetch();
    return $answered ? TRUE : FALSE;
  }

  /**
   * Determines if the user can view the correct answers
   *
   * @return boolean
   *   true iff the view may include the correct answers to the question
   */
  public function viewCanRevealCorrect() {
    $user = \Drupal::currentUser();
    // permission overrides the hook
    if (user_access('view any quiz question correct response')
     || $user->id() == $this->node->getAuthorId()) {
      return TRUE;
    }
    $results = module_invoke_all('answers_access', $this->node);
    $may_view_answers = in_array(TRUE, $results);
    return $may_view_answers;
  }

  /**
   * Utility function that returns the format of the node body
   */
  protected function getFormat() {
    $body = field_get_items($this->node, 'body');
    return ($body ? $body[0]['format'] : NULL);
  }
  
  /**
   * This may be overridden in subclasses. If it returns true,
   * it means the max_score is updated for all occurrences of 
   * this question in quizzes.  
   */
  protected function autoUpdateMaxScore() {
    return false;
  }
}
