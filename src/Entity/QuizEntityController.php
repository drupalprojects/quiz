<?php

namespace Drupal\quiz\Entity;

use EntityAPIController;
use stdClass;

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

  public function load($ids = array(), $conditions = array()) {
    $entities = parent::load($ids, $conditions);

    // quiz_entity_revision.review_options => serialize = TRUE already, not sure
    // why it's string here
    foreach ($entities as $entity) {
      if (!empty($entity->review_options) && is_string($entity->review_options)) {
        $entity->review_options = unserialize($entity->review_options);
      }
    }

    return $entities;
  }

  /**
   * Force save revision author ID.
   *
   * @global stdClass $user
   * @param QuizEntity $entity
   */
  protected function saveRevision($entity) {
    global $user;
    $entity->revision_uid = $user->uid;
    return parent::saveRevision($entity);
  }

}
