<?php

namespace Drupal\quiz\Controller\Admin;

use Drupal\quiz\Entity\QuizEntity;

class QuizResultsAdminController {

  public static function staticCallback(QuizEntity $quiz) {
    global $user;
    return views_embed_view('quiz_results', 'page', $quiz->qid, $user->uid);
  }

}
