<?php

class QuizResultController extends EntityAPIController {

  function delete($ids, \DatabaseTransaction $transaction = NULL) {
    parent::delete($ids, $transaction);
    foreach ($ids as $id) {
      // Fire the question types' implementations of question attempts deletion.
      $sql = 'SELECT result_id, question_nid, question_vid FROM {quiz_node_results_answers}
          WHERE result_id = (:result_id)';
      $result = db_query($sql, array(':result_id' => $id));
      foreach ($result as $record) {
        quiz_question_delete_result($id, $record->question_nid, $record->question_vid);
      }

      // Delete Quiz's records of any attempts at a question.
      db_delete('quiz_node_results_answers')
        ->condition('result_id', $id)
        ->execute();
    }
  }

  function load($ids = array(), $conditions = array()) {
    $quiz_results = parent::load($ids, $conditions);

    foreach ($quiz_results as $quiz_result) {
      $layout = entity_load('quiz_result_answer', FALSE, array('result_id' => $quiz_result->result_id));
      foreach ($layout as $question) {
        // @kludge
        // This is bulky but now we have to manually find the type and parents of
        // the question. This is the only information that is not stored in the
        // quiz attempt. We reference back to the node relationships for this
        // current version to get the hieararchy.
        $sql = "SELECT n.type, qnr_id, qnr_pid FROM {quiz_node_results} qr
        INNER JOIN {quiz_node_relationship} qnr on (qr.nid = qnr.parent_nid and qr.vid = qnr.parent_vid)
        INNER JOIN {node} n ON (qnr.child_nid = n.nid)
        WHERE qr.result_id = :result_id and n.nid = :nid";
        $result = db_query($sql, array(':result_id' => $quiz_result->result_id, ':nid' => $question->question_nid));
        $extra = $result->fetch();

        $quiz_result->layout[$question->number] = array(
          'display_number' => $question->number,
          'nid' => $question->question_nid,
          'vid' => $question->question_vid,
          'number' => $question->number,
          'type' => $extra->type,
          'qnr_id' => $extra->qnr_id,
          'qnr_pid' => $extra->qnr_pid,
        );
      }
      ksort($quiz_result->layout, SORT_NUMERIC);
    }
    return $quiz_results;
  }

}
