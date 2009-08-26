<?php

// $Id$

/**
 * The main classes for the multichoice question type.
 *
 * These inherit or implement code found in quiz_question.classes.inc.
 *
 * Sponsored by: Norwegian Centre for Telemedicine
 * Code: falcon
 *
 * Based on:
 * Other question types in the quiz framework.
 *
 *
 *
 * @file
 * Question type, enabling the creation of multiple choice and multiple answer questions.
 */

/**
 * TODO: Refactor superLoad
 * TODO: Add message to enable truefalse if needed, or do it automatically...
 * TODO: Make it easier to add questions to a quiz
 *   TODO: Make a question browser
 *   TODO: Fix how you can add newly created questions directly via manage questions page
 *   TODO: List the users newest questions in manage questions somehow
 *   TODO: Make it possible to add questions to multiple quizzes from the question creation form 
 * TODO: Consider changing the way the user settings are remembered. If a question is deleted
 * the current way might result in serious errors.
 * TODO: There were error messages when updating from 3.x to 4.x.
 * @todo: Make a simplified alternative way to view the creation form?
 */

/**
 * Implementation of QuizQuestion.
 */
class MultichoiceQuestion extends AbstractQuizQuestion implements QuizQuestionAbstract {

  private function forgive() {
    if ($this->node->choice_multi == 1) {
      for ($i = 0; is_array($this->node->alternatives[$i]); $i++) {
        $short = &$this->node->alternatives[$i];
        if ($short['score_if_chosen'] == $short['score_if_not_chosen'] 
        || !is_numeric($short['score_if_chosen']) 
        || !is_numeric($short['score_if_not_chosen'])) {
          if ($short['correct'] == 1) {
            $short['score_if_chosen'] = 1;
            $short['score_if_not_chosen'] = 0;
          } else {

            $short['score_if_chosen'] = 0;
            $short['score_if_not_chosen'] = 1;
          }
        }
      }
    }
    else {
      for ($i = 0;is_array($this->node->alternatives[$i]); $i++) {
        $short = $this->node->alternatives[$i];
        $short['score_if_chosen'] = $short['correct'];
        $short['score_if_not_chosen'] = 0;
      }
    }
  }

  /**
   * Run check_markup() on the field of the specified choice alternative
   * @param $alternativeIndex
   * @param $field
   * @return HTML markup
   */
  private function checkMarkup($alternativeIndex, $field, $check_user_access = TRUE) {
    $alternative = $this->node->alternatives[$alternativeIndex];
    return check_markup($alternative[$field], $alternative[$field.'_format'], $check_user_access);
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
    parent::save($is_new);
    $is_new_node = $is_new || $this->node->revision == 1;
    $this->forgive();
    if ($is_new_node) {
      $sql = 'INSERT INTO {quiz_multichoice_properties}
        (nid, vid, choice_multi, choice_random, choice_boolean)
        VALUES (%d, %d, %d, %d, %d)';
      db_query($sql, $this->node->nid, $this->node->vid, $this->node->choice_multi, $this->node->choice_random, $this->node->choice_boolean);
      for ($i = 0; is_array($this->node->alternatives[$i]); $i++) {
        if (strlen($this->checkMarkup($i, 'answer')) > 0) {
          $this->insertAlternative($i);
        }
      }
    }
    else {
      $sql = 'UPDATE {quiz_multichoice_properties}
        SET choice_multi = %d, choice_random = %d, choice_boolean = %d
        WHERE nid = %d AND vid = %d';
      db_query($sql, $this->node->choice_multi, $this->node->choice_random, $this->node->choice_boolean, $this->node->nid, $this->node->vid);
      $sql = 'SELECT id FROM {quiz_multichoice_answers}
              WHERE question_nid = %d AND question_vid = %d';
      $res = db_query($sql, $this->node->nid, $this->node->vid);
      $ids_to_delete = array();
      while($res_o = db_fetch_object($res)) {
        $ids_to_delete[] = $res_o->id;
      }
      for ($i = 0; is_array($this->node->alternatives[$i]); $i++) {
        $short = $this->node->alternatives[$i];
        if (drupal_strlen($this->checkMarkup($i, 'answer')) > 0) {
          if (!is_numeric($short['id'])) $this->insertAlternative($i);
          else {
            $this->updateAlternative($i);
            $key = array_search($short['id'], $ids_to_delete);
            $ids_to_delete[$key] = FALSE;
          }
        }
      }
      foreach($ids_to_delete as $id_to_delete) {
        if ($id_to_delete) {
          db_query('DELETE FROM {quiz_multichoice_answers} WHERE id = %d', $id_to_delete);
        }
      }
    }
    $this->saveUserSettings();
  }

  private function insertAlternative($i) {
    $sql = 'INSERT INTO {quiz_multichoice_answers}
            (answer, answer_format, feedback_if_chosen, feedback_if_chosen_format,
            feedback_if_not_chosen, feedback_if_not_chosen_format, score_if_chosen,
            score_if_not_chosen, question_nid, question_vid)
            VALUES(\'%s\', %d, \'%s\', %d, \'%s\', %d, %d, %d, %d, %d)';
    $short = $this->node->alternatives[$i];
    db_query($sql, $short['answer'], $short['answer_format'], $short['feedback_if_chosen'],
    $short['feedback_if_chosen_format'], $short['feedback_if_not_chosen'],
    $short['feedback_if_not_chosen_format'], $short['score_if_chosen'],
    $short['score_if_not_chosen'], $this->node->nid, $this->node->vid);
  }

  private function updateAlternative($i) {
    $sql = 'UPDATE {quiz_multichoice_answers}
            SET answer = \'%s\', answer_format = %d, feedback_if_chosen = \'%s\',
            feedback_if_chosen_format = %d, feedback_if_not_chosen = \'%s\',
            feedback_if_not_chosen_format = %d, score_if_chosen = %d, score_if_not_chosen = %d
            WHERE id = %d AND question_nid = %d AND question_vid = %d';
    $short = $this->node->alternatives[$i];
    db_query($sql, $short['answer'], $short['answer_format'], $short['feedback_if_chosen'],
    $short['feedback_if_chosen_format'], $short['feedback_if_not_chosen'],
    $short['feedback_if_not_chosen_format'], $short['score_if_chosen'],
    $short['score_if_not_chosen'], $short['id'], $this->node->nid, $this->node->vid);
  }

  /**
   * Implementation of validate
   *
   * (non-PHPdoc)
   * @see sites/all/modules/quiz-HEAD/question_types/quiz_question/QuizQuestion#validate()
   */
  public function validate(&$form) {
    if ($this->node->choice_multi == 0) {
      $found_one_correct = FALSE;
      for ($i = 0; is_array($this->node->alternatives[$i]); $i++) {
        $short = $this->node->alternatives[$i];
        if (strlen($this->checkMarkup($i,'answer')) < 1) continue;
        if ($short['correct'] == 1) {
          if ($found_one_correct) {
            form_set_error("choice_multi", t('You have several alternatives marked as correct. If this is done intentionally you must must allow multiple alternatives to be correct.'));
          }
          else {
            $found_one_correct = TRUE;
          }
        }
      }
      if (!$found_one_correct) {
        form_set_error('choice_multi', t('You have not marked any alternatives as correct. If there are no correct alternatives you should allow multiple answers.'));
      }
    }
    else {
      for ($i = 0; is_array($this->node->alternatives[$i]); $i++) {
        $short = $this->node->alternatives[$i];
        if (strlen($this->checkMarkup($i, 'answer')) < 1) continue;
        if ($short['score_if_chosen'] < $short['score_if_not_chosen'] && $short['correct']) {
          form_set_error("alternatives][$i][score_if_not_chosen", t('The alternative is marked as correct, but gives more points if you don\'t select it.'));
        }
        elseif ($short['score_if_chosen'] > $short['score_if_not_chosen'] && !$short['correct']) {
          form_set_error("alternatives][$i][score_if_chosen", t('The alternative is not marked as correct, but gives more points if you select it.'));
        }
      }
    }
  }
  /**
   * Implementation of delete
   *
   * (non-PHPdoc)
   * @see sites/all/modules/quiz-HEAD/question_types/quiz_question/QuizQuestion#delete()
   */
  public function delete($only_this_version = FALSE) {
    if ($only_this_version) {
      db_query('DELETE FROM {quiz_multichoice_properties} WHERE nid = %d AND vid = %d', $this->node->nid, $this->node->vid);
      $sql = 'DELETE FROM {quiz_multichoice_user_answers}
      		  WHERE result_id IN(
      		  	SELECT result_id
      		  	FROM {quiz_node_results}
      		  	WHERE question_nid = %d AND question_vid = %d
      		  )';
      db_query($sql, $this->node->nid, $this->node->vid);
    }
    else {
      db_query('DELETE FROM {quiz_multichoice_properties} WHERE nid = %d', $this->node->nid);
      $sql = 'DELETE FROM {quiz_multichoice_user_answers}
      		  WHERE result_id IN(
      		  	SELECT result_id
      		  	FROM {quiz_node_results}
      		  	WHERE question_nid = %d
      		  )';
      db_query($sql, $this->node->nid);
    }
  }

  /**
   * Implementation of load
   *
   * (non-PHPdoc)
   * @see sites/all/modules/quiz-HEAD/question_types/quiz_question/QuizQuestion#load()
   */
  public function getNodeProperties() {
    $sql = 'SELECT choice_multi, choice_random, choice_boolean
            FROM {quiz_multichoice_properties}
            WHERE nid = %d AND vid = %d';
    $res = db_query($sql, $this->node->nid, $this->node->vid);
    $to_return = db_fetch_array($res);
    $sql = 'SELECT id, answer, answer_format, feedback_if_chosen, feedback_if_chosen_format,
            feedback_if_not_chosen, feedback_if_not_chosen_format, score_if_chosen, score_if_not_chosen
            FROM {quiz_multichoice_answers}
            WHERE question_nid = %d AND question_vid = %d';
    $res = db_query($sql, $this->node->nid, $this->node->vid);
    $to_return['alternatives'] = array(); // init array so it can be iterated even if empty
    while ($res_arr = db_fetch_array($res)) {
      $to_return['alternatives'][] = $res_arr;
    }
    return $to_return;
  }

  /**
   * Implementation of view
   *
   */
  public function getNodeView() {
    $content = parent::getNodeView();

    $content['answers'] = array();

    $header = array('','');

    foreach ($this->node->alternatives as $i => $short) {
      $answer_markup = $this->checkMarkup($i, 'answer', FALSE);
      $is_correct = ($short['score_if_chosen'] > $short['score_if_not_chosen']);
      $p = drupal_get_path('module', 'multichoice');
      $image = $is_correct ? 'correct' : 'wrong';
      $rows[] = array(
        array(
          'data' => theme('image', 
            "$p/theme/images/$image.png", 
            t($image), 
            t('Score if chosen: @sc Score if not chosen: @nc', array(
              '@sc' => $short['score_if_chosen'],
              '@nc' => $short['score_if_not_chosen'])
            )
          ), 
          'width' => 35
        ),
        $answer_markup
      );
    }

    $content['answers'] = array(
       '#value' => theme('table', $header, $rows),
       '#weight' => 2,
    );

    return $content;
  }

  /**
   * Generates the question form.
   *
   * This is called whenever a question is rendered, either
   * to an administrator or to a quiz taker.
   */

  public function getAnsweringForm($context = NULL) {
    $form['question'] = array(
      '#type' => 'markup',
      '#value' => check_markup($this->node->body, $this->node->format, FALSE),
    );
    $form['tries[answer]'] = array(
      '#options' => array(),
      '#theme' => 'multichoice_alternative',
    );
    $form['tries[answer]']['#taking_quiz'] = _quiz_is_taking_context();
    $form['tries[answer]']['#correct_choice'] = array();
    for ($i = 0; is_array($this->node->alternatives[$i]); $i++) {
      $short = $this->node->alternatives[$i];
      $answer_markup = $this->checkMarkup($i, 'answer', FALSE);
      if (drupal_strlen($answer_markup) > 0) {
        if (! $form['tries[answer]']['#taking_quiz']) {
          // displaying outside a quiz-taking context
          $is_correct = ($short['score_if_chosen'] > $short['score_if_not_chosen']);
          $form['tries[answer]']['#correct_choice'][$short['id']] = $is_correct;
        }
        $form['tries[answer]']['#options'][$short['id']] = $answer_markup;
      }
    }
    if ($this->node->choice_random) {
      $form['tries[choice_order]'] = array(
        '#type' => 'hidden',
        '#value' => implode(',',$this->shuffle($form['tries[answer]']['#options'])),
      );
    }
    if ($this->node->choice_multi) {
      $form['tries[answer]']['#type'] = 'checkboxes';
      $form['tries[answer]']['#title'] = t('Choose');
    } else {
      $form['tries[answer]']['#type'] = 'radios';
      $form['tries[answer]']['#title'] = t('Choose one');
    }

    if (!$form['tries[answer]']['#taking_quiz']) {
      unset($form['tries[answer]']['#title']);
      $form['tries[answer]']['#disabled'] = TRUE;
    }
    return $form;
  }

  private function shuffle(&$array) {
    $newArray = array();
    $toReturn = array_keys($array);
    shuffle($toReturn);
    foreach($toReturn as $key) {
      $newArray[$key] = $array[$key];
    }
    $array = $newArray;
    return $toReturn;
  }
  /**
   * Implementation of getAdminForm
   *
   * (non-PHPdoc)
   * @see sites/all/modules/quiz-HEAD/question_types/quiz_question/QuizQuestion#getAdminForm()
   */
  public function getAdminForm($edit = NULL) {
    $form['multichoice_max_num_of_alts'] = array(
      '#type' => 'textfield',
      '#title' => t('Maximum number of alternatives allowed'),
      '#default_value' => variable_get('multichoice_max_num_of_alts', 2),
    );
    return system_settings_form($form);
  }

  /**
   * Implementation of getCreation form
   *
   * (non-PHPdoc)
   * @see sites/all/modules/quiz-HEAD/question_types/quiz_question/QuizQuestion#getCreationForm()
   */
  public function getCreationForm($form_state) {
    $form = array();
    $type = node_get_types('type', $this->node);

    $action = '/node/add/'. $type->type;

    if ($this->node->nid) {
      $action = '/node/'. $this->node->nid. '/edit';
    }
    $form['#action'] = "?q=$action";
    /*
     * Getting presets from the database
     */
    $form['#theme'][] = 'multichoice_creation_form';

    $form['alternatives'] = array(
      '#type' => 'fieldset',
      '#title' => t('Alternatives'),
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
      '#tree' => TRUE,
    );
    $form['alternatives']['settings'] = array(
      '#type' => 'fieldset',
      '#title' => t('Settings'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );
    $default_settings = $this->getDefaultAltSettings();
    $form['alternatives']['settings']['choice_multi'] = array(
      '#type' => 'checkbox',
      '#title' => t('Multiple answers'),
      '#description' => t('Allow any number of answers. If this box is not checked, one, and only one answer is allowed.'),
      '#default_value' => $default_settings['choice_multi'],
      '#parents' => array('choice_multi'),
    );
    $form['alternatives']['settings']['choice_random'] = array(
      '#type' => 'checkbox',
      '#title' => t('Random order'),
      '#description' => t('Present alternatives in random order when quiz is beeing taken.'),
      '#default_value' => $default_settings['choice_random'],
      '#parents' => array('choice_random'),
    );
    $form['alternatives']['settings']['choice_boolean'] = array(
      '#type' => 'checkbox',
      '#title' => t('Simple scoring'),
      '#description' => t('Give one point if everything is correct. Zero points otherwise.'),
      '#default_value' => $default_settings['choice_boolean'],
      '#parents' => array('choice_boolean'),
    );

    $form['alternatives']['settings']['desc'] = array(
      '#type' => 'markup',
      '#value' => t('Your settings will be remembered.'),
    );

    $form['alternatives']['#theme'][] = 'multichoice_creation_form';
    $i = 0;
    if (isset($form_state['choice_count'])) {
      $choice_count = $form_state['choice_count'];
    }
    else {
      $choice_count = max(variable_get('multichoice_max_num_of_alts', 2), is_array($this->node->alternatives) ? count($this->node->alternatives) : 0);
    }
    for (; $i < $choice_count; $i++) {
      $short = $this->node->alternatives[$i];
      $form['alternatives'][$i] = array(
        '#type' => 'fieldset',
        '#title' => t('Alternative !i',array('!i' => ($i+1))),
        '#collapsible' => TRUE,
        '#collapsed' => !($i < 2 || isset($short['answer'])),
      );
      $form['alternatives'][$i]['#theme'][] = 'multichoice_alternative_creation';
      $form['alternatives'][$i]['correct'] = array(
        '#type' => 'checkbox',
        '#title' => t('Correct'),
        '#default_value' => ($short['score_if_chosen'] > $short['score_if_not_chosen']),
        '#attributes' => array('onchange' => 'refreshScores(this)'),
      );

      $form['alternatives'][$i]['id'] = array(
        '#type' => 'value',
        '#value' => $short['id'],
      );
      $form['alternatives'][$i]["answer"] = array(
        '#type' => 'textarea',
        '#title' => t('Alternative !i', array('!i' => ($i+1))),
        '#default_value' => $short['answer'],
        '#required' => $i < 2,
      );

      $form['alternatives'][$i]['format'] = filter_form($short['answer_format'], NULL, array('alternatives', $i,'answer_format'));
      $form['alternatives'][$i]['advanced'] = array(
        '#type' => 'fieldset',
        '#title' => t('Advanced options'),
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
      );
      $form['alternatives'][$i]['advanced']['feedback_if_chosen'] = array(
        '#type' => 'textarea',
        '#title' => t('Feedback if chosen'),
        '#description' => t('This feedback is given to users who chooses this alternative.'),
        '#parents' => array('alternatives', $i, 'feedback_if_chosen'),
        '#default_value' => $short['feedback_if_chosen'],
      );
      $form['alternatives'][$i]['advanced']['format'] = filter_form($short['feedback_if_chosen_format'], NULL, array('alternatives', $i,'feedback_if_chosen_format'));
      $form['alternatives'][$i]['advanced']['helper']['feedback_if_not_chosen'] = array(
        '#type' => 'textarea',
        '#title' => t('Feedback if not chosen'),
        '#description' => t('This feedback is given to users who doesn\'t choose this alternative.'),
        '#parents' => array('alternatives', $i, 'feedback_if_not_chosen'),
        '#default_value' => $short['feedback_if_not_chosen'],
      );
      $form['alternatives'][$i]['advanced']['helper']['format'] = filter_form($short['feedback_if_not_chosen_format'], NULL, array('alternatives',$i,'feedback_if_not_chosen_format'));
      $default_value = $this->node->alternatives[$i]['score_if_chosen'];
      if (!isset($default_value)) $default_value = '0';
      $form['alternatives'][$i]['advanced']['score_if_chosen'] = array(
        '#type' => 'textfield',
        '#title' => t('Score if chosen'),
        '#size' => 4,
        '#maxlength' => 4,
        '#default_value' => $default_value,
        '#description' => t('This score is added to the users total score if the user chooses this alternative. Only used if multiple answers are allowed.'),
        '#attributes' => array(
          'onkeypress' => 'refreshCorrect(this)',
          'onkeyup' => 'refreshCorrect(this)',
          'onchange' => 'refreshCorrect(this)'
          ),
        '#parents' => array('alternatives', $i, 'score_if_chosen')
          );
          $default_value = $short['score_if_not_chosen'];
          if (!isset($default_value)) $default_value = '0';
          $form['alternatives'][$i]['advanced']['score_if_not_chosen'] = array(
      	'#type' => 'textfield',
        '#title' => t('Score if not chosen'),
      	'#size' => 4,
        '#maxlength' => 4,
        '#default_value' => $default_value,
        '#description' => t('This score is added to the users total score if the user doesn\'t choose this alternative. Only used if multiple answers are allowed.'),
        '#attributes' => array(
          'onkeypress' => 'refreshCorrect(this)',
          'onkeyup' => 'refreshCorrect(this)',
          'onchange' => 'refreshCorrect(this)'
          ),
        '#parents' => array('alternatives', $i, 'score_if_not_chosen')
          );
    }
    $form['alternatives']["placeholder"] = array(
      '#type' => 'markup',
      '#value' => '<DIV id=\'placeholder\'></DIV>',
    );
    $form['alternatives']['multichoice_add_alternative'] = array(
      '#type' => 'submit',
      '#value' => t('Add more alternatives'),
      '#submit' => array('multichoice_more_choices_submit'), // If no javascript action.
      '#ahah' => array(
        'path' => "multichoice/add_alternative_js",
        'wrapper' => 'placeholder',
        'effect' => 'slide',
        'method' => 'before',
    ),
    );
    return $form;
  }
  private function getDefaultAltSettings() {
    $to_return = array();
    if (isset($this->node->nid)) {
      $to_return['choice_multi'] = $this->node->choice_multi;
      $to_return['choice_random'] = $this->node->choice_random;
      $to_return['choice_boolean'] = $this->node->choice_boolean;
    }
    elseif ($to_return = $this->getUserSettings()) {
    }
    else {
      $to_return['choice_multi'] = 0;
      $to_return['choice_random'] = 0;
      $to_return['choice_boolean'] = 0;
    }
    return $to_return;
  }
  private function getUserSettings() {
    global $user;
    $sql = 'SELECT last_nid, last_vid
            FROM {quiz_multichoice_user_settings}
            WHERE uid = %d';
    $res = db_query($sql, $user->uid);
    if ($obj = db_fetch_object($res)) {
      $sql = 'SELECT choice_multi, choice_random, choice_boolean
              FROM {quiz_multichoice_properties}
              WHERE nid = %d AND vid = %d';
      $res = db_query($sql, $obj->last_nid, $obj->last_vid);
      if ($arr = db_fetch_array($res)) {
        return $arr;
      }
      else return false;
    }
    else return false;
  }
  private function saveUserSettings() {
    global $user;
    $sql = 'REPLACE INTO {quiz_multichoice_user_settings}
            (uid, last_nid, last_vid)
            VALUES (%d, %d, %d)';
    db_query($sql, $user->uid, $this->node->nid, $this->node->vid);
  }
  /**
   * Implementation of getMaximumScore.
   *
   * (non-PHPdoc)
   * @see sites/all/modules/quiz-DRUPAL-6--4/question_types/quiz_question/QuizQuestion#getMaximumScore()
   */
  public function getMaximumScore() {
    if ($this->node->choice_boolean) return 1;
    $max = 0;
    for ($i = 0; is_array($this->node->alternatives[$i]); $i++) {
      $short = $this->node->alternatives[$i];
      $max += max($short['score_if_chosen'], $short['score_if_not_chosen']);
    }
    return $max;
  }
}

/**
 * The short answer question response class.
 */
class MultichoiceResponse extends AbstractQuizQuestionResponse {
  /**
   * ID of the answers.
   */
  protected $user_answer_ids;
  protected $choice_order;

  public function __construct($result_id, $question_node, $tries = NULL) {
    parent::__construct($result_id, $question_node);
    $this->user_answer_ids = array();
    if (is_array($tries)) {
      $this->choice_order = $tries['choice_order'];
      unset($tries['choice_order']);
      if (is_array($tries['answer'])) {
        foreach ($tries['answer'] as $answer_id) $this->user_answer_ids[] = $answer_id;
      }
      elseif (isset($tries['answer'])) {
        $this->user_answer_ids[] = $tries['answer'];
      }
    }
    else {
      $sql = 'SELECT answer_id
              FROM {quiz_multichoice_user_answer_multi} uam
              JOIN {quiz_multichoice_user_answers} ua ON(uam.user_answer_id = ua.id)
              WHERE ua.result_id = %d AND ua.question_nid = %d AND ua.question_vid = %d';
      $res = db_query($sql, $result_id, $this->question->nid, $this->question->vid);
      while ($res_o = db_fetch_object($res)) {
        $this->user_answer_ids[] = $res_o->answer_id;
      }
    }
  }

  /**
   * Implementation of save
   */
  public function save() {
    $sql = "INSERT INTO {quiz_multichoice_user_answers}
            (result_id, question_vid, question_nid, choice_order)
            VALUES (%d, %d, %d, '%s')";
    db_query($sql, $this->rid, $this->question->vid, $this->question->nid, $this->choice_order);
    $user_answer_id = db_last_insert_id('{quiz_multichoice_user_answers}', 'id');
    for ($i = 0; $i < count($this->user_answer_ids); $i++) {
      $sql = 'INSERT INTO {quiz_multichoice_user_answer_multi}
              (user_answer_id, answer_id)
              VALUES(%d, %d)';
      db_query($sql, $user_answer_id, $this->user_answer_ids[$i]);
    }
  }

  /**
   * Implementation of delete
   */
  public function delete() {
    $sql = 'DELETE FROM {quiz_multichoice_user_answer_multi}
            WHERE user_answer_id IN(
              SELECT id FROM {quiz_multichoice_user_answers}
              WHERE question_nid = %d AND question_vid = %d AND result_id = %d
            )';
    db_query($sql, $this->nid, $this->question->vid, $this->rid);
    $sql = 'DELETE FROM {quiz_multichoice_user_answers}
    		WHERE result_id = %d AND question_nid = %d AND question_vid = %d';
    db_query($sql, $this->rid, $this->question->nid, $this->question->vid);
  }

  /**
   * Implementation of score
   *
   * @return uint
   */
  public function score() {
    if ($this->question->choice_boolean) {
      $score = 1;
      foreach ($this->question->alternatives as $key => $alt) {
        if (in_array($alt['id'], $this->user_answer_ids)) {
          if ($alt['score_if_chosen'] <= $alt['score_if_not_chosen']) $score = 0;
        }
        else {
          if ($alt['score_if_chosen'] > $alt['score_if_not_chosen']) $score = 0;
        }
      }
    }
    else {
      $score = 0;
      foreach ($this->question->alternatives as $key => $alt) {
        if (in_array($alt['id'], $this->user_answer_ids)) {
          $score += $alt['score_if_chosen'];
        }
        else {
          $score += $alt['score_if_not_chosen'];
        }
      }
    }
    return $score;
  }

  /**
   * Implementation of getResponse
   *
   * @return answer
   */
  public function getResponse() {
    return $this->user_answer_ids;
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
    check_markup($this->question->body, $this->question->filter, FALSE).
      '</div>';
    $result = '<div class="quiz_answer_feedback">';
    $result .= '<H4>'. t('Alternatives'). ':</H4>';
    $i = 0;
    $this->orderAlternatives($this->question->alternatives);
    while (is_array($this->question->alternatives[$i])) {
      $short = $this->question->alternatives[$i];
      if (drupal_strlen($this->checkMarkup($short['answer'], $short->answer_format)) > 0) {
        $row = array();
        $the = ($this->question->choice_multi == 1) ? t('a') : t('the');
        $p = drupal_get_path('module', 'multichoice');
        $rowspan = 1;
        $not = '';
        if ($short['score_if_chosen'] > $short['score_if_not_chosen']) {
          if (in_array($short['id'], $this->user_answer_ids)) {
            if (drupal_strlen($this->checkMarkup($short['feedback_if_chosen'], $short['feedback_if_chosen_format'])) > 0) {
              $rowspan = 2;
            }
            $row[] = array('data' => theme('image', "$p/theme/images/correct.png", t('Correct'), t('You chose !the correct answer', array('!the' => $the))), 'width' => 35, 'rowspan' => $rowspan);
          }
          else {
            if (drupal_strlen($this->checkMarkup($short['feedback_if_not_chosen'], $short['feedback_if_not_chosen_format'])) > 0) {
              $rowspan = 2;
              $not = '_not';
            }
            $row[] = array('data' => theme('image', "$p/theme/images/should.png", t('Should have chosen'), t('This is !the correct answer, but you didn\'t choose it', array('!the' => $the))), 'width' => 35, 'rowspan' => $rowspan);
          }
        }
        else {
          if (in_array($short['id'], $this->user_answer_ids)) {
            if (drupal_strlen($this->checkMarkup($short['feedback_if_chosen'], $short['feedback_if_chosen_format'])) > 0) {
              $rowspan = 2;
            }
            $row[] = array('data' => theme('image', "$p/theme/images/wrong.png", t('Wrong'), t('You chose !the wrong answer', array('!the' => $the))), 'width' => 35, 'rowspan' => $rowspan);
          }
          else {
            if (drupal_strlen($this->checkMarkup($short['feedback_if_not_chosen'], $short['feedback_if_not_chosen_format'])) > 0) {
              $rowspan = 2;
              $not = '_not';
            }
            $row[] = array('data' => '', 'width' => 35, 'rowspan' => $rowspan);
          }
        }
        $row[] = $this->checkMarkup($short['answer'], $short['answer_format'], FALSE);
        $rows[] = $row;
        if ($rowspan > 1) {
          $feedback = $this->checkMarkup($short['feedback_if'. $not .'_chosen'], $short['feedback_if'. $not .'_chosen_format'], FALSE);
          $rows[] = array("<H5>Feedback:</H5>$feedback");
        }
      }
      $i++;
    }
    $result .= theme('table', NULL, $rows);

    $result .= '</div>';

    return $slug . $result;
  }

  private function orderAlternatives(&$alternatives) {
    if (!$this->question->choice_random) return;
    $sql = "SELECT choice_order
            FROM {quiz_multichoice_user_answers}
            WHERE result_id = %d AND question_nid = %d AND question_vid = %d";
    $res = db_query($sql, $this->rid, $this->question->nid, $this->question->vid);
    $order = explode(',', db_result($res));
    $newAlternatives = array();
    foreach ($order as $value) {
      foreach ($alternatives as $alternative) {
        if ($alternative['id'] == $value) {
          $newAlternatives[] = $alternative;
          break;
        }
      }
    }
    $alternatives = $newAlternatives;
  }
  /**
   * Run check_markup() on the field of the specified choice alternative
   * @param $alternativeIndex
   * @param $field
   * @return HTML markup
   */
  private function checkMarkup($alternative, $format, $check_user_access = FALSE) {
    if(drupal_strlen($alternative) == 0) return '';
    return check_markup($alternative, $format, $check_user_access);
  }
}
