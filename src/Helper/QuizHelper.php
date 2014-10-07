<?php

namespace Drupal\quiz\Helper;

use Drupal\quiz\Helper\Quiz\AccessHelper;
use Drupal\quiz\Helper\Quiz\ResultHelper;
use Drupal\quiz\Helper\Quiz\SettingHelper;

class QuizHelper {

  private $settingHelper;
  private $resultHelper;
  private $accessHelper;

  /**
   * @return SettingHelper
   */
  public function getSettingHelper() {
    if (null === $this->settingHelper) {
      $this->settingHelper = new SettingHelper();
    }
    return $this->settingHelper;
  }

  public function setSettingHelper($settingHelper) {
    $this->settingHelper = $settingHelper;
    return $this;
  }

  /**
   * @return ResultHelper
   */
  public function getResultHelper() {
    if (null === $this->resultHelper) {
      $this->resultHelper = new ResultHelper();
    }
    return $this->resultHelper;
  }

  public function setResultHelper($resultHelper) {
    $this->resultHelper = $resultHelper;
    return $this;
  }

  /**
   * @return AccessHelper
   */
  public function getAccessHelper() {
    if (null !== $this->accessHelper) {
      $this->accessHelper = new AccessHelper();
    }
    return $this->accessHelper;
  }

  public function setAccessHelper($accessHelper) {
    $this->accessHelper = $accessHelper;
    return $this;
  }

  public function addQuestion($quiz, $question) {
    $quiz_questions = $this->getQuestions($quiz->nid, $quiz->vid);

    // Do not add a question if it's already been added (selected in an earlier checkbox)
    foreach ($quiz_questions as $q) {
      if ($question->vid == $q->vid) {
        return FALSE;
      }
    }

    // Otherwise let's add a relationship!
    $question->quiz_nid = $quiz->nid;
    $question->quiz_vid = $quiz->vid;
    _quiz_question_get_instance($question)->saveRelationships();
    quiz_update_max_score_properties(array($quiz->vid));
  }

  /**
   * Retrieve list of published questions assigned to quiz.
   *
   * This function should be used for question browsers and similiar... It should not be used to decide what questions
   * a user should answer when taking a quiz. quiz_build_question_list is written for that purpose.
   *
   * @param $quiz_nid
   *   Quiz node id.
   * @param $quiz_vid
   *   Quiz node version id.
   *
   * @return
   *   An array of questions.
   */
  public function getQuestions($quiz_nid, $quiz_vid = NULL) {
    $questions = array();
    $query = db_select('node', 'n');
    $query->fields('n', array('nid', 'type'));
    $query->fields('nr', array('vid', 'title'));
    $query->fields('qnr', array('question_status', 'weight', 'max_score', 'auto_update_max_score', 'qnr_id', 'qnr_pid'));
    $query->addField('n', 'vid', 'latest_vid');
    $query->join('node_revision', 'nr', 'n.nid = nr.nid');
    $query->leftJoin('quiz_node_relationship', 'qnr', 'nr.vid = qnr.child_vid');
    $query->condition('n.status', 1);
    $query->condition('qnr.parent_nid', $quiz_nid);
    if ($quiz_vid) {
      $query->condition('qnr.parent_vid', $quiz_vid);
    }
    $query->condition('qnr_pid', NULL, 'IS');
    $query->orderBy('qnr.weight');

    $result = $query->execute();
    foreach ($result as $question) {
      $questions[] = $question;
      $this->getSubQuestions($question->qnr_id, $questions);
    }

    foreach ($questions as &$node) {
      $node = quiz_node_map($node);
    }

    return $questions;
  }

  public function setQuestions(&$quiz, $questions, $set_new_revision = FALSE) {
    if ($set_new_revision) {
      // Create a new Quiz VID, even if nothing changed.
      $quiz->revision = 1;

      node_save($quiz);
    }

    // When node_save() calls all of the node API hooks, old quiz info is
    // automatically inserted into quiz_node_relationship. We could get clever and
    // try to do strategic updates/inserts/deletes, but that method has already
    // proven error prone as the module has gained complexity (See 5.x-2.0-RC2).
    // So we go with the brute force method:
    db_delete('quiz_node_relationship')
      ->condition('parent_nid', $quiz->nid)
      ->condition('parent_vid', $quiz->vid)
      ->execute();

    if (empty($questions)) {
      return TRUE; // This is not an error condition.
    }

    foreach ($questions as $question) {
      if ($question->state != QUESTION_NEVER) {
        $question_inserts[$question->qnr_id] = array(
          'parent_nid' => $quiz->nid,
          'parent_vid' => $quiz->vid,
          'child_nid' => $question->nid,
          // Update to latest OR use the version given.
          'child_vid' => $question->refresh ? db_query('SELECT vid FROM {node} WHERE nid = :nid', array(':nid' => $question->nid))->fetchField() : $question->vid,
          'question_status' => $question->state,
          'weight' => $question->weight,
          'max_score' => (int) $question->max_score,
          'auto_update_max_score' => (int) $question->auto_update_max_score,
          'qnr_pid' => $question->qnr_pid,
          'qnr_id' => !$set_new_revision ? $question->qnr_id : NULL,
          'old_qnr_id' => $question->qnr_id,
        );
        drupal_write_record('quiz_node_relationship', $question_inserts[$question->qnr_id]);
      }
    }

    // Update the parentage when a new revision is created.
    // @todo this is copy pasta from quiz_update_quiz_question_relationship
    foreach ($question_inserts as $question_insert) {
      db_update('quiz_node_relationship')
        ->condition('qnr_pid', $question_insert['old_qnr_id'])
        ->condition('parent_vid', $quiz->vid)
        ->condition('parent_nid', $quiz->nid)
        ->fields(array('qnr_pid' => $question_insert['qnr_id']))
        ->execute();
    }

    quiz_update_max_score_properties(array($quiz->vid));
    return TRUE;
  }

  /**
   * Retrieves a list of questions (to be taken) for a given quiz.
   *
   * If the quiz has random questions this function only returns a random
   * selection of those questions. This function should be used to decide
   * what questions a quiz taker should answer.
   *
   * This question list is stored in the user's result, and may be different
   * when called multiple times. It should only be used to generate the layout
   * for a quiz attempt and NOT used to do operations on the questions inside of
   * a quiz.
   *
   * @param $quiz
   *   Quiz node.
   * @return
   *   Array of question node IDs.
   */
  public function getQuestionList($quiz) {
    $questions = array();

    if ($quiz->randomization == 3) {
      $questions = _quiz_build_categorized_question_list($quiz);
    }
    else {
      // Get required questions first.
      $query = db_query('SELECT n.nid, n.vid, n.type, qnr.qnr_id, qnr.qnr_pid
    FROM {quiz_node_relationship} qnr
    JOIN {node} n ON qnr.child_nid = n.nid
    LEFT JOIN {quiz_node_relationship} qnr2 ON (qnr.qnr_pid = qnr2.qnr_id OR (qnr.qnr_pid IS NULL AND qnr.qnr_id = qnr2.qnr_id))
    WHERE qnr.parent_vid = :parent_vid
    AND qnr.question_status = :question_status
    AND n.status = 1
    ORDER BY qnr2.weight, qnr.weight', array(':parent_vid' => $quiz->vid, ':question_status' => QUESTION_ALWAYS));
      $i = 0;
      while ($question_node = $query->fetchAssoc()) {
        // Just to make it easier on us, let's use a 1-based index.
        $i++;
        $questions[$i] = $question_node;
      }

      // Get random questions for the remainder.
      if ($quiz->number_of_random_questions > 0) {
        $random_questions = _quiz_get_random_questions($quiz);
        $questions = array_merge($questions, $random_questions);
        if ($quiz->number_of_random_questions > count($random_questions)) {
          // Unable to find enough requested random questions.
          return FALSE;
        }
      }

      // Shuffle questions if required.
      if ($quiz->randomization > 0) {
        shuffle($questions);
      }
    }

    $count = 0;
    $display_count = 0;
    $questions_out = array();
    foreach ($questions as &$question) {
      $question_node = node_load($question['nid'], $question['vid']);
      $count++;
      $display_count++;
      $question['number'] = $count;
      if ($question['type'] != 'quiz_page') {
        $question['display_number'] = $display_count;
      }
      $questions_out[$count] = $question;
    }
    return $questions_out;
  }

  public function getSubQuestions($qnr_pid, &$questions) {
    $query = db_select('node', 'n');
    $query->fields('n', array('nid', 'type'));
    $query->fields('nr', array('vid', 'title'));
    $query->fields('qnr', array('question_status', 'weight', 'max_score', 'auto_update_max_score', 'qnr_id', 'qnr_pid'));
    $query->addField('n', 'vid', 'latest_vid');
    $query->innerJoin('node_revision', 'nr', 'n.nid = nr.nid');
    $query->innerJoin('quiz_node_relationship', 'qnr', 'nr.vid = qnr.child_vid');
    $query->condition('qnr_pid', $qnr_pid);
    $query->orderBy('weight');
    $result = $query->execute();
    foreach ($result as $question) {
      $questions[] = $question;
    }
  }

  /**
   * Get a list of all available quizzes.
   *
   * @param $uid
   *   An optional user ID. If supplied, only quizzes created by that user will be
   *   returned.
   *
   * @return
   *   A list of quizzes.
   */
  public function getQuizzesByUserId($uid) {
    $results = array();
    $args = array();
    $query = db_select('node', 'n')
      ->fields('n', array('nid', 'vid', 'title', 'uid', 'created'))
      ->fields('u', array('name'));
    $query->leftJoin('users', 'u', 'u.uid = n.uid');
    $query->condition('n.type', 'quiz');
    if ($uid != 0) {
      $query->condition('n.uid', $uid);
    }
    $query->orderBy('n.nid');
    $quizzes = $query->execute();
    foreach ($quizzes as $quiz) {
      $results[$quiz->nid] = (array) $quiz;
    }
    return $results;
  }

  public function copyQuestions($node) {
    // Find original questions.
    $query = db_query('SELECT child_nid, child_vid, question_status, weight, max_score, auto_update_max_score
    FROM {quiz_node_relationship}
    WHERE parent_vid = :parent_vid', array(':parent_vid' => $node->translation_source->vid));
    foreach ($query as $res_o) {
      $original_question = node_load($res_o->child_nid);

      // Set variables we can't or won't carry with us to the translated node to
      // NULL.
      $original_question->nid = $original_question->vid = $original_question->created = $original_question->changed = NULL;
      $original_question->revision_timestamp = $original_question->menu = $original_question->path = NULL;
      $original_question->files = array();
      if (isset($original_question->book['mlid'])) {
        $original_question->book['mlid'] = NULL;
      }

      // Set the correct language.
      $original_question->language = $node->language;

      // Save the node.
      node_save($original_question);

      // Save the relationship between the new question and the quiz.
      db_insert('quiz_node_relationship')
        ->fields(array(
          'parent_nid' => $node->nid,
          'parent_vid' => $node->vid,
          'child_nid' => $original_question->nid,
          'child_vid' => $original_question->vid,
          'question_status' => $res_o->question_status,
          'weight' => $res_o->weight,
          'max_score' => $res_o->max_score,
          'auto_update_max_score' => $res_o->auto_update_max_score,
        ))
        ->execute();
    }
  }

  public function countQuestion($vid) {
    $always_count = _quiz_get_num_always_questions($vid);
    $rand_count = db_query('SELECT number_of_random_questions FROM {quiz_node_properties} WHERE vid = :vid', array(':vid' => $vid))->fetchField();
    return $always_count + (int) $rand_count;
  }

  /**
   * Return highest score data for given quizzes.
   *
   * @param $nids
   *   nids for the quizzes we want to collect scores from.
   * @param $uid
   *   uid for the user we want to collect score for.
   * @param $include_num_questions
   *   Do we want to collect information about the number of questions in a quiz?
   *   This adds a performance hit.
   * @return
   *   Array of score data.
   *   For several takes on the same quiz, only returns highest score.
   */
  public function getScoreData($nids, $uid, $include_num_questions = FALSE) {
    // Validate that the nids are integers.
    foreach ($nids as $key => $nid) {
      if (!_quiz_is_int($nid)) {
        unset($nids[$key]);
      }
    }
    if (empty($nids)) {
      return array();
    }

    // Fetch score data for the validated nids.
    $to_return = array();
    $vids = array();
    $sql = 'SELECT n.title, n.nid, n.vid, p.number_of_random_questions as num_random_questions, r.score AS percent_score, p.max_score, p.pass_rate AS percent_pass
          FROM {node} n
          JOIN {quiz_node_properties} p
          ON n.vid = p.vid
          LEFT OUTER JOIN {quiz_node_results} r
          ON r.nid = n.nid AND r.uid = :uid
          LEFT OUTER JOIN (
            SELECT nid, max(score) as highest_score
            FROM {quiz_node_results}
            GROUP BY nid
          ) rm
          ON n.nid = rm.nid AND r.score = rm.highest_score
          WHERE n.nid in (' . implode(', ', $nids) . ')
          ';
    $res = db_query($sql, array(':uid' => $uid));
    foreach ($res as $res_o) {
      if (!$include_num_questions) {
        unset($res_o->num_random_questions);
      }
      if (!isset($to_return[$res_o->vid]) || $res_o->percent_score > $to_return[$res_o->vid]->percent_score) {
        $to_return[$res_o->vid] = $res_o; // Fetch highest score
      }
      // $vids will be used to fetch number of questions.
      $vids[] = $res_o->vid;
    }
    if (empty($vids)) {
      return array();
    }

    // Fetch total number of questions.
    if ($include_num_questions) {
      $res = db_query('SELECT COUNT(*) AS num_always_questions, parent_vid
            FROM {quiz_node_relationship}
            WHERE parent_vid IN (' . implode(', ', $vids) . ')
            AND question_status = ' . QUESTION_ALWAYS . '
            GROUP BY parent_vid');
      foreach ($res as $res_o) {
        $to_return[$res_o->parent_vid]->num_questions = $to_return[$res_o->parent_vid]->num_random_questions + $res_o->num_always_questions;
      }
    }

    return $to_return;
  }

  public function saveQuestionResult($quiz, $result, $options) {
    if (isset($result->is_skipped) && $result->is_skipped == TRUE) {
      if ($options['set_msg']) {
        drupal_set_message(t('Last question skipped.'), 'status');
      }
      $result->is_correct = FALSE;
      $result->score = 0;
    }
    else {
      // Make sure this is set.
      $result->is_skipped = FALSE;
    }
    if (!isset($result->score)) {
      $result->score = $result->is_correct ? 1 : 0;
    }

    // Points are stored pre-scaled in the quiz_node_results_answers table. We get the scale.
    if ($quiz->randomization < 2) {
      $scale = db_query("SELECT (max_score / (
                  SELECT max_score
                  FROM {quiz_question_properties}
                  WHERE nid = :nid AND vid = :vid
                )) as scale
                FROM {quiz_node_relationship}
                WHERE parent_nid = :parent_nid
                AND parent_vid = :parent_vid
                AND child_nid = :child_nid
                AND child_vid = :child_vid
               ", array(':nid' => $result->nid, ':vid' => $result->vid, ':parent_nid' => $quiz->nid, ':parent_vid' => $quiz->vid, ':child_nid' => $result->nid, ':child_vid' => $result->vid))->fetchField();
    }
    elseif ($quiz->randomization == 2) {
      $scale = db_query("SELECT (max_score_for_random / (
                  SELECT max_score
                  FROM {quiz_question_properties}
                  WHERE nid = :question_nid AND vid = :question_vid
                )) as scale
                FROM {quiz_node_properties}
                WHERE vid = :quiz_vid
               ", array(':question_nid' => $result->nid, ':question_vid' => $result->vid, ':quiz_vid' => $quiz->vid))->fetchField();
    }
    elseif ($quiz->randomization == 3) {
      if (isset($options['question_data']['tid'])) {
        $result->tid = $options['question_data']['tid'];
      }
      $scale = db_query("SELECT (max_score / (
                  SELECT max_score
                  FROM {quiz_question_properties}
                  WHERE nid = :nid AND vid = :vid
                )) as scale
                FROM {quiz_terms}
                WHERE vid = :vid
                AND tid = :tid
               ", array(':nid' => $result->nid, ':vid' => $result->vid, ':vid' => $quiz->vid, ':tid' => $result->tid))->fetchField();
    }
    $points = round($result->score * $scale);
    // Insert result data, or update existing data.
    $result_answer_id = db_query("SELECT result_answer_id
              FROM {quiz_node_results_answers}
              WHERE question_nid = :question_nid
              AND question_vid = :question_vid
              AND result_id = :result_id", array(':question_nid' => $result->nid, ':question_vid' => $result->vid, ':result_id' => $result->result_id))->fetchField();

    $entity = (object) array(
        'result_answer_id' => $result_answer_id,
        'question_nid' => $result->nid,
        'question_vid' => $result->vid,
        'result_id' => $result->result_id,
        'is_correct' => (int) $result->is_correct,
        'points_awarded' => $points,
        'answer_timestamp' => REQUEST_TIME,
        'is_skipped' => (int) $result->is_skipped,
        'is_doubtful' => (int) $result->is_doubtful,
        'number' => $options['question_data']['number'],
        'tid' => ($quiz->randomization == 3 && $result->tid) ? $result->tid : 0,
    );
    entity_save('quiz_result_answer', $entity);
  }

  /**
   * Find out if a quiz is available for taking or not
   *
   * @param $quiz
   *  The quiz node
   * @return
   *  TRUE if available
   *  Error message(String) if not available
   */
  public function isAvailable($quiz) {
    global $user;

    if ($user->uid == 0 && $quiz->takes > 0) {
      return t('This quiz only allows %num_attempts attempts. Anonymous users can only access quizzes that allows an unlimited number of attempts.', array('%num_attempts' => $quiz->takes));
    }

    $user_is_admin = user_access('edit any quiz content') || (user_access('edit own quiz content') && $quiz->uid == $user->uid);
    if ($user_is_admin || $quiz->quiz_always == 1) {
      return TRUE;
    }

    // Compare current GMT time to the open and close dates (which should still be
    // in GMT time).
    $now = REQUEST_TIME;

    if ($now >= $quiz->quiz_close || $now < $quiz->quiz_open) {
      return t('This quiz is closed.');
    }
    return TRUE;
  }

  /**
   * Check a user/quiz combo to see if the user passed the given quiz.
   *
   * This will return TRUE if the user has passed the quiz at least once, and
   * FALSE otherwise. Note that a FALSE may simply indicate that the user has not
   * taken the quiz.
   *
   * @param $uid
   *   The user ID.
   * @param $nid
   *   The node ID.
   * @param $vid
   *   The version ID.
   */
  public function isPassed($uid, $nid, $vid) {
    $passed = db_query('SELECT COUNT(result_id) AS passed_count FROM {quiz_node_results} qnrs
    INNER JOIN {quiz_node_properties} USING (vid, nid)
    WHERE qnrs.vid = :vid
      AND qnrs.nid = :nid
      AND qnrs.uid = :uid
      AND score >= pass_rate', array(':vid' => $vid, ':nid' => $nid, ':uid' => $uid))->fetchField();

    // Force into boolean context.
    return ($passed !== FALSE && $passed > 0);
  }

  /**
   * Finds out if a quiz has been answered or not.
   *
   * @return
   *   TRUE if there exists answers to the current question.
   */
  public function isAnswered($node) {
    if (!isset($node->nid)) {
      return FALSE;
    }
    $query = db_select('quiz_node_results', 'qnr');
    $query->addField('qnr', 'result_id');
    $query->condition('nid', $node->nid);
    $query->condition('vid', $node->vid);
    $query->range(0, 1);
    return $query->execute()->rowCount() > 0;
  }

}
