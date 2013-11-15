<?php
/**
 * @file
 * Contains \Drupal\quiz\Form\QuizQuestionConfigForm.
 */

namespace Drupal\quiz_question\Form;

use Drupal\Core\Form\ConfigFormBase;

/**
 * Defines a form to configure maintenance settings for this site.
 */
class QuizQuestionConfig extends ConfigFormBase {

  /**
   * {@inheritdoc}
   */
  public function getFormID() {
    return 'quiz_question_config';
  }

  /**
   * {@inheritdoc}
   */
  public function buildForm(array $form, array &$form_state) {
    $q_types = _quiz_question_get_implementations();

    $form['#validate'] = array();
    $form['#submit'] = array();

    // Go through all question types and merge their config forms
    foreach ($q_types as $type => $values) {
      $function = $type . '_config';
      if ($admin_form = $function()) {
        $form[$type] = $admin_form;
        $form[$type]['#type'] = 'fieldset';
        $form[$type]['#title'] = $values['name'];
        $form[$type]['#collapsible'] = TRUE;
        $form[$type]['#collapsed'] = TRUE;
        if (isset($admin_form['#validate'])) {
          $form['#validate'] = array_merge($form['#validate'], $admin_form['#validate']);
          unset($form[$type]['#validate']);
        }
        if (isset($admin_form['#submit'])) {
          $form['#submit'] = array_merge($form['#submit'], $admin_form['#submit']);
          unset($form[$type]['#submit']);
        }
      }
    }
    $form['#submit'][] = 'quiz_question_config_submit';
    return parent::buildForm($form, $form_state);
  }
}