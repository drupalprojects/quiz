<?php

namespace Drupal\quiz\Entity;

use EntityAPIController;

class QuizEntityController extends EntityAPIController {

  public function buildContent($entity, $view_mode = 'full', $langcode = NULL, $content = array()) {
    // Render Stats
    $content['quiz_entity'][$entity->qid]['stats'] = array(
      '#markup' => @theme('quiz_view_stats', array('node' => $entity)),
    );

    // Render take button
    $content['quiz_entity'][$entity->qid]['take'] = array(
      '#prefix' => '<div class="quiz-not-available">',
      '#suffix' => '</div>',
      '#markup' => 'quiz()->getQuizHelper()->isAvailable($quiz)',
    );

    return parent::buildContent($entity, $view_mode, $langcode, $content);
  }

}
