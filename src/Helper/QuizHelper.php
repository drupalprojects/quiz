<?php

namespace Drupal\quiz\Helper;

use Drupal\quiz\Helper\Quiz\AccessHelper;
use Drupal\quiz\Helper\Quiz\ResultHelper;
class QuizHelper {

  private $resultHelper;
  private $accessHelper;

  /**
  /**
   * @return ResultHelper
   */
  public function getResultHelper() {
    if (null === $this->resultHelper) {
      $this->resultHelper = new ResultHelper();
    }
    return $this->resultHelper;
  }

  public function setResultHelper($resultHelper) {
    $this->resultHelper = $resultHelper;
    return $this;
  }

  /**
   * @return AccessHelper
   */
  public function getAccessHelper() {
    if (null !== $this->accessHelper) {
      $this->accessHelper = new AccessHelper();
    }
    return $this->accessHelper;
  }

  public function setAccessHelper($accessHelper) {
    $this->accessHelper = $accessHelper;
    return $this;
  }
   * Retrieve list of published questions assigned to quiz.
   *
   * This function should be used for question browsers and similiar... It should not be used to decide what questions
   * a user should answer when taking a quiz. quiz_build_question_list is written for that purpose.
   *
   * @param $quiz_nid
   *   Quiz node id.
   * @param $quiz_vid
   *   Quiz node version id.
   *
   * @return
   *   An array of questions.
   */
  public function getQuestions($quiz_nid, $quiz_vid = NULL) {
    $questions = array();
    $query = db_select('node', 'n');
    $query->fields('n', array('nid', 'type'));
    $query->fields('nr', array('vid', 'title'));
    $query->fields('qnr', array('question_status', 'weight', 'max_score', 'auto_update_max_score', 'qnr_id', 'qnr_pid'));
    $query->addField('n', 'vid', 'latest_vid');
    $query->join('node_revision', 'nr', 'n.nid = nr.nid');
    $query->leftJoin('quiz_node_relationship', 'qnr', 'nr.vid = qnr.child_vid');
    $query->condition('n.status', 1);
    $query->condition('qnr.parent_nid', $quiz_nid);
    if ($quiz_vid) {
      $query->condition('qnr.parent_vid', $quiz_vid);
    }
    $query->condition('qnr_pid', NULL, 'IS');
    $query->orderBy('qnr.weight');

    $result = $query->execute();
    foreach ($result as $question) {
      $questions[] = $question;
      $this->getSubQuestions($question->qnr_id, $questions);
    }

    foreach ($questions as &$node) {
      $node = quiz_node_map($node);
    }

    return $questions;
  }

  public function getSubQuestions($qnr_pid, &$questions) {
    $query = db_select('node', 'n');
    $query->fields('n', array('nid', 'type'));
    $query->fields('nr', array('vid', 'title'));
    $query->fields('qnr', array('question_status', 'weight', 'max_score', 'auto_update_max_score', 'qnr_id', 'qnr_pid'));
    $query->addField('n', 'vid', 'latest_vid');
    $query->innerJoin('node_revision', 'nr', 'n.nid = nr.nid');
    $query->innerJoin('quiz_node_relationship', 'qnr', 'nr.vid = qnr.child_vid');
    $query->condition('qnr_pid', $qnr_pid);
    $query->orderBy('weight');
    $result = $query->execute();
    foreach ($result as $question) {
      $questions[] = $question;
    }
  }

  public function copyQuestions($node) {
    // Find original questions.
    $query = db_query('SELECT child_nid, child_vid, question_status, weight, max_score, auto_update_max_score
    FROM {quiz_node_relationship}
    WHERE parent_vid = :parent_vid', array(':parent_vid' => $node->translation_source->vid));
    foreach ($query as $res_o) {
      $original_question = node_load($res_o->child_nid);

      // Set variables we can't or won't carry with us to the translated node to
      // NULL.
      $original_question->nid = $original_question->vid = $original_question->created = $original_question->changed = NULL;
      $original_question->revision_timestamp = $original_question->menu = $original_question->path = NULL;
      $original_question->files = array();
      if (isset($original_question->book['mlid'])) {
        $original_question->book['mlid'] = NULL;
      }

      // Set the correct language.
      $original_question->language = $node->language;

      // Save the node.
      node_save($original_question);

      // Save the relationship between the new question and the quiz.
      db_insert('quiz_node_relationship')
        ->fields(array(
          'parent_nid' => $node->nid,
          'parent_vid' => $node->vid,
          'child_nid' => $original_question->nid,
          'child_vid' => $original_question->vid,
          'question_status' => $res_o->question_status,
          'weight' => $res_o->weight,
          'max_score' => $res_o->max_score,
          'auto_update_max_score' => $res_o->auto_update_max_score,
        ))
        ->execute();
    }
  }

}
