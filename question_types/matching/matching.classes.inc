<?php
// $Id$
/**
 * matching.classes
 *
 * This module uses the question interface to define the Matching question type.
 *
 * A Matching node defines a series of questions and answers and requires the user
 * to associate each answer with a question.
 *
 * @file
 */

/**
 * Implementation of Matching.
 */
class MatchingQuestion extends QuizQuestion {

  public function __construct(stdClass $node) {
    parent::__construct($node);
    if (empty($this->node->match)) $this->node->match = array();
  }

  public function saveNodeProperties($is_new = FALSE) {

    foreach ($this->node->match as $match) {
      if (empty($match['question']) || empty($match['answer'])) {
        continue;
      }
      $match['feedback'] = (isset($match['feedback'])) ? $match['feedback'] : '';
      if ($is_new || $this->node->revision == 1) {
        $sql = "INSERT INTO {quiz_matching_node} (nid, vid, question, answer, feedback) VALUES (%d, %d, '%s', '%s', '%s')";
        db_query($sql, $this->node->nid, $this->node->vid, $match['question'], $match['answer'], $match['feedback']);
      }
      else {
        //$match_id = $this->node->{'match_id_' . $i};
        $sql = "UPDATE {quiz_matching_node} SET question = '%s', answer = '%s', feedback = '%s' WHERE match_id = %d";
        db_query($sql, $match['question'], $match['answer'], $match['feedback'], $match['match_id']);
      }
    }
  }


  public function validateNode(array &$form) {
    // This space intentionally left blank. :)
  }


  public function delete($only_this_version = FALSE) {
    parent::delete($only_this_version);

    // Only delete a nid/vid.
    if ($only_this_version) {
      $sql = 'DELETE FROM {quiz_matching_node} WHERE nid = %d AND vid = %d';
      db_query($sql, $this->node->nid, $this->node->vid);
    }
    // Delete all versions of the quiz question.
    else {
      $sql = 'DELETE FROM {quiz_matching_node} WHERE nid = %d';
      db_query($sql, $this->node->nid, $this->node->vid);
    }
  }

  public function getNodeProperties() {
    if (isset($this->nodeProperties)) return $this->nodeProperties;
    $props = parent::getNodeProperties();

    $sql = "SELECT match_id, question, answer, feedback FROM {quiz_matching_node} WHERE nid = %d AND vid = %d";
    $results = db_query($sql, $this->node->nid, $this->node->vid);
    while ($result = db_fetch_object($results)) {
      $props['match'][] = array(
        'match_id' => $result->match_id,
        'question' => $result->question,
        'answer' => $result->answer,
        'feedback' => $result->feedback,
      );
    }
    $this->nodeProperties = $props;
    return $props;
  }

  public function getNodeView() {
    $content = parent::getNodeView();

    list($matches, $select_option) = $this->getSubquestions();

    if ($this->viewCanRevealCorrect()) {
      $header = array(t('Subquestion'), t('Correct match'));
      foreach ($matches as $match) {
        $rows[] = array($match['question'], $match['answer']);
      }
    } else {
      $header = array(t('Subquestion'), t('Random possible match'));
      // shuffle the answer column
      foreach ($matches as $match) {
        $sub_qs[] = $match['question'];
        $sub_as[] = $match['answer'];
      }
      shuffle($sub_as);
      foreach ($sub_qs as $i => $sub_q) {
        $rows[] = array($sub_q, $sub_as[$i]);
      }
    }

    $content['answers'] = array(
       '#value' => theme('table', $header, $rows),
       '#weight' => 2,
    );

    return $content;
  }

  // This is called whenever a question is rendered, either
  // to an administrator or to a quiz taker.
  public function getAnsweringForm(array $form_state = NULL, $rid) {
    $form = array();
    $form['question'] = array(
      '#type' => 'markup',
      '#value' => check_markup($this->node->body, $this->node->format, FALSE),
    );
    if (isset($rid)) {
      $response = new MatchingResponse($rid, $this->node);
      $responses = $response->getResponse();
    }
    list($matches, $select_option) = $this->getSubquestions();
    foreach ($matches as $match) {
      $title = $match['question'];
      $form['subquestions']['tries[' . $match['match_id'] . ']'] = array(
        '#type' => 'select',
        '#title' => $title,
        '#options' => $this->customShuffle($select_option),
      );
      if (isset($rid)) {
        $form['subquestions']['tries[' . $match['match_id'] . ']']['#default_value'] = $responses[$match['match_id']];
      }
    }
    $form['scoring_info'] = array(
      '#type' => 'markup',
      '#value' => '<p><em>'. t('You loose points by selecting incorrect options. You may leave an option blank to avoid loosing points.') .'</em></p>',
    );
    return variable_get('quiz_matching_shuffle_options', TRUE) ? array($form['question'], $this->customShuffle($form['subquestions']), $form['scoring_info']) : $form;
  }

  private function customShuffle(array $array = array()) {
    $new_array = array();
    $new_array['def'] = '';
    while (count($array)) {
      $element = array_rand($array);
      $new_array[$element] = $array[$element];
      unset($array[$element]);
    }
    return $new_array;
  }

  private function getSubquestions() {
    $matches = $select_option = array();
    $sql = "SELECT match_id, question, answer, feedback FROM {quiz_matching_node} WHERE nid = %d AND vid = %d";
    $results = db_query($sql, $this->node->nid, $this->node->vid);
    while ($result = db_fetch_object($results)) {
      $matches[] = array(
        'match_id' => $result->match_id,
        'question' => $result->question,
        'answer' => $result->answer,
        'feedback' => $result->feedback,
      );
      $select_option[$result->match_id] = $result->answer;
    }
    return array($matches, $select_option);
  }

  public function getBodyFieldTitle() {
    return t('Instruction');
  }
  
  public function getCreationForm(array $form_state = NULL) {
    $form['match'] = array(
      '#type' => 'fieldset',
      '#title' => t('Answer'),
      '#weight' => -4,
      '#tree' => TRUE,
      '#theme' => 'matching_question_form',
      '#description' => t('Write you pairs in the question and answer columns. For the user the question will be fixed and the answers will be shown as alternatives in a dropdown box.'),
    );
    for ($i = 1; $i <= variable_get('quiz_matching_form_size', 5); ++$i) {
      $form['match'][$i] = array(
        '#type' => 'fieldset',
        '#title' => t('Question '. $i),
      );
      $form['match'][$i]['match_id'] = array(
        '#type' => 'hidden',
        '#default_value' => isset($this->node->match[$i-1]['match_id']) ? $this->node->match[$i-1]['match_id'] : ''
      );
      $form['match'][$i]['question'] = array(
        '#type' => 'textarea',
        '#rows' => 2,
        '#default_value' => isset($this->node->match[$i-1]['question']) ? $this->node->match[$i-1]['question'] : ''
      );
      $form['match'][$i]['answer'] = array(
        '#type' => 'textarea',
        '#rows' => 2,
        '#default_value' => isset($this->node->match[$i-1]['answer']) ? $this->node->match[$i-1]['answer'] : ''
      );

      $form['match'][$i]['feedback'] = array(
        '#type' => 'textarea',
        '#rows' => 2,
        '#default_value' => isset($this->node->match[$i-1]['feedback']) ? $this->node->match[$i-1]['feedback'] : ''
      );
    }
    return $form;
  }

  public function getMaximumScore() {
    $to_return = 0;
    foreach ($this->node->match as $match) {
      if (empty($match['question']) || empty($match['answer'])) {
        continue;
      }
      $to_return++;
    }
    return $to_return;
  }

  public function getCorrectAnswer() {
    $correct_answers = array();
    //match_id  nid   vid   question  answer  feedback # {quiz_matching_node} fields

    $results = db_query("SELECT match_id, question, answer FROM {quiz_matching_node} WHERE nid = %d AND vid = %d", $this->node->nid, $this->node->vid);

    while ($result = db_fetch_object($results)) {
      $correct_answers[$result->match_id] = array(
        'match_id' => $result->match_id,
        'question' => $result->question,
        'answer' => $result->answer
      );
    }
    return $correct_answers;
  }
}

/**
 * Class that describes a "Directions question response". For
 * the most part, no real scoring takes place for a direction
 * node. However, there are a few behind-the-scenes tricks that
 * are done here to make the quiz-taking process a little easier.
 */

class MatchingResponse extends AbstractQuizQuestionResponse {

  public function __construct($result_id, stdClass $question_node, $answer = NULL) {
    parent::__construct($result_id, $question_node, $answer);
    $this->is_correct = $this->isCorrect();
    if (!isset($answer)) {
      $sql = 'SELECT ua.answer, score, ua.match_id
              FROM {quiz_matching_user_answers} ua
              JOIN {quiz_matching_node} n
              ON n.match_id = ua.match_id
              WHERE n.vid = %d';
      $res = db_query($sql, $question_node->vid);
      $this->answer = array();
      while ($obj = db_fetch_object($res)) {
        $this->answer[$obj->match_id] = $obj->answer;
      }
    }
  }

  public function isValid() {
    foreach ($this->answer as $value)
      if ($value != 'def') return TRUE;
    return t('You need to match at least one of the items.');
  }

  public function save() {
    $user_answers = isset($this->answer) ? $this->answer : array();
    // to prevent warning : Invalid argument supplied for foreach()
    foreach ((array)$user_answers as $key => $value) {
      $score = ($key == $value)? 1 : 0;
      $sql = "INSERT INTO {quiz_matching_user_answers} (match_id, result_id, answer, score) VALUES (%d, %d, %d, %d)";
        // This is expensive need to be changed
      db_query($sql, $key, $this->rid, (int)$value, $score);
    }
  }

  public function delete() {
    // FIXME implement delete in MatchingResponse
    //$sql = 'DELETE FROM {quiz_matching_user_answers} WHERE question_nid = %d AND question_vid = %d AND result_id = %d';
    //db_query($sql, $this->question->nid, $this->question->vid, $this->rid);
  }

  public function score() {
    $wrong_answer = 0;
    $correct_answer = 0;
    $user_answers = isset($this->answer['answer']) ? $this->answer['answer'] : $this->answer;
    $user_answers = isset($user_answers) ? $user_answers : $this->getUserAnswers();
    // to prevent warning : Invalid argument supplied for foreach()
    foreach ((array)$user_answers as $key => $value) {
      if ($key == $value) {
        $correct_answer++;
      }
      elseif($value == 0 || $value == 'def') {}
      else {
        $wrong_answer++;
      } 
    }
    $score = $correct_answer - $wrong_answer;
    return $score < 0 ? 0 : $score;
  }

  public function getResponse() {
    return $this->answer;
  }

  public function getUserAnswers() {
    if (isset($this->answer)) return $this->answer;
    $user_answers = array();
    // answer_id  match_id  result_id   score   answer #{quiz_matching_user_answers} fields
    $results = db_query("SELECT a.match_id, a.answer
                         FROM {quiz_matching_node} n
                         JOIN {quiz_matching_user_answers} a
                         ON n.match_id = a.match_id
                         WHERE nid = %d AND vid = %d AND result_id = %d
                         ", $this->question->nid, $this->question->vid, $this->rid);
    while ($result = db_fetch_object($results)) {
      $user_answers[$result->match_id] = $result->answer;
    }
    $this->answer = $user_answers;
    return $user_answers;
  }

  public function getReportFormResponse($showpoints = TRUE, $showfeedback = TRUE) {
    $rows = $innerheader = array();
    // Build the question answers header (add blank space for IE).
    $innerheader[] = t('Match');
    if ($showpoints) {
      $innerheader[] = t('Correct Answer');
    }
    $innerheader[] = t('Response');
    if ($showfeedback) {
      $innerheader[] = '&nbsp;';
    }
    //if (empty($this->question->answers)) {
    //  return t('Missing question.');
    //}
    $MatchingQuestion = new MatchingQuestion($this->question);
    $correct_answers = $MatchingQuestion->getCorrectAnswer();
    $user_answers = $this->getUserAnswers();
    foreach ($correct_answers as $correct_answer) {
      $cols = array();
      $id = $user_answers[$correct_answer['match_id']];
      $theme = ($correct_answer['answer'] == $correct_answers[$id]['answer']) ? 'quiz_score_correct' : 'quiz_score_incorrect';
      $cols[] = $correct_answer['question'];
      if ($showpoints)
        $cols[] = $correct_answer['answer'];
      $cols[] = $correct_answers[$id]['answer'];
      if ($showfeedback)
        $cols[] = array('data' => theme($theme), 'class' => 'quiz_summary_qcell');

      $rows[] = $cols;
    }
    // Add the cell with the question and the answers.
    $table = theme('table', $innerheader, $rows) .'<br />';
    return array(
      '#type' => 'markup',
      '#value' => $table,
    );
  }
}
