<?php

namespace Drupal\quiz\Helper\Node;

abstract class NodeHelper {

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
      $node->$date_field_name = $this->mktime(0, 0, 0, $prefix['month'], $prefix['day'], $prefix['year'], 0);
    }
    else {
      if (!_quiz_is_int($prefix, 1, 2147483647)) {
        form_set_error('quiz_open', t('Please supply a valid date.'));
      }
    }
  }

  /**
   * Formats local time values to GMT timestamp using time zone offset supplied.
   * All time values in the database are GMT and translated here prior to insertion.
   *
   * Time zone settings are applied in the following order:
   * 1. If supplied, time zone offset is applied
   * 2. If user time zones are enabled, user time zone offset is applied
   * 3. If neither 1 nor 2 apply, the site time zone offset is applied
   *
   * @param $hour
   * @param $minute
   * @param $second
   * @param $month
   * @param $day
   * @param $year
   * @param $offset
   *   Time zone offset to apply to the timestamp.
   * @return timestamp
   */
  private function mktime($hour, $minute, $second, $month, $day, $year, $offset = NULL) {
    global $user;
    //print $user->timezone. " and ". variable_get('date_default_timezone', 0);
    $timestamp = gmmktime($hour, $minute, $second, $month, $day, $year);
    if (variable_get('configurable_timezones', 1) && $user->uid && strlen($user->timezone)) {
      return $timestamp - $user->timezone;
    }
    else {
      return $timestamp - variable_get('date_default_timezone', 0);
    }
  }

  /**
   * If a quiz is saved as not randomized we should make sure all random questions
   * are converted to always.
   *
   * @param $quiz
   *   Quiz node.
   */
  protected function checkNumRandom(&$quiz) {
    if ($quiz->randomization == 2) {
      return;
    }

    db_delete('quiz_relationship')
      ->condition('question_status', QUESTION_RANDOM)
      ->condition('quiz_vid', $quiz->vid)
      ->execute();
  }

}
