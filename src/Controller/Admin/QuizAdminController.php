<?php

namespace Drupal\quiz\Controller\Admin;

use stdClass;

class QuizAdminController {

  /**
   * Main callback for admin/quiz/settings/config
   */
  public static function staticFormCallback($form, &$form_state) {
    $controller = new static();
    return $controller->getForm($form, $form_state);
  }

  /**
   * Renders the quiz node form for the admin pages
   *
   * This form is used to configure default values for the quiz node form
   */
  public function getForm($form, &$form_state) {
    // Create a dummy node to use as input for quiz_form
    $dummy_node = new stdClass();
    // def_uid is the uid of the default user holding the default values for the node form(no real user with this uid exists)
    $dummy_node->def_uid = variable_get('quiz_def_uid', 1);
    $settings = $this->loadUserSettings();
    $settings += (array) quiz()->getQuizHelper()->getSettingHelper()->getQuizDefaultSettings();
    foreach ($settings as $key => $value) {
      if (!isset($dummy_node->$key)) {
        $dummy_node->{$key} = $value;
      }
    }
    $form = quiz_form($dummy_node, $form_state);
    $form['direction'] = array(
      '#markup' => t('Here you can change the default quiz settings for new users.'),
      '#weight' => -10,
    );
    // unset values we can't or won't let the user edit default values for
    unset(
      $form['#quiz_check_revision_access'], $form['title'], $form['body_field'], $form['taking']['aid'], $form['taking']['addons'], $form['quiz_availability']['quiz_open'], $form['quiz_availability']['quiz_close'], $form['resultoptions'], $form['number_of_random_questions']
    );

    $form['remember_settings']['#type'] = 'value';
    $form['remember_settings']['#default_value'] = TRUE;

    $form['submit'] = array(
      '#type'   => 'submit',
      '#value'  => t('Save'),
      '#submit' => array(array($this, 'submit')),
    );

    $form['#validate'][] = array($this, 'validate');
    $form['#submit'][] = array($this, 'submit');

    return $form;
  }

  /**
   * Validation function for the quiz_admin_node_form form
   */
  public function validate($form, &$form_state) {
    // Create dummy node for quiz_validate
    $dummy_node = new stdClass();
    foreach ($form_state['values'] as $key => $value) {
      $dummy_node->{$key} = $value;
    }
    $dummy_node->resultoptions = array();

    // We use quiz_validate to validate the default values
    quiz_validate($dummy_node);
  }

  /**
   * Submit function for quiz_admin_node_form
   *
   * The default values are saved as the user settings for the "default user"
   * The default user is created when quiz is installed. He has a unique uid, but doesn't exist
   * as a real user.
   *
   * Why?
   * Default user settings can be loaded and saved using the same code and
   * database tables as any other user settings, making the code a lot easier to maintain.
   * Ref: http://en.wikipedia.org/wiki/Don%27t_repeat_yourself
   */
  function submit($form, &$form_state) {
    // We add the uid for the "default user"
    $form_state['values']['save_def_uid'] = variable_get('quiz_def_uid', NULL);
    $form_state['values']['nid'] = 0;
    $form_state['values']['vid'] = 0;
    $form_state['values']['aid'] = '';
    $this->saveUserSettings($form_state['values']);
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
   *
   * @see https://www.drupal.org/node/2353181
   */
  private function loadUserSettings($uid = NULL) {
    // The def_uid property is the default user id. It is used if there are no
    // settings store for the current user.
    $uid = isset($uid) ? $uid : $GLOBALS['user']->uid;

    $query = db_select('quiz_user_settings', 'qus')
      ->fields('qus')
      ->condition('uid', $uid);
    $res = $query->execute()->fetchAssoc();
    if (!empty($res)) {
      foreach ($res as $key => $value) {
        if (!in_array($key, array('nid', 'vid', 'uid'))) {
          $settings[$key] = $value;
        }
      }
      $settings['resultoptions'][] = db_select('quiz_node_result_options', 'qnro')
        ->fields('qnro')
        ->condition('nid', $res['nid'])
        ->condition('vid', $res['vid'])
        ->execute()
        ->fetchAll();
      return $settings;
    }
    return array();
  }

  /**
   * This is copied from _quiz_save_user_settings() in previous revision.
   */
  private function saveUserSettings($node) {
    global $user;
    $node = (object) $node;
    // We do not save settings if the node has been created by the system,
    // or if the user haven't requested it
    if (isset($node->auto_created) || !isset($node->remember_settings) || !$node->remember_settings) {
      return FALSE;
    }

    $summary_pass_format = filter_fallback_format();
    if (isset($node->summary_pass['format']) && !empty($node->summary_pass['format'])) {
      $summary_pass_format = $node->summary_pass['format'];
    }

    $summary_default_format = filter_fallback_format();
    if (isset($node->summary_default['format']) && !empty($node->summary_default['format'])) {
      $summary_default_format = $node->summary_default['format'];
    }

    db_merge('quiz_user_settings')
      ->key(array('uid' => $user->uid))
      ->fields(array(
        'uid'                    => isset($node->uid) ? $node->uid : $node->save_def_uid,
        'nid'                    => $node->nid,
        'vid'                    => $node->vid,
        'aid'                    => isset($node->aid) ? $node->aid : 0,
        'pass_rate'              => $node->pass_rate,
        'summary_pass'           => isset($node->summary_pass['value']) ? $node->summary_pass['value'] : '',
        'summary_pass_format'    => $summary_pass_format,
        'summary_default'        => $node->summary_default['value'],
        'summary_default_format' => $summary_default_format,
        'randomization'          => $node->randomization,
        'backwards_navigation'   => $node->backwards_navigation,
        'keep_results'           => $node->keep_results,
        'repeat_until_correct'   => $node->repeat_until_correct,
        'feedback_time'          => $node->feedback_time,
        'display_feedback'       => $node->display_feedback,
        'takes'                  => $node->takes,
        'show_attempt_stats'     => $node->show_attempt_stats,
        'time_limit'             => isset($node->time_limit) ? $node->time_limit : 0,
        'quiz_always'            => $node->quiz_always,
        'has_userpoints'         => isset($node->has_userpoints) ? $node->has_userpoints : 0,
        'allow_skipping'         => $node->allow_skipping,
        'allow_resume'           => $node->allow_resume,
        'allow_jumping'          => $node->allow_jumping,
        'show_passed'            => $node->show_passed,
      ))->execute();
    drupal_set_message(t('Default settings have been saved'));
  }

}
