<?php

namespace Drupal\quiz\Entity;

use EntityAPIController;

class QuizEntityController extends EntityAPIController {

  public function buildContent($entity, $view_mode = 'full', $langcode = NULL, $content = array()) {
    $extra_fields = field_extra_fields_get_display($this->entityType, $entity->type, $view_mode);

    // Render Stats
    if ($extra_fields['stats']['visible']) {
      $content['quiz_entity'][$entity->qid]['stats'] = array(
        '#markup' => @theme('quiz_view_stats', array('node' => $entity)),
        '#weight' => $extra_fields['stats']['weight'],
      );
    }

    // Render take button
    if ($extra_fields['take']['visible']) {
      $content['quiz_entity'][$entity->qid]['take'] = array(
        '#prefix' => '<div class="quiz-not-available">',
        '#suffix' => '</div>',
        '#markup' => 'quiz()->getQuizHelper()->isAvailable($quiz)',
        '#weight' => $extra_fields['take']['weight'],
      );
    }

    return parent::buildContent($entity, $view_mode, $langcode, $content);
  }

}
