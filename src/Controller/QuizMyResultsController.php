<?php

namespace Drupal\quiz\Controller;

use Drupal\quiz\Entity\QuizEntity;

class QuizMyResultsController {

  public static function staticCallback(QuizEntity $quiz) {
    global $user;
    return views_embed_view('quiz_user_results', 'page', $quiz->qid, $user->uid);
  }

}
