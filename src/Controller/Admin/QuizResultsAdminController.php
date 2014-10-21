<?php

namespace Drupal\quiz\Controller\Admin;

use Drupal\quiz\Entity\QuizEntity;

class QuizResultsAdminController {

  public static function staticCallback(QuizEntity $quiz) {
    return '…';
  }

}
