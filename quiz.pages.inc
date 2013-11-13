<?php

/**
 * @file
 * Page callback file for the quiz module.
 */


/**
 *  Page callback for quiz options form
 */
function quiz_options_form($form, &$form_state, $node) {
  $user = \Drupal::currentUser();
  $config = \Drupal::config('quiz.settings');
  $form = array();

  $form['nid'] = array(
    '#type' => 'value',
    '#value' => $node->id(),
  );
  
  $form['vid'] = array(
    '#type' => 'value',
    '#value' => $node->getRevisionId(),
  );

  $form['taking'] = array(
    '#type' => 'details',
    '#title' => t('Taking Options'),
    '#access' => $user->hasPermission('administer menu'),
    '#collapsed' => TRUE,
    '#group' => 'advanced',
    '#tree' => TRUE,
    '#weight' => -2,
    '#attributes' => array('class' => array('menu-link-form')),
  );
  
  $form['taking']['allow_resume'] = array(
    '#type' => 'checkbox',
    '#title' => t('Allow Resume'),
    '#default_value' => $node->allow_resume,
    '#description' => t('Whether to allow users to leave the @quiz incomplete and then resume it from where they left off.', array('@quiz' => QUIZ_NAME)),
  );
  $form['taking']['allow_skipping'] = array(
    '#type' => 'checkbox',
    '#title' => t('Allow Skipping questions'),
    '#default_value' => $node->allow_skipping,
    '#description' => t('Whether to allow users to skip questions in the @quiz', array('@quiz' => QUIZ_NAME)),
  );
  $form['taking']['allow_jumping'] = array(
    '#type' => 'checkbox',
    '#title' => t('Allow jumping'),
    '#default_value' => $node->allow_jumping,
    '#description' => t('Whether to allow users to jump between questions using a menu in the @quiz', array('@quiz' => QUIZ_NAME)),
  );
  $form['taking']['backwards_navigation'] = array(
    '#type' => 'checkbox',
    '#title' => t('Backwards navigation'),
    '#default_value' => $node->backwards_navigation,
    '#description' => t('Whether to allow user to go back and revisit their answers'),
  );
  $form['taking']['repeat_until_correct'] = array(
    '#type' => 'checkbox',
    '#title' => t('Repeat until correct'),
    '#default_value' => $node->repeat_until_correct,
    '#description' => t('Require the user to re-try the question until they answer it correctly.'),
  );
  $form['taking']['mark_doubtful'] = array(
    '#type' => 'checkbox',
    '#title' => t('Mark Doubtful'),
    '#default_value' => $node->mark_doubtful,
    '#description' => t('Allow user to mention if they are not sure about the answer'),
  );
  $form['taking']['show_passed'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show passed status'),
    '#default_value' => $node->show_passed,
    '#description' => t('Show the status, if the user has previously passed'),
  );

  $form['taking']['randomization'] = array(
    '#type' => 'radios',
    '#title' => t('Randomize questions'),
    '#options' => array(
      t('No randomization'),
      t('Random order'),
      t('Random questions'),
      t('Categorized random questions'),
    ),
    '#description' => t('The difference between "random order" and "random questions" is that with "random questions" questions are drawn randomly from a pool of questions. With "random order" the quiz will always consist of the same questions. With "Categorized random questions" you can choose several terms questions should be drawn from, and you can also choose how many questions that should be drawn from each, and max score for each term.'),
    '#default_value' => $node->randomization,
  );
  $form['taking']['feedback'] = array(
    '#type' => 'fieldset',
    '#title' => t('Feedback'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
  );
  $form['taking']['feedback']['feedback_time'] = array(
    '#title' => t('Feedback Time'),
    '#type' => 'radios',
    '#default_value' => $node->feedback_time,
    '#options' => _quiz_get_feedback_options(),
    '#description' => t('Indicates at what point feedback for each question will be given to the user'),
  );
  $form['taking']['feedback']['display_feedback'] = array(
    '#title' => t('Display solution'),
    '#type' => 'checkbox',
    '#default_value' => $node->display_feedback,
    '#description' => t('Display the users answers and the correct answers for all questions along with the score for each question.')
  );

  $options = array(t('Unlimited'));
  for ($i = 1; $i < 10; $i++) {
    $options[$i] = $i;
  }
  $form['taking']['multiple_takes'] = array(
    '#type' => 'fieldset',
    '#title' => t('Multiple takes'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
    '#attributes' => array('id' => 'multiple-takes-fieldset'),
  );
  $form['taking']['multiple_takes']['takes'] = array(
    '#type' => 'select',
    '#title' => t('Allowed number of attempts'),
    '#default_value' => $node->takes,
    '#options' => $options,
    '#description' => t('The number of times a user is allowed to take the @quiz. <strong>Anonymous users are only allowed to take quizzes that allow an unlimited number of attempts.</strong>', array('@quiz' => QUIZ_NAME)),
  );
  $form['taking']['multiple_takes']['show_attempt_stats'] = array(
    '#type' => 'checkbox',
    '#title' => t('Display allowed number of attempts'),
    '#default_value' => $node->show_attempt_stats,
    '#description' => t('Display the allowed number of attempts on the starting page for this quiz.'),
  );

  if ($user->hasPermission('delete any quiz results') || $user->hasPermission('delete results for own quiz')) {
    $form['taking']['multiple_takes']['keep_results'] = array(
      '#type' => 'radios',
      '#title' => t('These results should be stored for each user'),
      '#options' => array(
         t('The best'),
         t('The newest'),
         t('All'),
      ),
      '#default_value' => $node->keep_results,
    );
  }
  else {
    $form['taking']['multiple_takes']['keep_results'] = array(
      '#type' => 'value',
      '#value' => $node->keep_results,
    );
  }

  if (function_exists('jquery_countdown_add') && $config->get('quiz_has_timer')) {
    $form['taking']['addons'] = array(
      '#type' => 'fieldset',
      '#title' => t('Quiz Addons Properties'),
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
    );
    $form['taking']['addons']['time_limit'] = array(
      '#type' => 'textfield',
      '#title' => t(' Time Limit'),
      '#default_value' => isset($node->time_limit) ? $node->time_limit : 0,
      '#description' => t('Set the maximum allowed time in seconds for this @quiz. Use 0 for no limit.', array('@quiz' => QUIZ_NAME)),
    );
  }
  else {
    $form['taking']['addons']['time_limit'] = array(
      '#type' => 'value',
      '#value' => 0,
    );
  }
  
  if (function_exists('userpoints_userpointsapi') && $config->get('quiz_has_userpoints')) {
    $form['taking']['has_userpoints'] = array(
      '#type' => 'checkbox',
      '#default_value' => (isset($node->has_userpoints) ? $node->has_userpoints : 1),
      '#title' => t('Enable UserPoints Module Integration'),
      '#description' => t('If checked, marks scored in this @quiz will be credited to userpoints. For each correct answer 1 point will be added to user\'s point.', array('@quiz' => QUIZ_NAME)),
    );
  }

  // Set up the availability options.
  $form['availability'] = array(
    '#type' => 'details',
    '#title' => t('Availability'),
    '#access' => $user->hasPermission('administer menu'),
    '#collapsed' => TRUE,
    '#group' => 'advanced',
    '#tree' => TRUE,
    '#weight' => -1,
    '#attributes' => array('class' => array('menu-link-form')),
  );
  $form['availability']['quiz_always'] = array(
    '#type' => 'checkbox',
    '#title' => t('Always Available'),
    '#default_value' => $node->quiz_always,
    '#description' => t('Click this option to ignore the open and close dates.'),
  );
  $form['availability']['quiz_open'] = array(
    '#type' => 'date',
    '#title' => t('Open Date'),
    '#default_value' => _quiz_form_prepare_date($node->quiz_open),
    '#description' => t('The date this @quiz will become available.', array('@quiz' => QUIZ_NAME)),
    '#after_build' => array('_quiz_limit_year_options'),
  );
  $form['availability']['quiz_close'] = array(
    '#type' => 'date',
    '#title' => t('Close Date'),
    '#default_value' => _quiz_form_prepare_date($node->quiz_close, $config->get('quiz_default_close')),
    '#description' => t('The date this @quiz will cease to be available.', array('@quiz' => QUIZ_NAME)),
    '#after_build' => array('_quiz_limit_year_options'),
  );

  // Quiz summary options.
  $form['pass_fail'] = array(
    '#type' => 'details',
    '#title' => t('Pass/Fail'),
    '#access' => $user->hasPermission('administer menu'),
    '#collapsed' => TRUE,
    '#group' => 'advanced',
    '#tree' => TRUE,
    '#weight' => -1,
    '#attributes' => array('class' => array('menu-link-form')),
  );
  
  // If pass/fail option is checked, present the form elements.
  if ($config->get('quiz_use_passfail')) {
    $form['pass_fail']['pass_rate'] = array(
      '#type' => 'textfield',
      '#title' => t('Pass rate for @quiz (%)', array('@quiz' => QUIZ_NAME)),
      '#default_value' => $node->pass_rate,
      '#description' => t('Pass rate for the @quiz as a percentage score.', array('@quiz' => QUIZ_NAME)),
      '#required' => FALSE,
    );
    $form['pass_fail']['summary_pass'] = array(
      '#type' => 'text_format',
      '#base_type' => 'textarea',
      '#title' => t('Summary text if passed'),
      '#default_value' => $node->summary_pass,
      '#cols' => 60,
      '#description' => t("Summary for when the user gets enough correct answers to pass the @quiz. Leave blank if you don't want to give different summary text if they passed or if you are not using the 'percent to pass' option above. If you don't use the 'Percentage needed to pass' field above, this text will not be used.", array('@quiz' => QUIZ_NAME)),
      '#format' => isset($node->summary_pass_format) && !empty($node->summary_pass_format) ? $node->summary_pass_format : NULL,
    );
  }
  // If the pass/fail option is unchecked, use the default and hide it.
  else {
    $form['pass_fail']['pass_rate'] = array(
      '#type' => 'hidden',
      '#value' => $node->pass_rate,
      '#required' => FALSE,
    );
  }
  // We use a helper to enable the wysiwyg module to add an editor to the
  // textarea.
  $form['pass_fail']['helper']['summary_default'] = array(
    '#type' => 'text_format',
    '#base_type' => 'textarea',
    '#title' => t('Default summary text'),
    '#default_value' => $node->summary_default,
    '#cols' => 60,
    '#description' => t("Default summary. Leave blank if you don't want to give a summary."),
    '#format' => isset($node->summary_default_format) && !empty($node->summary_default_format) ? $node->summary_default_format : NULL,
  );

  // Number of random questions, max score and tid for random questions are set on
  // the manage questions tab. We repeat them here so that they're not removed
  // if the quiz is being updated.
  $num_rand = (isset($node->number_of_random_questions)) ? $node->number_of_random_questions : 0;
  $form['number_of_random_questions'] = array(
    '#type' => 'value',
    '#value' => $num_rand,
  );
  $max_score_for_random = (isset($node->max_score_for_random)) ? $node->max_score_for_random : 0;
  $form['max_score_for_random'] = array(
    '#type' => 'value',
    '#value' => $max_score_for_random,
  );
  $tid = (isset($node->tid)) ? $node->tid : 0;
  $form['tid'] = array(
    '#type' => 'value',
    '#value' => $tid,
  );

  $options = !empty($node->resultoptions) ? $node->resultoptions : array();
  $num_options = max(count($options), $config->get('quiz_max_result_options'));

  if ($num_options > 0) {
    $form['resultoptions'] = array(
      '#type' => 'details',
      '#title' => t('Result Comments'),
      '#access' => $user->hasPermission('administer menu'),
      '#collapsed' => TRUE,
      '#group' => 'advanced',
      '#tree' => TRUE,
      '#weight' => -1,
      '#attributes' => array('class' => array('menu-link-form')),
    );

    for ($i = 0; $i < $num_options; $i++) {
      $option = (count($options) > 0) ? array_shift($options) : NULL; // grab each option in the array
      $form['resultoptions'][$i] = array(
        '#type' => 'fieldset',
        '#title' => t('Result Option ') . ($i + 1),
        '#collapsible' => TRUE,
        '#collapsed' => FALSE,
      );
      $form['resultoptions'][$i]['option_name'] = array(
        '#type' => 'textfield',
        '#title' => t('The name of the result'),
        '#default_value' => isset($option['option_name']) ? $option['option_name'] : '',
        '#maxlength' => 40,
        '#size' => 40,
      );
      $form['resultoptions'][$i]['option_start'] = array(
        '#type' => 'textfield',
        '#title' => t('Percentage Start Range'),
        '#description' => t('Show this result for scored quizzes in this range (0-100).'),
        '#default_value' => isset($option['option_start']) ? $option['option_start'] : '',
        '#size' => 5,
      );
      $form['resultoptions'][$i]['option_end'] = array(
        '#type' => 'textfield',
        '#title' => t('Percentage End Range'),
        '#description' => t('Show this result for scored quizzes in this range (0-100).'),
        '#default_value' => isset($option['option_end']) ? $option['option_end'] : '',
        '#size' => 5,
      );
      $form['resultoptions'][$i]['option_summary'] = array(
        '#type' => 'text_format',
        '#base_type' => 'textarea',
        '#title' => t('Display text for the result'),
        '#default_value' => isset($option['option_summary']) ? $option['option_summary'] : '',
        '#description' => t('Result summary. This is the summary that is displayed when the user falls in this result set determined by his/her responses.'),
        '#format' => isset($option['option_summary_format']) ? $option['option_summary_format'] : NULL,
      );
      if (isset($option['option_id'])) {
        $form['resultoptions'][$i]['option_id'] = array(
          '#type' => 'hidden',
          '#value' => isset($option['option_id']) ? $option['option_id'] : '',
        );
      }
    }
  }
  $form['remember_settings'] = array(
    '#type' => 'checkbox',
    '#title' => t('Remember my settings'),
    '#description' => t('If this box is checked most of the quiz specific settings you have made will be remembered and will be your default settings next time you create a quiz.'),
    '#weight' => 2,
  );
 
  if (quiz_has_been_answered($node)) {
    $node->revision = 1;
    $node->log = t('The current revision has been answered. We create a new revision so that the reports from the existing answers stays correct.');
  }

  $form['delete'] = array(
    '#type' => 'button',
    '#value' => t('Delete'),
    '#weight' => 3,
  );
  
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#weight' => 3,
  );
  
  return $form;
}


/**
 *  Page callback for quiz options validate
 */
function quiz_options_form_validate($form, &$form_state) {
  $node = new \stdClass();

  $node->quiz_always                = $form_state['values']['availability']['quiz_always'];
  $node->quiz_open                  = $form_state['values']['availability']['quiz_open']; //TODO: Need to verify
  $node->quiz_close                 = $form_state['values']['availability']['quiz_close']; //TODO: Need to verify
  $node->pass_rate                  = $form_state['values']['pass_fail']['pass_rate'];
  $node->resultoptions              = $form_state['values']['resultoptions'];
  $node->allow_skipping             = $form_state['values']['taking']['allow_skipping'];
  $node->allow_jumping              = $form_state['values']['taking']['allow_jumping'];

  if (isset($form_state['values']['taking']['addons']['time_limit'])) {
    $node->time_limit = $form_state['values']['taking']['addons']['time_limit'];
  }
  $quiz_open = explode(' ' ,$node->quiz_open);
  $quiz_close = explode(' ' ,$node->quiz_close);

  // Don't check dates if the quiz is always available.
  if (!$node->quiz_always && count($quiz_open) == 3 && count($quiz_close) == 3) {
    if (mktime(0, 0, 0, $quiz_open[1], $quiz_open[0], $quiz_open[2]) > mktime(0, 0, 0, $quiz_close[1], $quiz_close[0], $quiz_close[2])) {
      form_set_error('quiz_close', t('Please make sure the close date is after the open date.'));
    }
  }

  if (!empty($node->pass_rate)) {
    if (!_quiz_is_int($node->pass_rate, 0, 100)) {
      form_set_error('pass_rate', t('The pass rate value must be a number between 0 and 100.'));
    }
  }

  if (isset($node->time_limit)) {
    if (!_quiz_is_int($node->time_limit, 0)) {
      form_set_error('time_limit', t('Time limit must be a non negative interger'));
    }
  }

  if (isset($node->resultoptions) && count($node->resultoptions) > 0) {
    $taken_values = array();
    $num_options = 0;
    foreach ($node->resultoptions as $option) {
      if (!empty($option['option_name'])) {
        $num_options++;
        if (empty($option['option_summary'])) {
          form_set_error('option_summary', t('Option has no summary text.'));
        }
        if ($node->pass_rate && (isset($option['option_start']) || isset($option['option_end']))) {

          // Check for a number between 0-100.
          foreach (array('option_start' => 'start', 'option_end' => 'end') as $bound => $bound_text) {
            if (!_quiz_is_int($option[$bound], 0, 100)) {
              form_set_error($bound, t('The range %start value must be a number between 0 and 100.', array('%start' => $bound_text)));
            }
          }

          // Check that range end >= start.
          if ($option['option_start'] > $option['option_end']) {
            form_set_error('option_start', t('The start must be less than the end of the range.'));
          }

          // Check that range doesn't collide with any other range.
          $option_range = range($option['option_start'], $option['option_end']);
          if ($intersect = array_intersect($taken_values, $option_range)) {
            form_set_error('option_start', t('The ranges must not overlap each other. (%intersect)', array('%intersect' => implode(',', $intersect))));
          }
          else {
            $taken_values = array_merge($taken_values, $option_range);
          }
        }
      }
      elseif (!_quiz_is_empty_html($option['option_summary']['value'])) {
        form_set_error('option_summary', t('Option has a summary, but no name.'));
      }
    }
  }
  if ($node->allow_jumping && !$node->allow_skipping) {
    form_set_error('allow_skipping', t('If jumping is allowed skipping also has to be allowed.'));
  }
}

/**
 *  Page callback for quiz options submit
 */
function quiz_options_form_submit($form, &$form_state) {
  $node = menu_get_object();
  $node->nid                        = $form_state['values']['nid'];
  $node->vid                        = $form_state['values']['vid'];
  //Taking
  $node->allow_resume               = $form_state['values']['taking']['allow_resume'];
  $node->allow_skipping             = $form_state['values']['taking']['allow_skipping'];
  $node->allow_jumping              = $form_state['values']['taking']['allow_jumping'];
  $node->backwards_navigation       = $form_state['values']['taking']['backwards_navigation'];
  $node->repeat_until_correct       = $form_state['values']['taking']['repeat_until_correct'];
  $node->mark_doubtful              = $form_state['values']['taking']['mark_doubtful'];
  $node->show_passed                = $form_state['values']['taking']['show_passed'];
  $node->randomization              = $form_state['values']['taking']['randomization'];
  $node->feedback_time              = $form_state['values']['taking']['feedback']['feedback_time'];
  $node->display_feedback           = $form_state['values']['taking']['feedback']['display_feedback'];
  $node->takes                      = $form_state['values']['taking']['multiple_takes']['takes'];
  $node->show_attempt_stats         = $form_state['values']['taking']['multiple_takes']['show_attempt_stats'];
  $node->keep_results               = $form_state['values']['taking']['multiple_takes']['keep_results'];
  $node->time_limit                 = $form_state['values']['taking']['addons']['time_limit'];
  //Availability
  $node->quiz_always                = $form_state['values']['availability']['quiz_always'];
  $node->quiz_open                  = strtotime(str_replace(' ', '-', $form_state['values']['availability']['quiz_open'])); //TODO: Need to verify
  $node->quiz_close                 = strtotime(str_replace(' ', '-', $form_state['values']['availability']['quiz_close'])); //TODO: Need to verify
  //Pass/Fail
  $node->pass_rate                  = $form_state['values']['pass_fail']['pass_rate'];
  $node->summary_pass               = $form_state['values']['pass_fail']['summary_pass'];
  $node->summary_default            = $form_state['values']['pass_fail']['helper']['summary_default'];
  //Result comments
  $node->resultoptions              = $form_state['values']['resultoptions'];
  $node->remember_settings          = $form_state['values']['remember_settings'];
  $node->number_of_random_questions = $form_state['values']['number_of_random_questions'];
  $node->max_score_for_random       = $form_state['values']['max_score_for_random'];  
  $node->has_userpoints             = isset($form_state['values']['taking']['has_userpoints']) ? $form_state['values']['taking']['has_userpoints'] : 0;

  _quiz_save_user_settings($node);
  // Quiz node vid (revision) was updated.
  if ($node->isNewRevision()) { //TODO: Need to handle new revisions.
    // Insert a new row in the quiz_node_properties table.
    $old_auto = isset($node->auto_created);
    $node->auto_created = TRUE;
    quiz_node_insert($node);
    if (!$old_auto) {
      unset($node->auto_created);
    }

    // Create new quiz-question relation entries in the quiz_node_relationship
    // table.
    quiz_update_quiz_question_relationship($node->old_vid, $node->getRevisionId(), $node->id()); //TODO: find equivalent for $node->old_vid in D8.
  }

  // Quiz node vid (revision) was not updated.
  else {
    // Update an existing row in the quiz_node_properties table.
    _quiz_common_presave_actions($node);
    $resource = db_update('quiz_node_properties')
      ->fields(array(
        'vid'                         => $node->getRevisionId(),
        'aid'                         => $node->aid,
        'randomization'               => $node->randomization,
        'backwards_navigation'        => $node->backwards_navigation,
        'repeat_until_correct'        => $node->repeat_until_correct,
        'quiz_open'                   => $node->quiz_open,
        'quiz_close'                  => $node->quiz_close,
        'takes'                       => $node->takes,
        'show_attempt_stats'          => $node->show_attempt_stats,
        'keep_results'                => $node->keep_results,
        'time_limit'                  => $node->time_limit,
        'pass_rate'                   => $node->pass_rate,
        'summary_pass'                => is_array($node->summary_pass) ? $node->summary_pass['value'] : $node->summary_pass,
        'summary_pass_format'         => is_array($node->summary_pass) ? $node->summary_pass['format'] : $node->summary_pass_format,
        'summary_default'             => is_array($node->summary_default) ? $node->summary_default['value'] : $node->summary_default,
        'summary_default_format'      => is_array($node->summary_default) ? $node->summary_default['format'] : $node->summary_default_format,
        'quiz_always'                 => $node->quiz_always,
        'feedback_time'               => $node->feedback_time,
        'display_feedback'            => $node->display_feedback,
        'number_of_random_questions'  => $node->number_of_random_questions,
        'has_userpoints'              => $node->has_userpoints,
        'allow_skipping'              => $node->allow_skipping,
        'allow_resume'                => $node->allow_resume,
        'allow_jumping'               => $node->allow_jumping,
        'show_passed'                 => $node->show_passed,
        'mark_doubtful'		            => $node->mark_doubtful
       ))
      ->condition('vid', $node->getRevisionId())
      ->condition('nid', $node->id())
      ->execute();
    _quiz_update_resultoptions($node);
  }
  _quiz_check_num_random($node);
  _quiz_check_num_always($node);
  quiz_update_max_score_properties(array($node->getRevisionId()));
  drupal_set_message(t('Some of the updated settings may not apply to quiz being taken already. To see all changes in action you need to start again.'), 'warning');
}

/**
 * User pages.
 * @file
 */

/**
 * Show result page for a given result id
 *
 * @param $result_id
 *  Result id
 */
function quiz_user_results($result_id) {
  $user = \Drupal::currentUser();

  $result = db_query('SELECT qnp.nid, qnp.vid, qnrs.uid
    FROM {quiz_node_properties} qnp
    INNER JOIN {quiz_node_results} qnrs ON qnrs.vid = qnp.vid
    WHERE qnrs.result_id = :rid', array(':rid' => $result_id))->fetch();

  if ($result->nid) {
    // User can view own results (quiz_menu sets access to 'own results').
    // User with role 'user results' can view other user's results.
    if ($result->uid != $user->id() && !$user->hasPermission('view any quiz results')) {
      drupal_access_denied();
      return;
    }

    $quiz = node_load($result->nid, $result->vid);
    $questions = _quiz_get_answers($quiz, $result_id);
    $score = quiz_calculate_score($quiz, $result_id);
    $summary = _quiz_get_summary_text($quiz, $score);
    $data = array(
      'quiz' => $quiz,
      'questions' => $questions,
      'score' => $score,
      'summary' => $summary,
      'rid' => $result_id,
    );
    return theme('quiz_user_summary', $data);
  }
  else {
    drupal_not_found();
  }
}

/**
 * Form for showing feedback, and for editing the feedback if necessary...
 *
 * @param $form_state
 *   FAPI form state(array)
 * @param $questions
 *   array of questions to inclide in the report
 * @param $showpoints
 *   Should points be included in the report? (Boolean)
 * @param $showfeedback
 *   Should feedback be included in the report? (Boolean)
 * @param $allow_scoring
 *   Should we allow the user to score results that needs manual scoring? (Boolean)
 * @return $form
 *   FAPI form array
 */
function quiz_report_form($form, $form_state, $questions, $showpoints = TRUE, $showfeedback = TRUE, $allow_scoring = FALSE) {
  $form = array();
  // The submit button is only shown if one or more of the questions has input elements
  $show_submit = FALSE;
  foreach ($questions as $question) {
    $module = quiz_question_module_for_type($question->getType());
    if (!$module) {
      return array();
    }
    $function = $module . '_report_form';
    $form_to_add = $function($question, $showpoints, $showfeedback, $allow_scoring);
    if (isset($form_to_add['submit'])) {
      $show_submit = TRUE;
    }
    $form[] = $form_to_add;
  }
  $form['#theme'] = 'quiz_report_form';
  $form['#showpoints'] = $showpoints;
  $form['#showfeedback'] = $showfeedback;
  $form['#tree'] = TRUE;
  if ($show_submit) {
    $form['submit'] = array(
      '#type' => 'submit',
      '#submit' => array('quiz_report_form_submit'),
      '#validate' => array('quiz_report_form_validate'),
      '#value' => t('Save Score'),
    );
  }
  return $form;
}

/**
 * Validate the report form
 */
function quiz_report_form_validate($form, &$form_state) {
  /* We go through the form state values and validates all
   * questiontypes with validation functions declared.
   */
  foreach ($form_state['values'] as $key => $q_values) {
    // Questions has numeric keys in the report form
    if (!is_numeric($key)) {
      continue;
    }
    // Questions store the name of the validation function with the key 'validate'
    if (!isset($q_values['validate'])) {
      continue;
    }
    // The validation function must exist
    if (!function_exists($q_values['validate'])) {
      continue;
    }
    // We call the validation function provided by the question
    call_user_func($q_values['validate'], $q_values, $key);
  }
}

/**
 * Submit the report form
 */
function quiz_report_form_submit($form, &$form_state) {
  /* We go through the form state values and submit all
   * questiontypes with validation functions declared.
   */
  global $user;
  foreach ($form_state['values'] as $key => $q_values) {
    // Questions has numeric keys in the report form
    if (!is_numeric($key)) {
      continue;
    }
    // Questions store the name of the validation function with the key 'submit'
    if (!isset($q_values['submit'])) {
      continue;
    }
    // The submit function must exist
    if (!function_exists($q_values['submit'])) {
      continue;
    }

    // Load the quiz
    if (!isset($quiz)) {
      $result = db_query('SELECT nid, uid, vid FROM {quiz_node_results} WHERE result_id = :result_id', array(':result_id' => $q_values['rid']))->fetchObject();
      $quiz = node_load($result->nid, $result->vid);
      $rid = $q_values['rid'];
    }

    $q_values['quiz'] = $quiz;

    // We call the submit function provided by the question
    call_user_func($q_values['submit'], $q_values);
  }
  // Scores may have been changed. We take the necessary actions
  quiz_update_total_score_fast($rid, $quiz->vid);
  $changed = db_update('quiz_node_results')
  ->fields(array(
    'is_evaluated' =>  1,
  ))
  ->condition('result_id', $rid)
  ->execute();
  $results_got_deleted = _quiz_maintain_results($quiz, $rid);

  // A message saying the quiz is unscored has already been set. We unset it here...
  if($changed > 0) {
    _quiz_remove_unscored_message();
  }

  // Notify the user if results got deleted as a result of him scoring an answer.
  $add = $quiz->keep_results == QUIZ_KEEP_BEST && $results_got_deleted ? ' ' . t('Note that this quiz is set to only keep each users best answer.') : '';

  $score_data = quiz_get_score_array($rid, $quiz->vid, TRUE);

  module_invoke_all('quiz_scored', $quiz, $score_data, $rid);

  drupal_set_message(t('The scoring data you provided has been saved.') . $add);
  if (user_access('score taken quiz answer') && !user_access('view any quiz results')) {
    if ($result && $result->uid == $user->uid) {
      $form_state['redirect'] = 'node/' . $quiz->nid . '/results/' . $rid;
    }
  }
}

/**
 * Helper function to remove the message saying the quiz haven't been scored
 */
function _quiz_remove_unscored_message() {
  if (is_array($_SESSION['messages']['warning'])) {
    // Search for the message, and remove it if we find it.
    foreach ($_SESSION['messages']['warning'] as $key => $val) {
      if ($val == t('This quiz has not been scored yet.')) {
        unset($_SESSION['messages']['warning'][$key]);
      }
    }
    // Clean up if the message array was left empty
    if (empty($_SESSION['messages']['warning'])) {
      unset($_SESSION['messages']['warning']);
      if (empty($_SESSION['messages'])) {
        unset($_SESSION['messages']);
      }
    }
  }
}

/**
 * Updates the total score using only one mySql query.
 *
 * @param $rid
 *  Result id
 * @param $quiz_vid
 *  Quiz node version id
 */
function quiz_update_total_score_fast($rid, $quiz_vid) {
	$subq1 = db_select('quiz_node_results_answers', 'a');
	$subq1
		->condition('a.result_id', $rid)
		->addExpression('SUM(a.points_awarded)');
	$res1 = $subq1->execute()->fetchField();

	$subq2 = db_select('quiz_node_properties', 'qnp');
	$subq2
		->condition('qnp.vid', $quiz_vid)
		->addField('qnp', 'max_score');
	$res2 = $subq2->execute()->fetchField();

	db_update('quiz_node_results')
		->expression('score', 'ROUND(100*(:res1/:res2))', array(':res1' => $res1, ':res2' => $res2))
		->condition('result_id', $rid)
		->execute();
}

/**
 * Returns an array of score information for a quiz
 *
 * @param unknown_type $rid
 * @param unknown_type $quiz_vid
 * @param unknown_type $is_evaluated
 */
function quiz_get_score_array($rid, $quiz_vid, $is_evaluated) {
  $properties = db_query('SELECT max_score, number_of_random_questions
          FROM {quiz_node_properties}
          WHERE vid = :vid', array(':vid' => $quiz_vid))->fetchObject();
  $total_score = db_query('SELECT SUM(points_awarded)
          FROM (quiz_node_results_answers)
          WHERE result_id = :result_id', array(':result_id' => $rid))->fetchField();

  return array(
    'question_count' => $properties->number_of_random_questions + _quiz_get_num_always_questions($quiz_vid),
    'possible_score' => $properties->max_score,
    'numeric_score' => $total_score,
    'percentage_score' => ($properties->max_score == 0) ? 0 : round(($total_score * 100) / $properties->max_score),
    'is_evaluated' => $is_evaluated,
  );
}

/**
 * Displays all the quizzes the user has taken part in.
 *
 * @param $user_id
 *  User id
 * @return
 *  HTML output for page.
 */
function quiz_get_user_results($uid) {
  $results = array();
  $dbresult = db_query('SELECT DISTINCT(n.nid), n.title, qnp.pass_rate, qnrs.result_id, qnrs.time_start, qnrs.time_end, qnrs.score, qnrs.is_evaluated
    FROM {node} n
    INNER JOIN {quiz_node_properties} qnp ON n.nid = qnp.nid
    INNER JOIN {quiz_node_results} qnrs ON qnrs.vid = qnp.vid
    INNER JOIN {users} u ON u.uid = qnrs.uid
    WHERE n.type = :type
      AND u.uid = :uid
    ORDER BY qnrs.result_id ASC', array(':type' => 'quiz', ':uid' => $uid));

  // Create an array out of the results.
  foreach ($dbresult as $result) {
    $result = (array) $result;
    $results[$result['result_id']] = $result;
  }
  return theme('quiz_get_user_results', array('results' => $results));
}

/**
 * Show results for the current quiz
 */
function quiz_my_results($node) {
  global $user;
  $results = array();
  $res = db_query('
    SELECT qnp.nid, qnp.pass_rate, qnrs.result_id, qnrs.time_start, qnrs.time_end, qnrs.score
    FROM {quiz_node_properties} qnp
    INNER JOIN {quiz_node_results} qnrs ON qnrs.vid = qnp.vid
    WHERE qnrs.uid = :uid AND qnrs.nid = :nid AND qnrs.is_evaluated = 1
    ORDER BY qnrs.result_id DESC',
    array(':uid' => $user->uid, ':nid' => $node->nid)
  );
  // Create an array out of the results.
  while ($line = $res->fetchAssoc()) {
    $results[$line['result_id']] = $line;
  }
  return theme('quiz_my_results_for_quiz', array('rows' => $results));
}

// THEME FUNCTIONS

/**
 * Theme the user results page.
 *
 * @param $results
 *  An array of quiz information.
 * @return
 *  Themed html.
 *
 * @ingroup themeable
 */
function theme_quiz_get_user_results($variables) {
  $results = $variables['results'];
  $rows = array();

  while (list($key, $result) = each($results)) {
    $interval = _quiz_format_duration($result['time_end'] - $result['time_start']);
    $passed = $result['score'] >= $result['pass_rate'];
    $grade = $passed ? t('Passed') : t('Failed');
    $passed_class = $passed ? 'quiz-passed' : 'quiz-failed';
    if (!is_numeric($result['score'])) {
      $score = t('In progress');
    }
    elseif (!$result['is_evaluated']) {
      $score = t('Not evaluated');
    }
    else {
      if (!empty($result['pass_rate']) && is_numeric($result['score'])) {
        $pre_score = '<span class = "' . $passed_class . '">';
        $post_score = ' %<br><em>' . $grade . '</em></span>';
      }
      else {
        $post_score = ' %';
      }
      $score = $pre_score . $result['score'] . $post_score;
    }
    $rows[] = array(
      'title'       => l($result['title'], 'node/' . $result['nid']),
      'time_start'  => format_date($result['time_start'], 'short'),
      'time_end'    => ($result['time_end'] > 0) ? format_date($result['time_end'], 'short') . '<br />' . t('Duration :  @value', array('@value' => $interval)) : t('In Progress'),

      'score'       => $score,
      'evaluated'   => $result['is_evaluated'] ? t('Yes') : t('No'),
      'op'          => l(t('View answers'), 'user/quiz/' . $result['result_id'] . '/userresults'),
    );

  }

  if (empty($rows)) {
    return t('No @quiz results found.', array('@quiz' => QUIZ_NAME));
  }

  $header = array(
    t('@quiz Title', array('@quiz' => QUIZ_NAME)),
    t('Started'),
    t('Finished'),
    t('Score'),
    t('Evaluated'),
    t('Operation'),
  );

  $output = theme('table', array('header' => $header, 'rows' => $rows));
  $output .= '<p><em>' . t('@quizzes that are not evaluated may have a different score and grade once evaluated.', array('@quizzes' => QUIZ_NAME)) . '</em></p>';
  return $output;
}

/**
 * Theme the user results page.
 *
 * @param $results
 *  An array of quiz information.
 * @return
 *  Themed html.
 *
 * @ingroup themeable
 */
function theme_quiz_my_results_for_quiz($variables) {
  $results = $variables['rows'];
  $rows = array();
  $with_passing = FALSE;
  while (list($key, $result) = each($results)) {
    $interval = _quiz_format_duration($result['time_end'] - $result['time_start']);
    $score = $result['score'] . ' %';

    $row = array(
      'time_start'  => format_date($result['time_start'], 'short'),
      'duration' => $interval,
      'score'       => $score,
    );
    if (!empty($result['pass_rate'])) {
      $with_passing = TRUE;
      $passed = $result['score'] >= $result['pass_rate'];
      $grade = $passed ? t('Passed') : t('Failed');
      $passed_class = $passed ? 'quiz-passed' : 'quiz-failed';
      $pre = '<span class = "' . $passed_class . '">';
      $row['passed'] = $pre . $grade . '</span>';
    }
    $row['more'] = l(t('More') . '...', 'node/' . $result['nid'] . '/myresults/' . $result['result_id']);
    $rows[] = $row;
  }

  if (empty($rows)) {
    return t('No @quiz results found.', array('@quiz' => QUIZ_NAME));
  }

  $header = array(
    t('Started'),
    t('Duration'),
    t('Score'),
  );
  if ($with_passing) {
    $header[] = t('Passed');
  }
  $header[] = '';
  $output = theme('table', array('header' => $header, 'rows' => $rows));
  return $output;
}

/**
 * Pass the correct mark to the theme so that theme authors can use an image.
 *
 * @ingroup themeable
 */
function theme_quiz_score_correct() {
  $variables = array(
    'path' => drupal_get_path('module', 'quiz') . '/images/correct.gif',
    'alt' => t('correct'),
    'title' => t('correct'),
  );
  return theme('image', $variables);
}

/**
 * Pass the incorrect mark to the theme so that theme authors can use an image.
 *
 * @ingroup themeable
 */
function theme_quiz_score_incorrect() {
  $variables = array(
    'path' => drupal_get_path('module', 'quiz') . '/images/incorrect.gif',
    'alt' => t('incorrect'),
    'title' => t('incorrect'),
  );
  return theme('image', $variables);
}

/**
 * Theme a progress indicator for use during a quiz.
 *
 * @param $question_number
 *  The position of the current question in the sessions' array.
 * @param $num_of_question
 *  The number of questions for this quiz as returned by quiz_get_number_of_questions().
 * @return
 *  Themed html.
 *
 * @ingroup themeable
 */
function theme_quiz_progress($variables) {
  $question_number = $variables['question_number'];
  $num_of_question = $variables['num_questions'];
  // TODO Number of parameters in this theme funcion does not match number of parameters found in hook_theme.
  // Determine the percentage finished (not used, but left here for other implementations).
  //$progress = ($question_number*100)/$num_of_question;

  // Get the current question # by adding one.
  $current_question = $question_number + 1;

  if ($variables['allow_jumping']) {
    $current_question = theme('quiz_jumper', array('current' => $current_question, 'num_questions' => $num_of_question));
  }

  $output  = '';
  $output .= '<div id="quiz_progress">';
  $output .= t('Question <span id="quiz-question-number">!x</span> of <span id="quiz-num-questions">@y</span>', array('!x' => $current_question, '@y' => $num_of_question));
  $output .= '</div>' . "\n";
  // Add div to be used by jQuery countdown
  if ($variables['time_limit']) {
    $output .= '<div class="countdown"></div>';
  }
  return $output;
}

/**
 * @todo Please document this function.
 * @see http://drupal.org/node/1354
 */
function theme_quiz_jumper($variables) {
  $current = $variables['current'];
  $num_questions = $variables['num_questions'];
  $output = '<select name="quiz-jumper" class="form-select" id="quiz-jumper">';
  for ($i = 1; $i <= $num_questions; $i++) {
    $extra = $i == $current ? ' selected="selected"' : '';
    $output .= '<option value="' . $i . '"' . $extra . '>' . $i . '</option>';
  }
  $output .= '</select><span id="quiz-jumper-no-js">' . $current . '</span>';
  drupal_add_js('
    (function ($) {
      Drupal.behaviors.quizJumper = {
        attach: function(context, settings) {
          $("#quiz-jumper:not(.quizJumper-processed)", context).show().addClass("quizJumper-processed").change(function(){
            $("[name=jump_to_question]").val($(this).val());
            $("#edit-submit").trigger("click");
          });
          $("#quiz-jumper-no-js:not(.quizJumper-processed)").hide().addClass("quizJumper-processed");
        }
      };
    })(jQuery);
  ', array('type' => 'inline', 'scope' => JS_DEFAULT));
  return $output;
}

/**
 * Theme the summary page after the quiz has been completed.
 *
 * @param $quiz
 *  The quiz node object.
 * @param $questions
 *  The questions array as defined by _quiz_get_answers.
 * @param $score
 *  Array of score information as returned by quiz_calculate_score().
 * @param $summary
 *  Filtered text of the summary.
 * @return
 *  Themed html.
 *
 * @ingroup themeable
 */
function theme_quiz_take_summary($variables) {
  $quiz = $variables['quiz'];
  $questions = $variables['questions'];
  $score = $variables['score'];
  $summary = $variables['summary'];
  $rid =  $variables['rid'];
  // Set the title here so themers can adjust.
  drupal_set_title($quiz->getTitle());

  // Display overall result.
  $output = '';
  if (!empty($score['possible_score'])) {
    if (!$score['is_evaluated']) {
      if (user_access('score taken quiz answer')) {
        $msg = t('Parts of this @quiz have not been evaluated yet. The score below is not final. <a class="self-score" href="!result_url">Click here</a> to give scores on your own.', array('@quiz' => QUIZ_NAME, '!result_url' => url('node/' . $quiz->id() . '/results/' . $rid)));
      }
      else {
        $msg = t('Parts of this @quiz have not been evaluated yet. The score below is not final.', array('@quiz' => QUIZ_NAME));
      }
      drupal_set_message($msg, 'warning');
    }
    $output .= '<div id="quiz_score_possible">' . t('You got %num_correct of %question_count possible points.', array('%num_correct' => $score['numeric_score'], '%question_count' => $score['possible_score'])) . '</div>' . "\n";
    $output .= '<div id="quiz_score_percent">' . t('Your score: %score %', array('%score' => $score['percentage_score'])) . '</div>' . "\n";
  }
  if (isset($summary['passfail'])) {
    $output .= '<div id="quiz_summary">' . $summary['passfail'] . '</div>' . "\n";
  }
  if (isset($summary['result'])) {
    $output .= '<div id="quiz_summary">' . $summary['result'] . '</div>' . "\n";
  }
  // Get the feedback for all questions. These are included here to provide maximum flexibility for themers
  if ($quiz->display_feedback) {
    $form = drupal_get_form('quiz_report_form', $questions);
    $output .= drupal_render($form);
  }
  return $output;
}

/**
 * Theme the summary page for user results.
 *
 * @param $quiz
 *  The quiz node object.
 * @param $questions
 *  The questions array as defined by _quiz_get_answers.
 * @param $score
 *  Array of score information as returned by quiz_calculate_score().
 * @param $summary
 *  Filtered text of the summary.
 * @return
 *  Themed html.
 *
 * @ingroup themeable
 */
function theme_quiz_user_summary($variables) {
  $quiz = $variables['quiz'];
  $questions = $variables['questions'];
  $score = $variables['score'];
  $summary = $variables['summary'];
  $rid = $variables['rid'];
  // Set the title here so themers can adjust.
  drupal_set_title($quiz->getTitle());

  if (!$score['is_evaluated']) {
    if (user_access('score taken quiz answer')) {
      $msg = t('Parts of this @quiz have not been evaluated yet. The score below is not final. <a class="self-score" href="!result_url">Click here</a> to give scores on your own.', array('@quiz' => QUIZ_NAME, '!result_url' => url('node/' . $quiz->id() . '/results/' . $rid)));
    }
    else {
      $msg = t('Parts of this @quiz have not been evaluated yet. The score below is not final.', array('@quiz' => QUIZ_NAME));
    }
  }

  // Display overall result.
  $output = '';
  $output .= '<div id="quiz_score_possible">' .
    t('You got %num_correct of %question_count possible points.', array('%num_correct' => $score['numeric_score'], '%question_count' => $score['possible_score'])) .
    '</div>';
  $output .= '<div id="quiz_score_percent">' .
    t('Your score was: @score %', array('@score' => $score['percentage_score'])) .
    '</div>';

  if (isset($summary['passfail'])) {
    $output .= '<div id="quiz_summary">' . $summary['passfail'] . '</div>' . "\n";
  }
  if (isset($summary['result'])) {
    $output .= '<div id="quiz_summary">' . $summary['result'] . '</div>' . "\n";
  }
  // Get the feedback for all questions.
  $form = drupal_get_form('quiz_report_form', $questions, FALSE, TRUE);
  $output .= drupal_render($form);
  return $output;
}

/**
 * Theme the "no feedback" option.
 *
 * @return
 *  Themed html feedback.
 *
 * @ingroup themeable
 */
function theme_quiz_no_feedback() {
  return t('Thanks for taking the quiz!');
}

/**
 * Theme the single question node
 *
 * @param $node
 *  The question node
 * @return
 *  Themed html feedback
 */
function theme_quiz_single_question_node($variables) {
  $content = $variables['content'];
  // This might seem meaningless, but it is designed this way to allow themes to add more
  // meaningful stuff here...
  return drupal_render($content['body']);
}

/**
 * Theme the stats on the views page
 *
 * @param $node
 *   The quiz node
 */
function theme_quiz_view_stats($variables) {
  $node = $variables['node'];
  // Fetch data
  $stats = array(
    array(
      'title' => t('Questions'),
      'data' => $node->number_of_questions,
    ),
  );
  if ($node->show_attempt_stats) {
    $takes = $node->takes == 0 ? t('Unlimited') : $node->takes;
    $stats[] = array(
      'title' => t('Attempts allowed'),
      'data' => $takes,
    );
  }
  if ($node->quiz_always) {
    $stats[] = array(
      'title' => t('Available'),
      'data' => t('Always'),
    );
  }
  else {
    $stats[] = array(
      'title' => t('Opens'),
      'data' => format_date($node->quiz_open, 'short'),
    );
    $stats[] = array(
      'title' => t('Closes'),
      'data' => format_date($node->quiz_close, 'short'),
    );
  }
  if (!empty($node->pass_rate)) {
    $stats[] = array(
      'title' => t('Pass rate'),
      'data' => $node->pass_rate . ' %',
    );
  }
  if (!empty($node->time_limit)) {
    $stats[] = array(
      'title' => t('Time limit'),
      'data' => _quiz_format_duration($node->time_limit),
    );
  }
  $stats[] = array(
    'title' => t('Backwards navigation'),
    'data' => $node->backwards_navigation ? t('Allowed') : t('Forbidden'),
  );
  // Format and output the data
  $out = '<table id="quiz-view-table">' . "\n";
  foreach ($stats as $stat) {
    $out .= '<tr><td class="quiz-view-table-title"><strong>' . $stat['title'] . ':</strong></td><td class="quiz-view-table-data"><em>' . $stat['data'] . '</em></td></tr>' . "\n";
  }
  $out .= '</table>' . "\n";
  return $out;
}



