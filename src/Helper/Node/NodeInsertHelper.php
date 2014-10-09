<?php

namespace Drupal\quiz\Helper\Node;

class NodeInsertHelper extends NodeHelper {

  public function execute($quiz) {
    global $user;

    // Need to set max_score if this is a cloned node
    $max_score = 0;

    // Copy all the questions belonging to the quiz if this is a new translation.
    if ($quiz->is_new && isset($quiz->translation_source)) {
      quiz_copy_questions($quiz);
    }

    // Add references to all the questions belonging to the quiz if this is a cloned quiz (node_clone compatibility)
    if ($quiz->is_new && isset($quiz->clone_from_original_nid)) {
      $old_quiz = node_load($quiz->clone_from_original_nid, NULL, TRUE);
      $max_score = $old_quiz->max_score;
      $questions = quiz()->getQuizHelper()->getQuestions($old_quiz->nid, $old_quiz->vid);

      // Format the current questions for referencing
      foreach ($questions as $question) {
        $nid = $questions['nid'];
        $questions[$nid]->state = $question->question_status;
        $questions[$nid]->refresh = 0;
      }

      quiz()->getQuizHelper()->setQuestions($quiz, $questions);
    }

    $this->presaveActions($quiz);

    // If the quiz is saved as not randomized we have to make sure that questions belonging to the quiz are saved as not random
    _quiz_check_num_random($quiz);
    _quiz_check_num_always($quiz);

    quiz_update_defaults($quiz);
    $this->insertResultOptions($quiz);
  }

}
