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

  /**
   * Save the Quiz result and do any post-processing to the result.
   *
   * @param type $entity
   * @param \DatabaseTransaction $transaction
   * @return boolean
   */
  function save($entity, \DatabaseTransaction $transaction = NULL) {
    if (empty($entity->time_start)) {
      $entity->time_start = REQUEST_TIME;
    }

    $new = !empty($entity->is_new);

    // Save the Quiz result.
    parent::save($entity, $transaction);

    // Post process the result.
    if ($new) {
      $quiz = node_load($entity->nid, $entity->vid);

      // Call @deprecated hook_quiz_begin().
      module_invoke_all('quiz_begin', $quiz, $entity->result_id);


      // Create question list.
      $questions = quiz_build_question_list($quiz);
      if ($questions === FALSE) {
        drupal_set_message(t('Not enough random questions were found. Please add more questions before trying to take this @quiz.', array('@quiz' => QUIZ_NAME)), 'error');
        return FALSE;
      }

      if (count($questions) == 0) {
        drupal_set_message(t('No questions were found. Please !assign_questions before trying to take this @quiz.', array('@quiz' => QUIZ_NAME, '!assign_questions' => l(t('assign questions'), 'node/' . $quiz->nid . '/quiz/questions'))), 'error');
        return FALSE;
      }

      $i = 0;
      foreach ($questions as $question) {
        $quiz_result_answer = entity_create('quiz_result_answer', array(
          'result_id' => $entity->result_id,
          'question_nid' => $question['nid'],
          'question_vid' => $question['vid'],
          'number' => ++$i,
        ));
        entity_save('quiz_result_answer', $quiz_result_answer);
      }

      if (!empty($entity->build_on_last)) {
        $build_on_last = $entity->build_on_last;
        // @kludge we need to reload to get the layout loaded on.
        $entity = quiz_result_load($entity->result_id);
        $entity->build_on_last = $build_on_last;

        // Build on the last attempt the user took. If this quiz has build on last
        // attempt set, we need to search for a previous attempt with the same
        // version of the current quiz.
        $quiz_result_old = self::findOldResult($entity);
        // Now clone the answers on top of the new result.
        quiz_clone_quiz_result($quiz_result_old, $entity);
      }
    }
  }

  /**
   * Find a result that is not the same as the passed result.
   *
   * Note: the Quiz result does not have an actually exist - in that case, it
   * will return the first completed result found.
   */
  public static function findOldResult($quiz_result) {
    $efq = new EntityFieldQuery;
    $result = $efq->entityCondition('entity_type', 'quiz_result')
      ->propertyCondition('uid', $quiz_result->uid)
      ->propertyCondition('nid', $quiz_result->nid)
      ->propertyCondition('vid', $quiz_result->vid)
      ->propertyCondition('result_id', isset($quiz_result->result_id) ? $quiz_result->result_id : 0, '!=')
      ->propertyCondition('time_start', 0, '>')
      ->propertyOrderBy('time_start', 'DESC')
      ->range(0, 1)
      ->execute();
    if (!empty($result['quiz_result'])) {
      $old_quiz_result = reset($result['quiz_result']);
      return quiz_result_load($old_quiz_result->result_id);
    }
    return FALSE;
  }

}
