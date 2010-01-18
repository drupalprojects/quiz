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
  }
  $filename = str_replace(' ', '-', "$quiz_title Result ID $rid.$export_type");
  results_export_invoke_file_transfer($filename, $output);
}

function results_export_invoke_file_transfer($filename, $output) {
  ob_start();
  $filepath = file_directory_temp() . '/' . $filename;
  $handle = @fopen($filepath, 'w');
  fwrite($handle, $output);
  fclose($handle);
  $headers = array('Content-Type: text/html', 'Content-Disposition: attachment; filename='. $filename);
  ob_clean();
  file_transfer($filepath, $headers);
  ob_end_clean();
}