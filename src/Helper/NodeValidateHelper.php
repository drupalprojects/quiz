<?php

namespace Drupal\quiz\Helper;

class NodeValidateHelper {

  public function execute($node) {
    // Don't check dates if the quiz is always available.
    if (!$node->quiz_always) {
      if (mktime(0, 0, 0, $node->quiz_open['month'], $node->quiz_open['day'], $node->quiz_open['year']) > mktime(0, 0, 0, $node->quiz_close['month'], $node->quiz_close['day'], $node->quiz_close['year'])) {
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
      // @todo when we have pages of questions, we have to check that jumping is
      // not enabled, and randomization is not enabled unless there is only 1 page
      form_set_error('allow_skipping', t('If jumping is allowed skipping also has to be allowed.'));
    }
  }

}
