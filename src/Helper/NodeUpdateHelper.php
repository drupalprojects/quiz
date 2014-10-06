<?php

namespace Drupal\quiz\Helper;

class NodeUpdateHelper {

  public function execute($node) {
    // Quiz node vid (revision) was updated.
    if (isset($node->revision) && $node->revision) {
      // Create new quiz-question relation entries in the quiz_node_relationship
      // table.
      quiz_update_quiz_question_relationship($node->old_vid, $node->vid, $node->nid);
    }

    // Update an existing row in the quiz_node_properties table.
    _quiz_common_presave_actions($node);

    quiz_update_defaults($node);
    _quiz_update_resultoptions($node);

    _quiz_check_num_random($node);
    _quiz_check_num_always($node);
    quiz_update_max_score_properties(array($node->vid));
    drupal_set_message(t('Some of the updated settings may not apply to quiz being taken already. To see all changes in action you need to start again.'), 'warning');
  }

}
