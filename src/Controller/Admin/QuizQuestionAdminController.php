<?php

namespace Drupal\quiz\Controller\Admin;

use Drupal\quiz\Entity\QuizEntity;

class QuizQuestionAdminController {

  public static function staticCallback(QuizEntity $quiz) {
    return '…';
  }

}
