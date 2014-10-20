<?php

namespace Drupal\quiz\Entity;

use EntityDefaultViewsController;

class QuizViewsController extends EntityDefaultViewsController {

  public function views_data() {
    $data = parent::views_data();

    $data['quiz_entity']['view_node']['field'] = array(
      'title'   => t('Link'),
      'help'    => t('Provide a simple link to the !quiz.', array('!quiz' => QUIZ_NAME)),
      'handler' => 'Drupal\quiz\Entity\Views\QuizEntityLink',
    );

    $data['quiz_entity']['edit_node']['field'] = array(
      'title'   => t('Edit link'),
      'help'    => t('Provide a simple link to edit the !quiz.', array('!quiz' => QUIZ_NAME)),
      'handler' => 'Drupal\quiz\Entity\Views\QuizEntityEditLink',
    );

    $data['quiz_entity']['delete_node']['field'] = array(
      'title'   => t('Delete link'),
      'help'    => t('Provide a simple link to delete the !quiz.', array('!quiz' => QUIZ_NAME)),
      'handler' => 'Drupal\quiz\Entity\Views\QuizEntityDeleteLink',
    );

    return $data;
  }

}
