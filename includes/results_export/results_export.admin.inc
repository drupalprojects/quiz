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

function results_export_as_teaser_view($nid, $export_type) {
  $results = _quiz_get_results($nid);
  $quiz = current($results);
  $header = array(t('node ID'), t('Quiz Title'), t('Username'), t('Result ID'), t('Start Time'), t('End Time'), t('Time Taken'), t('Score'));
  while (list($key, $result) = each($results)) {
    $rows[] = array(
	       'nid' => $result['nid'], 	 
	       'title' => $result['title'], 	 
	       'name' => check_plain($result['name']), 	 
	       'result_id' => $result['result_id'], 	 
	       'time_start' => format_date($result['time_start'], 'small'), 	 
	       'time_end' => ($result['time_end'] > 0) ? format_date($result['time_end'], 'small') : t('In Progress'), 	 
	       'time_taken' => ($result['time_end'] > 0) ? ($result['time_end'] - $result['time_start']) : (time() - $result['time_start']), 	 
	       'score' => ($result['time_end']) ? $result['score'] : t('--'), 	 
    );
  }
  if (!isset($rows)) {
    drupal_set_message(t('There are no results for this quiz'));
    drupal_goto('admin/quiz/reports/results');
    return;
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
      $node = node_load($nid);
      $output = qp('<?xml version="1.0" ?><quiz><quizinfo/><resultSet/></quiz>', 'quizinfo')
      ->append('<authorName>' . $node->name . '</authorName>')
      ->append('<authorId>' . $node->uid . '</authorId>')
      ->append('<title>' . $node->title . '</title>')
      ->append('<description>' . $node->body . '</description>')
      ->append('<nodeId>' . $node->nid . '</nodeId>')
      ->append('<createDate>' . $node->created .'</createDate>')
      ->append('<lastUpdateDate>' . $node->changed .'</lastUpdateDate>')
      ->top()
      ->find('resultSet');
      foreach ($rows as $row) {
        $output->branch()
        ->append('<result/>')
        ->find('result:last')
        ->append('<name>' . $row['name'] .'</name>')
        ->append('<resultId>' . $row['result_id'] . '</resultId>')
        ->append('<startTime>' .  $row['time_start'] . '</startTime>')
        ->append('<endTime>'. $row['time_end'] . '</endTime>')
        ->append('<timeTaken>' . $row['time_taken'] . '</timeTaken>')
        ->append('<score>' . $row['score'] . '</score>');
      }
      $output = $output->top()->xml();
      break;
       
    case 'csv':
      $output = implode(',', $header);
      foreach ($rows as $row) {
        $output .= "\n" . implode(',', $row);
      }
      break;
  }
  $filename = str_replace(' ', '-', $quiz['title'].'.'.$export_type);
  results_export_invoke_file_transfer($filename, $output);
}