<?php
// $Id$


function results_export_process($rid, $type) {
  module_load_include('inc', 'quiz', 'quiz.admin');
  $sql = "SELECT n.title FROM {node} n LEFT JOIN {quiz_node_results} as qnr ON n.nid = qnr.nid WHERE qnr.result_id = %d";
  $quiz_title = ($quiz_nid) ? check_plain(t('Quiz Result')) : db_result(db_query($sql, $rid));
  switch ($type) {
    case 'html':
      $result = qp(QueryPath::HTML_STUB)
        ->find('title')
        ->text($quiz_title)
        ->top()
        ->find('body')
        ->append(str_replace('&nbsp;', '', quiz_admin_results($rid)))
        ->find('table')
        ->attr('border', 1)
        ->attr('cellpadding', 4)
        ->attr('cellspacing', 4)
        ->top()
        ->html();
      break;
    case 'csv':
      $result = strip_tags(quiz_admin_results($rid));
      break;
  }
  $filename = str_replace(' ', '-', "$quiz_title Result ID $rid.html");
  $handle = @fopen('sites/default/files/' . $filename, 'w');
  fwrite($handle, $result);
  fclose($handle);
  $headers = array('Content-Type: text/html', 'Content-Disposition: attachment; filename='. $filename);
  $filepath = 'sites/default/files/' . $filename;
  ob_clean();
  file_transfer($filepath, $headers);
  ob_end_clean();
  return '';
}
