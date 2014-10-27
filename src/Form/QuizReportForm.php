<?php

namespace Drupal\quiz\Form;

class QuizReportForm {

  public static function staticCallback($form, $form_state, $questions) {
    $obj = new self();
    return $obj->getForm($form, $form_state, $questions);
  }

  /**
   * Form for showing feedback, and for editing the feedback if necessary...
   *
   * @param $form_state
   *   FAPI form state(array)
   * @param $questions
   *   array of questions to inclide in the report
   * @return $form
   *   FAPI form array
   */
  public function getForm($form, $form_state, $questions) {
    $form['#theme'] = 'quiz_report_form';
    $form['#tree'] = TRUE;
    $form['#submit'][] = array($this, 'formSubmit');

    foreach ($questions as $question) {
      if (!$module = quiz_question_module_for_type($question->type)) {
        return array();
      }
      $function = $module . '_report_form';
      $form_to_add = $function($question);
      if (isset($form_to_add['submit'])) {
        $show_submit = TRUE;
      }
      $form_to_add['#element_validate'][] = array($this, 'validateElement');
      $form[] = $form_to_add;
    }

    // The submit button is only shown if one or more of the questions has input elements
    if (!empty($show_submit)) {
      $form['submit'] = array(
        '#type'   => 'submit',
        '#submit' => array(array($this, 'formSubmit')),
        '#value'  => t('Save Score'),
      );
    }

    if (arg(4) === 'feedback') {
      // @todo figure something better than args.
      $quiz = __quiz_load_context_entity();
      $quiz_id = __quiz_entity_id($quiz);
      if (empty($_SESSION['quiz'][$quiz_id])) { // Quiz is done.
        $form['finish'] = array(
          '#type'   => 'submit',
          '#submit' => array(array($this, formEndSubmit)),
          '#value'  => t('Finish'),
        );
      }
      else {
        $form['next'] = array(
          '#type'   => 'submit',
          '#submit' => array(array($this, 'formSubmitFeedback')),
          '#value'  => t('Next question'),
        );
      }
    }

    return $form;
  }

  /**
   * Submit handler to go to the next question from the question feedback.
   */
  public function formSubmitFeedback($form, &$form_state) {
    $quiz_id = __quiz_get_context_id();
    $form_state['redirect'] = "quiz/{$quiz_id}/take/" . $_SESSION['quiz'][$quiz_id]['current'];
  }

  /**
   * Validate a single question sub-form.
   */
  function validateElement(&$element, &$form_state) {
    $question = node_load($element['nid']['#value'], $element['vid']['#value']);
    if ($quizQuestionResponse = _quiz_question_response_get_instance($element['result_id']['#value'], $question)) {
      $quizQuestionResponse->getReportFormValidate($element, $form_state);
    }
  }

  /**
   * Submit the report form
   *
   * We go through the form state values and submit all questiontypes with
   * validation functions declared.
   */
  public function formSubmit($form, &$form_state) {
    global $user;

    foreach ($form_state['values'] as $key => $q_values) {
      // Questions has numeric keys in the report form
      if (!is_numeric($key)) {
        continue;
      }
      // Questions store the name of the validation function with the key 'submit'
      if (!isset($q_values['submit'])) {
        continue;
      }
      // The submit function must exist
      if (!function_exists($q_values['submit'])) {
        continue;
      }

      // Load the quiz
      if (!isset($quiz)) {
        $result = db_query('SELECT nid, uid, vid FROM {quiz_results} WHERE result_id = :result_id', array(':result_id' => $q_values['result_id']))->fetchObject();
        $quiz = __quiz_load_from_result($result);
        $result_id = $q_values['result_id'];
      }

      $q_values['quiz'] = $quiz;

      // We call the submit function provided by the question
      call_user_func($q_values['submit'], $q_values);
    }

    // Scores may have been changed. We take the necessary actions
    $this->updateLastTotalScore($result_id, $quiz->vid);
    $changed = db_update('quiz_results')
      ->fields(array('is_evaluated' => 1))
      ->condition('result_id', $result_id)
      ->execute();
    $results_got_deleted = quiz()->getQuizHelper()->getResultHelper()->maintainResult($user, $quiz, $result_id);

    // A message saying the quiz is unscored has already been set. We unset it here...
    if ($changed > 0) {
      $this->removeUnscoredMessage();
    }

    // Notify the user if results got deleted as a result of him scoring an answer.
    $add = $quiz->keep_results == QUIZ_KEEP_BEST && $results_got_deleted ? ' ' . t('Note that this quiz is set to only keep each users best answer.') : '';

    $score_data = $this->getScoreArray($result_id, $quiz->vid, TRUE);

    module_invoke_all('quiz_scored', $quiz, $score_data, $result_id);

    drupal_set_message(t('The scoring data you provided has been saved.') . $add);
    if (user_access('score taken quiz answer') && !user_access('view any quiz results')) {
      if ($result && $result->uid == $user->uid) {
        $form_state['redirect'] = 'quiz-result/' . $result_id;
      }
    }
  }

  /**
   * Submit handler to go to the quiz results from the last question's feedback.
   */
  public function formEndSubmit($form, &$form_state) {
    $result_id = $_SESSION['quiz']['temp']['result_id'];
    $form_state['redirect'] = "quiz-result/{$result_id}";
  }

  /**
   * Helper function to remove the message saying the quiz haven't been scored
   */
  private function removeUnscoredMessage() {
    if (!empty($_SESSION['messages']['warning'])) {
      // Search for the message, and remove it if we find it.
      foreach ($_SESSION['messages']['warning'] as $key => $val) {
        if ($val == t('This quiz has not been scored yet.')) {
          unset($_SESSION['messages']['warning'][$key]);
        }
      }
      // Clean up if the message array was left empty
      if (empty($_SESSION['messages']['warning'])) {
        unset($_SESSION['messages']['warning']);
        if (empty($_SESSION['messages'])) {
          unset($_SESSION['messages']);
        }
      }
    }
  }

  /**
   * Returns an array of score information for a quiz
   *
   * @param unknown_type $result_id
   * @param unknown_type $quiz_vid
   * @param unknown_type $is_evaluated
   */
  private function getScoreArray($result_id, $quiz_vid, $is_evaluated) {
    $properties = db_query('SELECT max_score, number_of_random_questions
          FROM {quiz_node_properties}
          WHERE vid = :vid', array(':vid' => $quiz_vid))->fetchObject();
    $total_score = db_query('SELECT SUM(points_awarded)
          FROM {quiz_results_answers}
          WHERE result_id = :result_id', array(':result_id' => $result_id))->fetchField();

    return array(
      'question_count'   => $properties->number_of_random_questions + _quiz_get_num_always_questions($quiz_vid),
      'possible_score'   => $properties->max_score,
      'numeric_score'    => $total_score,
      'percentage_score' => ($properties->max_score == 0) ? 0 : round(($total_score * 100) / $properties->max_score),
      'is_evaluated'     => $is_evaluated,
    );
  }

  /**
   * Updates the total score using only one mySql query.
   *
   * @param $result_id
   * @param int $quiz_vid
   *  Quiz version ID
   */
  private function updateLastTotalScore($result_id, $quiz_vid) {
    $subq1 = db_select('quiz_results_answers', 'a');
    $subq1
      ->condition('a.result_id', $result_id)
      ->addExpression('SUM(a.points_awarded)');

    $score = $subq1->execute()->fetchField();
    $max_score = quiz_entity_single_load(NULL, $quiz_vid)->max_score;
    $final_score = round(100 * ($score / $max_score));

    db_update('quiz_results')
      ->expression('score', $final_score)
      ->condition('result_id', $result_id)
      ->execute();
  }

}
