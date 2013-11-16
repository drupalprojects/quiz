<?php


/**
 * The main classes for the matching question type.
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
 * Contains \Drupal\matching\MatchingQuestion.
 */

namespace Drupal\matching;

use Drupal\quiz_question\QuizQuestion;


/**
 * Extension of QuizQuestion.
 */
class MatchingQuestion extends QuizQuestion {

  /**
   * Constructor
   *
   * @param $node
   *  matching node
   */
  public function __construct($node) {
    parent::__construct($node);
    if (empty($this->node->match)) {
      $this->node->match = array();
    }
  }

  /**
   * Implementation of saveNodeProperties
   *
   * @see QuizQuestion#saveNodeProperties($is_new)
   */
  public function saveNodeProperties($is_new = FALSE) {
    // Loop through each question-answer combination
    foreach ($this->node->match as $match) {
      $match['feedback'] = (isset($match['feedback'])) ? $match['feedback'] : '';
      // match_id is not so it is a new question.
      if (empty($match['match_id']) || $this->node->isNewRevision()) {
        if (!empty($match['question']) && !empty($match['answer'])) {
          $sql = "INSERT INTO {quiz_matching_node} (nid, vid, question, answer, feedback) VALUES (%d, %d, '%s', '%s', '%s')";
          // TODO Please review the conversion of this statement to the D7 database API syntax.
          /* db_query($sql, $this->node->id(), $this->node->getRevisionId(), $match['question'], $match['answer'], $match['feedback']) */
          $id = db_insert('quiz_matching_node')
            ->fields(array(
              'nid' => $this->node->id(),
              'vid' => $this->node->getRevisionId(),
              'question' => $match['question'],
              'answer' => $match['answer'],
              'feedback' => $match['feedback'],
            ))
            ->execute();
        }
      }
      // match_id is set, user may remove or update existing question.
      else {
        if (empty($match['question']) && empty($match['answer'])) {
          // remove sub question.
          // TODO Please review the conversion of this statement to the D7 database API syntax.
          /* db_query("DELETE FROM {quiz_matching_node} WHERE match_id = %d", $match['match_id']) */
          db_delete('quiz_matching_node')
            ->condition('match_id', $match['match_id'])
            ->execute();
        }
        else {
          // update sub question.
          //$sql = "UPDATE {quiz_matching_node} SET question = '%s', answer = '%s', feedback = '%s' WHERE match_id = %d";
          // TODO Please review the conversion of this statement to the D7 database API syntax.
          /* db_query($sql, $match['question'], $match['answer'], $match['feedback'], $match['match_id']) */
          db_update('quiz_matching_node')
            ->fields(array(
              'question' => $match['question'],
              'answer' => $match['answer'],
              'feedback' => $match['feedback'],
            ))
            ->condition('match_id', $match['match_id'])
            ->execute();
        }
      }
    }
  }

  /**
   * Implementation of validateNode
   *
   * @see QuizQuestion#validateNode($form_state)
   */
  public function validateNode(array &$form_state) {
    // No validation is required
    // The first two pairs are required in the form, if there are other errors we forgive them
  }

  /**
   * Implementation of entityBuilder
   */
  public function entityBuilder(&$form_state) {
    $this->node->match = $form_state['values']['match'];
    $this->node->add_directly = $form_state['values']['add_directly'];
  }

  /**
   * Implementation of delete
   *
   * @see QuizQuestion#delete($only_this_version)
   */
  public function delete($only_this_version = FALSE) {
    parent::delete($only_this_version);
    if ($only_this_version) {
      $match_id = db_query('SELECT match_id FROM {quiz_matching_node} WHERE nid = :nid AND vid = :vid', array(':nid' => $this->node->id(), ':vid' => $this->node->getRevisionId()))->fetchCol();
      db_delete('quiz_matching_user_answers')
        ->condition('match_id', is_array($match_id) ? $match_id : array(0), 'IN')
        ->execute();

      db_delete('quiz_matching_node')
        ->condition('nid', $this->node->id())
        ->condition('vid', $this->node->getRevisionId())
        ->execute();
    }
    // Delete all versions of this question.
    else {
      $match_id = db_query('SELECT match_id FROM {quiz_matching_node} WHERE nid = :nid', array(':nid' => $this->node->id()))->fetchCol();
      db_delete('quiz_matching_user_answers')
        ->condition('match_id', is_array($match_id) ? $match_id : array(0), 'IN')
        ->execute();

      db_delete('quiz_matching_node')
        ->condition('nid', $this->node->id())
        ->execute();
    }
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

    //$sql = "SELECT match_id, question, answer, feedback FROM {quiz_matching_node} WHERE nid = %d AND vid = %d";
    $query = db_query('SELECT match_id, question, answer, feedback FROM {quiz_matching_node} WHERE nid = :nid AND vid = :vid', array(':nid' => $this->node->id(), ':vid' => $this->node->getRevisionId()));
    while ($result = $query->fetch()) {
      $props['match'][] = array(
        'match_id' => $result->match_id,
        'question' => $result->question,
        'answer' => $result->answer,
        'feedback' => $result->feedback,
      );
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

    list($matches, $select_option) = $this->getSubquestions();
    $subquestions = array();
    if ($this->viewCanRevealCorrect()) {
      foreach ($matches as $match) {
        $subquestions[] = array(
          'question' => $match['question'],
          'correct' => $match['answer'],
          'feedback' => $match['feedback']
        );
      }
    }
    else {
      // shuffle the answer column
      foreach ($matches as $match) {
        $sub_qs[] = $match['question'];
        $sub_as[] = $match['answer'];
      }
      shuffle($sub_as);
      foreach ($sub_qs as $i => $sub_q) {
        $subquestions[] = array(
          'question' => $sub_q,
          'random' => $sub_as[$i],
        );
      }
    }
    $content['answers'] = array(
      '#markup' => theme('matching_match_node_view', array('subquestions' => $subquestions)),
      '#weight' => 2,
    );
    return $content;
  }

  /**
   * Implementation of getAnsweringForm
   *
   * @see QuizQuestion#getAnsweringForm($form_state, $rid)
   */
  public function getAnsweringForm(array $form_state = NULL, $rid) {
    $form = parent::getAnsweringForm($form_state, $rid);
    //$form['#theme'] = 'matching_answering_form';

    if (isset($rid)) {
      // The question has already been answered. We load the answers
      $response = new MatchingResponse($rid, $this->node);
      $responses = $response->getResponse();
    }
    list($matches, $select_option) = $this->getSubquestions();
    $form['subquestions']['#theme'] = 'matching_subquestion_form';
    foreach ($matches as $match) {
      $form['subquestions'][$match['match_id']]['#question'] = check_markup($match['question']);
      $form['subquestions'][$match['match_id']]['tries[' . $match['match_id'] . ']'] = array(
        '#type' => 'select',
        '#options' => $this->customShuffle($select_option),
      );
      if (isset($rid)) {
        // If this question already has been answered
        $form['subquestions'][$match['match_id']]['tries[' . $match['match_id'] . ']']['#default_value'] = $responses[$match['match_id']];
      }
    }
    $form['scoring_info'] = array(
      '#markup' => '<p><em>' . t('You lose points by selecting incorrect options. You may leave an option blank to avoid losing points.') . '</em></p>',
    );
    if (variable_get('quiz_matching_shuffle_options', TRUE)) {
      $form['subquestions'] = $this->customShuffle($form['subquestions']);
    }
    return $form;
  }

  /**
   * Shuffles an array, but keeps the keys, and makes sure the first element is the default element
   *
   * @param $array
   *  Array to be shuffled
   * @return
   *  A shuffled version of the array with $array['def'] = '' as the first element
   */
  private function customShuffle(array $array = array()) {
    $new_array = array();
    $new_array['def'] = '';
    while (count($array)) {
      $element = array_rand($array);
      $new_array[$element] = $array[$element];
      unset($array[$element]);
    }
    return $new_array;
  }

  /**
   * Helper function to fetch subquestions
   *
   * @return
   *  Array with two arrays, matches and selected options
   */
  private function getSubquestions() {
    $matches = $select_option = array();
    //$sql = "SELECT match_id, question, answer, feedback FROM {quiz_matching_node} WHERE nid = %d AND vid = %d";
    $query = db_query('SELECT match_id, question, answer, feedback FROM {quiz_matching_node} WHERE nid = :nid AND vid = :vid', array(':nid' => $this->node->id(), ':vid' => $this->node->getRevisionId()));
    while ($result = $query->fetch()) {
      $matches[] = array(
        'match_id' => $result->match_id,
        'question' => $result->question,
        'answer' => $result->answer,
        'feedback' => $result->feedback,
      );
      $select_option[$result->match_id] = $result->answer;
    }
    return array($matches, $select_option);
  }

  /**
   * Implementation of getBodyFieldTitle
   *
   * @see QuizQuestion#getBodyFieldTitle()
   */
  public function getBodyFieldTitle() {
    return t('Instruction');
  }

  /**
   * Implementation of getCreationForm
   *
   * @see QuizQuestion#getCreationForm($form_state)
   */
  public function getCreationForm(array &$form_state = NULL) {
    $form['match'] = array(
      '#type' => 'fieldset',
      '#title' => t('Answer'),
      '#weight' => -4,
      '#tree' => TRUE,
      '#theme' => 'matching_question_form',
      '#description' => t('Write your pairs in the question and answer columns. For the user the question will be fixed and the answers will be shown as alternatives in a dropdown box.'),
    );
    for ($i = 1; $i <= variable_get('quiz_matching_form_size', 5); ++$i) {
      $form['match'][$i] = array(
        '#type' => 'fieldset',
        '#title' => t('Question ' . $i),
      );
      $form['match'][$i]['match_id'] = array(
        '#type' => 'value',
        '#default_value' => isset($this->node->match[$i -1]['match_id']) ? $this->node->match[$i -1]['match_id'] : ''
      );
      $form['match'][$i]['question'] = array(
        '#type' => 'textarea',
        '#rows' => 2,
        '#default_value' => isset($this->node->match[$i -1]['question']) ? $this->node->match[$i -1]['question'] : '',
        '#required' => $i < 3,
      );
      $form['match'][$i]['answer'] = array(
        '#type' => 'textarea',
        '#rows' => 2,
        '#default_value' => isset($this->node->match[$i -1]['answer']) ? $this->node->match[$i -1]['answer'] : '',
        '#required' => $i < 3,
      );

      $form['match'][$i]['feedback'] = array(
        '#type' => 'textarea',
        '#rows' => 2,
        '#default_value' => isset($this->node->match[$i -1]['feedback']) ? $this->node->match[$i -1]['feedback'] : ''
      );
    }
    return $form;
  }

  /**
   * Implementation of getMaximumScore
   *
   * @see QuizQuestion#getMaximumScore()
   */
  public function getMaximumScore() {
    $to_return = 0;
    foreach ($this->node->match as $match) {
      if (empty($match['question']) || empty($match['answer'])) {
        continue;
      }
      $to_return++;
    }
    // The maximum score = the number of sub-questions
    return $to_return;
  }

  /**
   * Get the correct answers for this question
   *
   * @return
   *  Array of correct answers
   */
  public function getCorrectAnswer() {
    $correct_answers = array();
    $query = db_query('SELECT match_id, question, answer, feedback FROM {quiz_matching_node} WHERE nid = :nid AND vid = :vid', array(':nid' => $this->node->id(), ':vid' => $this->node->getRevisionId()));
    while ($result = $query->fetch()) {
      $correct_answers[$result->match_id] = array(
        'match_id'  => $result->match_id,
        'question'  => $result->question,
        'answer'    => $result->answer,
        'feedback'  => $result->feedback,
      );
    }
    return $correct_answers;
  }
}
