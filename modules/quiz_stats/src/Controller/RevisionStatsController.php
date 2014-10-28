<?php

namespace Drupal\quiz_stats\Controller;

use stdClass;

class RevisionStatsController {

  private $vid;
  private $uid;

  /**
   * @param int $vid
   *   quiz revision id
   * @param int $uid
   *   User id
   */
  public function __construct($vid, $uid = 0) {
    $this->vid = $vid;
    $this->uid = $uid;
  }

  public static function staticCallback($vid, $uid = 0) {
    $obj = new static($vid, $uid);
    return $obj->render($vid, $uid);
  }

  /**
   * Get stats for a single quiz. Maybe also for a single user.
   *
   * @return
   *   HTML page with charts/graphs
   */
  public function render() {
    return array(
      '#theme'    => 'quiz_stats_charts',
      '#chart'    => $this->buildChartStructure(),
      '#attached' => array(
        'css' => array(drupal_get_path('module', 'quiz_stats') . '/quiz_stats.css')
      )
    );
  }

  private function buildChartStructure() {
    return array(
      'takeup'      => $this->getDateVSTakeupCountChart(),
      // line chart/graph showing quiz takeup date along x-axis and count along y-axis
      'status'      => $this->getQuizStatusChart($this->vid, $this->uid),
      // 3D pie chart showing percentage of pass, fail, incomplete quiz status
      'top_scorers' => $this->getQuizTopScorersChart(),
      // Bar chart displaying top scorers
      'grade_range' => $this->getQuizGradeRangeChart(),
    );
  }

  /**
   * Generates chart showing how often the quiz has been taken the last days
   *
   * @return
   *   HTML to display chart
   */
  private function getDateVSTakeupCountChart() {
    $end = 30;
    $sql = "SELECT COUNT(result_id) AS count,
            DATE_FORMAT(FROM_UNIXTIME(time_start), '%Y.%m.%e') AS date
            FROM {quiz_results}
            WHERE vid = :vid";
    $sql_args[':vid'] = $this->vid;
    if ($this->uid) {
      $sql .= " AND uid = :uid";
      $sql_args[':uid'] = $this->uid;
    }
    $sql .= " GROUP BY date ORDER BY time_start DESC";
    $days = $this->getLastDays($end);
    $results = db_query($sql, $sql_args);
    $res_o = $results->fetch();
    if (!is_object($res_o)) {
      return FALSE;
    }
    foreach ($days as $date => &$value) {
      if (isset($res_o->date) && $date == $res_o->date) {
        $value->value = $res_o->count;
        if ($res_o->count) {
          $valid_data = TRUE;
        }
        $res_o = $results->fetch();
      }
    }

    if (!$valid_data) {
      return FALSE;
    }

    // wrapping the chart output with div for custom theming.
    $chart = theme('date_vs_takeup_count', array('takeup' => $days));

    // generate date vs takeup count line chart
    return array(
      'chart'       => '<div id="date_vs_takeup_count" class="quiz-stats-chart-space">' . $chart . '</div>',
      'title'       => t('Activity'),
      'explanation' => t('This chart shows how many times the quiz has been taken the last !days days.', array(
        '!days' => $end
      )),
    );
  }

  /**
   * Generates a chart showing the status for all registered responses to a quiz
   *
   * @return
   *   HTML to render to chart/graph
   */
  private function getQuizStatusChart() {
    // get the pass rate of the given quiz by vid
    $pass_rate = db_query("SELECT pass_rate "
      . " FROM {quiz_entity_revision} "
      . " WHERE vid = :vid", array(':vid' => $this->vid))->fetchField();
    if (!$pass_rate) {
      return;
    }

    // get the count value of results row above and below pass rate
    $quiz = db_query("SELECT SUM(score >= $pass_rate) AS no_pass, "
      . " SUM(score < $pass_rate) AS no_fail, "
      . " SUM(is_evaluated = 0) AS no_incomplete "
      . " FROM {quiz_results} "
      . " WHERE vid = :vid", array(':vid' => $this->vid))->fetchAssoc();

    if (($quiz['no_pass'] + $quiz['no_fail'] + $quiz['no_incomplete']) < 1) {
      return FALSE; // no sufficient data
    }

    // generates quiz status chart 3D pie chart
    $chart = '<div id="get_quiz_status_chart" class="quiz-stats-chart-space">';
    $chart .= theme('get_quiz_status_chart', array('quiz' => $quiz));
    $chart .= '</div>';
    return array(
      'chart'       => $chart,
      'title'       => t('Status'),
      'explanation' => t('This chart shows the status for all attempts made to answer this revision of the quiz.'),
    );
  }

  /**
   * Generates the top scorers chart
   *
   * @return
   *   array with chart and metadata
   */
  private function getQuizTopScorersChart() {
    $top_scorers = array();
    $sql = 'SELECT name, score
      FROM {quiz_results} qnr
      LEFT JOIN {users} u ON (u.uid = qnr.uid)
      WHERE vid = :vid';
    $arg[':vid'] = $this->vid;
    if ($this->uid) {
      $sql .= ' AND qnr.uid = :uid';
      $arg[':uid'] = $this->uid;
    }
    $sql .= ' ORDER by score DESC';
    $results = db_query($sql, $arg);
    while ($result = $results->fetchAssoc()) {
      $key = $result['name'] . '-' . $result['score'];
      $top_scorers[$key] = $result;
    }

    if (!count($top_scorers)) {
      return FALSE;
    }

    $chart = theme('quiz_top_scorers', array('scorer' => $top_scorers));
    return array(
      'chart'       => '<div id="quiz_top_scorers" class="quiz-stats-chart-space">' . $chart . '</div>',
      'title'       => t('Top scorers'),
      'explanation' => t('This chart shows what question takers have the highest scores'),
    );
  }

  /**
   * Generates grade range chart
   *
   * @return
   *   array with chart and metadata
   */
  private function getQuizGradeRangeChart() {
    // @todo: make the ranges configurable
    $sql = 'SELECT SUM(score >= 0 && score < 20) AS zero_to_twenty,
              SUM(score >= 20 && score < 40) AS twenty_to_fourty,
              SUM(score >= 40 && score < 60) AS fourty_to_sixty,
              SUM(score >= 60 && score < 80) AS sixty_to_eighty,
              SUM(score >= 80 && score <= 100) AS eighty_to_hundred
            FROM {quiz_results}
            WHERE vid = :vid';
    $arg[':vid'] = $this->vid;
    if ($this->uid) {
      $sql .= ' AND uid = :uid';
      $arg[':uid'] = $this->uid;
    }
    $range = db_query($sql, $arg)->fetch();
    $count = $range->zero_to_twenty + $range->twenty_to_fourty + $range->fourty_to_sixty + $range->sixty_to_eighty + $range->eighty_to_hundred;
    if ($count < 2) {
      return FALSE;
    }

    // Get the charts
    $chart = theme('quiz_grade_range', array('range' => $range));

    // Return the chart with some meta data
    return array(
      'chart'       => '<div id="quiz_top_scorers" class="quiz-stats-chart-space">' . $chart . '</div>',
      'title'       => t('Distribution'),
      'explanation' => t('This chart shows the distribution of the scores on this quiz.'),
    );
  }

  /**
   * Get the timestamps for the last days
   *
   * @param $num_days
   *  How many of the last days we need timestamps for
   * @return
   *  Array of objects with timestamp and value. The value has '0' as its default value.
   */
  private function getLastDays($num_days) {
    $to_return = array();
    $now = REQUEST_TIME;
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

}
