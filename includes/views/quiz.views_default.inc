<?php
// $Id$
// Declare all the .view files in the views subdir that end in .view
function quiz_views_default_views() {
  $files = file_scan_directory(drupal_get_path('module', 'quiz'). 'includes/views/defaults', '.view');

  $views = array();
  foreach ($files as $path => $file) {
    require $path;
    $views[$file->name] = $view;
  }
  return $views;
}