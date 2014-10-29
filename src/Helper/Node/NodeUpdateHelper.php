<?php

namespace Drupal\quiz\Helper\Node;

use PDO;

class NodeUpdateHelper extends NodeHelper {

  public function execute($quiz) {
    // Quiz node vid (revision) was updated.
    if (isset($quiz->revision) && $quiz->revision) {
      // Create new quiz-question relation entries in the quiz_relationship table.
      $this->updateQuestionRelationship($quiz->old_vid, $quiz->vid, $quiz->qid);
    }

    $this->presaveActions($quiz);
    quiz()->getQuizHelper()->getSettingHelper()->updateUserDefaultSettings($quiz);
    $this->checkNumRandom($quiz);
    $this->checkNumAlways($quiz);
    quiz_update_max_score_properties(array($quiz->vid));
    drupal_set_message(t('Some of the updated settings may not apply to quiz being taken already. To see all changes in action you need to start again.'), 'warning');
  }

  /**
   * Copies quiz-question relation entries in the quiz_relationship table
   * from an old version of a quiz to a new.
   *
   * @param int $old_quiz_vid
   *   The quiz vid prior to a new revision.
   * @param int $new_quiz_vid
   *   The quiz vid of the latest revision.
   * @param int $quiz_qid
   *   The quiz id.
   */
  private function updateQuestionRelationship($old_quiz_vid, $new_quiz_vid, $quiz_qid) {
    // query for questions in previous version
    $result = db_select('quiz_relationship', 'qnr')
      ->fields('qnr', array('quiz_qid', 'question_nid', 'question_vid', 'question_status', 'weight', 'max_score', 'auto_update_max_score', 'qr_id', 'qr_pid'))
      ->condition('quiz_qid', $quiz_qid)
      ->condition('quiz_vid', $old_quiz_vid)
      ->condition('question_status', QUESTION_NEVER, '!=')
      ->execute();

    // only proceed if query returned data
    if ($result->rowCount()) {
      $questions = $result->fetchAll(PDO::FETCH_ASSOC);
      foreach ($questions as &$quiz_question) {
        $quiz_question['old_qr_id'] = $quiz_question['qr_id'];
        $quiz_question['quiz_qid'] = $quiz_qid;
        $quiz_question['quiz_vid'] = $new_quiz_vid;
        unset($quiz_question['qr_id']);
        drupal_write_record('quiz_relationship', $quiz_question);
      }

      // Update the parentage when a new revision is created.
      // @todo this is copy pasta from quiz_set_questions
      foreach ($questions as $question) {
        db_update('quiz_relationship')
          ->condition('qr_pid', $question['old_qr_id'])
          ->condition('quiz_qid', $quiz_qid)
          ->condition('quiz_vid', $new_quiz_vid)
          ->fields(array('qr_pid' => $question['qr_id']))
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
            'nid'       => $quiz_qid,
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
