<?php

namespace Drupal\quiz\Helper\HookImplementation;

class HookFieldExtraFields {

  public function execute() {
    $extra['node']['quiz'] = array(
      'display' => array(
        'take'  => array(
          'label'       => t('Take quiz button'),
          'description' => t('The take button.'),
          'weight'      => 10,
        ),
        'stats' => array(
          'label'       => t('Quiz summary'),
          'description' => t('Quiz summary'),
          'weight'      => 9,
        ),
      ),
      'form'    => array(
        'taking'            => array(
          'label'       => t('Taking options'),
          'description' => t('Fieldset for customizing how a quiz is taken'),
          'weight'      => 0,
        ),
        'quiz_availability' => array(
          'label'       => t('Availability options'),
          'description' => t('Fieldset for customizing when a quiz is available'),
          'weight'      => 0,
        ),
        'summaryoptions'    => array(
          'label'       => t('Summary options'),
          'description' => t('Fieldset for customizing summaries in the quiz reports'),
          'weight'      => 0,
        ),
        'resultoptions'     => array(
          'label'       => t('Result options'),
          'description' => t('Fieldset for customizing result comments in quiz reports'),
          'weight'      => 0,
        ),
        'remember_settings' => array(
          'label'       => t('Remember settings'),
          'description' => t('Checkbox for remembering quiz settings'),
          'weight'      => 0,
        ),
        'remember_global'   => array(
          'label'       => t('Remember as global'),
          'description' => t('Checkbox for remembering quiz settings'),
          'weight'      => 0,
        ),
      ),
    );

    if ($types = quiz_get_types()) {
      foreach (array_keys($types) as $name) {
        $extra['quiz_entity'][$name] = $extra['node']['quiz'];
      }
    }

    return $extra;
  }

}
