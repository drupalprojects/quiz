<?php

namespace Drupal\quiz\Helper\Node;

abstract class NodeHelper {

  /**
   * Insert call specific to result options.
   *
   * This is called by quiz_insert().
   *
   * @param $node
   *   The quiz node.
   */
  protected function insertResultOptions($quiz) {
    if (!isset($quiz->resultoptions)) {
      return;
    }

    $query = db_insert('quiz_node_result_options')
      ->fields(array('nid', 'vid', 'option_name', 'option_summary', 'option_summary_format', 'option_start', 'option_end'));

    foreach ($quiz->resultoptions as $id => $option) {
      if (!empty($option['option_name'])) {
        // When this function called direct from node form submit the $option['option_summary']['value'] and $option['option_summary']['format'] are we need
        // But when updating a quiz node eg. on manage questions page, this values come from loaded node, not from a submitted form.
        if (is_array($option['option_summary'])) {
          $option['option_summary_format'] = $option['option_summary']['format'];
          $option['option_summary'] = $option['option_summary']['value'];
        }
        $query->values(array(
          'nid' => $quiz->nid,
          'vid' => $quiz->vid,
          'option_name' => $option['option_name'],
          'option_summary' => $option['option_summary'],
          'option_summary_format' => $option['option_summary_format'],
          'option_start' => $option['option_start'],
          'option_end' => $option['option_end']
        ));
      }
    }

    $query->execute();
  }

}
