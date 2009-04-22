<?php

class quiz_views_handler_argument_user_uid_nullable extends views_handler_argument_user_uid {
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);
    unset($form['break_phrase']);
  }

  function query() {
    $group = $this->query->set_where_group('AND', 'qnr_user');
    $this->ensure_my_table();

    $operator = empty($this->options['not']) ? '=' : '!=';
    // By adding the ISNULL, joins can properly inform us about quiz state
    $this->query->add_where($group, "$this->table_alias.$this->real_field $operator %d OR ISNULL($this->table_alias.$this->real_field)", $this->argument);
  }
}