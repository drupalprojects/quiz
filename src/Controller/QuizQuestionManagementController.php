<?php

namespace Drupal\quiz\Controller;

class QuizQuestionManagementController {

  /**
   * Callback for node/%quiz_menu/quiz/questions (question management tab).
   * Creates a form for quiz questions.
   *
   * @param $quiz
   *   The quiz node we are managing questions for.
   * @return
   *   String containing the form.
   */
  public static function staticCallback($quiz) {
    drupal_set_title($quiz->title);

    if ($quiz->randomization >= 3) {
      return drupal_get_form('quiz_categorized_form', $quiz);
    }

    $mq_form = drupal_get_form('quiz_questions_form', $quiz);
    $manage_questions = drupal_render($mq_form);
    $question_bank = views_get_view('quiz_question_bank')->preview();

    // Insert into vert tabs
    $form['vert_tabs'] = array(
      '#type' => 'vertical_tabs',
      '#weight' => 0,
    );
    $form['vert_tabs']['question_admin'] = array(
      '#type' => 'fieldset',
      '#title' => t('Manage questions'),
      '#value' => $manage_questions,
    );
    $form['vert_tabs']['global_questions'] = array(
      '#type' => 'fieldset',
      '#title' => t('Question bank'),
      '#value' => $question_bank,
    );

    return $form;
  }

}
