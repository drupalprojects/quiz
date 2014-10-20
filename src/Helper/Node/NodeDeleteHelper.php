<?php

namespace Drupal\quiz\Helper\Node;

class NodeDeleteHelper {

  public function execute($node) {
    $result = db_query('SELECT result_id FROM {quiz_results} WHERE nid = :nid', array(':nid' => $node->nid));
    $result_ids = array();
    while ($result_id = $result->fetchField()) {
      $result_ids[] = $result_id;
    }
    quiz()->getQuizHelper()->getResultHelper()->deleteByIds($result_ids);

    // Remove quiz node records from table quiz_node_properties
    db_delete('quiz_node_properties')
      ->condition('nid', $node->nid)
      ->execute();
    // Remove quiz node records from table quiz_relationship
    db_delete('quiz_relationship')
      ->condition('quiz_qid', $node->nid)
      ->execute();
    // Remove quiz node records from table quiz_results
    db_delete('quiz_results')
      ->condition('nid', $node->nid)
      ->execute();
    // Remove quiz node records from table quiz_result_options
    db_delete('quiz_result_options')
      ->condition('nid', $node->nid)
      ->execute();
  }

}
