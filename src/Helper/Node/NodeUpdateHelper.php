<?php

namespace Drupal\quiz\Helper\Node;

use PDO;

class NodeUpdateHelper extends NodeHelper {

  public function execute($quiz) {
    // Quiz node vid (revision) was updated.
    if (isset($quiz->revision) && $quiz->revision) {
      // Create new quiz-question relation entries in the quiz_relationship table.
      $this->updateQuestionRelationship($quiz->old_vid, $quiz->vid, $quiz->nid);
    }

    // Update an existing row in the quiz_node_properties table.
    $this->presaveActions($quiz);

    quiz()->getQuizHelper()->getSettingHelper()->updateUserDefaultSettings($quiz);
    $this->updateResultOptions($quiz);

    _quiz_check_num_random($quiz);
    _quiz_check_num_always($quiz);
    quiz_update_max_score_properties(array($quiz->vid));
    drupal_set_message(t('Some of the updated settings may not apply to quiz being taken already. To see all changes in action you need to start again.'), 'warning');
  }

  /**
   * Modify result of option-specific updates.
   *
   * @param $node
   *   The quiz node.
   */
  private function updateResultOptions($quiz) {
    // Brute force method. Easier to get correct, and probably faster as well.
    db_delete('quiz_result_options')
      ->condition('vid', $quiz->vid)
      ->execute();
    $this->insertResultOptions($quiz);
  }

  /**
   * Copies quiz-question relation entries in the quiz_relationship table
   * from an old version of a quiz to a new.
   *
   * @param $old_quiz_vid
   *   The quiz vid prior to a new revision.
   * @param $new_quiz_vid
   *   The quiz vid of the latest revision.
   * @param $quiz_nid
   *   The quiz node id.
   */
  private function updateQuestionRelationship($old_quiz_vid, $new_quiz_vid, $quiz_nid) {
    // query for questions in previous version
    $result = db_select('quiz_relationship', 'qnr')
      ->fields('qnr', array('quiz_qid', 'child_nid', 'child_vid', 'question_status', 'weight', 'max_score', 'auto_update_max_score', 'qnr_id', 'qnr_pid'))
      ->condition('quiz_qid', $quiz_nid)
      ->condition('quiz_vid', $old_quiz_vid)
      ->condition('question_status', QUESTION_NEVER, '!=')
      ->execute();

    // only proceed if query returned data
    if ($result->rowCount()) {
      $questions = $result->fetchAll(PDO::FETCH_ASSOC);
      foreach ($questions as &$quiz_question) {
        $quiz_question['old_qnr_id'] = $quiz_question['qnr_id'];
        $quiz_question['quiz_qid'] = $quiz_nid;
        $quiz_question['quiz_vid'] = $new_quiz_vid;
        unset($quiz_question['qnr_id']);
        drupal_write_record('quiz_relationship', $quiz_question);
      }

      // Update the parentage when a new revision is created.
      // @todo this is copy pasta from quiz_set_questions
      foreach ($questions as $question) {
        db_update('quiz_relationship')
          ->condition('qnr_pid', $question['old_qnr_id'])
          ->condition('quiz_qid', $quiz_nid)
          ->condition('quiz_vid', $new_quiz_vid)
          ->fields(array('qnr_pid' => $question['qnr_id']))
          ->execute();
      }
    }

    /* Update terms if any */
    $result = db_select('quiz_terms', 'qt')
      ->fields('qt', array('nid', 'tid', 'weight', 'max_score', 'number'))
      ->condition('vid', $old_quiz_vid)
      ->execute();

    // only proceed if query returned data
    if ($result->rowCount()) {
      $insert_query = db_insert('quiz_terms')
        ->fields(array('nid', 'vid', 'tid', 'weight', 'max_score', 'number'));
      while ($quiz_term = $result->fetchAssoc()) {
        $insert_query->values(array(
          'nid'       => $quiz_nid,
          'vid'       => $new_quiz_vid,
          'tid'       => $quiz_term['tid'],
          'weight'    => $quiz_term['weight'],
          'max_score' => $quiz_term['max_score'],
          'number'    => $quiz_term['number'],
        ));
      }
      $insert_query->execute();
    }
  }

}
