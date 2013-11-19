<?php

/* @file
 * Contains \Drupal\multichoice\MultichoiceResponse.
 */

namespace Drupal\quiz_ddlines;

use Drupal\quiz_question\QuizQuestionResponse;


/**
 * Extension of QuizQuestionResponse
 */
class DDLinesResponse extends QuizQuestionResponse {

  // Contains a assoc array with label-ID as key
  // and hotspot-ID as value:
  protected $user_answers = array();

  /**
   * Constructor
   */
  public function __construct($result_id, $question_node, $tries = NULL) {
    parent::__construct($result_id, $question_node, $tries);

    // Is answers set in form?
    if (isset($tries)) {
      // Tries contains the answer decoded as JSON:
      // {"label_id":x,"hotspot_id":y},{...}
      $decoded = json_decode($tries);
      if(is_array($decoded)) {
        foreach ($decoded as $answer) {
          $this->user_answers[$answer->label_id] = $answer->hotspot_id;
        }
      }
    }
    // Load from database
    else {
      $res = db_query('SELECT label_id, hotspot_id FROM {quiz_ddlines_user_answers} ua
              LEFT OUTER JOIN {quiz_ddlines_user_answer_multi} uam ON(uam.user_answer_id = ua.id)
              WHERE ua.result_id = :result_id AND ua.question_nid = :question_nid AND ua.question_vid = :question_vid', array(':result_id' => $result_id, ':question_nid' => $this->question->id(), ':question_vid' => $this->question->getRevisionId()));
      while ($row = $res->fetch()) {
        $this->user_answers[$row->label_id] = $row->hotspot_id;
      }
    }
  }

  /**
   * Save the current response.
   */
  public function save() {
    $user_answer_id = db_insert('quiz_ddlines_user_answers')
      ->fields(array(
        'question_nid' => $this->question->id(),
        'question_vid' => $this->question->getRevisionId(),
        'result_id' => $this->rid,
      ))
      ->execute();

    // Each alternative is inserted as a separate row
    $query = db_insert('quiz_ddlines_user_answer_multi')
      ->fields(array('user_answer_id', 'label_id', 'hotspot_id'));
    foreach ($this->user_answers as $key => $value) {
      $query->values(array($user_answer_id, $key, $value));
    }
    $query->execute();

  }

  /**
   * Delete the response.
   */
  public function delete() {

    $user_answer_ids = array();
    $query = db_query('SELECT id FROM {quiz_ddlines_user_answers} WHERE question_nid = :nid AND question_vid = :vid AND result_id = :result_id', array(':nid' => $this->question->id(), ':vid' => $this->question->getRevisionId(), ':result_id' => $this->rid));
    while ($answer = $query->fetch()) {
      $user_answer_ids[] = $answer->id;
    }

    if (!empty($user_answer_ids)) {
      db_delete('quiz_ddlines_user_answer_multi')
        ->condition('user_answer_id', $user_answer_ids, 'IN')
        ->execute();
    }

    db_delete('quiz_ddlines_user_answers')
      ->condition('result_id', $this->rid)
      ->condition('question_nid', $this->question->id())
      ->condition('question_vid', $this->question->getRevisionId())
      ->execute();
  }

  /**
   * Calculate the score for the response.
   */
  public function score() {
    $results = $this->getDragDropResults();

    // Count number of correct answers:
    $correct_count = 0;

    foreach($results as $result) {
      $correct_count += ($result == AnswerStatus::CORRECT) ? 1 : 0;
    }

    return $correct_count;
  }

  /**
   * Get the user's response.
   */
  public function getResponse() {
    return $this->user_answers;
  }

  public function getReportFormResponse($showpoints = TRUE, $showfeedback = TRUE, $allow_scoring = FALSE) {
    // Have to do node_load, since quiz does not do this. Need the field_image...

   // print_r($this->question->{'field_image'}->getValue()); exit;

    //$img_field = field_get_items(node_load($this->question->id()), 'field_image');
    //$img_rendered = theme('image', array('path' => image_style_url('large',$img_field[0]['uri'])));

    $image_path = base_path() . drupal_get_path('module', 'quiz_ddlines').'/theme/images/';

    $html = '<h3>'.t('Your answers').'</h3>';
    $html .= '<div class="icon-descriptions"><div><img src="'.$image_path.'icon_ok.gif">'.t('Means alternative is placed on the correct spot').'</div>';
    $html .= '<div><img src="'.$image_path.'icon_wrong.gif">'.t('Means alternative is placed on the wrong spot, or not placed at all'). '</div></div>';
    $html .= '<div class="quiz-ddlines-user-answers" id="'.$this->question->id().'">';
    //$html .= $img_rendered;
    $html .= '</div>';
    $html .= '<h3>'.t('Correct answers').'</h3>';
    $html .= '<div class="quiz-ddlines-correct-answers" id="'.$this->question->id().'">';
    //$html .= $img_rendered;
    $html .= '</div>';

    // No form to put things in, are therefore using the js settings instead
    $settings = array();
    $correct_id = "correct-{$this->question->id()}";
    $settings[$correct_id] = json_decode($this->question->ddlines_elements);
    $elements = $settings[$correct_id]->elements;

    // Convert the user's answers to the same format as the correct answers
    $answers = clone $settings[$correct_id];
    // Keep everything except the elements:
    $answers->elements = array();

    $elements_answered = array();

    foreach ($this->user_answers as $label_id => $hotspot_id ) {

      if(!isset($hotspot_id)) {
        continue;
      }

      // Find correct answer:
      $element = array(
        'feedback_wrong' => '',
        'feedback_correct' => '',
        'color' => $this->getElementColor($elements, $label_id)
      );

      $label = $this->getLabel($elements, $label_id);
      $hotspot = $this->getHotspot($elements, $hotspot_id);

      if(isset($hotspot)) {
        $elements_answered[] = $hotspot->id;
        $element['hotspot'] = $hotspot;
      }

      if(isset($label)) {
        $elements_answered[] = $label->id;
        $element['label'] = $label;
      }

      $element['correct'] = $this->isAnswerCorrect($elements, $label_id, $hotspot_id);
      $answers->elements[] = $element;
    }

    // Need to add the alternatives not answered by the user.
    // Create dummy elements for these:
    foreach ($elements as $el) {
      if(!in_array($el->label->id, $elements_answered)) {
        $element = array(
          'feedback_wrong' => '',
          'feedback_correct' => '',
          'color' => $el->color,
          'label' => $el->label,
        );
        $answers->elements[] = $element;
      }

      if(!in_array($el->hotspot->id, $elements_answered)) {
        $element = array(
          'feedback_wrong' => '',
          'feedback_correct' => '',
          'color' => $el->color,
          'hotspot' => $el->hotspot,
        );
        $answers->elements[] = $element;
      }
    }

    $settings["answers-{$this->question->id()}"] = $answers;
    $settings['mode'] = 'result';
    $settings['execution_mode'] = $this->question->execution_mode;
    $settings['hotspot']['radius'] = $this->question->hotspot_radius;

    // Image path:
    $settings['quiz_imagepath'] = base_path() . drupal_get_path('module', 'quiz_ddlines').'/theme/images/';

    drupal_add_js(array('quiz_ddlines' => $settings), 'setting');

    _quiz_ddlines_add_js_and_css();

    return array('#markup' => $html);
  }

  private function getElementColor($list, $id) {
    foreach($list as $element) {
      if ($element->label->id == $id) {
        return $element->color;
      }
    }
  }

  private function getHotspot($list, $id) {
    foreach($list as $element) {
      if ($element->hotspot->id == $id) {
        return $element->hotspot;
      }
    }
  }

  private function getLabel($list, $id) {
    foreach($list as $element) {
      if ($element->label->id == $id) {
        return $element->label;
      }
    }
  }

  private function isAnswerCorrect($list, $label_id, $hotspot_id) {
    foreach ($list as $element) {
      if($element->label->id == $label_id) {
        return ($element->hotspot->id == $hotspot_id);
      }
    }

    return false;
  }

  /**
   *
   * Get a list of the labels, tagged correct, false, or no answer
   */
  private function getDragDropResults() {
    $results = array();

    // Iterate through the correct answers, and check
    // the users answer:
    foreach(json_decode($this->question->ddlines_elements)->elements as $element) {
      $source_id = $element->label->id;

      if(isset($this->user_answers[$source_id])) {
        $results[$element->label->id] = ($this->user_answers[$source_id] == $element->hotspot->id) ? AnswerStatus::CORRECT : AnswerStatus::WRONG;
      }
      else {
        $results[$element->label->id] = AnswerStatus::NO_ANSWER;
      }
    }

    return $results;
  }
}
