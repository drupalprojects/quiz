<?php
/**
 * @file
 * 
 * The main classes for the short answer question type.
 *
 * Contains Drupal\short_answer\ShortAnswerQuestion.
 */

namespace Drupal\short_answer;

use Drupal\quiz_question\QuizQuestion;
use Drupal\short_answer\ShortAnswerResponse;

/**
 * Extension of QuizQuestion.
 *
 * This could have extended long answer, except that that would have entailed
 * adding long answer as a dependency.
 */
class ShortAnswerQuestion extends QuizQuestion {

  // Constants for answer matching options
  const ANSWER_MATCH = 0;
  const ANSWER_INSENSITIVE_MATCH = 1;
  const ANSWER_REGEX = 2;
  const ANSWER_MANUAL = 3;

  /**
   * Implementation of saveNodeProperties
   *
   * @see QuizQuestion#saveNodeProperties($is_new)
   */
  public function saveNodeProperties($is_new = FALSE) {
    if ($is_new || $this->node->isNewRevision() == 1) {
      $id = db_insert('quiz_short_answer_node_properties')
        ->fields(array(
          'nid' => $this->node->id(),
          'vid' => $this->node->getRevisionId(),
          'correct_answer' => $this->node->correct_answer,
          'correct_answer_evaluation' => $this->node->correct_answer_evaluation,
        ))
        ->execute();
    }
    else {

      db_update('quiz_short_answer_node_properties')
        ->fields(array(
          'correct_answer' => $this->node->correct_answer,
          'correct_answer_evaluation' => $this->node->correct_answer_evaluation,
        ))
        ->condition('nid', $this->node->id())
        ->condition('vid', $this->node->getRevisionId())
        ->execute();
    }
  }

  /**
   * Implementation of validateNode
   *
   * @see QuizQuestion#validateNode($form_state)
   */
  public function validateNode(array &$form_state) {
    if ($form_state['values']['correct_answer_evaluation'] != self::ANSWER_MANUAL && empty($form_state['values']['correct_answer'])) {
      form_set_error('correct_answer', t('An answer must be specified for any evaluation type other than manual scoring.'));
    }
  }

  /**
   * Implementation of entityBuilder
   */
  public function entityBuilder(&$form_state) {
    $this->node->correct_answer = $form_state['values']['correct_answer'];
    $this->node->correct_answer_evaluation = $form_state['values']['correct_answer_evaluation'];
    $this->node->add_directly = $form_state['values']['add_directly'];
  }

  /**
   * Implementation of delete
   *
   * @see QuizQuestion#delete($only_this_version)
   */
  public function delete($only_this_version = FALSE) {
    parent::delete($only_this_version);
    $delete_ans = db_delete('quiz_short_answer_user_answers');
    $delete_ans->condition('question_nid', $this->node->id());

    $delete_node = db_delete('quiz_short_answer_node_properties');
    $delete_node->condition('nid', $this->node->id());

    if ($only_this_version) {
      $delete_ans->condition('question_vid', $this->node->getRevisionId());
      $delete_node->condition('vid', $this->node->getRevisionId());
    }

    $delete_ans->execute();
    $delete_node->execute();
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
    $res_a = db_query('SELECT correct_answer, correct_answer_evaluation FROM {quiz_short_answer_node_properties}
      WHERE nid = :nid AND vid = :vid', array(':nid' => $this->node->id(), ':vid' => $this->node->getRevisionId()))->fetchAssoc();
    $this->nodeProperties = (is_array($res_a)) ? array_merge($props, $res_a) : $props;
    return $this->nodeProperties;
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
        '#markup' => '<div class="quiz-solution">' . check_plain($this->node->correct_answer) . '</div>',
        '#weight' => 2,
      );
    }
    else {
      $content['answers'] = array(
        '#markup' => '<div class="quiz-answer-hidden">Answer hidden</div>',
        '#weight' => 2,
      );
    }
    return $content;
  }

  /**
   * Implementation of getAnsweringForm
   *
   * @see QuizQuestion#getAnsweringForm($form_state, $rid)
   */
  public function getAnsweringForm(array $form_state = NULL, $rid) {
    $form = parent::getAnsweringForm($form_state, $rid);
    $form['tries'] = array(
      '#type' => 'textfield',
      '#title' => t('Answer'),
      '#description' => t('Enter your answer here'),
      '#default_value' => '',
      '#size' => 60,
      '#maxlength' => 256,
      '#required' => FALSE,
      '#attributes' => array('autocomplete' => 'off'),
    );

    if (isset($rid)) {
      $response = new ShortAnswerResponse($rid, $this->node);
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
    $form['answer'] = array(
      '#type' => 'fieldset',
      '#title' => t('Answer'),
      '#description' => t('Provide the answer and the method by which the answer will be evaluated.'),
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
      '#weight' => -4,
    );

    $options = array(
      self::ANSWER_MATCH => t('Automatic and case sensitive'),
      self::ANSWER_INSENSITIVE_MATCH => t('Automatic. Not case sensitive'),
    );
    $access_regex = user_access('use regex for short answer');
    if ($access_regex) {
      $options[self::ANSWER_REGEX] = t('Match against a regular expression (answer must match the supplied regular expression)');
    }
    $options[self::ANSWER_MANUAL] = t('Manual');

    $form['answer']['correct_answer_evaluation'] = array(
      '#type' => 'radios',
      '#title' => t('Pick an evaluation method'),
      '#description' => t('Choose how the answer shall be evaluated.'),
      '#options' => $options,
      '#default_value' => isset($this->node->correct_answer_evaluation) ? $this->node->correct_answer_evaluation : self::ANSWER_MATCH,
      '#required' => FALSE,
    );
    if ($access_regex) {
      $form['answer']['regex_box'] = array(
        '#type' => 'fieldset',
        '#title' => t('About regular expressions'),
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
      );

      $form['answer']['regex_box']['regex_help'] = array(
        '#markup' => '<p>' .
      t('Regular expressions are an advanced syntax for pattern matching. They allow you to create a concise set of rules that must be met before a value can be considered a match.') .
         '</p><p>' .
      t('For more on regular expression syntax, visit !url.', array('!url' => l('the PHP regular expressions documentation', 'http://www.php.net/manual/en/book.pcre.php'))) .
         '</p>',
      );
    }

    $form['answer']['correct_answer'] = array(
       '#type' => 'textfield',
       '#title' => t('Correct answer'),
       '#description' => t('Specify the answer. If this question is manually scored, no answer needs to be supplied.'),
       '#default_value' => isset($this->node->correct_answer) ? $this->node->correct_answer : '',
       '#size' => 60,
       '#maxlength' => 256,
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
    return variable_get('short_answer_default_max_score', 5);
  }

  /**
   * Evaluate the correctness of an answer based on the correct answer and evaluation method.
   */
  public function evaluateAnswer($user_answer) {
    $score = 0;
    switch ($this->node->correct_answer_evaluation) {
      case self::ANSWER_MATCH:
        if ($user_answer == $this->node->correct_answer) {
          $score = $this->node->max_score;
        }
        break;
      case self::ANSWER_INSENSITIVE_MATCH:
        if (drupal_strtolower($user_answer) == drupal_strtolower($this->node->correct_answer)) {
          $score = $this->node->max_score;
        }
        break;
      case self::ANSWER_REGEX:
        if (preg_match($this->node->correct_answer, $user_answer) > 0) {
          $score = $this->node->max_score;
        }
        break;
    }
    return $score;
  }
}
