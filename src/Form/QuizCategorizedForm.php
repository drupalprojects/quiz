<?php

namespace Drupal\quiz\Form;

use Drupal\quiz\Controller\QuizQuestionManagementController;

class QuizCategorizedForm {

  public static function staticGet($form, $form_state, $quiz) {
    $obj = new static();
    return $obj->getForm($form, $form_state, $quiz);
  }

  /**
   * Form for managing what questions should be added to a quiz with categorized random questions.
   *
   * @param array $form_state
   *  The form state array
   * @param object $quiz
   *  The quiz node
   */
  public function getForm($form, $form_state, $quiz) {
    $form['#tree'] = TRUE;

    $form['#theme'] = 'quiz_categorized_form';
    $form['#validate'][] = array($this, 'formValidate');
    $form['#submit'][] = array($this, 'formSubmit');

    $this->existingTermsForm($form, $form_state, $quiz);
    $this->categorizedNewTermForm($form, $form_state, $quiz);

    $form['nid'] = array('#type' => 'value', '#value' => $quiz->nid);
    $form['vid'] = array('#type' => 'value', '#value' => $quiz->vid);
    $form['tid'] = array('#type' => 'value', '#value' => NULL);

    // Give the user the option to create a new revision of the quiz
    _quiz_add_revision_checkbox($form, $quiz);

    // Timestamp is needed to avoid multiple users editing the same quiz at the same time.
    $form['timestamp'] = array('#type' => 'hidden', '#default_value' => REQUEST_TIME);
    $form['submit'] = array('#type' => 'submit', '#value' => t('Submit'));

    return $form;
  }

  private function existingTermsForm(&$form, $form_state, $quiz) {
    $terms = quiz()->getQuizHelper()->getQuizTermsByVocabularyId($quiz->vid);
    if ($terms) {
      if (empty($form_state['input']) && !quiz()->getQuizHelper()->buildCategoziedQuestionList($quiz)) {
        drupal_set_message(t('There are not enough questions in the requested categories.'), 'error');
      }
    }
    foreach ($terms as $term) {
      $form[$term->tid]['name'] = array(
        '#markup' => check_plain($term->name),
      );
      $form[$term->tid]['number'] = array(
        '#type'          => 'textfield',
        '#size'          => 3,
        '#default_value' => $term->number,
      );
      $form[$term->tid]['max_score'] = array(
        '#type'          => 'textfield',
        '#size'          => 3,
        '#default_value' => $term->max_score,
      );
      $form[$term->tid]['remove'] = array(
        '#type'          => 'checkbox',
        '#default_value' => 0,
      );
      $form[$term->tid]['weight'] = array(
        '#type'          => 'textfield',
        '#size'          => 3,
        '#default_value' => $term->weight,
        '#attributes'    => array(
          'class' => array('term-weight')
        ),
      );
    }
  }

  /**
   * Form for adding new terms to a quiz
   *
   * @see quiz_categorized_form
   */
  private function categorizedNewTermForm(&$form, $form_state, $quiz) {
    $form['new'] = array(
      '#type'        => 'fieldset',
      '#title'       => t('Add category'),
      '#collapsible' => FALSE,
      '#collapsed'   => FALSE,
      '#tree'        => FALSE,
    );
    $form['new']['term'] = array(
      '#type'              => 'textfield',
      '#title'             => t('Category'),
      '#description'       => t('Type in the name of the term you would like to add questions from.'),
      '#autocomplete_path' => "node/$quiz->nid/questions/term_ahah",
      '#field_suffix'      => '<a id="browse-for-term" href="javascript:void(0)">' . t('browse') . '</a>',
    );
    $form['new']['number'] = array(
      '#type'        => 'textfield',
      '#title'       => t('Number of questions'),
      '#description' => t('How many questions would you like to draw from this term?'),
    );
    $form['new']['max_score'] = array(
      '#type'          => 'textfield',
      '#title'         => t('Max score for each question'),
      '#description'   => t('The number of points a user will be awarded for each question he gets correct.'),
      '#default_value' => 1,
    );
  }

  /**
   * Validate the categorized form
   */
  function formValidate($form, &$form_state) {
    if (_quiz_is_int(arg(1))) {
      if (node_last_changed(arg(1)) > $form_state['values']['timestamp']) {
        form_set_error('changed', t('This content has been modified by another user, changes cannot be saved.'));
      }
    }
    else {
      form_set_error('changed', t('A critical error has occured. Please report error code 28 on the quiz project page.'));
      return;
    }
    if (!empty($form_state['values']['term'])) {
      $tid = $this->getIdFromString($form_state['values']['term']);
      if ($tid === FALSE) {
        $terms = QuizQuestionManagementController::searchTerms($form_state['values']['term']);
        $num_terms = count($terms);
        if ($num_terms == 1) {
          $tid = key($terms);
        }
        elseif ($num_terms > 1) {
          form_set_error('term', t('You need to be more specific, or use the autocomplete feature. The term name you entered matches several terms: %terms', array('%terms' => implode(', ', $terms))));
        }
        elseif ($num_terms == 0) {
          form_set_error('term', t("The term name you entered doesn't match any registered question terms."));
        }
      }
      if (in_array($tid, array_keys($form))) {
        form_set_error('term', t('The category you are trying to add has already been added to this quiz.'));
      }
      else {
        form_set_value($form['tid'], $tid, $form_state);
      }

      if (!_quiz_is_int($form_state['values']['number'])) {
        form_set_error('number', t('The number of questions needs to be a positive integer'));
      }
      if (!_quiz_is_int($form_state['values']['max_score'], 0)) {
        form_set_error('max_score', t('The max score needs to be a positive integer or 0'));
      }
    }
  }

  /**
   * Searches for an id in the end of a string.
   *
   * Id should be written like "(id:23)"
   *
   * @param string $string
   *  The string where we will search for an id
   * @return int
   *  The matched integer
   */
  private function getIdFromString($string) {
    $matches = array();
    preg_match('/\(id:(\d+)\)$/', $string, $matches);
    return isset($matches[1]) ? (int) $matches[1] : FALSE;
  }

  /**
   * Submit the categorized form
   */
  public function formSubmit($form, $form_state) {
    $quiz = node_load($form_state['values']['nid'], $form_state['values']['vid']);
    $quiz->number_of_random_questions = 0;
    // Update the refresh latest quizzes table so that we know what the users latest quizzes are
    if (variable_get('quiz_auto_revisioning', 1)) {
      $is_new_revision = quiz_has_been_answered($quiz);
    }
    else {
      $is_new_revision = (bool) $form_state['values']['new_revision'];
    }
    if (!empty($form_state['values']['tid'])) {
      $quiz->number_of_random_questions += $this->categorizedAddTerm($form, $form_state);
    }
    $quiz->number_of_random_questions += $this->categorizedUpdateTerms($form, $form_state);
    if ($is_new_revision) {
      $quiz->revision = 1;
    }

    // We save the node to update its timestamp and let other modules react to the update.
    // We also do this in case a new revision is required...
    node_save($quiz);
  }

  /**
   * Adds a term to a categorized quiz
   *
   * This is a helper function for the submit function.
   */
  private function categorizedAddTerm($form, $form_state) {
    drupal_set_message(t('The term was added'));
    // Needs to be set to avoid error-message from db:
    $form_state['values']['weight'] = 0;
    drupal_write_record('quiz_terms', $form_state['values']);
    return $form_state['values']['number'];
  }

  /**
   * Update the categoriez belonging to a quiz with categorized random questions.
   *
   * Helper function for quiz_categorized_form_submit
   */
  private function categorizedUpdateTerms(&$form, &$form_state) {
    $ids = array('weight', 'max_score', 'number');
    $changed = array();
    $removed = array();
    $num_questions = 0;
    foreach ($form_state['values'] as $key => $existing) {
      if (!is_numeric($key)) {
        continue;
      }
      if (!$existing['remove']) {
        $num_questions += $existing['number'];
      }
      foreach ($ids as $id) {
        if ($existing[$id] != $form[$key][$id]['#default_value'] && !$existing['remove']) {
          $existing['nid'] = $form_state['values']['nid'];
          $existing['vid'] = $form_state['values']['vid'];
          $existing['tid'] = $key;
          if (empty($existing['weight'])) {
            $existing['weight'] = 1;
          }
          $changed[] = $form[$key]['name']['#markup'];
          drupal_write_record('quiz_terms', $existing, array('vid', 'tid'));
          break;
        }
        elseif ($existing['remove']) {
          db_delete('quiz_terms')
            ->condition('tid', $key)
            ->condition('vid', $form_state['values']['vid'])
            ->execute();
          $removed[] = $form[$key]['name']['#markup'];
          break;
        }
      }
    }
    if (!empty($changed)) {
      drupal_set_message(t('Updates were made for the following terms: %terms', array('%terms' => implode(', ', $changed))));
    }
    if (!empty($removed)) {
      drupal_set_message(t('The following terms were removed: %terms', array('%terms' => implode(', ', $removed))));
    }
    return $num_questions;
  }

}
