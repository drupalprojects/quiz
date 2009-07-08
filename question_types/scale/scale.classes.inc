<?php

// $Id$

/**
 * The main classes for the scale question type.
 *
 * These inherit or implement code found in quiz_question.classes.inc.
 *
 * If you are developing your own question type, the easiest place to start is with
 * quiz_question.truefalse.inc. This and long answer are good for understanding
 * question types that involve textual answers.
 *
 * @file
 */

/*
 * TODO: TESTING, TESTING, TESTING
 * TODO: Make manage presets section.
 * TODO: Make administration section.
 * TODO: Add readme file
 */

/**
 * Implementation of QuizQuestion.
 */
class ScaleQuestion implements QuizQuestion {

  /**
   * The current node for this question.
   */
  protected $node = NULL;

  /**
   * Constructor
   *
   * @param $node - the node object
   */
  public function __construct($node) {
    $this->node = $node;
  }
  /**
   * Implementation of save
   *
   * Stores the question in the database.
   *
   * @param is_new if - if the node is a new node...
   * (non-PHPdoc)
   * @see sites/all/modules/quiz-HEAD/question_types/quiz_question/QuizQuestion#save()
   */
  public function save($is_new = FALSE) {
    $is_new_node = $is_new || $this->node->revision == 1;
    $answer_collection_id = $this->saveAnswerCollection($is_new_node);
    if ($is_new_node) {
      $sql = 'INSERT INTO {quiz_scale_node_properties}
        (nid, vid, answer_collection_id)
        VALUES (%d, %d, %d)';
      db_query($sql, $this->node->nid, $this->node->vid, $answer_collection_id);
    }
    else {
      $sql = 'UPDATE {quiz_scale_node_properties}
        SET answer_collection_id = %d
        WHERE nid = %d AND vid = %d';
      db_query($sql, $answer_collection_id, $this->node->nid, $this->node->vid);
    }
  }
  /**
   * Stores the answer collection to the database, or reuses an existing collection.
   *
   * @param $is_new_node - the question is beeing inserted(not updated)
   * @return unknown_type
   */
  private function saveAnswerCollection($is_new_node) {
    global $user;
    $node_as_array = get_object_vars($this->node);
    $alternatives = array();
    // Fetching alternatives
    for ($i = 0; $i<10; $i++) {
      if (strlen($node_as_array['alternative'. $i]) > 0) {
        $alternatives[] = $node_as_array['alternative'. $i];
      }
    }
    if (!$is_new_node) {
      if ($old_collection_id = $this->processOldCollection($alternatives)) return $old_collection_id;
    }
    if ($answer_collection_id = $this->excistingCollection($alternatives)) return $answer_collection_id;
    $sql = 'INSERT INTO {quiz_scale_answer_collection}
        (preset, uid)
        VALUES ('. $this->node->save .', '. $user->uid .')';
    db_query($sql);
    $answer_collection_id = db_last_insert_id('quiz_scale_answer_collection', 'id');
    for ($i = 0; $i < count($alternatives); $i++) {
      $this->saveAlternative($alternatives[$i], $answer_collection_id);
    }
    return $answer_collection_id;
  }
  /**
   * Saves one alternative to the database
   *
   * @param $alternative - the alternative(String) to be saved.
   * @param $answer_collection_id - the id of the answer collection this alternative shall belong to.
   */
  private function saveAlternative($alternative, $answer_collection_id) {
    $sql = 'INSERT INTO {quiz_scale_answer}
        (answer_collection_id, answer)
        VALUES ('. $answer_collection_id .', \''. $alternative .'\')';
    db_query($sql);
  }
  /**
   * Finds out if the answer collection has been changed
   *
   * @param $alternatives - array of the alternatives to be saved
   * @return FALSE | answer_collection_id - returns false if collection has changed.
   */
  private function processOldCollection($alternatives) {
    /*
     * If new collection = old collection, return old collection id
     * Else, if old collection isn't used by anyone else, delete it and return false.
     */
    $old_collection_id = $this->node->{0}->answer_collection_id;
    $to_return = $old_collection_id;
    $sql = 'SELECT answer
        FROM {quiz_scale_answer}
        WHERE answer_collection_id = %d';
    $res = db_query($sql, $old_collection_id);
    $number_of_alternatives = 0;
    while ($res_o = db_fetch_object($res)) {
      $number_of_alternatives++;
      if ($number_of_alternatives > count($alternatives)) return FALSE;
      if ($res_o->answer != $alternatives[$number_of_alternatives-1]) return FALSE;
    }
    if ($number_of_alternatives != count($alternatives)) $to_return = FALSE;
    if ($to_return === FALSE) $this->deleteCollectionIfNotUsed($old_collection_id);
    return $to_return;
  }
  /**
   * Deletes an answer collection if it isn't beeing used.
   *
   * @param $answer_collection_id
   * @param $accept - if collection is used more than this many times we keep it.
   */

  private function deleteCollectionIfNotUsed($answer_collection_id, $accept=1) {
    $sql = 'SELECT uid, preset
        FROM {quiz_scale_answer_collection}
        WHERE id = %d';
    $res_o = db_fetch_object(db_query($sql, $answer_collection_id));
    if ($res_o->uid == 'NULL' || $res_o->preset == 1) return;
    $sql = 'SELECT COUNT(*)
        FROM {quiz_scale_node_properties}
        WHERE answer_collection_id = %d';
    $results = db_fetch_array(db_query($sql, $answer_collection_id));
    if ($results['COUNT(*)'] <= $accept) {
      $sql = 'DELETE FROM {quiz_scale_answer_collection}
            WHERE id = %d';
      db_query($sql, $answer_collection_id);
      $sql = 'DELETE FROM {quiz_scale_answer}
            WHERE answer_collection_id = %d';
      db_query($sql, $answer_collection_id);
    }
  }
  /**
   * Finds out if a collection allready exists.
   *
   * @param $alternatives - this is the collection that will be compared with the database.
   * @return unknown_type
   */
  private function excistingCollection($alternatives, $answer_collection_id = NULL, $id = NULL) {
    $my_alts = $alternatives;
    $sql = 'SELECT id, answer_collection_id
        FROM {quiz_scale_answer}
        WHERE answer = \''. array_pop($my_alts) .'\'';
    if (isset($answer_collection_id)) $sql .= ' AND answer_collection_id = '. $answer_collection_id;
    if (isset($id)) $sql .= ' AND id = '. ($id-1);
    $res = db_query($sql);
    if (!$res_o = db_fetch_object($res)) return FALSE;
    if (count($my_alts) == 0) return $answer_collection_id;
    $answer_collection_ids = array();
    do {
      $answer_collection_ids[] = $this->excistingCollection($my_alts, $res_o->answer_collection_id, $res_o->id);
    } while ($res_o = db_fetch_object($res));
    for ($i = 0; $i < count($answer_collection_ids); $i++) {
      if ($answer_collection_ids[$i]) return $answer_collection_ids[$i];
    }
    return FALSE;
  }
  /**
   * Implementation of validate
   *
   * (non-PHPdoc)
   * @see sites/all/modules/quiz-HEAD/question_types/quiz_question/QuizQuestion#validate()
   */
  public function validate($node, &$form) {
  }
  /**
   * Implementation of delete
   *
   * (non-PHPdoc)
   * @see sites/all/modules/quiz-HEAD/question_types/quiz_question/QuizQuestion#delete()
   */
  public function delete($only_this_version = FALSE) {
    if ($only_this_version) {
      db_query('DELETE FROM {quiz_scale_node_properties} WHERE nid = %d AND vid = %d', $this->node->nid, $this->node->vid);
      $sql = 'DELETE FROM {quiz_scale_user_answers}
            WHERE result_id IN(
              SELECT result_id
              FROM {quiz_node_results}
              WHERE nid = %d AND vid = %d
            )';
      db_query($sql, $this->node->nid, $this->node->vid);
    }
    else {
      db_query('DELETE FROM {quiz_scale_node_properties} WHERE nid = %d', $this->node->nid);
      $sql = 'DELETE FROM {quiz_scale_user_answers}
            WHERE result_id IN(
              SELECT result_id
              FROM {quiz_node_results}
              WHERE nid = %d
            )';
      db_query($sql, $this->node->nid);
    }
    $this->deleteCollectionIfNotUsed($this->node->{0}->answer_collection_id, 0);
  }

  /**
   * Implementation of load
   *
   * (non-PHPdoc)
   * @see sites/all/modules/quiz-HEAD/question_types/quiz_question/QuizQuestion#load()
   */
  public function load() {
    $to_return = array();
    $sql = 'SELECT id, answer, a.answer_collection_id
            FROM {quiz_scale_node_properties} p
            JOIN {quiz_scale_answer} a ON (p.answer_collection_id = a.answer_collection_id)
            WHERE nid = %d AND vid = %d';
    $res = db_query($sql, $this->node->nid, $this->node->vid);
    while ($res_o = db_fetch_object($res)) {
      $to_return[] = $res_o;
    }
    return $to_return;
  }
  /**
   * Implementation of view
   *
   * (non-PHPdoc)
   * @see sites/all/modules/quiz-HEAD/question_types/quiz_question/QuizQuestion#view()
   */
  public function view() {
    return $this->getQuestionForm($this->node);
  }

  /**
   * Generates the question form.
   *
   * This is called whenever a question is rendered, either
   * to an administrator or to a quiz taker.
   */

  public function getQuestionForm($node, $context = NULL) {

    $options = array();
    for ($i = 0; $i < 10; $i++) {
      if (strlen($node->{$i}->answer) > 0) {
        $options[$node->{$i}->id] = $node->{$i}->answer;
      }
    }
    $form['question'] = array(
      '#type' => 'markup',
      '#value' => $node->body
    );
    $form['tries'] = array(
      '#type' => 'radios',
      '#title' => t('Choose one'),
      '#options' => $options,
    );
    return $form;
  }
  /**
   * Implementation of getAdminForm
   *
   * (non-PHPdoc)
   * @see sites/all/modules/quiz-HEAD/question_types/quiz_question/QuizQuestion#getAdminForm()
   */
  public function getAdminForm($edit = NULL) {
    $form['empty'] = array(
      '#type' => 'markup',
      '#value' => t('There are currently no available configuration options.'),
    );
  }
  /**
   * Implementation of getCreation form
   *
   * (non-PHPdoc)
   * @see sites/all/modules/quiz-HEAD/question_types/quiz_question/QuizQuestion#getCreationForm()
   */
  public function getCreationForm($edit) {
    global $user;
    // Add JS
    $path = drupal_get_path('module', 'scale') .'/scale.js';
    drupal_add_js($path, 'module');

    /*
     * Getting presets from the database
     */
    $options = array(); //option array for select form element.
    $options['d'] = '-';
    $scale_element_names = array();
    $sql = 'SELECT answer_collection_id, answer
        FROM {quiz_scale_answer_collection} ac
        JOIN {quiz_scale_answer} a ON(ac.id = a.answer_collection_id)
        WHERE ac.preset = 1
        AND
          (ac.uid = '. $user->uid .'
          OR ac.uid IS NULL)
        ORDER BY answer_collection_id';
    $res = db_query($sql);

    $answer_collection_id;
    while (true) {
      if (!($res_o = db_fetch_object($res)) || ($res_o->answer_collection_id != $answer_collection_id)) {
        /*
         * We have gone through alle elements for one answer collection,
         * and needs to store the answer collections name and id in the options array...
         *
         * Attention: Do not remove the (count(scale_element_names)). It is beeing used by the js.
         */
        if (isset($answer_collection_id)) {
          $num_scale_elements = count($scale_element_names);
          $options[$answer_collection_id] = $scale_element_names[0] .' - '. $scale_element_names[$num_scale_elements-1] .' ('. $num_scale_elements .')';
        }
        if (!$res_o) break;
        $scale_element_names = array();
        $answer_collection_id = $res_o->answer_collection_id;
      }
      $scale_element_names[] = $res_o->answer;
      $form[$answer_collection_id .'-'. (count($scale_element_names) - 1)] = array(
        '#type' => 'hidden',
        '#title' => $answer_collection_id .'-'. (count($scale_element_names) - 1),
        '#value' => $res_o->answer,
      );
    }
    $form['answer'] = array(
      '#type' => 'fieldset',
      '#title' => t('Answer'),
      '#description' => t('Provide alternatives for the user to answer.'),
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
    );
    $form['answer']['presets'] = array(
      '#type' => 'select',
      '#title' => t('Presets'),
      '#options' => $options,
      '#default_value' => 'd',
      '#description' => t('Select a set of alternatives'),
      '#attributes' => array('onchange' => 'refreshAlternatives(this)'),
    );
    $form['answer']['alternatives'] = array(
      '#type' => 'fieldset',
      '#title' => t('Alternatives'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );
    for ($i=0; $i<10;$i++) {
      $form['answer']['alternatives']["alternative$i"] = array(
        '#type' => 'textfield',
        '#title' => t('Alternative !i', array('!i' => ($i+1))),
        '#size' => 60,
        '#maxlength' => 256,
        '#default_value' => $this->node->{$i}->answer,
      );
    }
    $form['answer']['alternatives']['save'] = array(
      '#type' => 'checkbox',
      '#title' => t('Save as a new preset'),
      '#description' => t('Current alternatives will be saved as a new preset'),
      '#default_value' => FALSE,
    );
    return $form;
  }

  public function getMaximumScore() {
    return 0;
  }
}

/**
 * The short answer question response class.
 */
class ScaleResponse extends AbstractQuizQuestionResponse {
  /**
   * ID of the answer.
   */
  protected $answer_id = 0;

  /**
   * Constructor
   *
   * @param $rid - response_id
   * @param $question - as an object
   * @param $answer - answer_id
   */
  public function __construct($rid, $question, $answer = NULL) {
    $this->rid = $rid;
    $this->question = $question;
    if (isset($answer)) $this->answer_id = $answer;
    else {
      $sql = 'SELECT answer_id
              FROM {quiz_scale_user_answers}
              WHERE result_id = %d AND question_nid = %d AND question_vid = %d';
      $res = db_query($sql,$rid, $this->question->nid, $this->question->vid);
      $res_o = db_fetch_object($res);
      $this->answer_id = $res_o->answer_id;
    }
    $sql = 'SELECT answer
            FROM {quiz_scale_answer}
            WHERE id = %d';
    $res = db_query($sql, $this->answer_id);
    $res_o = db_fetch_object($res);
    $this->answer = $res_o->answer;
  }
  /**
   * Implementation of save
   */
  public function save() {
    $sql = "INSERT INTO {quiz_scale_user_answers}
      (answer_id, result_id, question_vid, question_nid)
      VALUES (%d, %d, %d, %d)";
    db_query($sql, $this->answer_id, $this->rid, $this->question->vid, $this->question->nid);
  }
  /**
   * Implementation of delete
   */
  public function delete() {
    $sql = 'DELETE FROM {quiz_scale_user_answers}
        WHERE result_id = %d AND question_nid = %d AND question_vid = %d';
    db_query($sql, $this->rid, $this->question->nid, $this->question->vid);
  }
  /**
   * Implementation of score
   *
   * @return 0 - this question type is never scored.
   */
  public function score() {
    return 0;
  }
  /**
   * Implementation of isCorrect
   *
   * @return 0 - this question type is never scored
   * (non-PHPdoc)
   * @see sites/all/modules/quiz-HEAD/question_types/quiz_question/AbstractQuizQuestionResponse#isCorrect()
   */
  public function isCorrect() {
    return TRUE;
  }
  /**
   * Implementation of getResponse
   *
   * @return answer
   */
  public function getResponse() {
    return $this->answer;
  }
  /**
   * Implementation of format report.
   *
   * @param $showpoints
   * @param $showfeedback
   * @return report as html
   */
  public function formatReport($showpoints = TRUE, $showfeedback = TRUE) {
    $slug = '<div class="quiz_summary_question"><span class="quiz_question_bullet">Q:</span> '.
      check_markup($this->question->body) .
      '</div>';
    $result = '<div class="quiz_answer_feedback">';
    $result .= t('You answered:');
    $result .= '<BR/>';
    $result .= $this->answer;
    $result .= '</div>';

    return $slug . $result;
  }
}
