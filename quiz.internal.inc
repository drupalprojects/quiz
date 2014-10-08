<?php

/**
 * Get data for all terms belonging to a Quiz with categorized random questions
 *
 * @param int $vid
 *  version id for the quiz
 * @return array
 *  Array with all terms that belongs to the quiz as objects
 */
function _quiz_get_terms($vid) {
  $sql = 'SELECT td.name, qt.*
    FROM {quiz_terms} qt
    JOIN {taxonomy_term_data} td ON qt.tid = td.tid
    WHERE qt.vid = :vid
    ORDER BY qt.weight';
  return db_query($sql, array(':vid' => $vid))->fetchAll();
}

/**
 * If a quiz is saved with random categories we should make sure all questions
 * are removed from the quiz
 *
 * @param $node
 *   Quiz node.
 */
function _quiz_check_num_always(&$node) {
  if ($node->randomization != 3) {
    return;
  }
  db_delete('quiz_node_relationship')
    ->condition('parent_vid', $node->vid)
    ->execute();
}

/**
 * If a quiz is saved as not randomized we should make sure all random questions
 * are converted to always.
 *
 * @param $node
 *   Quiz node.
 */
function _quiz_check_num_random(&$node) {
  if ($node->randomization == 2) {
    return;
  }
  db_delete('quiz_node_relationship')
    ->condition('question_status', QUESTION_RANDOM)
    ->condition('parent_vid', $node->vid)
    ->execute();
}

/**
 * Get the number of compulsory questions for a quiz.
 *
 * @param $nid
 *   Quiz node id.
 * @param $vid
 *   Quiz node version id.
 * @return
 *   Number of compulsory questions.
 */
function _quiz_get_num_always_questions($vid) {
  return db_query('SELECT COUNT(*) FROM {quiz_node_relationship} qnr
          JOIN {node} n ON n.nid = qnr.child_nid
          WHERE n.status=1 AND qnr.parent_vid = :parent_vid AND qnr.question_status = :question_status', array(':parent_vid' => $vid, ':question_status' => QUESTION_ALWAYS))->fetchField();
}

/**
 * Modifies the format fieldset.
 *
 * Adds a class to all the format fieldsets and removes unwanted strings.
 * A javascript is added by the forms theme function to make sure all format
 * selectors follows the body field format selector.
 * Used when there are multiple format selectors on one page.
 *
 * Could be a deprecated function in d7
 *
 * @param $format
 *   The format fieldset.
 */
function _quiz_format_mod(&$format) {
  $format['#attributes']['class'] = array('quiz-filter');
  if (isset($format['format'])) {
    $format['format']['guidelines']['#value'] = ' ';
    foreach ($format as $key => $value) {
      if (is_numeric($key)) {
        $format[$key]['#value'] = ' ';
      }
    }
  }
}

/**
 * Limit the year options to the years 1970 - 2030 for form items of type date.
 *
 * Some systems don't support all the dates the forms api lets you choose from.
 * This function limits the options to dates most systems support.
 *
 * @param $form_element
 *   Form element of type date.
 *
 * @return
 *   Form element with a more limited set of years to choose from.
 */
function _quiz_limit_year_options($form_element) {
  $form_element['year']['#options'] = drupal_map_assoc(range(1970, 2030));
  return $form_element;
}

/**
 * Format a number of seconds to a hh:mm:ss format.
 *
 * @param $time_in_sec
 *   Integers time in seconds.
 *
 * @return
 *   String time in min : sec format.
 */
function _quiz_format_duration($time_in_sec) {
  $hours = intval($time_in_sec / 3600);
  $min = intval(($time_in_sec - $hours * 3600) / 60);
  $sec = $time_in_sec % 60;
  if (strlen($min) == 1) {
    $min = '0' . $min;
  }
  if (strlen($sec) == 1) {
    $sec = '0' . $sec;
  }
  return "$hours:$min:$sec";
}

/**
 * Helper function used when validating integers.
 *
 * @param $value
 *   The value to be validated.
 * @param $min
 *   The minimum value $value is allowed to be.
 * @param $max
 *   The maximum value $value is allowed to be.
 *
 * @return
 *   TRUE if integer in the allowed range. FALSE otherwise.
 */
function _quiz_is_int($value, $min = 1, $max = NULL) {
  $to_return = ((string) $value === (string) (int) $value);
  // $value is not an integer.
  if (!$to_return) {
    return FALSE;
  }
  // $value is too small.
  if ($value < $min) {
    return FALSE;
  }
  // $value is too big.
  if (isset($max)) {
    if ($value > $max) {
      return FALSE;
    }
  }
  // $value is an integer in the allowed range.
  return TRUE;
}

/**
 * Helper function used when validating plain text.
 *
 * @param $value
 *   The value to be validated.
 *
 * @return
 *   TRUE if plain text FALSE otherwise.
 */
function _quiz_is_plain($value) {
  return ($value === check_plain($value));
}

/**
 * Helper function used when figuring out if a textfield or textarea is empty.
 *
 * Solves a problem with some wysiwyg editors inserting spaces and tags without content.
 *
 * @param $html
 *   The html to evaluate
 *
 * @return
 *   TRUE if the field is empty(can still be tags there) false otherwise.
 */
function _quiz_is_empty_html($html) {
  return drupal_strlen(trim(str_replace('&nbsp;', '', strip_tags($html, '<img><object><embed>')))) == 0;
}

function _quiz_userpoints_type() {
  $userpoints_terms = taxonomy_get_tree(userpoints_get_vid());
  $userpoints_tids = array(0 => t('Select'));
  foreach ($userpoints_terms as $userpoints_term) {
    $userpoints_tids[$userpoints_term->tid] = str_repeat('-', $userpoints_term->depth) . $userpoints_term->name;
  }
  return $userpoints_tids;
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
  foreach (_quiz_get_vocabularies() as $vid => $vocabulary) {
    $temp = taxonomy_form($vid, $value);
    $options = array_merge($options, $temp['#options']);
  }
  return $options;
}

/**
 * Retrieve list of question types.
 *
 * @return
 *  Array of question types.
 */
function _quiz_get_question_types() {
  static $to_return = array();

  // We vastly improves performance by statically caching the question types.
  if (!empty($to_return)) {
    return $to_return;
  }

  // Get question types from the modules that defines them..
  $quiz_questions = module_invoke_all('quiz_question_info');
  if (empty($quiz_questions)) {
    drupal_set_message(t('You need to install and enable at least one question type(multichoice for instance) to use quiz.'), 'warning', FALSE);
    return array();
  }

  return $quiz_questions;
}

/**
 * Retrieve list of vocabularies for all quiz question types.
 *
 * @return
 *   An array containing a vocabulary list.
 */
function _quiz_get_vocabularies() {
  $vocabularies = array();
  $types = array_keys(_quiz_get_question_types());
  foreach ($types as $type) {
    foreach (taxonomy_get_vocabularies($type) as $vid => $vocabulary) {
      $vocabularies[$vid] = $vocabulary;
    }
  }
  return $vocabularies;
}

/**
 * Given a term ID, get all of the question nid/vids that have that ID.
 *
 * @param $tid
 *   Integer term ID.
 *
 * @return
 *   Array of nid/vid combos, like array(array('nid'=>1, 'vid'=>2)).
 */
function _quiz_get_random_taxonomy_question_ids($tid, $num_random) {
  if ($tid == 0) {
    return array();
  }

  // Select random questions by taxonomy.
  $term = taxonomy_term_load($tid);
  $tree = taxonomy_get_tree($term->vid, $term->tid);

  // Flatten the taxonomy tree, and just keep term id's.
  $term_ids[] = $term->tid;
  if (is_array($tree)) {
    foreach ($tree as $term) {
      $term_ids[] = $term->tid;
    }
  }
  $term_ids = implode(',', $term_ids);

  // Get all published questions with one of the allowed term ids.
  // TODO Please convert this statement to the D7 database API syntax.
  $result = db_query_range("SELECT n.nid, n.vid
    FROM {node} n
    INNER JOIN {taxonomy_index} tn USING (nid)
    WHERE n.status = 1 AND tn.tid IN ($term_ids)
    AND n.type IN ('" . implode("','", array_keys(_quiz_get_question_types()))
    . "') ORDER BY RAND()");

  $questions = array();
  while ($question_node = db_fetch_array($result)) {
    $question_node['random'] = TRUE;
    $questions[] = $question_node;
  }

  return $questions;
}

/**
 * Get the summary message for a completed quiz.
 *
 * Summary is determined by whether we are using the pass / fail options, how
 * the user did, and where the method is called from.
 *
 * @todo Need better feedback for when a user is viewing their quiz results
 *   from the results list (and possibily when revisiting a quiz they can't take
 *   again).
 *
 * @param $quiz
 *   The quiz node object.
 * @param $score
 *   The score information as returned by quiz_calculate_score().
 * @return
 *   Filtered summary text or null if we are not displaying any summary.
 */
function _quiz_get_summary_text($quiz, $score) {
  $summary = array();
  $admin = (arg(0) == 'admin');
  $quiz_format = (isset($quiz->body[LANGUAGE_NONE][0]['format'])) ? $quiz->body[LANGUAGE_NONE][0]['format'] : NULL;
  if (!$admin) {
    if (!empty($score['result_option'])) {
      // Unscored quiz, return the proper result option.
      $summary['result'] = check_markup($score['result_option'], $quiz_format);
    }
    else {
      $result_option = _quiz_pick_result_option($quiz->nid, $quiz->vid, $score['percentage_score']);
      $summary['result'] = is_object($result_option) ? check_markup($result_option->option_summary, $result_option->option_summary_format) : '';
    }
  }

  // If we are using pass/fail, and they passed.
  if ($quiz->pass_rate > 0 && $score['percentage_score'] >= $quiz->pass_rate) {
    // If we are coming from the admin view page.
    if ($admin) {
      $summary['passfail'] = t('The user passed this quiz.');
    }
    elseif (variable_get('quiz_use_passfail', 1) == 0) {
      // If there is only a single summary text, use this.
      if (trim($quiz->summary_default) != '') {
        $summary['passfail'] = check_markup($quiz->summary_default, $quiz_format);
      }
    }
    elseif (trim($quiz->summary_pass) != '') {
      // If there is a pass summary text, use this.
      $summary['passfail'] = check_markup($quiz->summary_pass, $quiz->summary_pass_format);
    }
  }
  // If the user did not pass or we are not using pass/fail.
  else {
    // If we are coming from the admin view page, only show a summary if we are
    // using pass/fail.
    if ($admin) {
      if ($quiz->pass_rate > 0) {
        $summary['passfail'] = t('The user failed this quiz.');
      }
      else {
        $summary['passfail'] = t('the user completed this quiz.');
      }
    }
    elseif (trim($quiz->summary_default) != '') {
      $summary['passfail'] = check_markup($quiz->summary_default, $quiz->summary_default_format);
    }
  }
  return $summary;
}

/**
 * Get summary text for a particular score from a set of result options.
 *
 * @param $qnid
 *   The quiz node id.
 * @param $qvid
 *   The quiz node revision id.
 * @param $score
 *   The user's final score.
 *
 * @return
 *   Summary text for the user's score.
 */
function _quiz_pick_result_option($qnid, $qvid, $score) {
  return db_query('SELECT option_summary, option_summary_format FROM {quiz_node_result_options}
      WHERE nid = :nid AND vid = :vid AND :option BETWEEN option_start AND option_end', array(':nid' => $qnid, ':vid' => $qvid, ':option' => $score)
    )->fetch();
}
