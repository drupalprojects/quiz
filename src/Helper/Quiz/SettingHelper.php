<?php

namespace Drupal\quiz\Helper\Quiz;

use Drupal\quiz\Helper\FormHelper;

class SettingHelper extends FormHelper {

  /**
   * Returns an array with quiz titles keyed with quiz ids.
   *
   * @return array
   *   Options suitable for a form, in which the value is nid.
   */
  public function getQuizOptions() {
    $options = array();
    $rows = db_query('SELECT q.qid, q.title FROM {quiz_entity} q WHERE 1');
    foreach ($rows as $quiz_row) {
      $options[$quiz_row->qid] = drupal_substr(check_plain($quiz_row->title), 0, 30);
    }
    return $options;
  }

  /**
   * Returns the users default settings.
   *
   * @param $node
   *   Quiz node.
   * @param $uid
   *   (optional) The uid of the user to get the settings for. Defaults to the
   *   current user (NULL).
   *
   * @return
   *   An array of settings. The array is empty in case no settings are available.
   */
  public function getUserDefaultSettings($legacy = TRUE) {
    global $user;

    if ($legacy && $entity = entity_load('quiz', FALSE, array('uid' => $user->uid, 'nid' => 0, 'vid' => 0))) {
      // We found user defaults.
      $defaults = reset($entity);
      unset($defaults->nid, $defaults->uid, $defaults->vid);
      return $defaults;
    }

    if ($legacy && $entity = entity_load('quiz', FALSE, array('uid' => 0, 'nid' => 0, 'vid' => 0))) {
      // Found global defaults.
      $defaults = reset($entity);
      unset($defaults->nid, $defaults->uid, $defaults->vid);
      return $defaults;
    }

    // No defaults set yet.
    return $this->getQuizDefaultSettings();
  }

  public function updateUserDefaultSettings($node) {
    global $user;

    $quiz = clone $node;
    $quiz->aid = !empty($quiz->aid) ? $quiz->aid : 0;
    $quiz->summary_pass = is_array($node->summary_pass) ? $node->summary_pass['value'] : $node->summary_pass;
    $quiz->summary_pass_format = is_array($node->summary_pass) ? $node->summary_pass['format'] : $node->summary_pass_format;
    $quiz->summary_default = is_array($node->summary_default) ? $node->summary_default['value'] : $node->summary_default;
    $quiz->summary_default_format = is_array($node->summary_default) ? $node->summary_default['format'] : $node->summary_default_format;
    $quiz->tid = isset($quiz->tid) ? $quiz->tid : 0;

    // Save the node values.
    $quiz_props = clone $quiz;
    $quiz_props->uid = 0;
    $this->saveQuizSettings($quiz_props);

    if (!empty($node->remember_settings)) {
      // Save user defaults.
      $user_defaults = clone $quiz_props;
      $user_defaults->nid = 0;
      $user_defaults->vid = 0;
      $user_defaults->uid = $user->uid;
      $this->saveQuizSettings($user_defaults);
    }

    if (!empty($node->remember_global)) {
      // Save global defaults.
      $global_defaults = clone $quiz_props;
      $global_defaults->uid = 0;
      $global_defaults->nid = 0;
      $global_defaults->vid = 0;
      return $this->saveQuizSettings($global_defaults);
    }
  }

  /**
   * Insert or update the quiz node properties accordingly.
   */
  public function saveQuizSettings($entity) {
    $sql = "SELECT qnp_id
      FROM {quiz_node_properties}
      WHERE (nid = :nid AND nid > 0 AND vid = :vid AND vid > 0)
        OR (uid = :uid and uid > 0)
        OR (nid = :nid and uid = :uid and vid = :vid)";
    $result = db_query($sql, array(':nid' => $entity->nid, ':uid' => $entity->uid, ':vid' => $entity->vid));
    $entity->qnp_id = $result->fetchField();
    return entity_save('quiz', $entity);
  }

  /**
   * Returns default values for all quiz settings.
   *
   * @todo also store this in the quiz_node_properties table
   *
   * @return
   *   Array of default values.
   */
  public function getQuizDefaultSettings() {
    return (object) array(
          'aid'                        => NULL,
          'allow_jumping'              => 0,
          'allow_resume'               => 1,
          'allow_skipping'             => 1,
          'always_available'           => TRUE,
          'backwards_navigation'       => 1,
          'has_userpoints'             => 0,
          'keep_results'               => 2,
          'mark_doubtful'              => 0,
          'max_score'                  => 0,
          'max_score_for_random'       => 1,
          'number_of_random_questions' => 0,
          'pass_rate'                  => 75,
          'quiz_always'                => 1,
          'quiz_close'                 => 0,
          'quiz_close'                 => $this->prepareDate(NULL, variable_get('quiz_default_close', 30)),
          'quiz_open'                  => 0,
          'quiz_open'                  => $this->prepareDate(),
          'randomization'              => 0,
          'repeat_until_correct'       => 0,
          'review_options'             => array('question' => array(), 'end' => array()),
          'show_attempt_stats'         => 1,
          'show_passed'                => 1,
          'summary_default'            => '',
          'summary_default_format'     => filter_fallback_format(),
          'summary_pass'               => '',
          'summary_pass_format'        => filter_fallback_format(),
          'takes'                      => 0,
          'tid'                        => 0,
          'time_limit'                 => 0,
          'userpoints_tid'             => 0,
    );
  }

}
