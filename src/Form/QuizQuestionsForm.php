<?php

namespace Drupal\quiz\Form;

use Drupal\quiz\Entity\QuizEntity;
use stdClass;

class QuizQuestionsForm {

  public static function staticGet($form, $form_state, $quiz) {
    module_load_include('admin.inc', 'quiz', 'quiz');
    $obj = new static();
    return $obj->formGet($form, $form_state, $quiz);
  }

  /**
   * Handles "manage questions" tab.
   *
   * Displays form which allows questions to be assigned to the given quiz.
   *
   * This function is not used if the question assignment type "categorized random questions" is chosen
   *
   * @param $form_state
   *  The form state variable
   * @param QuizEntity $quiz
   * @return
   *  HTML output to create page.
   */
  public function formGet($form, $form_state, $quiz) {
    $form['#submit'][] = array($this, 'formSubmit');
    $form['#validate'][] = array($this, 'formValidate');

    // Display questions in this quiz.
    $form['question_list'] = array(
        '#type'           => 'fieldset',
        '#title'          => t('Questions in this quiz'),
        '#theme'          => 'question_selection_table',
        '#collapsible'    => TRUE,
        '#attributes'     => array('id' => 'mq-fieldset'),
        'question_status' => array('#tree' => TRUE),
    );

    // Add randomization settings if this quiz allows randomized questions
    $this->addFieldsForRandomQuiz($form, $quiz);

    // @todo deal with $include_random
    $questions = quiz()->getQuizHelper()->getQuestions($quiz->qid, $quiz->vid);

    if (empty($questions)) {
      $form['question_list']['no_questions'] = array(
          '#markup' => '<div id = "no-questions">' . t('There are currently no questions in this quiz. Assign existing questions by using the question browser below. You can also use the links above to create new questions.') . '</div>',
      );
    }

    // We add the questions to the form array
    $types = _quiz_get_question_types();
    $this->addQuestionsToForm($form, $questions, $quiz, $types);

    // Show the number of questions in the table header.
    $always_count = isset($form['question_list']['titles']) ? count($form['question_list']['titles']) : 0;
    $form['question_list']['#title'] .= ' (' . $always_count . ')';

    // Give the user the option to create a new revision of the quiz
    _quiz_add_revision_checkbox($form, $quiz);

    // Timestamp is needed to avoid multiple users editing the same quiz at the same time.
    $form['timestamp'] = array('#type' => 'hidden', '#default_value' => REQUEST_TIME);

    $form['actions']['#type'] = 'actions';
    $form['actions']['submit'] = array(
        '#type'   => 'submit',
        '#value'  => t('Submit'),
        '#submit' => array(array($this, 'formSubmit')),
    );
    return $form;
  }

  /**
   * Fields for creating new questions are added to the quiz_questions_form
   *
   * @param $form
   *   FAPI form(array)
   * @param $types
   *   All the question types(array)
   * @param $quiz
   *   The quiz node
   */
  private function addFieldsForCreatingQuestions(&$form, &$types, &$quiz) {
    // Display links to create other questions.
    $form['additional_questions'] = array(
        '#type'        => 'fieldset',
        '#title'       => t('Create new question'),
        '#collapsible' => TRUE,
        '#collapsed'   => TRUE,
    );

    $url_query = drupal_get_destination();
    $url_query['quiz_qid'] = $quiz->qid;
    $url_query['quiz_vid'] = $quiz->vid;
    $create_question = FALSE;
    foreach ($types as $type => $info) {
      $url_type = str_replace('_', '-', $type);
      $options = array(
          'attributes' => array('title' => t('Create @name', array('@name' => $info['name']))),
          'query'      => $url_query,
      );
      $access = node_access('create', $type);
      if ($access) {
        $create_question = TRUE;
      }
      $form['additional_questions'][$type] = array(
          '#markup' => '<div class="add-questions">' . l($info['name'], "node/add/$url_type", $options) . '</div>',
          '#access' => $access,
      );
    }

    if (!$create_question) {
      $form['additional_questions']['create'] = array(
          '#type'   => 'markup',
          '#markup' => t('You have not enabled any question type module or no has permission been given to create any question.'),
        // @todo revisit UI text
      );
    }
  }

  /**
   * Add fields for random quiz to the quiz_questions_form
   *
   * @param $form
   *   FAPI form array
   * @param $quiz
   *   The quiz node(object)
   */
  private function addFieldsForRandomQuiz(&$form, $quiz) {
    if ($quiz->randomization != 2) {
      return;
    }
    $form['question_list']['random_settings'] = array(
        '#type'        => 'fieldset',
        '#title'       => t('Settings for random questions'),
        '#collapsible' => TRUE,
    );
    $form['question_list']['random_settings']['num_random_questions'] = array(
        '#type'          => 'textfield',
        '#size'          => 3,
        '#maxlength'     => 3,
        '#weight'        => -5,
        '#title'         => t('Number of random questions'),
        '#description'   => t('The number of questions to be randomly selected each time someone takes this quiz'),
        '#default_value' => isset($quiz->number_of_random_questions) ? $quiz->number_of_random_questions : 10,
    );
    $form['question_list']['random_settings']['max_score_for_random'] = array(
        '#type'          => 'textfield',
        '#size'          => 3,
        '#maxlength'     => 3,
        '#weight'        => -5,
        '#title'         => t('Max score for each random question'),
        '#default_value' => isset($quiz->max_score_for_random) ? $quiz->max_score_for_random : 1,
    );
    if ($quiz->randomization == 3) {
      $terms = $this->taxonomySelect($quiz->tid);
      if (!empty($terms) && function_exists('taxonomy_get_vocabularies')) {
        $form['question_list']['random_settings']['random_term_id'] = array(
            '#type'          => 'select',
            '#title'         => t('Terms'),
            '#size'          => 1,
            '#options'       => $this->taxonomySelect($quiz->tid),
            '#default_value' => $quiz->tid,
            '#description'   => t('Randomly select from questions with this term, or choose from the question pool below'),
            '#weight'        => -4,
        );
      }
    }
  }

  /**
   * Prints a taxonomy selection form for each vocabulary.
   *
   * @param $tid
   *   Default selected value(s).
   * @return
   *   HTML output to print to screen.
   */
  private function taxonomySelect($tid = 0) {
    $options = array();
    foreach (quiz()->getVocabularies() as $vid => $vocabulary) {
      $temp = taxonomy_form($vid, $tid);
      $options = array_merge($options, $temp['#options']);
    }
    return $options;
  }

  /**
   * Adds the questions in the $questions array to the form
   *
   * @param $form
   *   FAPI form(array)
   * @param $questions
   *   The questions to be added to the question list(array)
   * @param $quiz
   *   The quiz node(object)
   * @param $question_types
   *   array of all available question types
   */
  private function addQuestionsToForm(&$form, &$questions, &$quiz, &$question_types) {
    $form['question_list']['weights'] = array('#tree' => TRUE);
    $form['question_list']['qr_ids'] = array('#tree' => TRUE);
    $form['question_list']['qr_pids'] = array('#tree' => TRUE);
    $form['question_list']['max_scores'] = array('#tree' => TRUE);
    $form['question_list']['auto_update_max_scores'] = array('#tree' => TRUE);
    $form['question_list']['stayers'] = array('#tree' => TRUE);
    $form['question_list']['revision'] = array('#tree' => TRUE);
    if ($quiz->randomization == 2) {
      $form['question_list']['compulsories'] = array('#tree' => TRUE);
    }

    foreach ($questions as $question) {
      // @todo replace entire form with usage of question instance
      $question_node = node_load($question->nid, $question->vid);
      $instance = _quiz_question_get_instance($question_node);
      $fieldset = 'question_list';
      $id = $question->nid . '-' . $question->vid;

      $form[$fieldset]['weights'][$id] = array(
          '#type'          => 'textfield',
          '#size'          => 3,
          '#maxlength'     => 4,
          '#default_value' => isset($question->weight) ? $question->weight : 0,
      );

      $form[$fieldset]['qr_pids'][$id] = array(
          '#type'          => 'textfield',
          '#size'          => 3,
          '#maxlength'     => 4,
          '#default_value' => $question->qr_pid,
      );

      $form[$fieldset]['qr_ids'][$id] = array(
          '#type'          => 'textfield',
          '#size'          => 3,
          '#maxlength'     => 4,
          '#default_value' => $question->qr_id,
      );

      // Quiz directions don't have scoring...
      $form[$fieldset]['max_scores'][$id] = array(
          '#type'          => $instance->isGraded() ? 'textfield' : 'hidden',
          '#size'          => 2,
          '#maxlength'     => 2,
          '#disabled'      => isset($question->auto_update_max_score) ? $question->auto_update_max_score : FALSE,
          '#default_value' => isset($question->max_score) ? $question->max_score : 0,
          '#states'        => array(
              'disabled' => array("#edit-auto-update-max-scores-$id" => array('checked' => TRUE))
          ),
      );

      $form[$fieldset]['auto_update_max_scores'][$id] = array(
          '#type'          => $instance->isGraded() ? 'checkbox' : 'hidden',
          '#default_value' => isset($question->auto_update_max_score) ? $question->auto_update_max_score : 0,
      );

      // Add checkboxes to remove questions in js disabled browsers...
      $form[$fieldset]['stayers'][$id] = array(
          '#type'          => 'checkbox',
          '#default_value' => 0,
          '#attributes'    => array('class' => array('q-staying')),
      );

      //Add checkboxes to mark compulsory questions for randomized quizzes.
      if ($quiz->randomization == 2) {
        $form[$fieldset]['compulsories'][$id] = array(
            '#type'          => 'checkbox',
            '#default_value' => isset($question->question_status) ? ($question->question_status == QUESTION_ALWAYS) ? 1 : 0 : 0,
            '#attributes'    => array('class' => array('q-compulsory')),
        );
      }

      if (user_access('view quiz question outside of a quiz')) {
        $link_options = array(
            'attributes' => array('class' => array('handle-changes')),
        );
        $question_titles = l($question->title, 'node/' . $question->nid, $link_options);
      }
      else {
        $question_titles = check_plain($question->title);
      }

      $form[$fieldset]['titles'][$id] = array('#markup' => $question_titles);


      $form[$fieldset]['types'][$id] = array(
          '#markup'        => $question_types[$question->type]['name'],
          '#question_type' => $question->type,
      );

      $form[$fieldset]['view_links'][$id] = array(
          '#markup' => l(
            t('Edit'), 'node/' . $question->nid . '/edit', array(
              'query'      => drupal_get_destination(),
              'attributes' => array('class' => array('handle-changes')),
            )
          ),
          '#access' => node_access('update', node_load($question->nid, $question->vid)),
      );
      // For js enabled browsers questions are removed by pressing a remove link
      $form[$fieldset]['remove_links'][$id] = array(
          '#markup' => '<a href="#" class="rem-link">' . t('Remove') . '</a>',
      );
      // Add a checkbox to update to the latest revision of the question
      if ($question->vid == $question->latest_vid) {
        $update_cell = array('#markup' => t('<em>Up to date</em>'));
      }
      else {
        $update_cell = array(
            '#type'  => 'checkbox',
            '#title' => (l(t('Latest'), 'node/' . $question->nid . '/revisions/' . $question->latest_vid . '/view')
            . ' of ' .
            l(t('revisions'), 'node/' . $question->nid . '/revisions')
            ),
        );
      }
      $form[$fieldset]['revision'][$id] = $update_cell;
    }
  }

  /**
   * Validate that the supplied questions are real.
   */
  public function formValidate($form, $form_state) {
    if (_quiz_is_int(arg(1))) {
      if (node_last_changed(intval(arg(1))) > $form_state['values']['timestamp']) {
        form_set_error('changed', t('This content has been modified by another user, changes cannot be saved.'));
      }
    }
    else {
      form_set_error('changed', t('A critical error has occured. Please report error code 28 on the quiz project page.'));
      return;
    }

    $already_checked = array();
    $weight_map = $form_state['values']['weights'];

    // Make sure the number of random questions is a positive number
    if (isset($form_state['values']['num_random_questions']) && !_quiz_is_int($form_state['values']['num_random_questions'], 0)) {
      form_set_error('num_random_questions', 'The number of random questions needs to be a positive number');
    }

    // Make sure the max score for random questions is a positive number
    if (isset($form_state['values']['max_score_for_random']) && !_quiz_is_int($form_state['values']['max_score_for_random'], 0)) {
      form_set_error('max_score_for_random', 'The max score for random questions needs to be a positive number');
    }

    if (empty($weight_map)) {
      form_set_error('none', 'No questions were included.');
      return;
    }

    $question_types = array_keys(_quiz_get_question_types());

    foreach ($weight_map as $id => $weight) {
      list($nid, $vid) = explode('-', $id, 2);

      // If a node isn't one of the question types we remove it from the question list
      $has_questions = (Boolean) db_select('node', 'n')
          ->fields('n', array('nid'))
          ->condition('type', $question_types, 'IN')
          ->addTag('node_access')
          ->condition('n.nid', $nid)
          ->execute()
          ->fetchField();
      if (!$has_questions) {
        form_set_error('none', 'One of the supplied questions was invalid. It has been removed from the quiz.');
        unset($form_state['values']['weights'][$id]);
      }

      // We also make sure that we don't have duplicate questions in the quiz.
      elseif (in_array($nid, $already_checked)) {
        form_set_error('none', 'A duplicate question has been removed. You can only ask a question once per quiz.');
        unset($form_state['values']['weights'][$id]);
      }
      else {
        $already_checked[] = $nid;
      }
    }

    // We make sure max score is a positive number
    $max_scores = $form_state['values']['max_scores'];
    foreach ($max_scores as $id => $max_score) {
      if (!_quiz_is_int($max_score, 0)) {
        form_set_error("max_scores][$id", t('Max score needs to be a positive number'));
      }
    }
  }

  /**
   * Submit function for quiz_questions.
   *
   * Updates from the "manage questions" tab.
   */
  public function formSubmit($form, &$form_state) {
    /* @var $quiz \Drupal\quiz\Entity\QuizEntity */
    $quiz = 'node' === arg(0) ? node_load(arg(1)) : quiz_entity_single_load(arg(1));

    // Update the refresh latest quizzes table so that we know what the users latest quizzes are
    if (variable_get('quiz_auto_revisioning', 1)) {
      $is_new_revision = quiz_has_been_answered($quiz);
    }
    else {
      $is_new_revision = (bool) $form_state['values']['new_revision'];
    }

    $this->questionBrowserSubmit($form, $form_state);

    $weight_map = $form_state['values']['weights'];
    $qr_pids_map = $form_state['values']['qr_pids'];
    $qr_ids_map = $form_state['values']['qr_ids'];
    $max_scores = $form_state['values']['max_scores'];
    $auto_update_max_scores = $form_state['values']['auto_update_max_scores'];
    $refreshes = isset($form_state['values']['revision']) ? $form_state['values']['revision'] : NULL;
    $stayers = $form_state['values']['stayers'];
    $compulsories = isset($form_state['values']['compulsories']) ? $form_state['values']['compulsories'] : NULL;
    $num_random = isset($form_state['values']['num_random_questions']) ? $form_state['values']['num_random_questions'] : 0;
    $quiz->max_score_for_random = isset($form_state['values']['max_score_for_random']) ? $form_state['values']['max_score_for_random'] : 1;
    $term_id = isset($form_state['values']['random_term_id']) ? (int) $form_state['values']['random_term_id'] : 0;

    // Store what questions belong to the quiz
    $questions = $this->updateItems($quiz, $weight_map, $max_scores, $auto_update_max_scores, $is_new_revision, $refreshes, $stayers, $qr_ids_map, $qr_pids_map, $compulsories, $stayers);

    // If using random questions and no term ID is specified, make sure we have enough.
    if (empty($term_id)) {
      $assigned_random = 0;

      foreach ($questions as $question) {
        if ($question->state == QUESTION_RANDOM) {
          ++$assigned_random;
        }
      }

      // Adjust number of random questions downward to match number of selected questions..
      if ($num_random > $assigned_random) {
        $num_random = $assigned_random;
        drupal_set_message(t('The number of random questions for this @quiz have been lowered to %anum to match the number of questions you assigned.', array('@quiz' => QUIZ_NAME, '%anum' => $assigned_random), array('langcode' => 'warning')));
      }
    }
    else {
      // Warn user if not enough questions available with this term_id.
      $available_random = count(quiz()->getQuizHelper()->getRandomTaxonomyQuestionIds($term_id, $num_random));
      if ($num_random > $available_random) {
        $num_random = $available_random;
        drupal_set_message(t('There are currently not enough questions assigned to this term (@random). Please lower the number of random quetions or assign more questions to this taxonomy term before taking this @quiz.', array('@random' => $available_random, '@quiz' => QUIZ_NAME)), 'error');
      }
    }

    // Get sum of max_score
    $query = db_select('quiz_relationship', 'qnr');
    $query->addExpression('SUM(max_score)', 'sum');
    $query->condition('quiz_vid', $quiz->vid);
    $query->condition('question_status', QUESTION_ALWAYS);
    $score = $query->execute()->fetchAssoc();

    // Update the quiz's properties.
    $quiz->number_of_random_questions = $num_random ? $num_random : 0;
    $quiz->max_score_for_random = $quiz->max_score_for_random;
    $quiz->tid = $term_id;
    $quiz->max_score = $quiz->max_score_for_random * $quiz->number_of_random_questions + $score['sum'];
    $quiz->is_new_revision = $is_new_revision;

    if (entity_save('quiz_entity', $quiz)) {
      drupal_set_message(t('Questions updated successfully.'));
    }
    else {
      drupal_set_message(t('There was an error updating the @quiz.', array('@quiz' => QUIZ_NAME)), 'error');
    }
  }

  /**
   * Takes care of the browser part of the submitted form values.
   *
   * This function changes the form_state to reflect questions added via the browser.
   * (Especially if js is disabled)
   *
   *
   * @param $form
   *   FAPI form(array)
   * @param $form_state
   *   FAPI form_state(array)
   */
  private function questionBrowserSubmit($form, &$form_state) {
    // Find the biggest weight:
    $next_weight = max($form_state['values']['weights']);

    // If a question is chosen in the browser, add it to the question list if it isn't already there
    if (isset($form_state['values']['browser']['table']['titles'])) {
      foreach ($form_state['values']['browser']['table']['titles'] as $id) {
        if ($id !== 0) {
          $matches = array();
          preg_match('/([0-9]+)-([0-9]+)/', $id, $matches);
          $nid = $matches[1];
          $vid = $matches[2];
          $form_state['values']['weights'][$id] = ++$next_weight;
          $form_state['values']['max_scores'][$id] = quiz_question_get_max_score($nid, $vid);
          $form_state['values']['stayers'][$id] = 1;
        }
      }
    }
  }

  /**
   * Update a quiz set of items with new weights and membership
   * @param $quiz
   *   The quiz node
   * @param $weight_map
   *   Weights for each question(determines the order in which the question will be taken by the quiz taker)
   * @param $max_scores
   *   Array of max scores for each question
   * @param $is_new_revision
   *   Array of boolean values determining if the question is to be updated to the newest revision
   * @param $refreshes
   *   True if we are creating a new revision of the quiz
   * @param $stayers
   *   Questions added to the quiz
   * @param $compulsories
   *   Array of boolean values determining if the question is compulsory or not.
   * @return array set of questions after updating
   */
  private function updateItems($quiz, $weight_map, $max_scores, $auto_update_max_scores, $is_new_revision, $refreshes, $stayers, $qr_ids, $qr_pids, $compulsories = NULL) {
    $questions = array();
    foreach ($weight_map as $id => $weight) {
      if ($stayers[$id]) {
        continue;
      }
      list($nid, $vid) = explode('-', $id, 2);
      $question = new stdClass();
      $question->nid = (int) $nid;
      $question->vid = (int) $vid;
      if (isset($compulsories)) {
        if ($compulsories[$id] == 1) {
          $question->state = QUESTION_ALWAYS;
        }
        else {
          $question->state = QUESTION_RANDOM;
          $max_scores[$id] = $quiz->max_score_for_random;
        }
      }
      else {
        $question->state = QUESTION_ALWAYS;
      }
      $question->weight = $weight;
      $question->max_score = $max_scores[$id];
      $question->auto_update_max_score = $auto_update_max_scores[$id];
      $question->qr_pid = $qr_pids[$id] > 0 ? $qr_pids[$id] : NULL;
      $question->qr_id = $qr_ids[$id] > 0 ? $qr_ids[$id] : NULL;
      $question->refresh = (isset($refreshes[$id]) && $refreshes[$id] == 1);

      // Add item as an object in the questions array.
      $questions[] = $question;
    }

    // Save questions.
    quiz()->getQuizHelper()->setQuestions($quiz, $questions, $is_new_revision);

    return $questions;
  }

}
