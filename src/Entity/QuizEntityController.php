<?php

namespace Drupal\quiz\Entity;

use EntityAPIController;

class QuizEntityController extends EntityAPIController {

  /**
   * @param QuizEntity $quiz
   */
  public function buildContent($quiz, $view_mode = 'full', $langcode = NULL, $content = array()) {
    $extra_fields = field_extra_fields_get_display($this->entityType, $quiz->type, $view_mode);

    // Render Stats
    if ($extra_fields['stats']['visible']) {
      $content['quiz_entity'][$quiz->qid]['stats'] = array(
        '#markup' => theme('quiz_view_stats', array('quiz' => $quiz)),
        '#weight' => $extra_fields['stats']['weight'],
      );
    }

    // Render take button
    if ($extra_fields['take']['visible']) {
      $content['quiz_entity'][$quiz->qid]['take'] = array(
        '#prefix' => '<div class="quiz-not-available">',
        '#suffix' => '</div>',
        '#access' => quiz()->getQuizHelper()->isAvailable($quiz),
        '#weight' => $extra_fields['take']['weight'],
        '#markup' => l(t('Start quiz'), 'quiz/' . $quiz->qid . '/take'),
      );
    }

    return parent::buildContent($quiz, $view_mode, $langcode, $content);
  }

}
