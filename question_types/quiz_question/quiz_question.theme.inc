<?php
// $Id$

/*
 * @file
 * Themes for the quiz question module.
 */

/**
 * Theme the question creation form
 * 
 * @param $form
 *  Question creation form
 */
function theme_quiz_question_creation_form($form) {
  module_load_include('inc', 'quiz', 'quiz.admin');
  quiz_set_auto_title();
}