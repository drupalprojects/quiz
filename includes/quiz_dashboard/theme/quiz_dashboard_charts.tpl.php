<?php 
// $ID$

/**
 * @file
 * Theming the charts page
 * 
 * Variables available:
 * $charts (array)
 * 
 * The following charts are available:
 * $charts['top_scorers'] (array or FALSE if chart doesn't exist)
 * $charts['takeup'] (array or FALSE if chart doesn't exist)
 * $charts['status'] (array or FALSE if chart doesn't exist)
 * $charts['grade_range'] (array or FALSE if chart doesn't exist)
 * 
 * Each chart has a title, an image and an explanation like this:
 * $charts['top_scorers']['title'] (string)
 * $charts['top_scorers']['chart'] (string - img tag - google chart)
 * $charts['top_scorers']['explanation'] (string)
 */

if (!function_exists('_quiz_dashboard_print_chart')) {
  function _quiz_dashboard_print_chart(&$chart) {
    if (is_array($chart))
      print '<h2 class="quiz-charts-title">'. $chart['title'] .'</h2>'. $chart['chart'] . $chart['explanation'];
  }
}
_quiz_dashboard_print_chart($charts['takeup']);
_quiz_dashboard_print_chart($charts['top_scorers']);
_quiz_dashboard_print_chart($charts['status']);
_quiz_dashboard_print_chart($charts['grade_range']);
?>