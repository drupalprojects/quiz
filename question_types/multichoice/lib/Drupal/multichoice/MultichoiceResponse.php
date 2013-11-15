<?php

/* @file
 * Contains \Drupal\multichoice\MultichoiceResponse.
 */

namespace Drupal\multichoice;

use Drupal\quiz_question\QuizQuestionResponse;

/**
 * Extension of QuizQuestionResponse
 */
class MultichoiceResponse extends QuizQuestionResponse {
  /**
   * ID of the answers.
   */
  protected $user_answer_ids;
  protected $choice_order;

  /**
   * Constructor
   */
  public function __construct($result_id, $question_node, $tries = NULL) {
    parent::__construct($result_id, $question_node, $tries);
    $this->user_answer_ids = array();
    // tries is the tries part of the post data
    if (is_array($tries)) {
      if (isset($tries['choice_order'])) {
        $this->choice_order = $tries['choice_order'];
      }
      unset($tries['choice_order']);
      if (isset($tries['answer']) && is_array($tries['answer'])) {
        foreach ($tries['answer'] as $answer_id) {
          $this->user_answer_ids[] = $answer_id;
          $this->answer = $this->user_answer_ids; //@todo: Stop using user_answer_ids and only use answer instead...
        }
      }
      elseif (isset($tries['answer'])) {
        $this->user_answer_ids[] = $tries['answer'];
      }
    }
    // We load the answer from the database
    else {
      $res = db_query('SELECT answer_id FROM {quiz_multichoice_user_answers} ua
              LEFT OUTER JOIN {quiz_multichoice_user_answer_multi} uam ON(uam.user_answer_id = ua.id)
              WHERE ua.result_id = :result_id AND ua.question_nid = :question_nid AND ua.question_vid = :question_vid', array(':result_id' => $result_id, ':question_nid' => $this->question->id(), ':question_vid' => $this->question->getRevisionId()));
      while ($res_o = $res->fetch()) {
        $this->user_answer_ids[] = $res_o->answer_id;
      }
    }
  }

  /**
   * Implementation of isValid
   *
   * @see QuizQuestionResponse#isValid()
   */
  public function isValid() {
    if ($this->question->choice_multi) {
      return TRUE;
    }
    if (empty($this->user_answer_ids)) {
      return t('You must provide an answer');
    }
    // Perform extra check since FAPI isn't being used:
    if (count($this->user_answer_ids) > 1) {
      return t('You are only allowed to select one answer');
    }
    return TRUE;
  }

  /**
   * Implementation of save
   *
   * @see QuizQuestionResponse#save()
   */
  public function save() {
    $user_answer_id = db_insert('quiz_multichoice_user_answers')
      ->fields(array(
        'result_id' => $this->rid,
        'question_vid' => $this->question->getRevisionId(),
        'question_nid' => $this->question->id(),
        'choice_order' => $this->choice_order
      ))
      ->execute();

    $query = db_insert('quiz_multichoice_user_answer_multi')
      ->fields(array('user_answer_id', 'answer_id'));
    for ($i = 0; $i < count($this->user_answer_ids); $i++) {
      $query->values(array($user_answer_id, $this->user_answer_ids[$i]));
    }
      $query->execute();
  }

  /**
   * Implementation of delete
   *
   * @see QuizQuestionResponse#delete()
   */
  public function delete() {
    $user_answer_id = array();
    $query = db_query('SELECT id FROM {quiz_multichoice_user_answers} WHERE question_nid = :nid AND question_vid = :vid AND result_id = :result_id', array(':nid' => $this->question->id(), ':vid' => $this->question->getRevisionId(), ':result_id' => $this->rid));
    while ($user_answer = $query->fetch()) {
      $user_answer_id[] = $user_answer->id;
    }

    if (!empty($user_answer_id)) {
      db_delete('quiz_multichoice_user_answer_multi')
        ->condition('user_answer_id', $user_answer_id, 'IN')
        ->execute();
    }

    db_delete('quiz_multichoice_user_answers')
      ->condition('result_id', $this->rid)
      ->condition('question_nid', $this->question->id())
      ->condition('question_vid', $this->question->getRevisionId())
      ->execute();
  }

  /**
   * Implementation of score
   *
   * @return uint
   *
   * @see QuizQuestionResponse#score()
   */
  public function score() {
    if ($this->question->choice_boolean || $this->isAllWrong()) {
      $score = $this->getMaxScore();
      foreach ($this->question->alternatives as $key => $alt) {
        if (in_array($alt['id'], $this->user_answer_ids)) {
          if ($alt['score_if_chosen'] <= $alt['score_if_not_chosen']) {
            $score = 0;
          }
        }
        else {
          if ($alt['score_if_chosen'] > $alt['score_if_not_chosen']) {
            $score = 0;
          }
        }
      }
    }
    else {
      $score = 0;
      foreach ($this->question->alternatives as $key => $alt) {
        if (in_array($alt['id'], $this->user_answer_ids)) {
          $score += $alt['score_if_chosen'];
        }
        else {
          $score += $alt['score_if_not_chosen'];
        }
      }
    }
    return $score;
  }

  /**
   * If all answers in a question is wrong
   *
   * @return boolean
   *  TRUE if all answers are wrong. False otherwise.
   */
  public function isAllWrong() {
    foreach ($this->question->alternatives as $key => $alt) {
      if ($alt['score_if_chosen'] > 0 || $alt['score_if_not_chosen'] > 0) {
        return FALSE;
      }
    }
    return TRUE;
  }

  /**
   * Implementation of getResponse
   *
   * @return answer
   *
   * @see QuizQuestionResponse#getResponse()
   */
  public function getResponse() {
    return $this->user_answer_ids;
  }

  /**
   * Implementation of getReportFormResponse
   *
   * @see getReportFormResponse($showpoints, $showfeedback, $allow_scoring)
   */
  public function getReportFormResponse($showpoints = TRUE, $showfeedback = TRUE, $allow_scoring = FALSE) {
    $i = 0;
    $this->orderAlternatives($this->question->alternatives);

    // Find the alternative with the highest score
    if ($this->question->choice_multi == 0) {
      $max_score_if_chosen = -999;
      while (isset($this->question->alternatives[$i]) && is_array($this->question->alternatives[$i])) {
        $short = $this->question->alternatives[$i];
        if ($short['score_if_chosen'] > $max_score_if_chosen) {
          $max_score_if_chosen = $short['score_if_chosen'];
        }
        $i++;
      }
      $i = 0;
    }
    // Fetch all data for the report
    $data = array();
    while (isset($this->question->alternatives[$i])) {
      $short = $this->question->alternatives[$i];
      if (drupal_strlen($this->checkMarkup($short['answer'], $short['answer_format'])) > 0) {
        $alternative = array();

        // Did the user choose the alternative?
        $alternative['is_chosen'] = in_array($short['id'], $this->user_answer_ids);

        // Questions where multiple answers isn't allowed are scored differently...
        if ($this->question->choice_multi == 0) {

          if ($this->question->choice_boolean == 0) {
            if ($short['score_if_chosen'] > $short['score_if_not_chosen']) {
              $alternative['is_correct'] = $short['score_if_chosen'] < $max_score_if_chosen ? 1 : 2;
            }
            else {
              $alternative['is_correct'] = 0;
            }
          }

          else {
            $alternative['is_correct'] = $short['score_if_chosen'] > $short['score_if_not_chosen'] ? 2 : 0;
          }
        }

        // Questions where multiple answers are allowed
        else {
          $alternative['is_correct'] = $short['score_if_chosen'] > $short['score_if_not_chosen'] ? 2 : 0;
        }

        $alternative['answer'] = $this->checkMarkup($short['answer'], $short['answer_format'], FALSE);

        $not = $alternative['is_chosen'] ? '' : '_not';
        $alternative['feedback'] = $this->checkMarkup($short['feedback_if' . $not . '_chosen'], $short['feedback_if' . $not . '_chosen_format'], FALSE);
        $data[] = $alternative;
      }
      $i++;
    }
    // Return themed report
    return array('#markup' => theme('multichoice_response', array('data' => $data)));
  }

  /**
   * Order the alternatives according to the choice order stored in the database
   *
   * @param array $alternatives
   *  The alternatives to be ordered
   */
  private function orderAlternatives(array &$alternatives) {
    if (!$this->question->choice_random) {
      return;
    }
    $result = db_query('SELECT choice_order FROM {quiz_multichoice_user_answers}
            WHERE result_id = :result_id AND question_nid = :question_nid AND question_vid = :question_vid', array(':result_id' => $this->rid, ':question_nid' => $this->question->id(), ':question_vid' => $this->question->getRevisionId()))->fetchField();
    if (!$result) {
      return;
    }
    $order = explode(',', $result);
    $newAlternatives = array();
    foreach ($order as $value) {
      foreach ($alternatives as $alternative) {
        if ($alternative['id'] == $value) {
          $newAlternatives[] = $alternative;
          break;
        }
      }
    }
    $alternatives = $newAlternatives;
  }
  /**
   * Run check_markup() on the field of the specified choice alternative
   *
   * @param $alternative
   *  String to be checked
   * @param $format
   *  The input format to be used
   * @param $check_user_access
   *  Whether or not we are to check the users access to the chosen format
   * @return HTML markup
   */
  private function checkMarkup($alternative, $format, $check_user_access = FALSE) {
    // If the string is empty we don't run it through input filters(They might add empty tags).
    if (drupal_strlen($alternative) == 0) {
      return '';
    }
    return check_markup($alternative, $format, $langcode = '' /* TODO Set this variable. */, $check_user_access);
  }
}
