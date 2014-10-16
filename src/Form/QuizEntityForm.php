<?php

namespace Drupal\quiz\Form;

use Drupal\quiz\Entity\QuizEntity;
use Drupal\quiz\Helper\FormHelper;

class QuizEntityForm extends FormHelper {

  /** @var QuizEntity */
  private $quiz;

  public function __construct($quiz) {
    $this->quiz = $quiz;
  }

  /**
   * Main endpoint to get structure for quiz entity editing form.
   *
   * @param array $form
   * @param array $form_state
   * @param string $op
   * @return array
   */
  public function get($form, &$form_state, $op) {
    $form['title'] = array(
      '#type'          => 'textfield',
      '#title'         => t('Title'),
      '#default_value' => isset($this->quiz->title) ? $this->quiz->title : '',
      '#description'   => t('The name of this @quiz.', array('@quiz' => QUIZ_NAME)),
      '#required'      => TRUE,
      '#weight'        => -20,
    );

    $form['vtabs'] = array('#type' => 'vertical_tabs');

    $this->defineTakingOptions($form);
    $this->defineAvailabilityOptionsFields($form);
    $this->definePassFailOptionsFields($form);
    $this->defineResultFeedbackFields($form);
    $this->defineRememberConfigOptionsFields($form);
    $this->defineRevisionOptionsFields($form);

    $form['actions'] = array('#type' => 'action');
    $form['actions']['submit'] = array('#type' => 'submit', '#value' => t('Save'));
    $form['#validate'][] = array($this, 'validate');
    $form['#submit'][] = array($this, 'submit');

    return $form;
  }

  private function defineTakingOptions(&$form) {
    $form['taking'] = array(
      '#type'        => 'fieldset',
      '#title'       => t('Taking options'),
      '#collapsed'   => isset($settings_loaded) ? $settings_loaded : FALSE, // @todo: Why check non-existent var?
      '#collapsible' => TRUE,
      '#attributes'  => array('id' => 'taking-fieldset'),
      '#group'       => 'vtabs',
      '#weight'      => -2,
    );
    $form['taking']['allow_resume'] = array(
      '#type'          => 'checkbox',
      '#title'         => t('Allow resume'),
      '#default_value' => $this->quiz->allow_resume,
      '#description'   => t('Allow users to leave this @quiz incomplete and then resume it from where they left off.', array('@quiz' => QUIZ_NAME)),
    );
    $form['taking']['allow_skipping'] = array(
      '#type'          => 'checkbox',
      '#title'         => t('Allow skipping'),
      '#default_value' => $this->quiz->allow_skipping,
      '#description'   => t('Allow users to skip questions in this @quiz.', array('@quiz' => QUIZ_NAME)),
    );
    $form['taking']['allow_jumping'] = array(
      '#type'          => 'checkbox',
      '#title'         => t('Allow jumping'),
      '#default_value' => $this->quiz->allow_jumping,
      '#description'   => t('Allow users to jump to any question using a menu or pager in this @quiz.', array('@quiz' => QUIZ_NAME)),
    );
    $form['taking']['allow_change'] = array(
      '#type'          => 'checkbox',
      '#title'         => t('Allow changing answers'),
      // https://www.drupal.org/node/2354355#comment-9241781
      '#default_value' => isset($this->quiz->allow_change) ? $this->quiz->allow_change : 1,
      '#description'   => t('If the user is able to visit a previous question, allow them to change the answer.'),
    );
    $form['taking']['backwards_navigation'] = array(
      '#type'          => 'checkbox',
      '#title'         => t('Backwards navigation'),
      '#default_value' => $this->quiz->backwards_navigation,
      '#description'   => t('Allow users to go back and revisit questions already answered.'),
    );
    $form['taking']['repeat_until_correct'] = array(
      '#type'          => 'checkbox',
      '#title'         => t('Repeat until correct'),
      '#default_value' => $this->quiz->repeat_until_correct,
      '#description'   => t('Require the user to retry the question until answered correctly.'),
    );
    $form['taking']['mark_doubtful'] = array(
      '#type'          => 'checkbox',
      '#title'         => t('Mark doubtful'),
      '#default_value' => $this->quiz->mark_doubtful,
      '#description'   => t('Allow users to mark their answers as doubtful.'),
    );
    $form['taking']['show_passed'] = array(
      '#type'          => 'checkbox',
      '#title'         => t('Show passed status'),
      '#default_value' => $this->quiz->show_passed,
      '#description'   => t('Show a message if the user has previously passed the @quiz.', array('@quiz' => QUIZ_NAME)),
    );

    $form['taking']['randomization'] = array(
      '#type'          => 'radios',
      '#title'         => t('Randomize questions'),
      '#options'       => array(
        t('No randomization'),
        t('Random order'),
        t('Random questions'),
        t('Categorized random questions'),
      ),
      '#description'   => t('<strong>Random order</strong> - all questions display in random order')
      . '<br/>' . t("<strong>Random questions</strong> - specific number of questions are drawn randomly from this Quiz's pool of questions")
      . '<br/>' . t('<strong>Categorized random questions</strong> - specific number of questions are drawn from each specified taxonomy term'),
      '#default_value' => $this->quiz->randomization,
    );
    $form['taking']['review_options'] = array(
      '#type'        => 'fieldset',
      '#title'       => t('Review options'),
      '#collapsible' => FALSE,
      '#collapsed'   => FALSE,
      '#tree'        => TRUE,
    );

    $review_options = quiz()->getQuizHelper()->getFeedbackHelper()->getOptions();
    foreach (array('question' => 'After the question', 'end' => 'After the quiz') as $key => $when) {
      $form['taking']['review_options'][$key] = array(
        '#title'         => $when,
        '#type'          => 'checkboxes',
        '#options'       => $review_options,
        '#default_value' => isset($this->quiz->review_options[$key]) ? $this->quiz->review_options[$key] : array(),
      );
    }

    $form['taking']['multiple_takes'] = array(
      '#type'        => 'fieldset',
      '#title'       => t('Multiple takes'),
      '#collapsible' => FALSE,
      '#collapsed'   => FALSE,
      '#attributes'  => array('id' => 'multiple-takes-fieldset'),
      '#description' => t('Allow users to take this quiz multiple times.'),
    );
    $form['taking']['multiple_takes']['takes'] = array(
      '#type'          => 'select',
      '#title'         => t('Allowed number of attempts'),
      '#default_value' => $this->quiz->takes,
      '#options'       => array(t('Unlimited')) + range(0, 10),
      '#description'   => t('The number of times a user is allowed to take this @quiz. <strong>Anonymous users are only allowed to take quizzes that allow an unlimited number of attempts.</strong>', array('@quiz' => QUIZ_NAME)),
    );
    $form['taking']['multiple_takes']['show_attempt_stats'] = array(
      '#type'          => 'checkbox',
      '#title'         => t('Display allowed number of attempts'),
      '#default_value' => $this->quiz->show_attempt_stats,
      '#description'   => t('Display the allowed number of attempts on the starting page for this quiz.'),
    );

    if (user_access('delete any quiz results') || user_access('delete results for own quiz')) {
      $form['taking']['multiple_takes']['keep_results'] = array(
        '#type'          => 'radios',
        '#title'         => t('Store results'),
        '#description'   => t('These results should be stored for each user.'),
        '#options'       => array(t('The best'), t('The newest'), t('All')),
        '#default_value' => $this->quiz->keep_results,
      );
    }
    else {
      $form['taking']['multiple_takes']['keep_results'] = array(
        '#type'  => 'value',
        '#value' => $this->quiz->keep_results,
      );
    }

    if (function_exists('jquery_countdown_add') && variable_get('quiz_has_timer', 0)) {
      $form['taking']['addons'] = array(
        '#type'        => 'fieldset',
        '#title'       => t('Quiz add-ons'),
        '#collapsible' => TRUE,
        '#collapsed'   => FALSE,
      );
      $form['taking']['addons']['time_limit'] = array(
        '#type'          => 'textfield',
        '#title'         => t('Time limit'),
        '#default_value' => isset($this->quiz->time_limit) ? $this->quiz->time_limit : 0,
        '#description'   => t('Set the maximum allowed time in seconds for this @quiz. Use 0 for no limit.', array('@quiz' => QUIZ_NAME)),
      );
    }
    else {
      $form['taking']['addons']['time_limit'] = array('#type' => 'value', '#value' => 0);
    }

    if (function_exists('userpoints_userpointsapi') && variable_get('quiz_has_userpoints', 1)) {
      $form['userpoints'] = array(
        '#type'        => 'fieldset',
        '#title'       => t('Userpoints'),
        '#collapsible' => TRUE,
        '#collapsed'   => FALSE,
        '#group'       => 'vtabs',
      );
      $form['userpoints']['has_userpoints'] = array(
        '#type'          => 'checkbox',
        '#default_value' => (isset($this->quiz->has_userpoints) ? $this->quiz->has_userpoints : 1),
        '#title'         => t('Enable UserPoints Module Integration'),
        '#description'   => t('If checked, marks scored in this @quiz will be credited to userpoints. For each correct answer 1 point will be added to user\'s point.', array('@quiz' => QUIZ_NAME)),
      );
      $form['userpoints']['userpoints_tid'] = array(
        '#type'          => 'select',
        '#options'       => $this->getUserpointsType(),
        '#title'         => t('Userpoints Category'),
        '#states'        => array(
          'visible' => array(':input[name=has_userpoints]' => array('checked' => TRUE)),
        ),
        '#default_value' => isset($this->quiz->userpoints_tid) ? $this->quiz->userpoints_tid : 0,
        '#description'   => t('Select the category to which user points to be added. To add new category see <a href="!url">admin/structure/taxonomy/userpoints</a>', array('!url' => url('admin/structure/taxonomy/userpoints'))),
      );
    }
  }

  /**
   * Set up the availability options.
   */
  private function defineAvailabilityOptionsFields(&$form) {
    $form['quiz_availability'] = array(
      '#type'        => 'fieldset',
      '#title'       => t('Availability options'),
      '#collapsed'   => TRUE,
      '#collapsible' => TRUE,
      '#attributes'  => array('id' => 'availability-fieldset'),
      '#group'       => 'vtabs',
    );
    $form['quiz_availability']['quiz_always'] = array(
      '#type'          => 'checkbox',
      '#title'         => t('Always available'),
      '#default_value' => $this->quiz->quiz_always,
      '#description'   => t('Ignore the open and close dates.'),
    );

    $form['quiz_availability']['quiz_open'] = array(
      '#type'          => 'date',
      '#title'         => t('Open date'),
      '#default_value' => $this->prepareDate($this->quiz->quiz_open),
      '#description'   => t('The date this @quiz will become available.', array('@quiz' => QUIZ_NAME)),
    );
    $form['quiz_availability']['quiz_close'] = array(
      '#type'          => 'date',
      '#title'         => t('Close date'),
      '#default_value' => $this->prepareDate($this->quiz->quiz_close, variable_get('quiz_default_close', 30)),
      '#description'   => t('The date this @quiz will become unavailable.', array('@quiz' => QUIZ_NAME)),
    );

    // Limit the year options to the years 1970 - 2030 for form items of type date.
    // Some systems don't support all the dates the forms api lets you choose from.
    // This function limits the options to dates most systems support.
    $form['quiz_availability']['quiz_open']['#after_build'] = $form['quiz_availability']['quiz_close']['#after_build'] = function ($form_element) {
      $form_element['year']['#options'] = drupal_map_assoc(range(1970, 2030));
      return $form_element;
    };
  }

  private function definePassFailOptionsFields(&$form) {
    // Quiz summary options.
    $form['summaryoptions'] = array(
      '#type'        => 'fieldset',
      '#title'       => t('Pass/fail options'),
      '#collapsible' => TRUE,
      '#collapsed'   => TRUE,
      '#attributes'  => array('id' => 'summaryoptions-fieldset'),
      '#group'       => 'vtabs',
    );
    // If pass/fail option is checked, present the form elements.
    if (variable_get('quiz_use_passfail', 1)) {
      $form['summaryoptions']['pass_rate'] = array(
        '#type'          => 'textfield',
        '#title'         => t('Passing rate for @quiz (%)', array('@quiz' => QUIZ_NAME)),
        '#default_value' => $this->quiz->pass_rate,
        '#description'   => t('Passing rate for this @quiz as a percentage score.', array('@quiz' => QUIZ_NAME)),
        '#required'      => FALSE,
      );
      $form['summaryoptions']['summary_pass'] = array(
        '#type'          => 'text_format',
        '#base_type'     => 'textarea',
        '#title'         => t('Summary text if passed'),
        '#default_value' => $this->quiz->summary_pass,
        '#cols'          => 60,
        '#description'   => t("Summary text for when the user passes the @quiz. Leave blank to not give different summary text if passed, or if not using the \"percent to pass\" option above. If not using the \"percentage needed to pass\" field above, this text will not be used.", array('@quiz' => QUIZ_NAME)),
        '#format'        => isset($this->quiz->summary_pass_format) && !empty($this->quiz->summary_pass_format) ? $this->quiz->summary_pass_format : NULL,
      );
    }
    // If the pass/fail option is unchecked, use the default and hide it.
    else {
      $form['summaryoptions']['pass_rate'] = array(
        '#type'     => 'hidden',
        '#value'    => $this->quiz->pass_rate,
        '#required' => FALSE,
      );
    }
    // We use a helper to enable the wysiwyg module to add an editor to the
    // textarea.
    $form['summaryoptions']['helper']['summary_default'] = array(
      '#type'          => 'text_format',
      '#base_type'     => 'textarea',
      '#title'         => t('Default summary text'),
      '#default_value' => $this->quiz->summary_default,
      '#cols'          => 60,
      '#description'   => t("Default summary. Leave blank if you don't want to give a summary."),
      '#format'        => isset($this->quiz->summary_default_format) && !empty($this->quiz->summary_default_format) ? $this->quiz->summary_default_format : NULL,
    );

    // Number of random questions, max score and tid for random questions are set on
    // the manage questions tab. We repeat them here so that they're not removed
    // if the quiz is being updated.
    $num_rand = (isset($this->quiz->number_of_random_questions)) ? $this->quiz->number_of_random_questions : 0;
    $form['number_of_random_questions'] = array(
      '#type'  => 'value',
      '#value' => $num_rand,
    );
    $max_score_for_random = (isset($this->quiz->max_score_for_random)) ? $this->quiz->max_score_for_random : 0;
    $form['max_score_for_random'] = array(
      '#type'  => 'value',
      '#value' => $max_score_for_random,
    );
    $tid = (isset($this->quiz->tid)) ? $this->quiz->tid : 0;
    $form['tid'] = array(
      '#type'  => 'value',
      '#value' => $tid,
    );
  }

  private function defineResultFeedbackFields(&$form) {
    $options = !empty($this->quiz->resultoptions) ? $this->quiz->resultoptions : array();
    $num_options = max(count($options), variable_get('quiz_max_result_options', 5));

    if ($num_options > 0) {
      $form['resultoptions'] = array(
        '#type'        => 'fieldset',
        '#title'       => t('Result feedback'),
        '#collapsible' => TRUE,
        '#collapsed'   => TRUE,
        '#tree'        => TRUE,
        '#attributes'  => array('id' => 'resultoptions-fieldset'),
        '#group'       => 'vtabs',
      );

      for ($i = 0; $i < $num_options; $i++) {
        $option = (count($options) > 0) ? array_shift($options) : NULL; // grab each option in the array
        $form['resultoptions'][$i] = array(
          '#type'        => 'fieldset',
          '#title'       => t('Result Option ') . ($i + 1),
          '#collapsible' => TRUE,
          '#collapsed'   => FALSE,
        );
        $form['resultoptions'][$i]['option_name'] = array(
          '#type'          => 'textfield',
          '#title'         => t('Range title'),
          '#default_value' => isset($option['option_name']) ? $option['option_name'] : '',
          '#maxlength'     => 40,
          '#size'          => 40,
          '#description'   => 'e.g., "A" or "Passed"',
        );
        $form['resultoptions'][$i]['option_start'] = array(
          '#type'          => 'textfield',
          '#title'         => t('Percentage low'),
          '#description'   => t('Show this result for scored quizzes in this range (0-100).'),
          '#default_value' => isset($option['option_start']) ? $option['option_start'] : '',
          '#size'          => 5,
        );
        $form['resultoptions'][$i]['option_end'] = array(
          '#type'          => 'textfield',
          '#title'         => t('Percentage high'),
          '#description'   => t('Show this result for scored quizzes in this range (0-100).'),
          '#default_value' => isset($option['option_end']) ? $option['option_end'] : '',
          '#size'          => 5,
        );
        $form['resultoptions'][$i]['option_summary'] = array(
          '#type'          => 'text_format',
          '#base_type'     => 'textarea',
          '#title'         => t('Feedback'),
          '#default_value' => isset($option['option_summary']) ? $option['option_summary'] : '',
          '#description'   => t("This is the text that will be displayed when the user's score falls in this range."),
          '#format'        => isset($option['option_summary_format']) ? $option['option_summary_format'] : NULL,
        );
        if (isset($option['option_id'])) {
          $form['resultoptions'][$i]['option_id'] = array(
            '#type'  => 'hidden',
            '#value' => isset($option['option_id']) ? $option['option_id'] : '',
          );
        }
      }
    }
  }

  private function defineRememberConfigOptionsFields(&$form) {
    $form['remember_settings'] = array(
      '#type'        => 'checkbox',
      '#title'       => t('Remember my settings'),
      '#description' => t('If this box is checked most of the quiz specific settings you have made will be remembered and will be everyone\'s default settings next time they create a quiz.'),
      '#weight'      => -15,
    );

    $form['remember_global'] = array(
      '#type'        => 'checkbox',
      '#title'       => t('Remember as global'),
      '#description' => t('If this box is checked most of the quiz specific settings you have made will be remembered and will be everyone\'s default settings next time you create a quiz.'),
      '#weight'      => -15,
      '#access'      => user_access('administer quiz configuration'),
    );

    if (quiz_has_been_answered($this->quiz) && (!user_access('manual quiz revisioning') || variable_get('quiz_auto_revisioning', 1))) {
      $this->quiz->revision = 1;
      $this->quiz->log = t('The current revision has been answered. We create a new revision so that the reports from the existing answers stays correct.');
    }
  }

  private function defineRevisionOptionsFields(&$form) {
    $form['revision_information'] = array(
      '#type'        => 'fieldset',
      '#title'       => t('Revision information'),
      '#collapsible' => TRUE,
      '#collapsed'   => TRUE,
      '#group'       => 'vtabs',
      '#attributes'  => array('class' => array('node-form-revision-information')),
      '#attached'    => array('js' => array(drupal_get_path('module', 'node') . '/node.js')),
      '#weight'      => 20,
      '#access'      => TRUE,
    );

    $form['revision_information']['revision'] = array(
      '#type'          => 'checkbox',
      '#title'         => t('Create new revision'),
      '#default_value' => FALSE,
      '#state'         => array('checked' => array('textarea[name="log"]' => array('empty' => FALSE))),
    );

    $form['revision_information']['log'] = array(
      '#type'          => 'textarea',
      '#title'         => t('Revision log message'),
      '#row'           => 4,
      '#default_value' => '',
      '#description'   => t('Provide an explanation of the changes you are making. This will help other authors understand your motivations.'),
    );

    if (variable_get('quiz_auto_revisioning', 1) || !user_access('manual quiz revisioning')) {
      $form['revision_information']['revision']['#type'] = 'value';
      $form['revision_information']['revision']['#value'] = $form['revision_information']['revision']['#default_value'];
      $form['revision_information']['log']['#type'] = 'value';
      $form['revision_information']['log']['#value'] = $form['revision_information']['log']['#default_value'];
      $form['revision_information']['#access'] = FALSE;
    }
  }

  public function validate($form, &$form_state) {
    form_set_error('title', 'working…');
  }

  public function submit($form, &$form_state) {
    $values = &$form_state['values'];
  }

}
