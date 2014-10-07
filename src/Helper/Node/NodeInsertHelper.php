<?php

namespace Drupal\quiz\Helper\Node;

class NodeInsertHelper {

  public function execute($node) {
    global $user;

    // Need to set max_score if this is a cloned node
    $max_score = 0;

    // Copy all the questions belonging to the quiz if this is a new translation.
    if ($node->is_new && isset($node->translation_source)) {
      quiz_copy_questions($node);
    }

    // Add references to all the questions belonging to the quiz if this is a cloned quiz (node_clone compatibility)
    if ($node->is_new && isset($node->clone_from_original_nid)) {
      $old_quiz = node_load($node->clone_from_original_nid, NULL, TRUE);

      $max_score = $old_quiz->max_score;

      $questions = quiz_get_questions($old_quiz->nid, $old_quiz->vid);

      // Format the current questions for referencing
      foreach ($questions as $question) {
        $nid = $questions['nid'];
        $questions[$nid]->state = $question->question_status;
        $questions[$nid]->refresh = 0;
      }

      quiz_set_questions($node, $questions);
    }

    _quiz_common_presave_actions($node);

    // If the quiz is saved as not randomized we have to make sure that questions belonging to the quiz are saved as not random
    _quiz_check_num_random($node);
    _quiz_check_num_always($node);

    quiz_update_defaults($node);
    _quiz_insert_resultoptions($node);
  }

}
