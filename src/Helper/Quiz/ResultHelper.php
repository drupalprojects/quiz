<?php

namespace Drupal\quiz\Helper\Quiz;

use stdClass;

class ResultHelper {

  /**
   * Update a score for a quiz.
   *
   * This updates the quiz node results table.
   *
   * It is used in cases where a quiz score is changed after the quiz has been
   * taken. For example, if a long answer question is scored later by a human,
   * then the quiz should be updated when that answer is scored.
   *
   * Important: The value stored in the table is the *percentage* score.
   *
   * @param $quiz
   *   The quiz node for the quiz that is being scored.
   * @param $result_id
   *   The result ID to update.
   * @return
   *   The score as an integer representing percentage. E.g. 55 is 55%.
   */
  public function updateTotalScore($quiz, $result_id) {
    $score = quiz_calculate_score($quiz, $result_id);
    db_update('quiz_node_results')
      ->fields(array(
        'score' => $score['percentage_score'],
      ))
      ->condition('result_id', $result_id)
      ->execute();
    if ($score['is_evaluated']) {
      // Call hook_quiz_scored().
      module_invoke_all('quiz_scored', $quiz, $score, $result_id);
      _quiz_maintain_results($quiz, $result_id);
      db_update('quiz_node_results')
        ->fields(array('is_evaluated' => 1))
        ->condition('result_id', $result_id)
        ->execute();
    }
    return $score['percentage_score'];
  }

  /**
   * Delete quiz results.
   *
   * @param $result_ids
   *   Result ids for the results to be deleted.
   */
  public function deleteByIds($result_ids) {
    if (empty($result_ids)) {
      return;
    }

    $sql = 'SELECT result_id, question_nid, question_vid FROM {quiz_node_results_answers}
          WHERE result_id IN(:result_id)';
    $result = db_query($sql, array(':result_id' => $result_ids));
    foreach ($result as $record) {
      quiz_question_delete_result($record->result_id, $record->question_nid, $record->question_vid);
    }

    db_delete('quiz_node_results_answers')
      ->condition('result_id', $result_ids, 'IN')
      ->execute();

    db_delete('quiz_node_results')
      ->condition('result_id', $result_ids, 'IN')
      ->execute();
  }

  /**
   * Load a specific result answer.
   */
  public function loadAnswerResult($result_id, $nid, $vid) {
    $sql = 'SELECT * from {quiz_node_results_answers} WHERE result_id = :result_id AND question_nid = :nid AND question_vid = :vid';
    $result = db_query($sql, array(':result_id' => $result_id, ':nid' => $nid, ':vid' => $vid));
    if ($row = $result->fetch()) {
      return entity_load_single('quiz_result_answer', $row->result_answer_id);
    }
  }

  /**
   * Get answer data for a specific result.
   *
   * @param $result_id
   *   Result id.
   *
   * @return
   *   Array of answers.
   */
  public function getAnswers($quiz, $result_id) {
    $questions = array();
    $ids = db_query("SELECT question_nid, question_vid, type, rs.max_score, qt.max_score as term_max_score
                   FROM {quiz_node_results_answers} ra
                   LEFT JOIN {node} n ON (ra.question_nid = n.nid)
                   LEFT JOIN {quiz_node_results} r ON (ra.result_id = r.result_id)
                   LEFT OUTER JOIN {quiz_node_relationship} rs ON (ra.question_vid = rs.child_vid) AND rs.parent_vid = r.vid
                   LEFT OUTER JOIN {quiz_terms} qt ON (qt.vid = :vid AND qt.tid = ra.tid)
                   WHERE ra.result_id = :rid
                   ORDER BY ra.number, ra.answer_timestamp", array(':vid' => $quiz->vid, ':rid' => $result_id));
    while ($line = $ids->fetch()) {
      // Questions picked from term id's won't be found in the quiz_node_relationship table
      if ($line->max_score === NULL) {
        if ($quiz->randomization == 2 && isset($quiz->tid) && $quiz->tid > 0) {
          $line->max_score = $quiz->max_score_for_random;
        }
        elseif ($quiz->randomization == 3) {
          $line->max_score = $line->term_max_score;
        }
      }
      $module = quiz_question_module_for_type($line->type);
      if (!$module) {
        continue;
      }
      // Invoke hook_get_report().
      $report = module_invoke($module, 'get_report', $line->question_nid, $line->question_vid, $result_id);
      if (!$report) {
        continue;
      }
      $questions[$line->question_nid] = $report;
      // Add max score info to the question.
      if (!isset($questions[$line->question_nid]->score_weight)) {
        if ($questions[$line->question_nid]->max_score == 0) {
          $score_weight = 0;
        }
        else {
          $score_weight = $line->max_score / $questions[$line->question_nid]->max_score;
        }
        $questions[$line->question_nid]->qnr_max_score = $line->max_score;
        $questions[$line->question_nid]->score_weight = $score_weight;
      }
    }
    return $questions;
  }

  /**
   * Calculates the score user received on quiz.
   *
   * @param $quiz
   *   The quiz node.
   * @param $result_id
   *   Quiz result ID.
   *
   * @return array
   *   Contains three elements: question_count, num_correct and percentage_score.
   */
  public function calculateScore($quiz, $result_id) {
    // 1. Fetch all questions and their max scores
    $questions = db_query('SELECT a.question_nid, a.question_vid, n.type, r.max_score
    FROM {quiz_node_results_answers} a
    LEFT JOIN {node} n ON (a.question_nid = n.nid)
    LEFT OUTER JOIN {quiz_node_relationship} r ON (r.child_vid = a.question_vid) AND r.parent_vid = :vid
    WHERE result_id = :rid', array(':vid' => $quiz->vid, ':rid' => $result_id));
    // 2. Callback into the modules and let them do the scoring. @todo after 4.0: Why isn't the scores already saved? They should be
    // Fetched from the db, not calculated....
    $scores = array();
    $count = 0;
    foreach ($questions as $question) {
      // Questions picked from term id's won't be found in the quiz_node_relationship table
      if ($question->max_score === NULL && isset($quiz->tid) && $quiz->tid > 0) {
        $question->max_score = $quiz->max_score_for_random;
      }

      // Invoke hook_quiz_question_score().
      // We don't use module_invoke() because (1) we don't necessarily want to wed
      // quiz type to module, and (2) this is more efficient (no NULL checks).
      $mod = quiz_question_module_for_type($question->type);
      if (!$mod) {
        continue;
      }
      $function = $mod . '_quiz_question_score';

      if (function_exists($function)) {
        $score = $function($quiz, $question->question_nid, $question->question_vid, $result_id);
        // Allow for max score to be considered.
        $scores[] = $score;
      }
      else {
        drupal_set_message(t('A quiz question could not be scored: No scoring info is available'), 'error');
        $dummy_score = new stdClass();
        $dummy_score->possible = 0;
        $dummy_score->attained = 0;
        $scores[] = $dummy_score;
      }
      ++$count;
    }
    // 3. Sum the results.
    $possible_score = 0;
    $total_score = 0;
    $is_evaluated = TRUE;
    foreach ($scores as $score) {
      $possible_score += $score->possible;
      $total_score += $score->attained;
      if (isset($score->is_evaluated)) {
        // Flag the entire quiz if one question has not been evaluated.
        $is_evaluated &= $score->is_evaluated;
      }
    }

    // 4. Return the score.
    return array(
      'question_count'   => $count,
      'possible_score'   => $possible_score,
      'numeric_score'    => $total_score,
      'percentage_score' => ($possible_score == 0) ? 0 : round(($total_score * 100) / $possible_score),
      'is_evaluated'     => $is_evaluated,
    );
  }

  public function isResultCompleted($result_id) {
    // Check if the quiz taking has been completed.
    $time_end = db_query('SELECT time_end FROM {quiz_node_results} WHERE result_id = :result_id', array(':result_id' => $result_id))->fetchField();
    return $time_end > 0;
  }

  /**
   * Deletes all results associated with a given user.
   *
   * @param int $uid
   *  The users id
   */
  public function deleteByUserId($uid) {
    $res = db_query("SELECT result_id FROM {quiz_node_results} WHERE uid = :uid", array(':uid' => $uid));
    $result_ids = array();
    while ($result_id = $res->fetchField()) {
      $result_ids[] = $result_id;
    }
    $this->deleteByIds($result_ids);
  }

  /**
   * Deletes results for a quiz according to the keep results setting
   *
   * @param $quiz
   *  The quiz node to be maintained
   * @param $result_id
   *  The result id of the latest result for the current user
   * @return
   *  TRUE if results where deleted.
   */
  public function maintainResult($account, $quiz, $result_id) {
    // Do not delete results for anonymous users
    if ($account->uid == 0) {
      return;
    }

    switch ($quiz->keep_results) {
      case QUIZ_KEEP_ALL:
        return FALSE;
      case QUIZ_KEEP_BEST:
        $best_result_id = db_query('SELECT result_id FROM {quiz_node_results}
          WHERE nid = :nid AND uid = :uid AND is_evaluated = :is_evaluated
          ORDER BY score DESC', array(':nid' => $quiz->nid, ':uid' => $account->uid, ':is_evaluated' => 1)
          )
          ->fetchField();
        if (!$best_result_id) {
          return;
        }
        $res = db_query('SELECT result_id FROM {quiz_node_results}
          WHERE nid = :nid AND uid = :uid AND result_id != :best_rid AND is_evaluated = :is_evaluated', array(':nid' => $quiz->nid, ':uid' => $account->uid, ':is_evaluated' => 1, ':best_rid' => $best_result_id)
        );
        $result_ids = array();
        while ($result_id2 = $res->fetchField()) {
          $result_ids[] = $result_id2;
        }
        $this->deleteByIds($result_ids);
        return !empty($result_ids);
      case QUIZ_KEEP_LATEST:
        $res = db_query('SELECT result_id FROM {quiz_node_results}
              WHERE nid = :nid AND uid = :uid AND is_evaluated = :is_evaluated AND result_id != :result_id', array(':nid' => $quiz->nid, ':uid' => $account->uid, ':is_evaluated' => 1, ':result_id' => $result_id));
        $result_ids = array();
        while ($result_id2 = $res->fetchField()) {
          $result_ids[] = $result_id2;
        }
        $this->deleteByIds($result_ids);
        return !empty($result_ids);
    }
  }

  /**
   * Delete quiz responses for quizzes that haven't been finished.
   *
   * This was _quiz_delete_old_in_progress()
   *
   * @param $quiz
   *   A quiz node where old in progress results shall be deleted.
   * @param $uid
   *   The userid of the user the old in progress results belong to.
   */
  public function deleteIncompletedResultsByUserId($quiz, $uid) {
    $res = db_query('SELECT qnr.result_id FROM {quiz_node_results} qnr
          WHERE qnr.uid = :uid
          AND qnr.nid = :nid
          AND qnr.time_end = :time_end
          AND qnr.vid < :vid', array(':uid' => $uid, ':nid' => $quiz->nid, ':time_end' => 1, ':vid' => $quiz->vid));
    $result_ids = array();
    while ($result_id = $res->fetchField()) {
      $result_ids[] = $result_id;
    }
    $this->deleteByIds($result_ids);
  }

}
