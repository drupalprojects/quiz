<?php

namespace Drupal\quiz\Entity;

use EntityDefaultUIController;

class QuizUiController extends EntityDefaultUIController {

  public function hook_menu() {
    $items = parent::hook_menu();
    $items['admin/content/quiz']['type'] = MENU_LOCAL_TASK;
    return $items;
  }

}
