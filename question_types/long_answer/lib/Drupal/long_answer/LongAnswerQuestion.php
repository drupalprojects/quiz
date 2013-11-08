<?php

/**
 * Long answer classes.
 *
 * @file
 * Classes modelling the long answer question and the long answer question response.
 * Contains \Drupal\long_answer\LongAnswerQuestion.
 */

namespace Drupal\long_answer;

use Drupal\quiz_question\QuizQuestion;

/**
 * Extension of QuizQuestion.
 */
class LongAnswerQuestion extends QuizQuestion {
  /**
   * Implementation of saveNodeProperties
   * @see QuizQuestion#saveNodeProperties($is_new)
   */
  public function saveNodeProperties($is_new = FALSE) {
    if (!isset($this->node->feedback)) {
      $this->node->feedback = '';
    }
    if ($is_new || $this->node->revision == 1) {
      $id = db_insert('quiz_long_answer_node_properties')
        ->fields(array(
          'nid' => $this->node->nid,
          'vid' => $this->node->vid,
          'rubric' => $this->node->rubric,
        ))
        ->execute();
    }
    else {
      db_update('quiz_long_answer_node_properties')
        ->fields(array('rubric' => $this->node->rubric))
        ->condition('nid', $this->node->nid)
        ->condition('vid', $this->node->vid)
        ->execute();
    }
  }

  /**
   * Implementation of validateNode
   *
   * @see QuizQuestion#validateNode($form)
   */
  public function validateNode(array &$form) { }

  /**
   * Implementation of delete
   *
   * @see QuizQuestion#delete($only_this_version)
   */
  public function delete($only_this_version = FALSE) {
    if ($only_this_version) {
      db_delete('quiz_long_answer_user_answers')
        ->condition('question_nid', $this->node->nid)
        ->condition('question_vid', $this->node->vid)
        ->execute();
      db_delete('quiz_long_answer_node_properties')
        ->condition('nid', $this->node->nid)
        ->condition('vid', $this->node->vid)
        ->execute();
    }
    else {
      db_delete('quiz_long_answer_node_properties')
        ->condition('nid', $this->node->nid)
        ->execute();
      db_delete('quiz_long_answer_user_answers')
        ->condition('question_nid', $this->node->nid)
        ->execute();
    }
    parent::delete($only_this_version);
  }

  /**
   * Implementation of getNodeProperties
   *
   * @see QuizQuestion#getNodeProperties()
   */
  public function getNodeProperties() {
    if (isset($this->nodeProperties)) {
      return $this->nodeProperties;
    }
    $props = parent::getNodeProperties();

    $res_a = db_query('SELECT rubric FROM {quiz_long_answer_node_properties}
      WHERE nid = :nid AND vid = :vid', array(':nid' => $this->node->nid, ':vid' => $this->node->vid))->fetchAssoc();

    if (is_array($res_a)) {
      $props = array_merge($props, $res_a);
    }

    $this->nodeProperties = $props;
    return $props;
  }

  /**
   * Implementation of getNodeView
   *
   * @see QuizQuestion#getNodeView()
   */
  public function getNodeView() {
    $content = parent::getNodeView();
    if ($this->viewCanRevealCorrect()) {
      $content['answers'] = array(
        '#type' => 'item',
        '#title' => t('Rubric'),
        '#markup' => '<div class="quiz-solution">' . check_markup($this->node->rubric, $this->getFormat()) . '</div>',
        '#weight' => 1,
      );
    }
    else {
      $content['answers'] = array(
        '#markup' => '<div class="quiz-answer-hidden">Answer hidden</div>',
        '#weight' => 1,
      );
    }
    return $content;
  }

  /**
   * Implementation of getAnweringForm
   *
   * @see QuizQuestion#getAnsweringForm($form_state, $rid)
   */
  public function getAnsweringForm(array $form_state = NULL, $rid) {
    $form = parent::getAnsweringForm($form_state, $rid);
    $form['#theme'] = 'long_answer_answering_form';

    $form['tries'] = array(
      '#type' => 'textarea',
      '#title' => t('Answer'),
      '#description' => t('Enter your answer here. If you need more space, click on the grey bar at the bottom of this area and drag it down.'),
      '#rows' => 15,
      '#cols' => 60,
      '#required' => FALSE, // See isValid() in LongAnswerResponse
    );
    if (isset($rid)) {
      $response = new LongAnswerResponse($rid, $this->node);
      $form['tries']['#default_value'] = $response->getResponse();
    }
    return $form;
  }

  /**
   * Implementation of getCreationForm
   *
   * @see QuizQuestion#getCreationForm($form_state)
   */
  public function getCreationForm(array &$form_state = NULL) {
    $form['rubric'] = array(
       '#type' => 'textarea',
       '#title' => t('Rubric'),
       '#description' => t('Specify the criteria for grading the response.'),
       '#default_value' => isset($this->node->rubric) ? $this->node->rubric : '',
       '#size' => 60,
       '#maxlength' => 512,
       '#required' => FALSE,
    );
    return $form;
  }

  /**
   * Implementation of getMaximumScore
   *
   * @see QuizQuestion#getMaximumScore()
   */
  public function getMaximumScore() {
    return variable_get('long_answer_default_max_score', 10);
  }
}
