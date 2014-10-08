<?php

namespace Drupal\quiz\Helper\Node;

abstract class NodeHelper {

  /**
   * Insert call specific to result options.
   *
   * This is called by quiz_insert().
   *
   * @param $node
   *   The quiz node.
   */
  protected function insertResultOptions($quiz) {
    if (!isset($quiz->resultoptions)) {
      return;
    }

    $query = db_insert('quiz_node_result_options')
      ->fields(array('nid', 'vid', 'option_name', 'option_summary', 'option_summary_format', 'option_start', 'option_end'));

    foreach ($quiz->resultoptions as $id => $option) {
      if (!empty($option['option_name'])) {
        // When this function called direct from node form submit the $option['option_summary']['value'] and $option['option_summary']['format'] are we need
        // But when updating a quiz node eg. on manage questions page, this values come from loaded node, not from a submitted form.
        if (is_array($option['option_summary'])) {
          $option['option_summary_format'] = $option['option_summary']['format'];
          $option['option_summary'] = $option['option_summary']['value'];
        }
        $query->values(array(
          'nid' => $quiz->nid,
          'vid' => $quiz->vid,
          'option_name' => $option['option_name'],
          'option_summary' => $option['option_summary'],
          'option_summary_format' => $option['option_summary_format'],
          'option_start' => $option['option_start'],
          'option_end' => $option['option_end']
        ));
      }
    }

    $query->execute();
  }

  /**
   * Common actions that need to be done before a quiz is inserted or updated
   *
   * @param $quiz
   *   Quiz node
   */
  protected function presaveActions(&$quiz) {
    $this->translateFormDate($quiz, 'quiz_open');
    $this->translateFormDate($quiz, 'quiz_close');

    if (empty($quiz->pass_rate)) {
      $quiz->pass_rate = 0;
    }

    if ($quiz->randomization < 2) {
      $quiz->number_of_random_questions = 0;
    }
  }

  /**
   * Handles the start and end times in a node form submission.
   * - Takes the array from form_date() and turns it into a timestamp
   * - Adjusts times for time zone offsets.
   * - Adapted from event.module
   *
   * @param $node The submitted node with form data.
   * @param $date_field_name The name of the date ('quiz_open' or 'quiz_close') to translate.
   */
  private function translateFormDate(&$node, $date_field_name) {
    $prefix = $node->$date_field_name;
    // If we have all the parameters, re-calculate $node->event_$date .
    if (is_array($prefix) && isset($prefix['year']) && isset($prefix['month']) && isset($prefix['day'])) {
      // Build a timestamp based on the date supplied and the configured timezone.
      $node->$date_field_name = _quiz_mktime(0, 0, 0, $prefix['month'], $prefix['day'], $prefix['year'], 0);
    }
    else {
      if (!_quiz_is_int($prefix, 1, 2147483647)) {
        form_set_error('quiz_open', t('Please supply a valid date.'));
      }
    }
  }

}
