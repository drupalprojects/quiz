<?php

/////////////////////////////////
// Date and time routines for use with quiz module.
// - Based on event module
// - All references to event variables should be optional
/////////////////////////////////

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
function _quiz_mktime($hour, $minute, $second, $month, $day, $year, $offset = NULL) {
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
function _quiz_date($format, $timestamp, $offset = NULL) {
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
function _quiz_form_prepare_date($time = '', $offset = 0) {
  // If this is empty, get the current time.
  if ($time == '') {
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
      'day' => _quiz_date('j', $time),
      'month' => _quiz_date('n', $time),
      'year' => _quiz_date('Y', $time),
    );
  }
  return $time_array;
}
