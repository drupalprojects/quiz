<?php

/*
 * @file
 * Administration file for Quiz Dashboard Module
 *
 */

function quiz_dashboard_get_basic_stats($uid = 0) {
  $results = _quiz_get_quizzes();
  return theme('quiz_dashboard_get_basic_stats', $results);
}

function quiz_dashboard_get_adv_stats() {
  $output = '';
  $output = _get_date_vs_takeup_count_chart($uid);
  // line chart/graph showing quiz takeup date along x-axis and count along y-axis
  $output .= '<div style="margin-bottom:26px;"></div>';
  $output .= _get_quiz_status_chart($uid);
  // 3D pie chart showing percentage of pass, fail, incomplete quiz status
  $output .= '<div style="margin-bottom:26px;"></div>';
  $output .= _get_quiz_top_scorers();
  return $output;
}

function _get_quiz_top_scorers() {
  $top_scorers = array();
  $nid = arg(5);
  $sql = 'SELECT name, score FROM {quiz_node_results} qnr
    LEFT JOIN {users} u ON (u.uid = qnr.uid)
    WHERE nid = %d ORDER by score DESC';
  $results = db_query_range($sql, $nid, 0, 10);
  while ($result = db_fetch_array($results)) {
    $key = $result['name'] . '-' . $result['score'];
    $top_scorers[$key] = $result;
  }
  return theme('quiz_top_scorers', $top_scorers);
}

function theme_quiz_top_scorers($attendees) {
  $chart = array(
    '#chart_id' => 'quiz_top_scorers',
    '#title' => t('Quiz Top Scorers'),
    '#type' => CHART_TYPE_BAR_V_GROUPED,
    '#size' => chart_size(500, 500),
    '#grid_lines' => chart_grid_lines(10, 10),
    //'#bar_size' => chart_bar_size(15, 5),
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

function _get_quiz_status_chart($uid) {
  $nid = arg(5);
  $pass_rate = db_result(db_query("SELECT pass_rate FROM {quiz_node_properties} WHERE nid = %d", intval($nid)));
  $sql = "SELECT SUM(score > $pass_rate) AS no_pass, SUM(score < $pass_rate) AS no_fail, SUM(score = 0) AS no_incomplete FROM {quiz_node_results} WHERE nid = %d";
  $quiz = db_fetch_array(db_query($sql, intval($nid)));
  return theme('get_quiz_status_chart', $quiz);
}

function _get_date_vs_takeup_count_chart($uid = 0) {
  $takeup = array();
  $sql = "SELECT COUNT(result_id) AS count,
            DATE(FROM_UNIXTIME(time_start)) AS date
            FROM {quiz_node_results} GROUP BY date";
  $results = db_query($sql);
  while ($result = db_fetch_array($results)) {
    $takeup[] = $result;
  }
  return theme('date_vs_takeup_count', $takeup);
}

function _quiz_get_questions_count($nid, $uid = 0) {
  $sql = "SELECT COUNT(child_nid) FROM {quiz_node_relationship} WHERE parent_nid = %d";
  $sql = ($uid != 0) ? "$sql AND uid = %d" : $sql;
  return db_result(db_query($sql, $nid, $uid));
}

function _quiz_get_attempt_count($nid, $uid = 0) {
  $sql = "SELECT COUNT(result_id) FROM {quiz_node_results} WHERE nid = %d";
  $sql = ($uid != 0) ? "$sql AND uid = %d" : $sql;
  return db_result(db_query($sql, $nid, $uid));
}

// Theme functions

function theme_get_quiz_status_chart($quiz) {
  $quiz['total'] = $quiz['no_pass'] + $quiz['no_fail'] + $quiz['no_incomplete'];
  $chart = array();
  $chart = array(
    '#chart_id' => 'quiz_status_chart',
    '#title' => t('Quiz Status Chart'),
    '#type' => CHART_TYPE_PIE_3D,
    '#size' => chart_size(600, 300),
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
    'pass' => ($quiz['no_pass'] / $quiz['total']) * 100.00 . '%',
    'fail' => ($quiz['no_fail'] / $quiz['total']) * 100 . '%',
    'incomplete' => ($quiz['no_incomplete'] / $quiz['total']) * 100 . '%',
  );
  return chart_render($chart);
}


function theme_date_vs_takeup_count($takeup) {
  $chart = array();
  $chart = array(
    '#chart_id' => 'test_chart',
    '#title' => chart_title(t('Date vs Quiz Takeup'), 'cc0000', 15),
    '#type' => CHART_TYPE_LINE,
    '#size' => chart_size(600, 400),
    '#chart_fill' => chart_fill('c', 'eeeeee'),
    '#grid_lines' => chart_grid_lines(20, 20, 1, 5),
    '#adjust_resolution' => TRUE,
  );

  foreach ($takeup as $t) {
    $chart['#data'][] = $t['count'];
    $chart['#labels'][] = format_date(strtotime($t['date']), 'custom', 'd M');
  }

  $chart['#mixed_axis_labels'][CHART_AXIS_Y_LEFT][0][] = chart_mixed_axis_range_label(0, 10);
  $chart['#mixed_axis_labels'][CHART_AXIS_Y_LEFT][1][] = chart_mixed_axis_label(t('Count'), 95);

  $chart['#mixed_axis_labels'][CHART_AXIS_X_BOTTOM][1][] = chart_mixed_axis_label(t('Mon'));
  $chart['#mixed_axis_labels'][CHART_AXIS_X_BOTTOM][1][] = chart_mixed_axis_label(t('Tue'));
  $chart['#mixed_axis_labels'][CHART_AXIS_X_BOTTOM][1][] = chart_mixed_axis_label(t('Wed'));
  $chart['#mixed_axis_labels'][CHART_AXIS_X_BOTTOM][1][] = chart_mixed_axis_label(t('Thu'));
  $chart['#mixed_axis_labels'][CHART_AXIS_X_BOTTOM][1][] = chart_mixed_axis_label(t('Fri'));
  $chart['#mixed_axis_labels'][CHART_AXIS_X_BOTTOM][2][] = chart_mixed_axis_label(t('Days of the week'), 50);
  return chart_render($chart);
}

function theme_quiz_dashboard_get_basic_stats($results = array()) {
  $rows = array();
  if (empty($results)) {
    return '<p>' . t('No questions were found.') . '</p>';
  }
  $header = array(
    'nid' => t('Quiz Nid <br/> <h6> Link to Quiz </h6>'),
    'title' => t('Title <br/> <h6> Link to Report </h6>'),
    'creator' => t('Creator Name <h6> Link to Profile </h6>'),
    'created' => t('Create Date'),
    'questions_count' => t('Questions Count'),
    'attempt_count' => t('Attempt Count'),
  );
  foreach ($results as $result) {
    $rows[] = array(
      'nid' => l($result['nid'], 'node/'. $result['nid']),
      'title' => l(check_plain($result['title']), 'admin/quiz/reports/dashboard/creator/'. $result['nid'] .'/view'),
      'creator' => l(check_plain($result['name']), 'user/'. $result['uid']),
      'created' => format_date($result['created'], 'small'),
      'questions_count' => _quiz_get_questions_count($result['nid']),
      'attempt_count' => _quiz_get_attempt_count($result['nid']),
    );
  }
  module_load_include('inc', 'quiz', 'quiz.pages');
  quiz_add_table_sorter();
  // defined in quiz.page.inc includes css and js for jQuery table sorter
  return theme('table', $header, $rows, array('id' => 'tablesorter'));
}
