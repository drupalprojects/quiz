<?php

namespace Drupal\quiz\Controller\Admin;

use Drupal\quiz\Entity\QuizEntity;

class QuizRevisionsAdminController {

  public static function staticCallback(QuizEntity $quiz) {
    return '…';
  }

}
