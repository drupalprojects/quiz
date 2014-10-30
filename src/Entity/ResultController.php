<?php

namespace Drupal\quiz\Entity;

use DatabaseTransaction;
use EntityAPIController;

class ResultController extends EntityAPIController {

  public function delete($ids, DatabaseTransaction $transaction = NULL) {
    $return = parent::delete($ids, $transaction);

    $select = db_select('quiz_results_answers', 'answer');
    $select->fields('answer', array('result_id', 'question_nid', 'question_vid'));
    $select->condition('answer.result_id', $ids);
    $result = $select->execute();
    while ($record = $result->fetchAll()) {
      quiz_question_delete_result($record->result_id, $record->question_nid, $record->question_vid);
    }

    db_delete('quiz_results_answers')->condition('result_id', $ids)->execute();
    db_delete('quiz_results')->condition('result_id', $ids)->execute();

    return $return;
  }

}
