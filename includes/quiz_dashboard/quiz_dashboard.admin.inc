<?php

/*
 * @file
 * Administration file for Quiz Dashboard Module
 *
 */

function quiz_dashboard_get_basic_stats($uid = 0) {
  $results = _quiz_get_quizzes();
  return empty($results) ? t('No Statistics found.') : theme('quiz_dashboard_get_basic_stats', $results);
}

function quiz_dashboard_get_adv_stats($nid = 0, $uid = 0) {
  $output = '';
  $output = _get_date_vs_takeup_count_chart($nid, $uid);
  // line chart/graph showing quiz takeup date along x-axis and count along y-axis
  $output .= '<div style="margin-bottom:26px;"></div>';
  $output .= _get_quiz_status_chart($nid, $uid);
  // 3D pie chart showing percentage of pass, fail, incomplete quiz status
  $output .= '<div style="margin-bottom:26px;"></div>';
  $output .= _get_quiz_top_scorers_chart($nid, $uid);
  // Bar chart displaying top scorers
  $output .= '<div style="margin-bottom:26px;"></div>';
  $output .= _get_quiz_grade_range_chart($nid, $uid);
  return $output;
}


function _get_quiz_grade_range_chart($nid = 0, $uid = 0) {
  $sql = "SELECT SUM(score >= 0 && score <= 20) AS zero_to_twenty,
    SUM(score >= 20 && score <= 40) AS twenty_to_fourty,
    SUM(score >= 40 && score <= 60) AS fourty_to_sixty,
    SUM(score >= 60 && score <= 80) AS sixty_to_eighty,
    SUM(score >= 80 && score <= 100) AS eighty_to_hundred
    FROM {quiz_node_results}
    WHERE nid = %d";
  if ($uid != 0) {
    $sql .= " AND uid = %d";
  }
  $range = db_fetch_object(db_query($sql, $nid, $uid));
  return theme('quiz_grade_range', $range);
}



function _get_quiz_top_scorers_chart($nid = 0, $uid = 0) {
  $top_scorers = array();
  $sql = 'SELECT name, score FROM {quiz_node_results} qnr
    LEFT JOIN {users} u ON (u.uid = qnr.uid)
    WHERE nid = %d';
  if ($uid != 0) {
    $sql .= " AND qnr.uid = %d";
  }
  $sql .= ' ORDER by score DESC';
  $results = db_query_range($sql, $nid, $uid, 0, 10);
  while ($result = db_fetch_array($results)) {
    $key = $result['name'] . '-' . $result['score'];
    $top_scorers[$key] = $result;
  }
  return theme('quiz_top_scorers', $top_scorers);
}



function _get_quiz_status_chart($nid = 0, $uid = 0) {
  // get the pass rate of the given quiz by nid
  $pass_rate = db_result(db_query("SELECT pass_rate FROM {quiz_node_properties} WHERE nid = %d", intval($nid)));
  // get the count value of results row above and below pass rate
  $sql = "SELECT SUM(score > $pass_rate) AS no_pass, SUM(score < $pass_rate) AS no_fail, SUM(score = 0) AS no_incomplete FROM {quiz_node_results} WHERE nid = %d";
  $quiz = db_fetch_array(db_query($sql, intval($nid)));
  // generates quiz status chart 3D pie chart
  return theme('get_quiz_status_chart', $quiz);
}

function _get_date_vs_takeup_count_chart($nid = 0, $uid = 0) {
  $start = 0;
  $end = 10;
  $takeup = array();
  $sql = "SELECT COUNT(result_id) AS count,
            DATE(FROM_UNIXTIME(time_start)) AS date
            FROM {quiz_node_results}";
  if ($uid != 0) {
    $sql .= " WHERE uid = %d";
  }
  $sql .=" GROUP BY date ORDER BY date DESC";
  $results = db_query_range($sql, $uid, $start, $end);
  while ($result = db_fetch_array($results)) {
    $takeup[] = $result;
  }
  // generate date vs takeup count line chart
  return theme('date_vs_takeup_count', $takeup);
}


function _quiz_get_questions_count($nid, $uid = 0) {
  $sql = "SELECT COUNT(child_nid) FROM {quiz_node_relationship} WHERE parent_nid = %d";
  if ($uid != 0) {
    $sql .= " AND uid = %d";
  }
  return db_result(db_query($sql, $nid, $uid));
}

function _quiz_get_attempt_count($nid, $uid = 0) {
  $sql = "SELECT COUNT(result_id) FROM {quiz_node_results} WHERE nid = %d";
  if ($uid != 0) {
    $sql .= " AND uid = %d";
  }
  return db_result(db_query($sql, $nid, $uid));
}

// Theme functions

function theme_quiz_grade_range($range) {
  $chart_width = 600;
  $chart_height = 400;

  $chart = array();
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
  $chart['#data'][][] = $range->zero_to_twenty;
  $chart['#data'][][] = $range->twenty_to_fourty;
  $chart['#data'][][] = $range->fourty_to_sixty;
  $chart['#data'][][] = $range->sixty_to_eighty;
  $chart['#data'][][] = $range->eighty_to_hundred;
  // chart color
  $chart['#data_colors'][] = chart_unique_color($range->zero_to_twenty);
  $chart['#data_colors'][] = chart_unique_color($range->twenty_to_fourty);
  $chart['#data_colors'][] = chart_unique_color($range->fourty_to_sixty);
  $chart['#data_colors'][] = chart_unique_color($range->sixty_to_eighty);
  $chart['#data_colors'][] = chart_unique_color($range->eighty_to_hundred);
  // chart x-axis label
  $chart['#mixed_axis_labels'][CHART_AXIS_X_BOTTOM][2][] = chart_mixed_axis_label(t('Quiz Grade Range'), 50);
  // chart y-axis label and data
  $chart['#mixed_axis_labels'][CHART_AXIS_Y_LEFT][1][] = chart_mixed_axis_label(t('Score'), 90);
  $chart['#mixed_axis_labels'][CHART_AXIS_Y_LEFT][0][] = chart_mixed_axis_range_label(0, 10);
  // chart description on the right side
  $chart['#legends'][] = t('0 to 20 % - ') . $range->zero_to_twenty;
  $chart['#legends'][] = t('20 to 40 % - ') . $range->twenty_to_fourty;
  $chart['#legends'][] = t('40 to 60 % - ') . $range->fourty_to_sixty;
  $chart['#legends'][] = t('60 to 80 % - ') . $range->sixty_to_eighty;
  $chart['#legends'][] = t('80 to 100 % - ') . $range->eighty_to_hundred;
  return chart_render($chart);
}

function theme_quiz_top_scorers($attendees) {
  $chart = array();
  $chart = array(
    '#chart_id' => 'quiz_top_scorers',
    '#title' => t('Quiz Top Scorers'),
    '#type' => CHART_TYPE_BAR_V_GROUPED,
    '#size' => chart_size(600, 400),
    '#grid_lines' => chart_grid_lines(10, 10),
    '#bar_size' => chart_bar_size(20, 15),
    '#adjust_resolution' => TRUE,
  );
  foreach ($attendees as $attendee) {
    $chart['#data'][][] = $attendee['score'];
    $chart['#data_colors'][] = chart_unique_color($attendee['score']);
    $chart['#mixed_axis_labels'][CHART_AXIS_X_BOTTOM][2][] = chart_mixed_axis_label(t('Quiz Attendees'), 50);
    $chart['#legends'][] = $attendee['name'] . ' ' . $attendee['score'];
  }
  $chart['#mixed_axis_labels'][CHART_AXIS_Y_LEFT][1][] = chart_mixed_axis_label(t('Score'), 90);
  $chart['#mixed_axis_labels'][CHART_AXIS_Y_LEFT][0][] = chart_mixed_axis_range_label(0, 100);
  return chart_render($chart);
}

function theme_get_quiz_status_chart($quiz) {
  $quiz['total'] = $quiz['no_pass'] + $quiz['no_fail'] + $quiz['no_incomplete'];
  $chart = array();
  $chart = array(
    '#chart_id' => 'quiz_status_chart',
    '#title' => t('Quiz Status Chart'),
    '#type' => CHART_TYPE_PIE_3D,
    '#size' => chart_size(500, 200),
    '#adjust_resolution' => TRUE,
  );
  $chart['#data'] = array(
    'pass' => $quiz['no_pass'],
    'fail' => $quiz['no_fail'],
    'incomplete' => $quiz['no_incomplete']
  );
  $chart['#labels'] = array(
    'pass' => t('Pass ') . $quiz['no_pass'] . '/' . $quiz['total'],
    'fail' => t('Fail ') . $quiz['no_fail'] . '/' . $quiz['total'],
    'incomplete' => t('Incomplete ') . $quiz['no_incomplete'] . '/' . $quiz['total'],
  );
  ini_set('precision', '3');
  $chart['#legends'] = array(
    'pass' => ($quiz['no_pass'] / $quiz['total']) * 100.00 . t('% pass'),
    'fail' => ($quiz['no_fail'] / $quiz['total']) * 100 . t('% fail'),
    'incomplete' => ($quiz['no_incomplete'] / $quiz['total']) * 100 . t('% incomplete'),
  );
  return chart_render($chart);
}


function theme_date_vs_takeup_count($takeup) {
  $max_count = 0;
  $chart = array();
  $chart = array(
    '#chart_id' => 'test_chart',
    '#title' => t('Date vs Quiz Takeup'),
    '#type' => CHART_TYPE_LINE,
    '#size' => chart_size(600, 400),
    //'#chart_fill' => chart_fill('c', 'eeeeee'),
    '#grid_lines' => chart_grid_lines(10, 10, 1, 5),
    '#adjust_resolution' => TRUE,
  );
  while (!empty($takeup)) {
    $t = array_pop($takeup);
    $chart['#data'][] = $t['count'];
    $chart['#mixed_axis_labels'][CHART_AXIS_X_BOTTOM][1][] = chart_mixed_axis_label(format_date(strtotime($t['date']), 'custom', 'd M'));
    $max_count = ($max_count > $t['count']) ? $max_count : $t['count'];
    $test[] = array($t['count'], chart_mixed_axis_label(format_date(strtotime($t['date']), 'custom', 'd M')));
  }
  $chart['#mixed_axis_labels'][CHART_AXIS_Y_LEFT][0][] = chart_mixed_axis_range_label(0, $max_count);
  $chart['#mixed_axis_labels'][CHART_AXIS_Y_LEFT][1][] = chart_mixed_axis_label(t('Count'), 90);
  $chart['#mixed_axis_labels'][CHART_AXIS_X_BOTTOM][2][] = chart_mixed_axis_label(t('Days of the week'), 50);
  return chart_render($chart);
}

function theme_quiz_dashboard_get_basic_stats($results = array()) {
  $header = $rows = array();
  //$title_link = user_access('access author dashboard') ? 'admin/quiz/reports/dashboard/creator/'. $result['nid'] .'/view' : 'user/%/dashboard/%/view'; .
  $title_link = user_access('access author dashboard') ? 'admin/quiz/reports/dashboard/creator/' : 'user/'. arg(1) .'/dashboard/';
  if (empty($results)) {
    return '<p>' . t('No questions were found.') . '</p>';
  }
  $header = array(
    'nid' => t('Quiz Nid <br/> <h6> Link to Quiz </h6>'),
    'title' => t('Title <br/> <h6> Link to Report </h6>'),
    'creator' => t('Author Name <h6> Link to Profile </h6>'),
    'created' => t('Create Date'),
    'questions_count' => user_access('access author dashboard') ? t('Questions Count') : t(''),
    'attempt_count' => user_access('access author dashboard') ? t('Attempt Count') : t(''),
  );
  foreach ($results as $result) {
    $rows[] = array(
      'nid' => l($result['nid'], 'node/'. $result['nid']),
      'title' => l(check_plain($result['title']), $title_link . $result['nid'] .'/view'),
      'creator' => l(check_plain($result['name']), 'user/'. $result['uid']),
      'created' => format_date($result['created'], 'small'),
      'questions_count' => user_access('access author dashboard') ? _quiz_get_questions_count($result['nid']) : t(''),
      'attempt_count' => user_access('access author dashboard') ? _quiz_get_attempt_count($result['nid']) : t(''),
    );
  }
  module_load_include('inc', 'quiz', 'quiz.pages');
  // FIX IT need to get community feedback.
  //quiz_add_table_sorter();
  // quiz_add_table_sorter(); function defined in quiz.page.inc includes css and js for jQuery table sorter.
  return theme('table', $header, $rows, array('id' => 'tablesorter'));
}
