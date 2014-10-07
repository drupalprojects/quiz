<?php

namespace Drupal\quiz;

use Drupal\quiz\Helper\NodeHelper;
use Drupal\quiz\Helper\QuizHelper;

/**
 * Wrapper for helper classes.
 *
 * Quiz.nodeHelper — Helper for node-hook implementations.
 * Quiz.quizHelper — Helper for quiz node/object.
 *
 * Extends this class and sub classes if you would like override things.
 *
 * You should not create object directly from this class, use quiz() factory
 * function instead — which support overriding from module's side.
 */
class Quiz {

  private $nodeHelper;
  private $quizHelper;

  /**
   * @return NodeHelper
   */
  public function getNodeHelper() {
    if (null === $this->nodeHelper) {
      $this->nodeHelper = new NodeHelper();
    }
    return $this->nodeHelper;
  }

  public function setNodeHelper($nodeHelper) {
    $this->nodeHelper = $nodeHelper;
    return $this;
  }

  /**
   * @return QuizHelper
   */
  public function getQuizHelper() {
    if (null === $this->quizHelper) {
      $this->quizHelper = new QuizHelper();
    }
    return $this->quizHelper;
  }

  public function setQuizHelper($quizHelper) {
    $this->quizHelper = $quizHelper;
    return $this;
  }

  /**
   * Returns the titles for all quizzes the user has access to.
   *
   * @return quizzes
   *   Array with nids as keys and titles as values.
   */
  public function getAllTitles() {
    return db_select('node', 'n')
        ->fields('n', array('nid', 'title'))
        ->condition('n.type', 'quiz')
        ->addTag('node_access')
        ->execute()
        ->fetchAllKeyed();
  }

  /**
   * Returns the titles for all quizzes the user has access to.
   *
   * @return quizzes
   *   Array with nids as keys and (array with vid as key and title as value) as values.
   *   Like this: array($nid => array($vid => $title))
   */
  public function getAllRevisionTitles() {
    $query = db_select('node', 'n');
    $query->join('node_revision', 'nr', 'nr.nid = n.nid');
    $query->fields('nr', array('nid', 'vid', 'title'))
      ->condition('n.type', 'quiz')
      ->execute();

    $to_return = array();
    while ($res_o = $query->fetch()) {
      $to_return[$res_o->nid][$res_o->vid] = $res_o->title;
    }
    return $to_return;
  }

}
