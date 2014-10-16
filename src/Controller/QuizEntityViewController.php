<?php

namespace Drupal\quiz\Controller;

use Drupal\quiz\Entity\QuizEntity;

class QuizEntityViewController {

  /** @var QuizEntity */
  private $quiz;

  /** @var string */
  private $view_mode;

  /** @var string */
  private $langCode;

  public function __construct(QuizEntity $quiz, $view_mode = 'default', $langcode = NULL) {
    $this->quiz = $quiz;
    $this->view_mode = $view_mode;
  }

  /**
   * Callback for node/%quiz_menu/take
   */
  public static function staticCallback($quiz, $view_mode = 'default', $langcode = NULL) {
    $controller = new static($quiz, $view_mode, $langcode);
    return $controller->render();
  }

  public function render() {
    return '…';
  }

}
