<?php

/**
 * @file quiz.api.php
 * Hooks provided by Quiz module.
 *
 * These entity types provided by Quiz also have entity API hooks. There are a
 * few additional Quiz specific hooks defined in this file.
 *
 * quiz (settings for quiz nodes)
 * quiz_result (quiz attempt/result)
 * quiz_result_answer (answer to a specific question in a quiz result)
 * quiz_question (generic settings for question nodes)
 * quiz_question_relationship (relationship from quiz to question)
 *
 * So for example
 *
 * hook_quiz_result_presave($quiz_result)
 *   - Runs before a result is saved to the DB.
 * hook_quiz_question_relationship_insert($quiz_question_relationship)
 *   - Runs when a new question is added to a quiz.
 *
 * You can also use Rules to build conditional actions based off of these
 * events.
 *
 * Enjoy :)
 */

/**
 * Implements hook_quiz_begin().
 *
 * Fired when a new quiz result is created.
 *
 * @deprecated
 *
 * Use hook_quiz_result_insert().
 */
function hook_quiz_begin($quiz, $result_id) {

}

/**
 * Implements hook_quiz_finished().
 *
 * Fired after the last question is submitted.
 *
 * @deprecated
 *
 * Use hook_quiz_result_update().
 */
function hook_quiz_finished($quiz, $score, $data) {

}

/**
 * Implements hook_quiz_scored().
 *
 * Fired when a quiz is evaluated.
 *
 * @deprecated
 *
 * Use hook_quiz_result_update().
 */
function hook_quiz_scored($quiz, $score, $result_id) {

}

/**
 * Implements hook_quiz_question_info().
 *
 * Define a new question type. The question provider must extend QuizQuestion,
 * and the response provider must extend QuizQuestionResponse. See those classes
 * for additional implementation details.
 */
function hook_quiz_question_info() {
  return array(
    'long_answer' => array(
      'name' => t('Example question type'),
      'description' => t('An example question type that does something.'),
      'question provider' => 'ExampleAnswerQuestion',
      'response provider' => 'ExampleAnswerResponse',
      'module' => 'quiz_question',
    ),
  );
}
