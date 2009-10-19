<?php

// $Id$

/*
 * @file
 * Administration file for Quiz Dashboard Module
 *
 */

// Callback function


function quiz_dashboard_revision_selector_page($nid) {
  $sql = 'SELECT vid FROM {node_revisions} WHERE nid = %d ORDER BY vid';
  $res = db_query($sql, $nid);
  $count = 0; //uses this variable to slightly increase performance
  $vids = array();
  while ($res_o = db_fetch_object($res)) {
    $vids[] = $res_o->vid;
    $count++;
  }
  if ($count < 1) 
    return t('Something went wrong. Please try again');
  else if ($count == 1) {
    drupal_goto('admin/quiz/reports/dashboard/creator/'. $vids[0] .'/view');
  }
  else {
    $content = array();
    $content['explanation'] = t('There are !num revisions of this quiz that has been taken.
    Different revisions may have different scoring, difficulity and other differences affecting its statistics.
    Because of this you have to choose the revision you want to see statistics from.', array('!num' => $count));
    $content['links'] = array();
    foreach ($vids as $key => $value) {
      $content['links'][] = 'admin/quiz/reports/dashboard/creator/'. $value .'/view';
    }
    return theme('quiz_dashboard_revision_selector', $content);
  } 
}

/*
 * @function
 *   callback function
 *
 * @param $uid
 *   uid of user whose results statistics need to generated and displayed
 *
 * @see quiz.module
 *
 * @return
 *   HTML table containing quiz node title, link to report, questions count
 *   and attempt count
 */
function quiz_dashboard_get_basic_stats($uid = 0) {
  $results = _quiz_get_quizzes();
  return empty($results) ? t('No Statistics found.') : theme('quiz_dashboard_get_basic_stats', $results);
}

/*
 * @function
 *   callback function
 *
 * @param $vid
 *   quiz revision id
 *
 * @param $uid
 *   uid of user whose results statistics need to generated and displayed
 *
 * @return
 *   HTML page with charts/graphs
 */
function quiz_dashboard_get_adv_stats($vid = 0, $uid = 0) {
  $output = '<div class="quiz-dashboard-chart-wrapper">';
  $output .= _get_date_vs_takeup_count_chart($vid, $uid);
  // line chart/graph showing quiz takeup date along x-axis and count along y-axis
  $output .= _get_quiz_status_chart($vid, $uid);
  // 3D pie chart showing percentage of pass, fail, incomplete quiz status
  $output .= _get_quiz_top_scorers_chart($vid, $uid);
  // Bar chart displaying top scorers
  $output .= _get_quiz_grade_range_chart($vid, $uid);
  if (!strlen($output)) {
    drupal_set_message(t('Insufficient data, unable to generate chart'), 'warning');
  }
  return $output;
}

// End callback function

// model function generates data structure from table and invokes theming functions

/*
 * @function
 *   generates grade range chart
 *
 * @param $uid
 *   uid of user whose results statistics need to generated and displayed
 *
 * @param $vid
 *   revision id of quiz node
 *
 * @return
 *   HTML to render to chart/graph
 */
function _get_quiz_grade_range_chart($vid = 0, $uid = 0) {
  // ToDo make the range configurable
  $sql = "SELECT SUM(score >= 0 && score <= 20) AS zero_to_twenty,
    SUM(score >= 20 && score <= 40) AS twenty_to_fourty,
    SUM(score >= 40 && score <= 60) AS fourty_to_sixty,
    SUM(score >= 60 && score <= 80) AS sixty_to_eighty,
    SUM(score >= 80 && score <= 100) AS eighty_to_hundred
    FROM {quiz_node_results}
    WHERE vid = %d";
  if ($uid != 0) {
    $sql .= " AND uid = %d";
  }
  $range = db_fetch_object(db_query($sql, $vid, $uid));
  $count = $range->zero_to_twenty + $range->twenty_to_fourty + $range->fourty_to_sixty + $range->sixty_to_eighty + $range->eighty_to_hundred;
  if ($count < 2) {
    return;
  }
  $output = '<div id="quiz_top_scorers" class="quiz-dashboard-chart-space">';
  $output .= theme('quiz_grade_range', $range);
  $output .= '</div>';
  return $output;
}

/*
 * @function
 *   generates chart containing top 10 users name and their score
 *
 * @param $vid
 *   revision id of quiz node
 *
 * @param $uid
 *   uid of user whose results statistics need to generated and displayed
 *
 * @return
 *   HTML to render to chart/graph
 */
function _get_quiz_top_scorers_chart($vid = 0, $uid = 0) {
  $top_scorers = array();
  $sql = 'SELECT name, score FROM {quiz_node_results} qnr
    LEFT JOIN {users} u ON (u.uid = qnr.uid)
    WHERE vid = %d';
  if ($uid != 0) {
    $sql .= " AND qnr.uid = %d";
  }
  $sql .= ' ORDER by score DESC';
  $results = db_query_range($sql, $vid, $uid, 0, 10);
  while ($result = db_fetch_array($results)) {
    $key = $result['name'] . '-' . $result['score'];
    $top_scorers[$key] = $result;
  }
  $output = '<div id="quiz_top_scorers" class="quiz-dashboard-chart-space">';
  $output .= theme('quiz_top_scorers', $top_scorers);
  $output .= '</div>';
  return $output;
}

/*
 * @function
 *   generates a chart based on user's quiz status pass, fail, progress
 *
 * @param $vid
 *   revision id of quiz node
 *
 * @param $uid
 *   uid of user whose results statistics need to generated and displayed *
 *
 * @return
 *   HTML to render to chart/graph
 */
function _get_quiz_status_chart($vid = 0, $uid = 0) {
  // get the pass rate of the given quiz by vid
  $pass_rate = db_result(db_query("SELECT pass_rate FROM {quiz_node_properties} WHERE vid = %d", intval($vid)));
  // get the count value of results row above and below pass rate
  $sql = "SELECT SUM(score > $pass_rate) AS no_pass, SUM(score < $pass_rate) AS no_fail, SUM(score = 0) AS no_incomplete FROM {quiz_node_results} WHERE vid = %d";
  $quiz = db_fetch_array(db_query($sql, intval($vid)));
  if (($quiz['no_pass'] + $quiz['no_fail'] + $quiz['no_incomplete']) < 3) {
    return; // no sufficient data
  }
  // generates quiz status chart 3D pie chart
  $output = '<div id="get_quiz_status_chart" class="quiz-dashboard-chart-space">';
  $output .= theme('get_quiz_status_chart', $quiz);
  $output .= '</div>';
  return $output;
}

/*
 * @function
 *   generates quiz takeup count vs date chart
 *
 * @param $uid
 *   user id
 *
 * @param $vid
 *   revision id of quiz
 *
 * @return
 *   HTML to display chart
 */
function _get_date_vs_takeup_count_chart($vid = 0, $uid = 0) {
  $start = 0;
  $end = 30;
  $takeup = array();
  $sql = "SELECT COUNT(result_id) AS count,
            DATE_FORMAT(FROM_UNIXTIME(time_start), '%Y.%m.%e') AS date
            FROM {quiz_node_results}
            WHERE vid = %d";
  $sql_args = array($vid);
  if ($uid != 0) {
    $sql .= " AND uid = %d";
    $sql_args[] = $uid;
  }
  $sql .=" GROUP BY date ORDER BY time_start DESC";
  $results = db_query_range($sql, $sql_args, $start, $end);
  $days = _quiz_get_last_days($end);
  $res_o = db_fetch_object($results);
  foreach ($days as $date => &$value) {
   if ($date == $res_o->date) {
      $value->value = $res_o->count;
      $res_o = db_fetch_object($results);
    }
  }
  $output = '<div id="date_vs_takeup_count" class="quiz-dashboard-chart-space">';
  // wrapping the chart output with div for custom theming.
  $output .= theme('date_vs_takeup_count', $days);
  // generate date vs takeup count line chart
  $output .= '</div>';
  return $output;
}

/*
 * @function
 *   computes the number of questions been assigned
 *   to a quiz node.
 *
 * @param $vid
 *   quiz revision id
 *
 * @param $uid
 *   user id for filtering based on author optionally
 *
 * @return
 *   Imteger count of questions.
 */
function _quiz_get_questions_count($vid, $uid = 0) {
  $sql = "SELECT COUNT(child_nid) FROM {quiz_node_relationship} WHERE parent_vid = %d";
  if ($uid != 0) {
    $sql .= " AND uid = %d";
  }
  return db_result(db_query($sql, $vid, $uid));
}

/*
 * @function
 *   computes quiz attempt count
 *
 * @param $vid
 *   quiz revision id
 *
 * @param $uid
 *   user id of user for filtering
 *
 * @return
 *   Integer containing number of times that a quiz has
 *   been attended, can be filtered by uid optionally.
 */
function _quiz_get_attempt_count($vid, $uid = 0) {
  $sql = "SELECT COUNT(result_id) FROM {quiz_node_results} WHERE vid = %d";
  if ($uid != 0) {
    $sql .= " AND uid = %d";
  }
  return db_result(db_query($sql, $vid, $uid));
}

// End model functions

// Theme functions

/*
 * @function
 *   Theme function for quiz grade range chart
 *   generates Chart using CHART API function
 *
 * @param $range
 *   array containg quiz results data structure
 *
 * @return
 *   HTML to render/display chart
 */
function theme_quiz_grade_range($range) {
  $chart_width = 600;
  $chart_height = 400;

  $chart = array(
    '#chart_id' => 'quiz_grade_range',
    '#title' => t('Quiz Grade Range'),
    '#type' => CHART_TYPE_BAR_V_GROUPED,
    '#size' => chart_size($chart_width, $chart_height),
    '#grid_lines' => chart_grid_lines(10, 10),
    '#bar_size' => chart_bar_size(20, 15),
    '#adjust_resolution' => TRUE,
  );
  // chart data
  $chart['#data'][][] = $range->eighty_to_hundred;
  $chart['#data'][][] = $range->sixty_to_eighty;
  $chart['#data'][][] = $range->fourty_to_sixty;
  $chart['#data'][][] = $range->twenty_to_fourty;
  $chart['#data'][][] = $range->zero_to_twenty;
    
  $max = max($range->zero_to_twenty,
             $range->twenty_to_fourty,
             $range->fourty_to_sixty,
             $range->sixty_to_eighty,
             $range->eighty_to_hundred);
  // chart color
  $chart['#data_colors'][] = '00ff00';
  $chart['#data_colors'][] = '66ff00';
  $chart['#data_colors'][] = 'ffff00';
  $chart['#data_colors'][] = 'ff6600';
  $chart['#data_colors'][] = 'ff0000';
  // chart x-axis label
  $chart['#mixed_axis_labels'][CHART_AXIS_X_BOTTOM][2][] = chart_mixed_axis_label(t('Quiz Grade Range'), 50);
  // chart y-axis label and data
  $chart['#mixed_axis_labels'][CHART_AXIS_Y_LEFT][1][] = chart_mixed_axis_label(t('Attempts'), 90);
  $chart['#mixed_axis_labels'][CHART_AXIS_Y_LEFT][0][] = chart_mixed_axis_range_label(0, $max);
  // chart description on the right side
  $chart['#legends'][] = t('80 to 100 % - ') . $range->eighty_to_hundred;
  $chart['#legends'][] = t('60 to 80 % - ') . $range->sixty_to_eighty;
  $chart['#legends'][] = t('40 to 60 % - ') . $range->fourty_to_sixty;
  $chart['#legends'][] = t('20 to 40 % - ') . $range->twenty_to_fourty;
  $chart['#legends'][] = t('0 to 20 % - ') . $range->zero_to_twenty;
    
  return chart_render($chart);
}

/*
 * @function
 *   generates a chart of quiz top scorers
 *
 * @param $attendees
 *   array containing quiz data structure
 *
 * @return
 *   HTML to render quiz top scorers chart
 */
function theme_quiz_top_scorers($attendees) {
  $chart = array(
    '#chart_id' => 'quiz_top_scorers',
    '#title' => t('Quiz Top Scorers'),
    '#type' => CHART_TYPE_BAR_V_GROUPED,
    '#size' => chart_size(600, 400),
    '#grid_lines' => chart_grid_lines(10, 10),
    '#bar_size' => chart_bar_size(20, 15),
  );
  foreach ($attendees as $attendee) {
    $chart['#data'][][] = $attendee['score'];
    $chart['#data_colors'][] = chart_unique_color(count($chart['#data']));
    $chart['#mixed_axis_labels'][CHART_AXIS_X_BOTTOM][2][] = chart_mixed_axis_label(t('Quiz Attendees'), 50);
    $chart['#legends'][] = $attendee['name'] . ' ' . $attendee['score'] .' %';
  }
  $chart['#mixed_axis_labels'][CHART_AXIS_Y_LEFT][1][] = chart_mixed_axis_label(t('Score') .' (%)', 90);
  $chart['#mixed_axis_labels'][CHART_AXIS_Y_LEFT][0][] = chart_mixed_axis_range_label(0, 100);
  return chart_render($chart);
}

/*
 * @function
 *   generates quiz status chart number of pass, fail, incomplete
 *
 * @param $quiz
 *   array of quiz data structure
 *
 * @return
 *   HTML to render quiz status chart
 */
function theme_get_quiz_status_chart($quiz) {
  $quiz['total'] = $quiz['no_pass'] + $quiz['no_fail'] + $quiz['no_incomplete'];
  $chart = array(
    '#chart_id' => 'quiz_status_chart',
    '#title' => t('Quiz Status Chart'),
    '#type' => CHART_TYPE_PIE_3D,
    '#size' => chart_size(500, 200),
    '#adjust_resolution' => TRUE,
  );
  $chart['#data'] = array(
    'pass' => $quiz['no_pass'],
    'incomplete' => $quiz['no_incomplete'],
    'fail' => $quiz['no_fail'],
  );
  $chart['#data_colors'] = array(
    'pass' => '00ff00',
    'incomplete' => 'ffff00',
    'fail' => 'ff0000',
  );
  ini_set('precision', '3');
  // FIX IT deprecated.
  $chart['#legends'] = array(
    'pass' => ($quiz['no_pass'] / $quiz['total']) * 100 . t('% pass'),
    'incomplete' => ($quiz['no_incomplete'] / $quiz['total']) * 100 . t('% incomplete'),
    'fail' => ($quiz['no_fail'] / $quiz['total']) * 100 . t('% fail'),
  );
  return chart_render($chart);
}

/*
 * @function
 *   generates date vs takeup count chart
 *
 * @param $takeup
 *   Array quiz data structure
 *
 * @return
 *   HTML to render/display chart
 */
function theme_date_vs_takeup_count($days) {
  $days = array_reverse($days);
  $max_count = 0;
  $chart = array(
    '#chart_id' => 'test_chart',
    '#title' => t('Date vs Quiz Takeup'),
    '#type' => CHART_TYPE_LINE,
    '#size' => chart_size(600, 400),
    //'#chart_fill' => chart_fill('c', 'eeeeee'),
    '#grid_lines' => chart_grid_lines(10, 10, 1, 5),
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
    $chart['#mixed_axis_labels'][CHART_AXIS_X_BOTTOM][1][] = $x_label;
    $interval++;
    $max_count = ($max_count > $count) ? $max_count : $count;
  }
  $chart['#mixed_axis_labels'][CHART_AXIS_Y_LEFT][0][] = chart_mixed_axis_range_label(0, $max_count);
  $chart['#mixed_axis_labels'][CHART_AXIS_Y_LEFT][1][] = chart_mixed_axis_label(t('Count'), 90);
  $chart['#mixed_axis_labels'][CHART_AXIS_X_BOTTOM][2][] = chart_mixed_axis_label(t('Days of the week'), 50);
  return chart_render($chart);
}

/*
 * @function
 *   generates table of results from quiz data structure.
 *
 * @param @results
 *   Array containing quiz results data structure
 *
 * @return
 *   HTML table containing quiz title, author, questions count, attempt count
 */
function theme_quiz_dashboard_get_basic_stats($results = array()) {
  $header = $rows = array();
  //$title_link = user_access('access author dashboard') ? 'admin/quiz/reports/dashboard/creator/'. $result['vid'] .'/view' : 'user/%/dashboard/%/view'; .
  $title_link = user_access('access author dashboard') ? 'admin/quiz/reports/dashboard/creator/' : 'user/'. arg(1) .'/dashboard/';
  if (empty($results)) {
    return '<p>' . t('No questions were found.') . '</p>';
  }
  $header = array(
    'title' => t('Quiz'),
    'creator' => t('Author'),
    'created' => t('Created'),
    'questions_count' => user_access('access author dashboard') ? t('Questions') : '',
    'attempt_count' => user_access('access author dashboard') ? t('Attempts') : '',
  );
  $p = drupal_get_path('module', 'quiz_dashboard');
  $chart_icon = theme('image', "$p/chart.png", t('Charts'), t('See charts'));
  foreach ($results as $result) {
    $rows[] = array(
      'title' => l($chart_icon .' '. $result['title'], $title_link . $result['nid'] .'/revisions', array('html' => TRUE)),
      'creator' => l($result['name'], 'user/'. $result['uid']),
      'created' => format_date($result['created'], 'small'),
      'questions_count' => user_access('access author dashboard') ? _quiz_get_questions_count($result['nid']) : t(''),
      'attempt_count' => user_access('access author dashboard') ? _quiz_get_attempt_count($result['nid']) : t(''),
    );
  }
  module_load_include('inc', 'quiz', 'quiz.pages');
  $cc = '<I>'. t('Chart icon from !url', array('!url' => 'pinvoke.com')) .'</I>';
  // FIX IT need to get community feedback.
  //quiz_add_table_sorter();
  // quiz_add_table_sorter(); function defined in quiz.page.inc includes css and js for jQuery table sorter.
  return theme('table', $header, $rows, array('id' => 'tablesorter')) . $cc;
}

function _quiz_get_last_days($num_days) {
  $to_return = array();
  $now = time();
  $one_day = 86400;
  for ($i = 0; $i < $num_days; $i++) {
    $timestamp = $now - ($one_day * $i);
    $to_add = new stdClass();
    $to_add->timestamp = $timestamp;
    $to_add->value = '0';
    $to_return[date('Y.m.j', $timestamp)] = $to_add;
  }
  return $to_return;
}