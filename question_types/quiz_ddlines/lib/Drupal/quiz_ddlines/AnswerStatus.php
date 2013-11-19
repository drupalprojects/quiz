<?php


namespace Drupal\quiz_ddlines;

/**
 * Status for each alternative as "enum"
 */

class AnswerStatus {
  const NO_ANSWER = 0;
  const WRONG = 1;
  const CORRECT = 2;

  private static $titles;
  private static $css_class;

  public static function init() {
    self::$titles = array(
    t('You did not move this alternative to any hotspot'),
    t('Wrong answer'),
    t('Correct answer'));

    self::$css_class = array('no-answer','wrong','correct');
  }

  public static function getTitle($status) {
   return self::$titles[$status];
  }

  public static function getCssClass($status) {
    return self::$css_class[$status];
  }
}

// @todo: verify once.
AnswerStatus::init();
