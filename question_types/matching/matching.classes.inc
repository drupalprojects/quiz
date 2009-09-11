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

  public function save($is_new = FALSE) {
    parent::save($is_new);

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
    $props = parent::getNodeProperties();

    $sql = "SELECT match_id, question, answer, feedback FROM {quiz_matching_node} WHERE nid = %d AND vid = %d";
    $results = db_query($sql, $this->node->nid, $this->node->vid);
    while ($result = db_fetch_object($results)) {
      $props['answer'][] = array(
        'match_id' => $result->match_id,
        'question' => $result->question,
        'answer' => $result->answer,
        'feedback' => $result->feedback,
      );
    }
    return $props;
  }

  public function getNodeView() {
    $content = parent::getNodeView();

    $content['answers'] = array();

    $header = array(t('Subquestion'), t('Correct match'));
    list($questions, $select_option) = $this->getSubquestions();
    foreach ($questions as $question) {
      $rows[] = array($question['question'], $question['answer']);
    }

    $content['answers'] = array(
       '#value' => theme('table', $header, $rows),
       '#weight' => 2,
    );
    return $content;
  }

  // This is called whenever a question is rendered, either
  // to an administrator or to a quiz taker.
  public function getAnsweringForm(array $form_state = NULL) {
    list($questions, $select_option) = $this->getSubquestions();
    foreach ($questions as $question) {
      $title = $question['question'];
      if (!_quiz_is_taking_context()) {
        $title .= " (answer is '$question[answer]')";
      }
      $form['tries[' . $question['match_id'] . ']'] = array(
        '#type' => 'select',
        '#title' => $title,
        '#options' => $this->customShuffle($select_option),
      );
    }
    return variable_get('quiz_matching_shuffle_options', TRUE) ? $this->customShuffle($form) : $form;
  }

  private function customShuffle(array $array = array()) {
    $new_array = array();
    while (count($array)) {
      $element = array_rand($array);
      $new_array[$element] = $array[$element];
      unset($array[$element]);
    }
    return $new_array;
  }

  private function getSubquestions() {
    $questions = $select_option = array();
    $sql = "SELECT match_id, question, answer, feedback FROM {quiz_matching_node} WHERE nid = %d AND vid = %d";
    $results = db_query($sql, $this->node->nid, $this->node->vid);
    while ($result = db_fetch_object($results)) {
      $questions[] = array(
        'match_id' => $result->match_id,
        'question' => $result->question,
        'answer' => $result->answer,
        'feedback' => $result->feedback,
      );
      $select_option[$result->match_id] = $result->answer;
    }
    return array($questions, $select_option);
  }

  public function getAdminForm(array $form_state = NULL) {
    $form['matching'] = array(
      '#type' => 'fieldset',
      '#title' => t('Matching Settings'),
      '#description' => t('Matching Questions Settings and Configuration'),
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
    );
    $form['matching']['quiz_matching_form_size'] = array(
      '#type' => 'textfield',
      '#title' => t('Match Question Size'),
      '#description' => t('Number of question allowed to wrap under a match type question.'),
      '#default_value' => variable_get('quiz_matching_form_size', 5),
    );
    $form['matching']['quiz_matching_shuffle_options'] = array(
      '#type' => 'checkbox',
      '#title' => t('Shuffle Matching Options'),
      '#default_value' => variable_get('quiz_matching_shuffle_options', TRUE),
      '#description' => t('If checked matching options will be shuffled'),
    );
    return system_settings_form($form);
  }

  public function getCreationForm(array $form_state = NULL) {
    $form['match'] = array(
      '#type' => 'fieldset',
      '#title' => t('Match Question'),
      '#weight' => -4,
      '#tree' => TRUE,
    );
    for ($i = 1; $i <= variable_get('quiz_matching_form_size', 5); ++$i) {
      $form['match'][$i] = array(
        '#type' => 'fieldset',
        '#title' => t('Question '. $i),
      );
      $form['match'][$i]['match_id'] = array(
        '#type' => 'hidden',
        '#default_value' => isset($this->node->answer[$i-1]['match_id']) ? $this->node->answer[$i-1]['match_id'] : ''
      );
      $form['match'][$i]['question'] = array(
        '#type' => 'textarea',
        '#title' => t('Question'),
        '#rows' => 2,
        '#default_value' => isset($this->node->answer[$i-1]['question']) ? $this->node->answer[$i-1]['question'] : ''
      );
      $form['match'][$i]['answer'] = array(
        '#type' => 'textarea',
        '#title' => t('Match'),
        '#rows' => 2,
        '#default_value' => isset($this->node->answer[$i-1]['answer']) ? $this->node->answer[$i-1]['answer'] : ''
      );

      $form['match'][$i]['feedback'] = array(
        '#type' => 'textarea',
        '#title' => t('Feedback / Reason'),
        '#rows' => 2,
        '#default_value' => isset($this->node->answer[$i-1]['feedback']) ? $this->node->answer[$i-1]['feedback'] : ''
      );
    }
    return $form;
  }

  public function getMaximumScore() {
    return count($this->node->answer);
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
      $sql = 'SELECT ua.answer, score
              FROM {quiz_matching_user_answers} ua
              JOIN {quiz_matching_node} n
              ON n.match_id = ua.match_id
              WHERE n.vid = %d';
      $res = db_query($sql, $question_node->vid);
      $this->answer = array();
      while ($obj = db_fetch_object($res)) {
        $this->answer[$obj->answer] = $obj->answer;
      }
    }
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
    $user_answers = isset($this->answer) ? $this->answer : $this->getUserAnswers();
    $user_answers = isset($user_answers) ? $user_answers : array();
    // to prevent warning : Invalid argument supplied for foreach()
    foreach ((array)$user_answers as $key => $value) {
      if ($key != $value) {
        $wrong_answer++;
      }
      else $correct_answer++;
    }
    $score = $correct_answer - $wrong_answer;
    return $score < 0 ? 0 : $score;
  }

  public function getResponse() {
    return $this->answer;
  }

  public function getUserAnswers() {
    $user_answers = array();
    // answer_id  match_id  result_id   score   answer #{quiz_matching_user_answers} fields
    $results = db_query("SELECT match_id, answer FROM {quiz_matching_user_answers} WHERE result_id = %d", $this->rid);
    while ($result = db_fetch_object($results)) {
      $user_answers[$result->match_id] = $result->answer;
    }
    return $user_answers;
  }

  public function formatReport($showpoints = TRUE, $showfeedback = TRUE) {
    $rows = $innerheader = array();
    // Build the question answers header (add blank space for IE).
    $innerheader[] = t('Match');
    if ($showpoints) {
      $innerheader[] = t('Correct Answer');
    }
    $innerheader[] = t('User Answer');
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
      $id = $user_answers[$correct_answer['match_id']];
      $theme = ($correct_answer['answer'] == $correct_answers[$id]['answer']) ? 'quiz_score_correct' : 'quiz_score_incorrect';
      $rows[] = array(
        'match' => $correct_answer['question'],
        'correct_answer' => ($showpoints) ? $correct_answer['answer'] : '',
        'user_answer' => ($showfeedback) ? $correct_answers[$id]['answer'] : '',
        'image' => ($showfeedback) ? array('data' => theme($theme), 'class' => 'quiz_summary_qcell') : '',
      );
    }
    // Add the cell with the question and the answers.
    $q_output = '<div class="quiz_summary_question"><span class="quiz_question_bullet">Q:</span> '. check_markup($this->question->body, $this->question->format, FALSE) .'</div>';
    $q_output .= theme('table', $innerheader, $rows) .'<br />';
    return $q_output;
  }
}
