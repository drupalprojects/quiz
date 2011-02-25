<?php

/**
 * @file
 * User page callbacks for the quiz_question module.
 */


/**
 * Menu callback. Prints a listing of containers that reference the question node nodes.
 *
 * @param $qid
 *   ID of question to look up
 * @return
 *   String containing the markup.
 */
function quiz_question_references($node) {
  $parents = _quiz_question_parents($node);

  if (empty($parents)) {
    return '<div>No references.</div>';
  } else {
    $output = '<div>';
    foreach ($parents as $parent)
      $output .= node_view($parent, true);
    $output .= '</div>';
    return $output;
  }
}

/**
 * @param $question_nid
 *
 * @return
 * a list of nodes that are parents to the question node
 */
function _quiz_question_parents($question_node) {
  $parents = array();
  // Get all the questions (ignore `question_status`)
  $sql = "SELECT DISTINCT parent_nid as nid
    FROM {quiz_node_relationship}
    WHERE child_vid = %d
      AND child_nid = %d
    ORDER BY weight";
  $result = db_query($sql, $question_node->vid, $question_node->nid);
  while ($parent_node_info = db_fetch_array($result)) { // nid, vid
    $parent_nodes[] = node_load($parent_node_info['nid']);
  }
  return $parent_nodes;
}
