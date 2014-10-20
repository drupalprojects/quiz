<?php

namespace Drupal\quiz\Controller;

class QuizEntityViewController {

  /**
   * Callback for node/%quiz_menu/take
   */
  public static function staticCallback($quiz, $view_mode = 'default', $langcode = NULL) {
    return entity_view('quiz_entity', array($quiz), $view_mode, $langcode);
  }

}
