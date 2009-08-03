<?php

// $Id$

/**
 * The main classes for the choice question type.
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

/*
 * @todo: Ajax to add more alternatives?
 * @todo: A page for importing questions from the old multichoice module, and uninstall it afterwards.
 * WHEN: This module is good enough to take over
 * REMEMBER: Also move all user answers
 *
 * @todo: Be able to have max score in quizes.
 * TODO: Make the entire alternative clickable in student view.
 * TODO: Ajax answer removal
 * TODO: Fjerne input formatene.
 * TODO: Må slutte å bruke forløkka for å lagre ting...
 * TODO: Remove error messages from ahah.
 * @todo: Be able to have max score in quizzes.
>>>>>>> 1.1.2.15
 */

/**
 * Implementation of QuizQuestion.
 */
class ChoiceQuestion implements QuizQuestion {

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

  private function forgive() {
    if ($this->node->choice_multi == 1) {
      for ($i = 0; is_array($this->node->alternatives[$i]); $i++) {
        $short = &$this->node->alternatives[$i];
        if ($short['score_if_chosen'] == $short['score_if_not_chosen'] ||
        !is_numeric($short['score_if_chosen'] || !is_numeric($short['score_if_not_chosen']))) {
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
    $this->forgive();
    if ($is_new_node) {
      $sql = 'INSERT INTO {quiz_choice_properties}
        (nid, vid, choice_multi)
        VALUES (%d, %d, %d)';
      db_query($sql, $this->node->nid, $this->node->vid, $this->node->choice_multi);
      for ($i = 0; is_array($this->node->alternatives[$i]); $i++) {
        if (strlen(strip_tags($this->node->alternatives[$i]['answer'])) > 0) {
          $this->insertAlternative($i);
        }
      }
    }
    else {
      $sql = 'UPDATE {quiz_choice_properties}
        SET choice_multi = %d
        WHERE nid = %d AND vid = %d';
      db_query($sql, $this->node->choice_multi, $this->node->nid, $this->node->vid);
      $sql = 'SELECT id FROM {quiz_choice_answer}
              WHERE question_nid = %d AND question_vid = %d';
      $res = db_query($sql, $this->node->nid, $this->node->vid);
      $ids_to_delete = array();
      while($res_o = db_fetch_object($res)) {
        $ids_to_delete[] = $res_o->id;
      }
      for ($i = 0; is_array($this->node->alternatives[$i]); $i++) {
        $short = $this->node->alternatives[$i];
        if (strlen(strip_tags($short['answer'])) > 0) {
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
          db_query('DELETE FROM {quiz_choice_answer} WHERE id = %d', $id_to_delete);
        }
      }
    }
  }

  private function insertAlternative($i) {
    $sql = 'INSERT INTO {quiz_choice_answer}
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
    $sql = 'UPDATE {quiz_choice_answer}
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
  public function validate($node, &$form) { 
    if ($node->choice_multi == 0) {
      $found_one_correct = FALSE;
      for ($i = 0; is_array($this->node->alternatives[$i]); $i++) {
        $short = $this->node->alternatives[$i];
        if (strlen(strip_tags($short['answer'])) < 1) continue;
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
        if (strlen(strip_tags($short['answer'])) < 1) continue;
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
      db_query('DELETE FROM {quiz_choice_properties} WHERE nid = %d AND vid = %d', $this->node->nid, $this->node->vid);
      $sql = 'DELETE FROM {quiz_choice_user_answers}
      		  WHERE result_id IN(
      		  	SELECT result_id
      		  	FROM {quiz_node_results}
      		  	WHERE nid = %d AND vid = %d
      		  )';
      db_query($sql, $this->node->nid, $this->node->vid);
    }
    else {
      db_query('DELETE FROM {quiz_choice_properties} WHERE nid = %d', $this->node->nid);
      $sql = 'DELETE FROM {quiz_choice_user_answers}
      		  WHERE result_id IN(
      		  	SELECT result_id
      		  	FROM {quiz_node_results}
      		  	WHERE nid = %d
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
  public function load() {
    $to_return = array();
    $sql = 'SELECT choice_multi
            FROM {quiz_choice_properties} 
            WHERE nid = %d AND vid = %d';
    $res = db_query($sql, $this->node->nid, $this->node->vid);
    $to_return['choice_multi'] = db_fetch_object($res)->choice_multi;
    $sql = 'SELECT id, answer, answer_format, feedback_if_chosen, feedback_if_chosen_format, 
            feedback_if_not_chosen, feedback_if_not_chosen_format, score_if_chosen, score_if_not_chosen
            FROM {quiz_choice_answer}
            WHERE question_nid = %d AND question_vid = %d';
    $res = db_query($sql, $this->node->nid, $this->node->vid);
    $counter = 0;
    while($res_arr = db_fetch_array($res)){
      $to_return['alternatives'][$counter] = $res_arr;
      $counter++; 

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

    drupal_add_css(drupal_get_path('module', 'choice') .'/theme/choice.css', 'module', 'all');
    $display_object = menu_get_object();
    $taking_quiz = $display_object->type == 'quiz';
    for ($i = 0; is_array($node->alternatives[$i]); $i++) {
      $short = $node->alternatives[$i];
      if (drupal_strlen(strip_tags($short['answer'])) > 0) {
        $markup = check_markup($short['answer'].$add, $short['answer_format']);
        if (! $taking_quiz) {
          // displaying outside a quiz-taking context
          $is_correct = $short['score_if_chosen'] > $short['score_if_not_chosen'];
          if ($is_correct)
            $markup = '<div class="quiz_choice_answer_correct">'.$markup.'</span>';
        }
        $options[$short['id']] = $markup;
      }
    }
    $form['question'] = array(
      '#type' => 'markup',
      '#value' => check_markup($node->body, $node->format, FALSE),
    );
    $form['tries'] = array(
      '#type' => $type,
      '#options' => $options,
      '#theme' => 'choice_alternative',
    );

    if ($node->choice_multi) {
      $form['tries']['#type'] = 'checkboxes';
      $form['tries']['#title'] = t('Choose');
    } else {
      $form['tries']['#type'] = 'radios';
      $form['tries']['#title'] = t('Choose one');
    }

    if (!$taking_quiz) {
      unset($form['tries']['#title']);
      $form['tries']['#disabled'] = TRUE;
    }

    return $form;
  }

  /**
   * Implementation of getAdminForm
   *
   * (non-PHPdoc)
   * @see sites/all/modules/quiz-HEAD/question_types/quiz_question/QuizQuestion#getAdminForm()
   */
  public function getAdminForm($edit = NULL) {
    $form['choice_max_num_of_alts'] = array(
      '#type' => 'textfield',
      '#title' => t('Maximum number of alternatives allowed'),
      '#default_value' => variable_get('choice_max_num_of_alts', 2),
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
    /*
     * Getting presets from the database
     */
    $form['#theme'][] = 'choice_creation_form';

    $form['alternatives'] = array(
      '#type' => 'fieldset',
      '#title' => t('Alternatives'),
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
      '#tree' => TRUE,
    );
    $form['alternatives']['choice_multi'] = array(
      '#type' => 'checkbox',
      '#title' => t('Allow multiple answers'),
      '#description' => t('I this box is checked any number of answers are allowed. If not, one, and only one answer is allowed.'),
      '#default_value' => $this->node->choice_multi,
      '#parents' => array('choice_multi'),
    );
    $form['alternatives']['#theme'][] = 'choice_creation_form';
    $i = 0;
    if (isset($form_state['choice_count'])) {
      $choice_count = $form_state['choice_count'];
    }
    else {
      $choice_count = max(variable_get('choice_max_num_of_alts', 2), is_array($this->node->alternatives) ? count($this->node->alternatives) : 0);
    }
    for (; $i < $choice_count; $i++) {
      $short = $this->node->alternatives[$i];
      $form['alternatives'][$i] = array(
        '#type' => 'fieldset',
        '#title' => t('Alternative !i',array('!i' => ($i+1))),
        '#collapsible' => TRUE,
        '#collapsed' => !($i < 2 || isset($short['answer'])),
      );
      $form['alternatives'][$i]['#theme'][] = 'choice_alternative_creation';
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
    $form['alternatives']['choice_add_alternative'] = array(
      '#type' => 'submit',
      '#value' => t('Add more alternatives'),
      '#submit' => array('choice_more_choices_submit'), // If no javascript action.
      '#ahah' => array(
        'path' => "choice/add_alternative_js",
        'wrapper' => 'placeholder',
        'effect' => 'slide',
        'method' => 'before',
      ),
    );
    return $form;
  }

  /**
   * Implementation of getMaximumScore.
   *
   * (non-PHPdoc)
   * @see sites/all/modules/quiz-DRUPAL-6--4/question_types/quiz_question/QuizQuestion#getMaximumScore()
   */
  public function getMaximumScore() {
    $sql = 'SELECT score_if_chosen, score_if_not_chosen
            FROM {quiz_choice_answer}
            WHERE question_nid = %d AND question_vid = %d';
    $res = db_query($sql, $this->node->nid, $this->node->vid);
    $max = 0;
    while ($res_o = db_fetch_object($res)){
      $max += max($res_o->score_if_chosen, $res_o->score_if_not_chosen);
    }
    return $max;
  }
}

/**
 * The short answer question response class.
 */
class ChoiceResponse extends AbstractQuizQuestionResponse {
  /**
   * ID of the answers.
   */
  protected $user_answer_ids;

  /**
   * Constructor
   *
   * @param $rid - response_id
   * @param $question - as an object
   * @param $answer - answer_id
   */
  public function __construct($rid, $question, $user_answer_ids = NULL) {
    $this->rid = $rid;
    $this->question = $question;
    $this->user_answer_ids = array();
    if (is_array($user_answer_ids)) {
      foreach ($user_answer_ids as $answer_id) $this->user_answer_ids[] = $answer_id;
    }
    elseif (isset($user_answer_ids)) $this->user_answer_ids[] = $user_answer_ids;
    else {
      $sql = 'SELECT answer_id
              FROM {quiz_choice_user_answer_multi} uam
              JOIN {quiz_choice_user_answers} ua ON(uam.user_answer_id = ua.id)
              WHERE ua.result_id = %d AND ua.nid = %d AND ua.vid = %d';
      $res = db_query($sql, $rid, $this->question->nid, $this->question->vid);
      while ($res_o = db_fetch_object($res)) {
        $this->user_answer_ids[] = $res_o->answer_id;
      }
    }
  }

  /**
   * Implementation of save
   */
  public function save() {
    $sql = "INSERT INTO {quiz_choice_user_answers}
            (result_id, vid, nid)
            VALUES (%d, %d, %d)";
    db_query($sql, $this->rid, $this->question->vid, $this->question->nid);
    $user_answer_id = db_last_insert_id('{quiz_choice_user_answers}', 'id');
    for ($i = 0; $i < count($this->user_answer_ids); $i++) {
      $sql = 'INSERT INTO {quiz_choice_user_answer_multi}
              (user_answer_id, answer_id)
              VALUES(%d, %d)';
      db_query($sql, $user_answer_id, $this->user_answer_ids[$i]);
    }
  }

  /**
   * Implementation of delete
   */
  public function delete() {
    $sql = 'DELETE FROM {quiz_choice_user_answer_multi}
            WHERE user_answer_id IN(
              SELECT id FROM {quiz_choice_user_answers}
              WHERE nid = %d AND vid = %d AND result_id = %d
            )';
    db_query($sql, $this->nid, $this->question->vid, $this->question->rid);
    $sql = 'DELETE FROM {quiz_choice_user_answers}
    		WHERE result_id = %d AND nid = %d AND vid = %d';
    db_query($sql, $this->rid, $this->question->nid, $this->question->vid);
  }

  /**
   * Implementation of score
   *
   * @return uint
   */
  public function score() {
    $score = 0;
    foreach ($this->question->alternatives as $key => $alt) {
      if (in_array($alt['id'], $this->user_answer_ids)) {
        $score += $alt['score_if_chosen'];
      }
      else {
        $score += $alt['score_if_not_chosen'];
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
    while (is_array($this->question->alternatives[$i])) {
      $short = $this->question->alternatives[$i];
      if (drupal_strlen(strip_tags($short['answer'])) > 0) {
        $row = array();         
        $the = ($this->question->choice_multi == 1) ? t('a') : t('the');
        $p = drupal_get_path('module', 'choice');
        $rowspan = 1;
        $not = '';
        if ($short['score_if_chosen'] > $short['score_if_not_chosen']) {
          if (in_array($short['id'], $this->user_answer_ids)) {
            if (drupal_strlen(strip_tags($short['feedback_if_chosen'])) > 0) {
              $rowspan = 2;
            }
            $row[] = array('data' => theme('image', "$p./theme/images/correct.png", t('Correct'), t('You chose !the correct answer', array('!the' => $the))), 'width' => 35, 'rowspan' => $rowspan);
          }
          else {
            if (drupal_strlen(strip_tags($short['feedback_if_not_chosen'])) > 0) {
              $rowspan = 2;
              $not = '_not';
            }
            $row[] = array('data' => theme('image', "$p./theme/images/should.png", t('Should have chosen'), t('This is !the correct answer, but you didn\'t choose it', array('!the' => $the))), 'width' => 35, 'rowspan' => $rowspan);
          }
        }
        else {
          if (in_array($short['id'], $this->user_answer_ids)) {
            if (drupal_strlen(strip_tags($short['feedback_if_chosen'])) > 0) {
              $rowspan = 2;
            }
            $row[] = array('data' => theme('image', "$p./theme/images/wrong.png", t('Wrong'), t('You chose !the wrong answer', array('!the' => $the))), 'width' => 35, 'rowspan' => $rowspan);
          }
          else {
            if (drupal_strlen(strip_tags($short['feedback_if_not_chosen'])) > 0) {
              $rowspan = 2;
              $not = '_not';
            }
            $row[] = array('data' => '', 'width' => 35, 'rowspan' => $rowspan);
          }
        }
        $row[] = check_markup($short['answer'], $short['answer_format'], FALSE);
        $rows[] = $row;
        if ($rowspan > 1) {
          $feedback = check_markup($short['feedback_if'. $not .'_chosen'], $short['feedback_if'. $not .'_chosen_format'], FALSE);
          $rows[] = array("<H5>Feedback:</H5>$feedback");
        }
      }
      $i++;
    }
    $result .= theme('table', NULL, $rows);

    $result .= '</div>';

    return $slug . $result;
  }
}
