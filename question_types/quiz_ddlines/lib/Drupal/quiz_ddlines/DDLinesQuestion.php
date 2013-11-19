<?php


/**
 * The main classes for the quiz_ddlines question type.
 *
 * These inherit or implement code found in quiz_question.classes.inc.
 *
 * Sponsored by: Norwegian Centre for Telemedicine
 * Code: falcon
 *
 * Based on:
 * Other question types in the quiz framework.
 *
 *
 *
 * @file
 * Question type, enabling the creation of multiple choice and multiple answer questions.
 * Contains \Drupal\multichoice\MultichoiceQuestion.
 */

namespace Drupal\quiz_ddlines;

use Drupal\quiz_question\QuizQuestion;

/**
 * Extension of QuizQuestion.
 */
class DDLinesQuestion extends QuizQuestion {
  /**
   * Get the form used to create a new question.
   *
   * @param
   *  FAPI form state
   * @return
   *  Must return a FAPI array.
   */

  public function getCreationForm(array &$form_state = NULL) {

    $elements = '';
    if(isset($this->node->translation_source)) {
      $elements = $this->node->translation_source->ddlines_elements;
    }
    elseif(isset($this->node->ddlines_elements)) {
      $elements = $this->node->ddlines_elements;
    }

    $form['ddlines_elements'] = array(
      '#type' => 'hidden',
      '#default_value' => $elements,
    );

    $default_settings = $this->getDefaultAltSettings();

    $form['settings'] = array(
      '#type' => 'fieldset',
      '#title' => t('Settings'),
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
      '#weight' => -3,
    );
    $form['settings']['feedback_enabled'] = array(
      '#type' => 'checkbox',
      '#title' => t('Enable feedback'),
      '#description' => t('When taking the test, and this option is enabled, a wrong placement of an alternative, will make it jump back. Also, this makes it possible to add comments to both correct and wrong answers.'),
      '#default_value' => isset($this->node->translation_source) ? $this->node->translation_source->feedback_enabled : $default_settings['feedback']['enabled'],
      '#parents' => array('feedback_enabled'),
    );
    $form['settings']['hotspot_radius'] = array(
      '#type' => 'textfield',
      '#title' => t('Hotspot radius'),
      '#description' => t('The radius of the hotspot in pixels'),
      '#default_value' => isset($this->node->translation_source) ? $this->node->translation_source->hotspot_radius : $default_settings['hotspot']['radius'],
      '#parents' => array('hotspot_radius'),
    );

    $form['settings']['execution_mode'] = array(
      '#type' => 'radios',
      '#title' => t('Execution mode'),
      '#description' => t('The mode for taking the test.'),
      '#default_value' => isset($this->node->translation_source) ? $this->node->translation_source->execution_mode : $default_settings['execution_mode'],
      '#options' => array (
        0 => t('With lines'),
        1 => t('Drag label'),
      ),
      '#parents' => array('execution_mode'),
    );

    drupal_add_library('system','ui.resizable');

    $default_settings['mode'] = 'edit';
    $default_settings['editmode'] = ($this->node->id()) ? 'update' : 'add';

    drupal_add_js(array('quiz_ddlines' => $default_settings), 'setting');
    _quiz_ddlines_add_js_and_css();

    return $form;
  }

  /**
   * This makes max_score beeing updated for all occurrences of
   * this question in quizzes.
   */
  protected function autoUpdateMaxScore() {
    return true;
  }

  /**
   * Helper function provding the default settings for the creation form.
   *
   * @return
   *  Array with the default settings
   */
  private function getDefaultAltSettings() {
    $settings = array();
    $config = \Drupal::config('quiz_ddlines.settings');

    // If the node exists, use saved value
    if ($this->node->id()) {
      $settings['feedback']['enabled'] = $this->node->feedback_enabled;
      $settings['hotspot']['radius'] = $this->node->hotspot_radius;
      $settings['execution_mode'] = $this->node->execution_mode;
    }
    else {
      $settings['feedback']['enabled'] = 0;
      $settings['hotspot']['radius'] = ($config->get('quiz_ddlines_hotspot_radius')) ? : Defaults::HOTSPOT_RADIUS;
      $settings['execution_mode'] = 0;
    }

    // Pick these from settings:
    $settings['feedback']['correct'] = ($config->get('quiz_ddlines_feedback_correct'))? : t('Correct');
    $settings['feedback']['wrong'] = ($config->get('quiz_ddlines_feedback_wrong')) ? : t('Wrong');
    $settings['canvas']['width'] = ($config->get('quiz_ddlines_canvas_width')) ? : Defaults::CANVAS_WIDTH;
    $settings['canvas']['height'] = ($config->get('quiz_ddlines_canvas_height')) ? : Defaults::CANVAS_HEIGHT;
    $settings['pointer']['radius'] = ($config->get('quiz_ddlines_pointer_radius')) ? : Defaults::POINTER_RADIUS;

    return $settings;
  }

  /**
   * Generates the question form.
   *
   * This is called whenever a question is rendered, either
   * to an administrator or to a quiz taker.
   */
  public function getAnsweringForm(array $form_state = NULL, $rid) {

    $default_settings = $this->getDefaultAltSettings();
    $default_settings['mode'] = 'take';
    drupal_add_js(array('quiz_ddlines' => $default_settings), 'setting');

    $form = parent::getAnsweringForm($form_state, $rid);

    $form['helptext'] = array(
      '#markup' => t('Answer this question by dragging each rectangular label to the correct circular hotspot.'),
      '#weight' => 0,
    );

    // Form element containing the correct answers
    $form['ddlines_elements'] = array(
      '#type' => 'hidden',
      '#default_value' => isset($this->node->ddlines_elements) ? $this->node->ddlines_elements : '',
    );

    // Form element containing the user answers
    // The quiz module requires this element to be named "tries":
    $form['tries'] = array (
      '#type' => 'hidden',
      '#default_value' => '',
    );

    $field_image = $this->node->field_image->getValue();
    $uri = $field_image[0]['entity'];
    $image_uri = $uri->getFileUri();

    // Image Styling.
    $style = entity_load('image_style', 'large');
    $image_url = $style->buildUrl($image_uri);

    $form['image'] = array (
      '#markup' => '<div class="image-preview">'.theme('image', array('uri' => $image_url)).'</div>',
      /*'#weight' => 0,*/
    );
    _quiz_ddlines_add_js_and_css();

    return $form;
  }

  /**
   * Get the maximum possible score for this question.
   */
  public function getMaximumScore() {
    // 1 point per correct hotspot location
    $ddlines_elements = json_decode($this->node->ddlines_elements);

    $max_score = isset($ddlines_elements->elements) ? sizeof($ddlines_elements->elements) : 0;

    return $max_score;
  }

  /**
   * Save question type specific node properties
   */
  public function saveNodeProperties($is_new = FALSE) {
    if ($is_new || $this->node->isNewRevision()) {
      $id = db_insert('quiz_ddlines_node')
        ->fields(array(
          'nid' => $this->node->id(),
          'vid' => $this->node->getRevisionId(),
          'feedback_enabled' => $this->node->feedback_enabled,
          'hotspot_radius' => $this->node->hotspot_radius,
          'ddlines_elements' => $this->node->ddlines_elements,
          'execution_mode' => $this->node->execution_mode,
        ))
        ->execute();
    }
    else {
      db_update('quiz_ddlines_node')
        ->fields(array(
          'ddlines_elements' => $this->node->ddlines_elements,
          'hotspot_radius' => $this->node->hotspot_radius,
          'feedback_enabled' => $this->node->feedback_enabled,
          'execution_mode' => $this->node->execution_mode,
        ))
        ->condition('nid', $this->node->id())
        ->condition('vid', $this->node->getRevisionId())
        ->execute();
    }
  }

  /**
   * Implementation of getNodeProperties
   *
   * @see QuizQuestion#getNodeProperties()
   */
  public function getNodeProperties() {
    if (isset($this->nodeProperties) && !empty($this->nodeProperties)) {
      return $this->nodeProperties;
    }
    $props = parent::getNodeProperties();

    $res_a = db_query('SELECT feedback_enabled, hotspot_radius, ddlines_elements, execution_mode FROM {quiz_ddlines_node} WHERE nid = :nid AND vid = :vid', array(':nid' => $this->node->id(), ':vid' => $this->node->getRevisionId()))->fetchAssoc();

    if (is_array($res_a)) {
      $props = array_merge($props, $res_a);
    }
    $this->nodeProperties = $props;
    return $props;
  }

  /**
   * Provides validation for question before it is created.
   *
   * When a new question is created and initially submited, this is
   * called to validate that the settings are acceptible.
   *
   * @param $form
   *  The processed form.
   */
  public function validateNode(array &$form_state) {
    // Nothing todo here
  }

  /**
   * Implementation of entityBuilder
   */
  public function entityBuilder(&$form_state) {
    $this->node->feedback_enabled = $form_state['values']['feedback_enabled'];
    $this->node->hotspot_radius = $form_state['values']['hotspot_radius'];
    $this->node->ddlines_elements = $form_state['values']['ddlines_elements'];
    $this->node->execution_mode = $form_state['values']['execution_mode'];
    $this->node->add_directly = $form_state['values']['add_directly'];
  }

  /**
   * Implementation of delete
   *
   * @see QuizQuestion#delete()
   */
  public function delete($only_this_version = FALSE) {
    $delete_node = db_delete('quiz_ddlines_node')->condition('nid', $this->node->id());
    $delete_results = db_delete('quiz_ddlines_user_answers')->condition('question_nid', $this->node->id());

    if ($only_this_version) {
      $delete_node->condition('vid', $this->node->getRevisionId());
      $delete_results->condition('question_vid', $this->node->getRevisionId());
    }

    // Delete from table quiz_ddlines_user_answer_multi
    $user_answer_ids = array();
    if ($only_this_version) {
      $query = db_query('SELECT id FROM {quiz_ddlines_user_answers} WHERE question_nid = :nid AND question_vid = :vid', array(':nid' => $this->node->id(), ':vid' => $this->node->getRevisionId()));
    }
    else {
      $query = db_query('SELECT id FROM {quiz_ddlines_user_answers} WHERE question_nid = :nid', array(':nid' => $this->node->id()));
    }
    while ($user_answer = $query->fetch()) {
      $user_answer_ids[] = $user_answer->id;
    }

    if (count($user_answer_ids)) {
      db_delete('quiz_ddlines_user_answer_multi')
        ->condition('user_answer_id', $user_answer_ids, 'IN')
        ->execute();
    }

    $delete_node->execute();
    $delete_results->execute();
    parent::delete($only_this_version);
  }
}