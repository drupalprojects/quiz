<?php

/**
 * @file
 * Administration file for Quiz stats Module
 *
 */

/**
 * Returns statistics for all available quizzes
 *
 * @param int $uid
 *   User id
 * @return
 *   HTML table
 */
function quiz_stats_get_basic_stats($uid = 0) {
  $results = quiz()->getQuizHelper()->getQuizzesByUserId($uid);
  return empty($results) ? t('No Statistics found.') : theme('quiz_stats_get_basic_stats', array('results' => $results));
}

/**
 * Computes quiz attempt count
 *
 * @param int $vid
 *   quiz id
 * @param int $uid
 *   user id for filtering
 * @return
 *   Integer containing number of times that a quiz has
 *   been attended, can be filtered by uid.
 */
function _quiz_get_attempt_count($nid, $uid = 0) {
  $sql = 'SELECT COUNT(result_id) FROM {quiz_results} WHERE nid = :nid';
  $arg[':nid'] = $nid;
  if ($uid != 0) {
    $sql .= ' AND uid = :uid';
    $arg[':uid'] = $uid;
  }
  return db_query($sql, $arg)->fetchField();
}

// Theme functions

/**
 * Theme function for quiz grade range chart generates Chart using CHART API function
 *
 * @param $range
 *   array containg quiz results data structure
 *
 * @return
 *   HTML to render/display chart
 */
function theme_quiz_grade_range($variables) {
  $range = $variables['range'];

  $max = max((array) $range);
  $count = array_sum((array) $range);

  $chart = array(
    '#type'       => 'chart',
    '#chart_type' => 'column',
    '#title'      => t('Score distribution'),
  );
  $chart['range'] = array(
    '#type'   => 'chart_data',
    '#title'  => t('% of quiz takers'),
    '#data'   => array(
      ($range->zero_to_twenty / $count) * 100,
      $range->twenty_to_fourty / $count * 100,
      $range->fourty_to_sixty / $count * 100,
      $range->sixty_to_eighty / $count * 100,
      $range->eighty_to_hundred / $count * 100,
    ),
    '#suffix' => '%',
  );
  $chart['xaxis'] = array(
    '#title'  => 'Score',
    '#type'   => 'chart_xaxis',
    '#labels' => array(
      '0-20%',
      '20-40%',
      '40-60%',
      '60-80%',
      '80-100%',
    ),
  );
  $chart['yaxis'] = array(
    '#title' => 'Distribution',
    '#type'  => 'chart_yaxis',
    '#max'   => $max * 100,
  );
  return drupal_render($chart);
}

/**
 * generates a chart of quiz top scorers
 *
 * @param $attendees
 *   array containing quiz data structure
 *
 * @return
 *   HTML to render quiz top scorers chart
 */
function theme_quiz_top_scorers($variables) {
  $attendees = $variables['scorer'];
  $chart = array(
    '#chart_id'   => 'quiz_top_scorers',
    '#title'      => t('Quiz Top Scorers'),
    '#type'       => CHART_TYPE_BAR_V_GROUPED,
    '#size'       => chart_size(600, 400),
    '#grid_lines' => chart_grid_lines(10, 10),
    '#bar_size'   => chart_bar_size(20, 15),
  );
  foreach ($attendees as $attendee) {
    $chart['#data'][][] = $attendee['score'];
    $chart['#data_colors'][] = chart_unique_color(count($chart['#data']));
    $chart['#mixed_axis_labels'][CHART_AXIS_X_BOTTOM][2][] = chart_mixed_axis_label(t('Quiz Attendees'), 50);
    $chart['#legends'][] = $attendee['name'] . ' ' . $attendee['score'] . ' %';
  }
  $chart['#mixed_axis_labels'][CHART_AXIS_Y_LEFT][1][] = chart_mixed_axis_label(t('Score') . ' (%)', 100);
  $chart['#mixed_axis_labels'][CHART_AXIS_Y_LEFT][0][] = chart_mixed_axis_range_label(0, 100);
  return theme('chart', array('chart' => $chart));
}

/**
 * generates quiz status chart number of pass, fail, incomplete
 *
 * @param $quiz
 *   array of quiz data structure
 *
 * @return
 *   HTML to render quiz status chart
 */
function theme_get_quiz_status_chart($variables) {
  $quiz = $variables['quiz'];
  $quiz['total'] = $quiz['no_pass'] + $quiz['no_fail'] + $quiz['no_incomplete'];

  $chart = array(
    '#type'       => 'chart',
    '#title'      => t('Quiz results'),
    '#chart_type' => 'pie',
  );

  $chart['pie_data'] = array(
    '#type'   => 'chart_data',
    '#title'  => t('Result'),
    '#labels' => array('Pass', 'Incomplete', 'Fail'),
    '#data'   => array(
      ($quiz['no_pass'] / $quiz['total']) * 100,
      ($quiz['no_incomplete'] / $quiz['total']) * 100,
      ($quiz['no_fail'] / $quiz['total']) * 100,
    ),
  );
  return drupal_render($chart);
}

/**
 * Generates date vs takeup count chart
 *
 * @param $takeup
 *   Array quiz data structure
 * @return
 *   HTML to render/display chart
 */
function theme_date_vs_takeup_count($variables) {
  $days = $variables['takeup'];
  $days = array_reverse($days);
  $max_count = 0;
  $chart = array(
    '#chart_id'          => 'test_chart',
    '#title'             => '',
    '#type'              => CHART_TYPE_LINE,
    '#size'              => chart_size(600, 400),
    //'#chart_fill' => chart_fill('c', 'eeeeee'),
    '#grid_lines'        => chart_grid_lines(10, 10, 1, 5),
    '#adjust_resolution' => TRUE,
  );
  $interval = 1;
  foreach ($days as $date => $obj) {
    $count = $obj->value;
    $chart['#data'][] = $count;
    $x_label = '|';
    if ($interval == 7) {
      $x_label = chart_mixed_axis_label(format_date($obj->timestamp, 'custom', 'd M'));
      $interval = 0;
    }
    $chart['#mixed_axis_labels'][CHART_AXIS_X_BOTTOM][0][] = $x_label;
    $interval++;
    $max_count = ($max_count > $count) ? $max_count : $count;
  }
  $chart['#mixed_axis_labels'][CHART_AXIS_Y_LEFT][0][] = chart_mixed_axis_range_label(0, $max_count);

  // The chart module has some bugs. If we have the time we should provide patches instead of tricking it to work...
  $chart['#mixed_axis_labels'][CHART_AXIS_Y_LEFT][1] = array('|', chart_mixed_axis_label(t('Attempts')));
  $chart['#mixed_axis_labels'][CHART_AXIS_X_BOTTOM][1] = array('|', chart_mixed_axis_label(t('Date')), '|');
  return theme('chart', array('chart' => $chart));
}

/**
 * Generates table of results from quiz data structure.
 *
 * @param @results
 *   Array containing quiz results data structure
 * @return
 *   HTML table containing quiz title, author, questions count, attempt count
 */
function theme_quiz_stats_get_basic_stats($variables) {
  $results = $variables['results'];
  if (empty($results)) {
    return '<p>' . t('No questions were found.') . '</p>';
  }
  $rows = array();
  $header = array(
    'title'   => t('Quiz'),
    'creator' => t('Author'),
    'created' => t('Created'),
  );
  if (user_access('access author stats')) {
    $header['questions_count'] = t('Questions');
    $header['attempt_count'] = t('Attempts');
  }
  $p = drupal_get_path('module', 'quiz_stats');
  $chart_icon = theme('image', array('path' => "$p/chart.png", 'width' => t('Charts'), 'height' => t('See charts')));
  foreach ($results as $result) {
    $title_link = user_access('access author stats') ? 'node/' . $result['nid'] . '/quiz/statistics' : 'user/' . arg(1) . '/stats/';
    $row = array(
      'title'   => l($chart_icon . ' ' . $result['title'], $title_link, array('html' => TRUE)),
      'creator' => l($result['name'], 'user/' . $result['uid']),
      'created' => format_date($result['created'], 'short'),
    );
    if (user_access('access author stats')) {
      $row['questions_count'] = quiz()->getQuizHelper()->countQuestion($result['vid']);
      $row['attempt_count'] = _quiz_get_attempt_count($result['nid']);
    }
    $rows[] = $row;
  }
  module_load_include('inc', 'quiz', 'quiz.pages');
  $cc = '<em>' . t('Chart icon from !url', array('!url' => 'pinvoke.com')) . '</em>';
  return theme('table', array('header' => $header, 'rows' => $rows, 'attributes' => array('id' => 'tablesorter'))) . $cc;
}
