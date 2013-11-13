<?php
/**
 * @file
 * Contains \Drupal\quiz\Form\QuizAdminSettings.
 */
namespace Drupal\quiz\Form;
use Drupal\Core\Form\ConfigFormBase;

// QUIZ ADMIN

// Quiz Admin Settings

/**
 * This builds the main settings form for the quiz module.
 */
class QuizAdminSettings extends ConfigFormBase {
  /**
   * {@inheritdoc}
   */
  public function getFormID() {
    return 'quiz_admin_settings';
  }
  /**
   * {@inheritdoc}
   */
  public function buildForm(array $form, array &$form_state) {
    $form['quiz_global_settings'] = array(
      '#type' => 'fieldset',
      '#title' => t('Global Configuration'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
      '#description' => t('Control aspects of the Quiz module\'s display'),
    );

    $form['quiz_global_settings']['quiz_auto_revisioning'] = array(
      '#type' => 'checkbox',
      '#title' => t('Auto revisioning'),
      '#default_value' => \Drupal::config('quiz.settings')->get('quiz_auto_revisioning'),
      '#description' => t('It is strongly recommended that auto revisioning is always on. It makes sure that when a question or quiz is changed a new revision is created if the current revision has been answered. If this feature is switched off result reports might be broken because a users saved answer might be connected to a wrong version of the quiz and/or question she was answering. All sorts of errors might appear.'),
    );

    $form['quiz_global_settings']['quiz_durod'] = array(
      '#type' => 'checkbox',
      '#title' => t('Delete results when a user is deleted'),
      '#default_value' => \Drupal::config('quiz.settings')->get('quiz_durod'),
      '#description' => t('When a user is deleted delete any and all results for that user.'),
    );

    $form['quiz_global_settings']['quiz_index_questions'] = array(
      '#type' => 'checkbox',
      '#title' => t('Index questions'),
      '#default_value' => \Drupal::config('quiz.settings')->get('quiz_index_questions'),
      '#description' => t('If you turn this off questions will not show up in search results.'),
    );

    $form['quiz_global_settings']['quiz_default_close'] = array(
      '#type' => 'textfield',
      '#title' => t('Default number of days before a @quiz is closed', array('@quiz' => QUIZ_NAME)),
      '#default_value' => \Drupal::config('quiz.settings')->get('quiz_default_close'),
      '#size' => 4,
      '#maxlength' => 4,
      '#description' => t('Supply a number of days to calculate the default close date for new quizzes.'),
    );

    $form['quiz_global_settings']['quiz_use_passfail'] = array(
      '#type' => 'checkbox',
      '#title' => t('Allow quiz creators to set a pass/fail option when creating a @quiz.', array('@quiz' => strtolower(QUIZ_NAME))),
      '#default_value' => \Drupal::config('quiz.settings')->get('quiz_use_passfail'),
      '#description' => t('Check this to display the pass/fail options in the @quiz form. If you want to prohibit other quiz creators from changing the default pass/fail percentage, uncheck this option.', array('@quiz' => QUIZ_NAME)),
    );

    $form['quiz_global_settings']['quiz_max_result_options'] = array(
      '#type' => 'textfield',
      '#title' => t('Maximum Result Options'),
      '#description' => t('Set the maximum number of result options (categorizations for scoring a quiz). Set to 0 to disable result options.'),
      '#default_value' => \Drupal::config('quiz.settings')->get('quiz_max_result_options'),
      '#size' => 2,
      '#maxlength' => 2,
      '#required' => FALSE,
    );

    $form['quiz_global_settings']['quiz_remove_partial_quiz_record'] = array(
      '#type' => 'select',
      '#title' => t('Remove Incomplete Quiz Records (older than)'),
      '#options' => quiz_remove_partial_quiz_record_value(),
      '#description' => t('Number of days that you like to keep the incomplete quiz records'),
      '#default_value' => (\Drupal::config('quiz.settings')->get('quiz_remove_partial_quiz_record'))? : quiz_remove_partial_quiz_record_value(),
    );

    $form['quiz_global_settings']['quiz_autotitle_length'] = array(
      '#type' => 'textfield',
      '#title' => t('Length of automatically set question titles'),
      '#size' => 3,
      '#maxlength' => 3,
      '#description' => t('Integer between 0 and 128. If the question creator doesn\'t set a question title the system will make a title automatically. Here you can decide how long the autotitle can be.'),
      '#default_value' => \Drupal::config('quiz.settings')->get('quiz_autotitle_length'),
    );

    $target = array(
      'attributes' => array(
        'target' => '_blank'
      ),
    );

    $links = array(
      '!views' => l(t('Views'), 'http://drupal.org/project/views', $target),
      '!cck' => l(t('CCK'), 'http://drupal.org/project/cck', $target),
      '!jquery_countdown' => l(t('JQuery Countdown'), 'http://drupal.org/project/jquery_countdown', $target),
      '!userpoints' => l(t('UserPoints'), 'http://drupal.org/project/userpoints', $target),
      '@quiz' => QUIZ_NAME,
    );

    $form['quiz_addons'] = array(
      '#type' => 'fieldset',
      '#title' => t('Addons Configuration'),
      '#description' => t('Quiz can integrate with other d.o modules like !views, !cck, !userpoints and !jquery_countdown. Here you can configure the way Quiz integrates with other modules. Disabled checkboxes indicates that modules are not enabled/installed', $links),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );

    $form['quiz_addons']['quiz_has_userpoints'] = array(
      '#type' => 'checkbox',
      '#title' => t('Enable UserPoints Module Integration'),
      '#default_value' => \Drupal::config('quiz.settings')->get('quiz_has_userpoints'),
      '#description' => t('!userpoints is an <strong>optional</strong> module for Quiz. It provides ways for users to gain or lose points for performing certain actions on your site like attending @quiz. You will need to install the !userpoints module to use this feature.', $links),
      '#disabled' => !module_exists('userpoints'),
    );

    $form['quiz_addons']['quiz_has_timer'] = array(
      '#type' => 'checkbox',
      '#title' => t('Display Timer for Timed Quiz'),
      '#default_value' => \Drupal::config('quiz.settings')->get('quiz_has_timer'),
      '#description' => t("!jquery_countdown is an <strong>optional</strong> module for Quiz. It is used to display a timer when taking a quiz. Without this timer, the user will not know how long he or she has left to complete the @quiz", $links),
      '#disabled' => !function_exists('jquery_countdown_add'),
    );

    $form['quiz_look_feel'] = array(
      '#type' => 'fieldset',
      '#title' => t('Look and Feel Settings'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
      '#description' => t('Control aspects of the Quiz module\'s display'),
    );

    $form['quiz_look_feel']['quiz_name'] = array(
      '#type' => 'textfield',
      '#title' => t('Display name'),
      '#default_value' => QUIZ_NAME,
      '#description' => t('Change the name of the quiz type. Do you call it <em>test</em> or <em>assessment</em> instead? Change the display name of the module to something else. Currently, it is called @quiz. By default, it is called <em>Quiz</em>.',
        array('@quiz' => QUIZ_NAME)),
      '#required' => TRUE,
    );

    $form['quiz_email_settings'] = array(
      '#type' => 'fieldset',
      '#title' => t('Email Settings'),
      '#description' => t('Send results to quiz author/attendee via e-mail. Configure e-mail subject and body.'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );

    $form['quiz_email_settings']['taker'] = array(
      '#type' => 'fieldset',
      '#title' => t('E-mail for Quiz Takers'),
      '#collapsible' => FALSE,
    );

    $form['quiz_email_settings']['taker']['quiz_email_results'] = array(
      '#type' => 'checkbox',
      '#title' => t('E-mail results to quiz takers'),
      '#default_value' => \Drupal::config('quiz.settings')->get('quiz_email_results'),
      '#description' => t('Check this to send users their results at the end of a quiz.')
    );

    $form['quiz_email_settings']['taker']['quiz_email_results_subject_taker'] = array(
      '#type' => 'textfield',
      '#title' => t('Configure E-mail Subject'),
      '#description' => t('This format will be used when sending quiz results at the end of a quiz.'),
      '#default_value' => (\Drupal::config('quiz.settings')->get('quiz_email_results_subject_taker')) ? : quiz_email_results_format('subject', 'taker'),
    );

    $form['quiz_email_settings']['taker']['quiz_email_results_body_taker'] = array(
      '#type' => 'textarea',
      '#title' => t('Configure E-mail Format'),
      '#description' => t('This format will be used when sending quiz results at the end of a quiz. !title(quiz title), !sitename, !taker(quiz takers username), !date(time when quiz was finished), !minutes(How many minutes the quiz taker spent taking the quiz), !desc(description of the quiz), !correct(points attained), !total(max score for the quiz), !percentage(percentage score), !url(url to the result page) and !author are placeholders.'),
      '#default_value' => (\Drupal::config('quiz.settings')->get('quiz_email_results_body_taker')) ? : quiz_email_results_format('body', 'taker'),
    );

    $form['quiz_email_settings']['author'] = array(
      '#type' => 'fieldset',
      '#title' => t('E-mail for Quiz Authors'),
      '#collapsible' => FALSE,
    );

    $form['quiz_email_settings']['author']['quiz_results_to_quiz_author'] = array(
      '#type' => 'checkbox',
      '#title' => t('E-mail all results to quiz author.'),
      '#default_value' => \Drupal::config('quiz.settings')->get('quiz_results_to_quiz_author'),
      '#description' => t('Check this to send quiz results for all users to the quiz author.'),
    );

    $form['quiz_email_settings']['author']['quiz_email_results_subject'] = array(
      '#type' => 'textfield',
      '#title' => t('Configure E-mail Subject'),
      '#description' => t('This format will be used when sending quiz results at the end of a quiz. Authors and quiz takers gets the same format.'),
      '#default_value' => (\Drupal::config('quiz.settings')->get('quiz_email_results_subject')) ? : quiz_email_results_format('subject', 'author'),
    );

    $form['quiz_email_settings']['author']['quiz_email_results_body'] = array(
      '#type' => 'textarea',
      '#title' => t('Configure E-mail Format'),
      '#description' => t('This format will be used when sending quiz results at the end of a quiz. !title(quiz title), !sitename, !taker(quiz takers username), !date(time when quiz was finished), !minutes(How many minutes the quiz taker spent taking the quiz), !desc(description of the quiz), !correct(points attained), !total(max score for the quiz), !percentage(percentage score), !url(url to the result page) and !author are placeholders.'),
      '#default_value' => (\Drupal::config('quiz.settings')->get('quiz_email_results_body')) ? : quiz_email_results_format('body', 'author'),
    );

    $form['def_settings_link'] = array(
      '#markup' => '<p>' . t('Default values for the quiz creation form can be edited <a href="!url">here</a>', array('!url' => url('admin/config/quiz/quiz_form'))) . '</p>',
    );

    return parent::buildForm($form, $form_state);
  }

  /**
   * Validation of the Form Settings form.
   *
   * Checks the values for the form administration form for quiz settings.
   */
  public function validateForm(array &$form, array &$form_state) {
    if (!_quiz_is_int($form_state['values']['quiz_default_close'])) {
      form_set_error('quiz_default_close', t('The default number of days before a quiz is closed must be a number greater than 0.'));
    }

    if (!_quiz_is_int($form_state['values']['quiz_autotitle_length'], 0, 128)) {
      form_set_error('quiz_autotitle_length', t('The autotitle length value must be an integer between 0 and 128.'));
    }

    if (!_quiz_is_int($form_state['values']['quiz_max_result_options'], 0, 100)) {
      form_set_error('quiz_max_result_options', t('The number of resultoptions must be an integer between 0 and 100.'));
    }

    if (!_quiz_is_plain($form_state['values']['quiz_name'])) {
      form_set_error('quiz_name', t('The quiz name must be plain text.'));
    }

    /*if (!_quiz_is_plain($form_state['values']['quiz_action_type']))
     form_set_error('quiz_action_type', t('The action type must be plain text.'));*/
  }

  /**
   * Submit the admin settings form
   */
  public function submitForm(array &$form, array &$form_state) {
    define(QUIZ_NAME, $form_state['values']['quiz_name']);

    $config = \Drupal::config('quiz.settings');
    $config->set('quiz_name', $form_state['values']['quiz_name'])
      ->set('quiz_auto_revisioning', $form_state['values']['quiz_auto_revisioning'])
      ->set('quiz_durod', $form_state['values']['quiz_durod'])
      ->set('quiz_index_questions', $form_state['values']['quiz_index_questions'])
      ->set('quiz_default_close', $form_state['values']['quiz_default_close'])
      ->set('quiz_use_passfail', $form_state['values']['quiz_use_passfail'])
      ->set('quiz_max_result_options', $form_state['values']['quiz_max_result_options'])
      ->set('quiz_remove_partial_quiz_record', $form_state['values']['quiz_remove_partial_quiz_record'])
      ->set('quiz_autotitle_length', $form_state['values']['quiz_autotitle_length'])
      ->set('quiz_has_userpoints', $form_state['values']['quiz_has_userpoints'])
      ->set('quiz_has_timer', $form_state['values']['quiz_has_timer'])
      ->set('quiz_email_results', $form_state['values']['quiz_email_results'])
      ->set('quiz_email_results_subject_taker', $form_state['values']['quiz_email_results_subject_taker'])
      ->set('quiz_email_results_body_taker', $form_state['values']['quiz_email_results_body_taker'])
      ->set('quiz_results_to_quiz_author', $form_state['values']['quiz_results_to_quiz_author'])
      ->set('quiz_email_results_subject', $form_state['values']['quiz_email_results_subject'])
      ->set('quiz_email_results_body', $form_state['values']['quiz_email_results_body'])
      ->set('quiz_email_results_body', $form_state['values']['quiz_email_results_body'])
      ->save();

    menu_cache_clear_all(); // Equivalent to D7 menu_rebuild();

    parent::submitForm($form, $form_state);
  }
}
?>