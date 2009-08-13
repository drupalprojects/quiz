<?php

// $Id$

/**
 * @file
 * ajax quiz admin page
 */

/**
 * @function
 *   callback for ajax quiz menu
 *
 * @param $node
 *   quiz node object
 *
 * @return
 *   HTML quiz description
 */
function ajax_quiz_instruction($node) {
  // If this is an AJAX request, return the AJAX result.
  if (isset($_REQUEST['ajax_load_example']) && $_REQUEST['ajax_load_example']) {
    _ajax_quiz_get_questions($node);
  }
  else {
    drupal_add_js(drupal_get_path('module', 'ajax_quiz') . '/ajax_quiz.js');
    $start = l(t('Start'), $_GET['q'], array('attributes' => array('class' => 'ajax-load-example')));
    return theme('ajax_quiz_instruction', $node->body, $start);
  }
}


/**
 * function that responds to AJAX call
 *
 * @param @node
 *   quiz node object
 *
 */
function _ajax_quiz_get_questions($node) {
  $data = theme('status_messages');
  $data .= node_view($node, FALSE, TRUE);
  $result = array(
    'content' => $data,
    // Put the Javascript callback you will use here.
    // You can if you wish leave out this line and instead
    // call your callback directly in your Javascript. See
    // comments in ajax_load_example.js.
    '__callbacks' => array('Drupal.AjaxLoadExample.formCallback'),
  );
  // Call drupal_alter('ajax_data', ...). This is what allows ajax_load to
  // add its data and register its Javascript callback.
  // The second argument is the data to be altered.
  // The third argument is a unique identifier for this AJAX data operation.
  // The fourth and optional argument is the original data that was the subject
  // of the ajax operation--in this case, a form ID.
  drupal_alter('ajax_data', $result, 'ajax_load_example', 'node_view');
  drupal_json($result);
}

// Theme functions

/**
 * theme function displays quiz instruction and start quiz link.
 *
 * @param $instruction
 *   quiz node body
 *
 * @param start
 *   link to start the quiz
 *
 * @return
 *   HTML containing quiz instruction and start quiz link.
 */
function theme_ajax_quiz_instruction($instruction, $start) {
  $output = '<div class="ajax-quiz-wrapper">';
  $output .= '<div class="ajax-quiz-description">' . $instruction .'</div>';
  $output .= '<div class="ajax-quiz-start">' . $start . '</div>';
  $output .= '</div>';
  return $output;
}
