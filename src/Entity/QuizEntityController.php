<?php

namespace Drupal\quiz\Entity;

use DatabaseTransaction;
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
      $vids[] = $entity->vid;
      if (!empty($entity->review_options) && is_string($entity->review_options)) {
        $entity->review_options = unserialize($entity->review_options);
      }
    }

    if (!empty($vids)) {
      $result_options = db_select('quiz_result_options', 'ro')
        ->fields('ro')
        ->condition('ro.vid', $vids)
        ->execute();
      foreach ($result_options->fetchAll() as $result_option) {
        $entities[$result_option->nid]->resultoptions[] = (array) $result_option;
      }
    }

    return $entities;
  }

  public function save($quiz, DatabaseTransaction $transaction = NULL) {
    // QuizFeedbackTest::testFeedback() failed without this, mess!
    if (empty($quiz->is_new_revision)) {
      $quiz->is_new = $quiz->revision = 0;
    }

    if ($return = parent::save($quiz, $transaction)) {
      $this->saveResultOptions($quiz);
      return $return;
    }
  }

  private function saveResultOptions(QuizEntity $quiz) {
    db_delete('quiz_result_options')
      ->condition('vid', $quiz->vid)
      ->execute();

    $query = db_insert('quiz_result_options')
      ->fields(array('nid', 'vid', 'option_name', 'option_summary', 'option_summary_format', 'option_start', 'option_end'));

    foreach ($quiz->resultoptions as $option) {
      if (empty($option['option_name'])) {
        continue;
      }

      // When this function called direct from node form submit the
      // $option['option_summary']['value'] and $option['option_summary']['format'] are we need
      // But when updating a quiz node eg. on manage questions page, this values
      // come from loaded node, not from a submitted form.
      if (is_array($option['option_summary'])) {
        $option['option_summary_format'] = $option['option_summary']['format'];
        $option['option_summary'] = $option['option_summary']['value'];
      }

      $query->values(array(
        'nid'                   => $quiz->qid,
        'vid'                   => $quiz->vid,
        'option_name'           => $option['option_name'],
        'option_summary'        => $option['option_summary'],
        'option_summary_format' => $option['option_summary_format'],
        'option_start'          => $option['option_start'],
        'option_end'            => $option['option_end']
      ));
    }

    $query->execute();
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
