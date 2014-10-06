<?php

namespace Drupal\quiz\Helper;

class NodePresaveHelper {

  public function execute($node) {
    if ($node->type == 'quiz') {
      // Convert the action id to the actual id from the MD5 hash.
      // Why the actions module does this I do not know? Maybe to prevent
      // invalid values put into the options value="" field.
      if (!empty($node->aid) && $aid = actions_function_lookup($node->aid)) {
        $node->aid = $aid;
      }

      if (variable_get('quiz_auto_revisioning', 1)) {
        $node->revision = (quiz_has_been_answered($node)) ? 1 : 0;
      }

      // If this is a programmatic save, ensure we use the defaults.
      $defaults = quiz_get_defaults();
      foreach ($defaults as $property => $value) {
        if (!isset($node->$property)) {
          $node->$property = $defaults->$property;
        }
      }
    }

    if (isset($node->is_quiz_question) && variable_get('quiz_auto_revisioning', 1)) {
      $node->revision = (quiz_question_has_been_answered($node)) ? 1 : 0;
    }
  }

}
