<?php

/**
 * Administrator interface for Quiz module.
 *
 * @file
 */

/**
 * Adds checkbox for creating new revision. Checks it by default if answers exists.
 *
 * @param $form
 *   FAPI form(array)
 * @param $quiz
 *   Quiz node(object)
 */
function _quiz_add_revision_checkbox(&$form, &$quiz) {
  // Recomend and preselect to create the quiz as a new revision if it already has been answered
  if (quiz_has_been_answered($quiz)) {
    $rev_default = TRUE;
    $rev_description = t('This quiz has been answered. To maintain correctnes of existing answer reports changes should be saved as a new revision.');
  }
  else {
    $rev_default = in_array('revision', variable_get('node_options_quiz', array()));
    $rev_description = t('Allow question status changes to create a new revision of the quiz?');
  }

  if (user_access('manual quiz revisioning') && !variable_get('quiz_auto_revisioning', 1)) {
    $form['new_revision'] = array(
      '#type'          => 'checkbox',
      '#default_value' => $rev_default,
      '#title'         => t('New revision'),
      '#description'   => $rev_description,
    );
  }
  else {
    $form['new_revision'] = array('#type' => 'value', '#value' => $rev_default);
  }
}

// THEME FUNCTIONS

/**
 * Theme the quiz node form
 *
 * Adds js to enhance the ux.
 *
 * @param $form
 *   FAPI form array
 */
function theme_quiz_node_form($variables) {
  $form = $variables['form'];
  $path = drupal_get_path('module', 'quiz') . '/theme/quiz_node_form.js';
  drupal_add_js($path);
  return theme('node_form', array('form' => $form));
}

/**
 * Theme a question selection table, adding drag and drop support.
 */
function theme_question_selection_table($variables) {
  $form = $variables['form'];
  drupal_add_tabledrag('question-list', 'match', 'parent', 'qnr-pid', 'qnr-pid', 'qnr-id', TRUE, 1);
  drupal_add_tabledrag('question-list', 'order', 'sibling', 'question-list-weight');

  // Building headers
  $headers = array(t('Question'), t('Type'), t('Actions'), t('Update'), t('Max score'), t('Auto update max score'), t('Delete'));
  if (isset($form['compulsories'])) {
    $headers[] = t('Compulsory');
  }
  $headers[] = t('Weight');
  $headers[] = t('Parent ID');
  $headers[] = array(
    'data'  => t('ID'),
    'class' => array('tabledrag-hide'),
  );

  // Building table body
  $rows = array();
  if (!empty($form['titles'])) {
    foreach (element_children($form['titles']) as $id) {
      $form['weights'][$id]['#attributes']['class'] = array('question-list-weight');
      $form['qnr_ids'][$id]['#attributes']['class'] = array('qnr-id');
      $form['qnr_pids'][$id]['#attributes']['class'] = array('qnr-pid');
      $rows[] = _quiz_get_question_row($form, $id);
    }
    // Make sure the same fields aren't rendered twice
    unset($form['types'], $form['view_links'], $form['remove_links'], $form['stayers']);
    unset($form['max_scores'], $form['auto_update_max_scores'], $form['revision'], $form['weights'], $form['titles'], $form['compulsories'], $form['qnr_ids'], $form['qnr_pids']);
  }
  $html_attr = array('id' => 'question-list');

  // We hide the table if no questions have been added so that jQuery can show it the moment the first question is beeing added.
  if (isset($form['no_questions'])) {
    $html_attr['style'] = "display:none;";
  }

  $table = theme('table', array('header' => $headers, 'rows' => $rows, 'attributes' => $html_attr));

  return drupal_render($form['random_settings'])
    . $table
    . drupal_render_children($form);
}

/**
 * Delete a single result, or all results for a given user and a given quiz.
 *
 * @param $result_id
 *   result if for the result to be deleted
 * @param $nid
 *   Node id for the quiz the result belongs to. If set all the users results for this quiz will be deleted.
 */
function _quiz_delete_results($result_id, $nid = NULL) {
  $result_ids = array();

  // We are to delete all results for a certain user on a certain quiz.
  if (isset($nid)) {
    // TODO: We should be able to rewrite this one to use the DBTNG.
    $res = db_query('SELECT result_id
            FROM {quiz_node_results}
            WHERE nid = :nid AND uid = (
              SELECT uid
              FROM {quiz_node_results}
              WHERE result_id = :result_id
            )', array(':nid' => $nid, ':result_id' => $result_id));

    while ($result = $res->fetchField()) {
      $result_ids[] = $result;
    }
  }
  // We are only to delete one specific result
  else {
    $result_ids[] = $result_id;
  }
  quiz()->getQuizHelper()->getResultHelper()->deleteByIds($result_ids);
}

// HELPER FUNCTIONS

/**
 * Helper function for theme_question_selection_table
 *
 * TODO: DELETE
 *
 * @see quiz_questions_form()
 * @see theme_question_selection_table()
 *
 * @param $sub_form
 *   Form definition array for a filtered questions list
 * @param $id
 *   Identifier used in $sub_form
 * @return table row
 *   Array defining a table row
 */
function _quiz_get_question_row($sub_form, $id) {
  $question_types = _quiz_get_question_types();
  $type = $sub_form['types'][$id]['#markup'];

  $action = theme('item_list', array(
    'items'      => array(
      drupal_render($sub_form['view_links'][$id]),
      '<SPAN CLASS="q-remove" STYLE="display:none">' . drupal_render($sub_form['remove_links'][$id]) . '</SPAN>',
    ),
    'attributes' => array(
      'class' => array('links', 'inline'),
    ),
  ));
  $qnr_pid = $sub_form['qnr_pids'][$id]['#default_value'];
  $data_array = array(
    // The checkbox and the title
    ($qnr_pid ? theme('indentation', array('size' => 1)) : NULL) . drupal_render($sub_form['titles'][$id]),
    $type,
    $action,
    isset($sub_form['revision'][$id]) ? drupal_render($sub_form['revision'][$id]) : t("Up to date"),
    drupal_render($sub_form['max_scores'][$id]),
    drupal_render($sub_form['auto_update_max_scores'][$id]),
    drupal_render($sub_form['stayers'][$id]),
  );
  if (isset($sub_form['compulsories'])) {
    $data_array[] = drupal_render($sub_form['compulsories'][$id]);
  }
  $data_array[] = drupal_render($sub_form['weights'][$id]);
  $data_array[] = drupal_render($sub_form['qnr_pids'][$id]);
  $data_array[] = array(
    'class' => array('tabledrag-hide'),
    'data'  => drupal_render($sub_form['qnr_ids'][$id]),
  );

  $leaf_class = $sub_form['types'][$id]['#question_type'] != 'quiz_page' ? 'tabledrag-leaf' : '';

  return array(
    'class' => array('q-row', 'draggable', $leaf_class),
    'id'    => 'q-' . $id,
    'data'  => $data_array
  );
}

/**
 * Adds inline js to automatically set the question's node title.
 */
function quiz_set_auto_title() {
  $max_length = variable_get('quiz_autotitle_length', 50);
  drupal_add_js('
  (function ($) {
  $(document).ready(function () {
    function quizStripTags(str) {
      return str.replace(/<\/?[^>]+>/gi, \'\');
    }

    function quizUpdateTitle() {
      var body = $("#edit-body textarea:eq(1)").val();
      if (quizStripTags(body).length > ' . $max_length . ') {
        $("#edit-title").val(quizStripTags(body).substring(0, ' . $max_length . ' - 3) + "...");
      }
      else {
        $("#edit-title").val(quizStripTags(body).substring(0, ' . $max_length . '));
      }
    }
    $("#edit-body textarea").keyup(quizUpdateTitle);
    // Do not use auto title if a title already has been set
    if($("#edit-title").val().length > 0){
      $("#edit-body textarea:eq(1)").unbind("keyup", quizUpdateTitle);
    }
    $("#edit-title").keyup(function() {
      $("#edit-body textarea:eq(1)").unbind("keyup", quizUpdateTitle);
    });
  });
  })(jQuery);
  ', array('type' => 'inline', 'scope' => JS_DEFAULT));
}

/**
 * Themes a categorized quiz form
 */
function theme_quiz_categorized_form($variables) {
  $form = $variables['form'];
  $output = '';
  $rows = array();
  foreach ($form as $key => &$existing) {
    if (!is_numeric($key)) {
      continue;
    }
    $cols = array();
    $cols[] = drupal_render($existing['name']);
    $cols[] = drupal_render($existing['number']);
    $cols[] = drupal_render($existing['max_score']);
    $cols[] = drupal_render($existing['remove']);
    $cols[] = drupal_render($existing['weight']);
    $rows[] = array(
      'data'  => $cols,
      'class' => array('draggable'),
    );
  }
  if (!empty($rows)) {
    $header = array(
      t('Category'),
      t('Number of questions'),
      t('Max score per question'),
      t('Remove'),
      t('Weight'),
    );
    drupal_add_tabledrag('existing-terms', 'order', 'sibling', 'term-weight', NULL, NULL, TRUE);
    $output .= theme('table', array('header' => $header, 'rows' => $rows, 'attributes' => array('id' => 'existing-terms')));
  }
  $output .= drupal_render_children($form);
  drupal_add_js(
    "(function ($) {
     Drupal.behaviors.quiz_categorized = {
     attach: function(context) {
      $('#browse-for-term:not(.quiz-processed)').click(function(event) {
        event.preventDefault();
        $('#edit-term').focus().val('*').trigger('keyup');
      }).addClass('quiz-processed');
      $('#edit-term').click(function(){
        if ($(this).val() == '*') {
          $(this).val('');
        }
      });
    }};}(jQuery));"
    , array('type' => 'inline', 'group' => JS_DEFAULT));
  return $output;
}

/**
 * Prints a taxonomy selection form for each vocabulary.
 *
 * @param $value
 *   Default selected value(s).
 * @return
 *   HTML output to print to screen.
 */
function _quiz_taxonomy_select($value = 0) {
  $options = array();
  foreach (quiz()->getVocabularies() as $vid => $vocabulary) {
    $temp = taxonomy_form($vid, $value);
    $options = array_merge($options, $temp['#options']);
  }
  return $options;
}
