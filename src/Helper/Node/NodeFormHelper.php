<?php

namespace Drupal\quiz\Helper\Node;

use Drupal\quiz\Helper\FormHelper;

class NodeFormHelper extends FormHelper {

  public function execute(&$node) {
    $form = array();

    if (function_exists('userpoints_userpointsapi') && variable_get('quiz_has_userpoints', 1)) {
      $form['userpoints']['userpoints_tid'] = array(
        '#title'         => t('Userpoints Category'),
        '#type'          => 'select',
        '#options'       => $this->getUserpointsType(),
        '#default_value' => isset($node->userpoints_tid) ? $node->userpoints_tid : 0,
      );
    }

    if (quiz_has_been_answered($node) && (!user_access('manual quiz revisioning') || variable_get('quiz_auto_revisioning', 1))) {
      $node->revision = 1;
      $node->log = t('The current revision has been answered. We create a new revision so that the reports from the existing answers stays correct.');
    }

    return $form;
  }

}
