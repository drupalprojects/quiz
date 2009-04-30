<?php
// $Id$

class quiz_views_handler_filter_user_quiz_state extends views_handler_filter {
  var $states = array();
  function init(&$view, $options) {
    parent::init($view, $options);
    $this->states = array(
      'not_started' => t('Not Started'),
      'in_progress' => t('In Progress'),
      'finished' => t('Finished'),
    );
  }

  function options_definition() {
    $options = parent::option_definition();
    $options['quiz_state'] = array('default' => 'finished');
    return $options;
  }

  function options_form(&$form, &$form_state) {
    $form['quiz_state'] = array(
      '#type' => 'radios',
      '#description' => t('The final result set will only include quiz results in the state you select here.'),
      '#title' => t('Quiz State'),
      '#options' => $this->states,
      '#default_value' => $this->options['quiz_state'],
    );
  }

  function admin_summary() {
    return 'is ' . $this->states[$this->options['quiz_state']];
  }

  function query() {
    $this->ensure_my_table();
    if ($this->options['quiz_state'] == 'not_started') {
      $this->query->add_where(0, "$this->table_alias.time_end IS NULL");
    }
    else {
      // $field = $this->query->add_field($table_alias, "time_end");
      $operator = $this->options['quiz_state'] == 'in_progress' ? '=' : '>';
      // $this->query->add_where(0, "$table_alias.$field $operator 0");
      $this->query->add_where(0, "$this->table_alias.time_end $operator 0");
    }
  }
}
