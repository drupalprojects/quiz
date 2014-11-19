<?php

/*
 * @file
 * Handles filter question status.
 */
class quiz_views_handler_filter_question_status extends views_handler_filter_in_operator {
  function get_value_options() {
    if (!isset($this->value_options)) {
      $this->value_title = t('Question Status in Quiz');
      $this->value_options = array(
        QUIZ_QUESTION_RANDOM => t('Random'), // 'Random-ly' better?
        QUIZ_QUESTION_ALWAYS => t('Always'),
        QUIZ_QUESTION_NEVER => t('Never'),
      );
    }
  }
}
