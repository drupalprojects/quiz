<?php

namespace Drupal\quiz\Controller;

class QuizResultController {

  /**
   * Callback for node/%quiz_menu/quiz/results/%quiz_rid/view
   *
   * Quiz result report page for the quiz admin section
   *
   * @param $quiz
   *   The quiz node
   * @param $result_id
   *   The result id
   */
  public static function staticCallback($quiz, $result_id) {
    // Make sure we have the right version of the quiz
    $result = db_query('SELECT vid, uid FROM {quiz_node_results} WHERE result_id = :result_id', array(':result_id' => $result_id))->fetchObject();
    if ($quiz->vid != $result->vid) {
      $quiz = node_load($quiz->nid, $result->vid);
    }

    // Get all the data we need.
    $questions = _quiz_get_answers($quiz, $result_id);
    $score = quiz_calculate_score($quiz, $result_id);
    $summary = _quiz_get_summary_text($quiz, $score);

    // Lets add the quiz title to the breadcrumb array.
    # $breadcrumb = drupal_get_breadcrumb();
    # $breadcrumb[] = l(t('Quiz Results'), 'admin/quiz/reports/results');
    # $breadcrumb[] = l($quiz->title, 'admin/quiz/reports/results/' . $quiz->nid);
    # drupal_set_breadcrumb($breadcrumb);

    $data = array(
      'quiz' => $quiz,
      'questions' => $questions,
      'score' => $score,
      'summary' => $summary,
      'result_id' => $result_id,
      'account' => user_load($result->uid),
    );
    return theme('quiz_result', $data);
  }

}
