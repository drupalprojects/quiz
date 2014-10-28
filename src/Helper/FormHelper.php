<?php

namespace Drupal\quiz\Helper;

/**
 * Date and time routines for use with quiz module.
 * - Based on event module
 * - All references to event variables should be optional
 */
class FormHelper {

  /**
   * Formats a GMT timestamp to local date values using time zone offset supplied.
   * All timestamp values in event nodes are GMT and translated for display here.
   *
   * Pulled from event
   *
   * Time zone settings are applied in the following order
   * 1. If supplied, time zone offset is applied
   * 2. If user time zones are enabled, user time zone offset is applied
   * 3. If neither 1 nor 2 apply, the site time zone offset is applied
   *
   * @param $format
   *   The date() format to apply to the timestamp.
   * @param $timestamp
   *   The GMT timestamp value.
   * @param $offset
   *   Time zone offset to apply to the timestamp.
   * @return gmdate() formatted date value
   */
  private function date($format, $timestamp, $offset = NULL) {
    global $user;

    if (isset($offset)) {
      $timestamp += $offset;
    }
    elseif (variable_get('configurable_timezones', 1) && $user->uid && strlen($user->timezone)) {
      $timestamp += $user->timezone;
    }
    else {
      $timestamp += variable_get('date_default_timezone', 0);
    }

    // make sure we apply the site first day of the week setting for dow requests
    $result = gmdate($format, $timestamp);
    return $result;
  }

  /**
   * Takes a time element and prepares to send it to form_date().
   *
   * @param $time
   *   The time to be turned into an array. This can be:
   *   - A timestamp when from the database.
   *   - An array (day, month, year) when previewing.
   *   - NULL for new nodes.
   *
   * @return
   *   An array for form_date (day, month, year).
   */
  protected function prepareDate($time = '', $offset = 0) {
    if (!$time) { // If this is empty, get the current time.
      $time = REQUEST_TIME + $offset * 86400;
    }

    // If we are previewing, $time will be an array so just pass it through.
    $time_array = array();
    if (is_array($time)) {
      $time_array = $time;
    }
    // Otherwise build the array from the timestamp.
    elseif (is_numeric($time)) {
      $time_array = array(
        'day'   => $this->date('j', $time),
        'month' => $this->date('n', $time),
        'year'  => $this->date('Y', $time),
      );
    }

    return $time_array;
  }

  public function getUserpointsType() {
    $userpoints_terms = taxonomy_get_tree(userpoints_get_vid());
    $userpoints_tids = array(0 => t('Select'));
    foreach ($userpoints_terms as $userpoints_term) {
      $userpoints_tids[$userpoints_term->tid] = str_repeat('-', $userpoints_term->depth) . $userpoints_term->name;
    }
    return $userpoints_tids;
  }

}
