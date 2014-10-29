<?php

namespace Drupal\quiz\Controller\Admin;

use Drupal\quiz\Entity\QuizEntity;
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
    $dummy_quiz = new stdClass();
    // def_uid is the uid of the default user holding the default values for the node form(no real user with this uid exists)
    foreach (quiz_get_defaults() as $key => $value) {
      if (!isset($dummy_quiz->$key)) {
        $dummy_quiz->{$key} = $value;
      }
    }

    $form = quiz_form($dummy_quiz, $form_state);
    $form['direction'] = array(
        '#markup' => t('Here you can change the default quiz settings for new users.'),
        '#weight' => -10,
    );

    // unset values we can't or won't let the user edit default values for
    unset(
      $form['#quiz_check_revision_access'], $form['title'], $form['body_field'], $form['taking']['aid'], $form['taking']['addons'], $form['quiz_availability']['quiz_open'], $form['quiz_availability']['quiz_close'], $form['resultoptions'], $form['number_of_random_questions'], $form['#quiz_check_revision_access'], $form['title'], $form['body_field'], $form['taking']['aid'], $form['taking']['addons'], $form['quiz_availability']['quiz_open'], $form['quiz_availability']['quiz_close'], $form['resultoptions'], $form['number_of_random_questions'], $form['remember_global']
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
    // Generate the node object:
    $quiz = (object) $form_state['values'];
    $quiz->qid = 0;
    $quiz->vid = 0;
    $quiz->uid = 0;
    quiz()->getQuizHelper()->getSettingHelper()->updateUserDefaultSettings($quiz);
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
    global $user;

    // The def_uid property is the default user id. It is used if there are no
    // settings store for the current user.
    $query = db_select('quiz_user_settings', 'qus')
      ->fields('qus')
      ->condition('uid', isset($uid) ? $uid : $user->uid);

    if (!$res = $query->execute()->fetchAssoc()) {
      foreach ($res as $key => $value) {
        if (!in_array($key, array('nid', 'vid', 'uid'))) {
          $settings[$key] = $value;
        }
      }

      $settings['resultoptions'][] = db_select('quiz_result_options', 'qnro')
        ->fields('qnro')
        ->condition('quiz_qid', $res['nid'])
        ->condition('quiz_vid', $res['vid'])
        ->execute()
        ->fetchAll();
      return $settings;
    }
    return array();
  }

}
