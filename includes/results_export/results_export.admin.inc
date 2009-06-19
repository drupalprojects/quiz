<?php
// $Id$


function results_export_as_full_view($rid, $export_type) {
  module_load_include('inc', 'quiz', 'quiz.admin');
  $sql = "SELECT n.title FROM {node} n LEFT JOIN {quiz_node_results} as qnr ON n.nid = qnr.nid WHERE qnr.result_id = %d";
  $quiz_title = ($quiz_nid) ? check_plain(t('Quiz Result')) : db_result(db_query($sql, $rid));
  switch ($export_type) {
    case 'html':
      $output = qp(QueryPath::HTML_STUB)
        ->find('title')
        ->text($quiz_title)
        ->find(':root body')
        ->append(str_replace('&nbsp;', '', quiz_admin_results($rid)))
        ->find('table')
        ->attr('border', 1)
        ->top()
        ->html();
      break;

    case 'csv':
      $output = strip_tags(quiz_admin_results($rid));
      break;
  }
  $filename = str_replace(' ', '-', "$quiz_title Result ID $rid.html");
  results_export_call_file_transfer($filename, $output);
}

function results_export_call_file_transfer($filename, $output){
  $handle = @fopen('sites/default/files/' . $filename, 'w');
  fwrite($handle, $output);
  fclose($handle);
  $headers = array('Content-Type: text/html', 'Content-Disposition: attachment; filename='. $filename);
  $filepath = 'sites/default/files/' . $filename;
  ob_clean();
  file_transfer($filepath, $headers);
  ob_end_clean();
}

function results_export_as_teaser_view($nid, $export_type) {
  $results = _quiz_get_results($nid);
  $quiz = current($results);
  $header = array(t('node ID'), t('Quiz Title'), t('Username'), t('Result ID'), t('Score'), t('Quiz Start Time'), t('Quiz End Time'));
  while (list($key, $result) = each($results)) {
    $rows[] = array(
      $result['nid'],
      $result['title'],
      check_plain($result['name']),
      $result['result_id'],
      format_date($result['time_start'], 'small'),
      ($result['time_end'] > 0) ? format_date($result['time_end'], 'small') : t('In Progress'),
      ($result['time_end']) ? $result['score'] : t('In Progress'),
    );
  }
  switch ($export_type) {
    case 'html':
      $output = qp(QueryPath::HTML_STUB)
        ->find('title')
        ->text($quiz['title'])
        ->find(':root body')
        ->append(str_replace('&nbsp;', '', theme('table', $header, $rows)))
        ->find('table')
        ->attr('border', 1)
        ->top()
        ->html();
    break;

    case 'xml':
    break;

    case 'csv':
      $output = implode(",", $header);
      foreach ($rows as $row) {
        $output .= "\n" . implode(",", $row);
      }
      break;
  }
  $filename = str_replace(' ', '-', $quiz['title'] . '.csv');
  results_export_call_file_transfer($filename, $output);
}
