<?php

/*
 * @file
 * Handles Default views.
 * // Declare all the .view files in the views subdir that end in .view
 */

function quiz_views_default_views() {
<<<<<<< quiz.views_default.inc
<<<<<<< quiz.views_default.inc
<<<<<<< quiz.views_default.inc
  $files = file_scan_directory(QUIZ_VIEWS_DIR . '/defaults', '/.inc/');
=======
  $files = file_scan_directory(QUIZ_VIEWS_DIR . '/defaults', '.view.inc');
>>>>>>> 1.4
=======
  $files = file_scan_directory(QUIZ_VIEWS_DIR . '/defaults', '.inc');
>>>>>>> 1.2.2.2
=======
  $files = file_scan_directory(QUIZ_VIEWS_DIR . '/defaults', '.view.inc');
>>>>>>> 1.4

  $views = array();
  foreach ($files as $path => $file) {
    require DRUPAL_ROOT . '/' . $path;
    $views[$file->name] = $view;
  }
  return $views;
}
