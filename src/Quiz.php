<?php

namespace Drupal\quiz;

use Drupal\quiz\Helper\MailHelper;
use Drupal\quiz\Helper\NodeHelper;
use Drupal\quiz\Helper\QuizHelper;

/**
 * Wrapper for helper classes. We just use classes to organise functions, make
 * them easier to access, able to override, there is no OOP in helper classes
 * yet.
 *
 * Quiz.nodeHelper — Helper for node-hook implementations.
 * Quiz.quizHelper — Helper for quiz node/object.
 * Quiz.mailHelper — Build/format email messages.
 * Quiz.quizHelper.settingHelper - Get/Set/… quiz settings.
 * Quiz.quizHelper.resultHelper — Helper methods for quiz's results.
 * Quiz.quizHelper.accessHelper — Access helpers
 * Quiz.quizHelper.feedbackHelper — Helper methods for quiz's feedback.
 *
 * Extends this class and sub classes if you would like override things.
 *
 * You should not create object directly from this class, use quiz() factory
 * function instead — which support overriding from module's side.
 */
class Quiz {

  private $nodeHelper;
  private $quizHelper;
  private $mailHelper;

  /**
   * @return NodeHelper
   */
  public function getNodeHelper() {
    if (null === $this->nodeHelper) {
      $this->nodeHelper = new NodeHelper();
    }
    return $this->nodeHelper;
  }

  /**
   * Inject node helper.
   *
   * @param NodeHelper $nodeHelper
   * @return \Drupal\quiz\Quiz
   */
  public function setNodeHelper($nodeHelper) {
    $this->nodeHelper = $nodeHelper;
    return $this;
  }

  /**
   * @return QuizHelper
   */
  public function getQuizHelper() {
    if (null === $this->quizHelper) {
      $this->quizHelper = new QuizHelper();
    }
    return $this->quizHelper;
  }

  /**
   * Inject quizHelper.
   *
   * @param QuizHelper $quizHelper
   * @return \Drupal\quiz\Quiz
   */
  public function setQuizHelper($quizHelper) {
    $this->quizHelper = $quizHelper;
    return $this;
  }

  /**
   * @return MailHelper
   */
  public function getMailHelper() {
    if (null === $this->mailHelper) {
      $this->mailHelper = new MailHelper();
    }
    return $this->mailHelper;
  }

  /**
   * Inject mail helper.
   *
   * @param MailHelper $mailHelper
   * @return \Drupal\quiz\Quiz
   */
  public function setMailHelper($mailHelper) {
    $this->mailHelper = $mailHelper;
    return $this;
  }

  /**
   * Format a number of seconds to a hh:mm:ss format.
   *
   * @param $time_in_sec
   *   Integers time in seconds.
   *
   * @return
   *   String time in min : sec format.
   */
  function formatDuration($time_in_sec) {
    $hours = intval($time_in_sec / 3600);
    $min = intval(($time_in_sec - $hours * 3600) / 60);
    $sec = $time_in_sec % 60;
    if (strlen($min) == 1) {
      $min = '0' . $min;
    }
    if (strlen($sec) == 1) {
      $sec = '0' . $sec;
    }
    return "$hours:$min:$sec";
  }

  /**
   * Retrieve list of vocabularies for all quiz question types.
   *
   * @return
   *   An array containing a vocabulary list.
   */
  function getVocabularies() {
    $vocabularies = array();
    $types = array_keys(_quiz_get_question_types());
    foreach ($types as $type) {
      foreach (taxonomy_get_vocabularies($type) as $vid => $vocabulary) {
        $vocabularies[$vid] = $vocabulary;
      }
    }
    return $vocabularies;
  }

  /**
   * Modifies the format fieldset.
   *
   * Adds a class to all the format fieldsets and removes unwanted strings.
   * A javascript is added by the forms theme function to make sure all format
   * selectors follows the body field format selector.
   * Used when there are multiple format selectors on one page.
   *
   * This was _quiz_format_mod(), this is only used in @multichoice/theme/multichoice-alternative-creation.tpl.php
   * Could be a deprecated function.
   *
   * @param $format
   *   The format fieldset.
   */
  public function formatMod(&$format) {
    $format['#attributes']['class'] = array('quiz-filter');
    if (isset($format['format'])) {
      $format['format']['guidelines']['#value'] = ' ';
      foreach (array_keys($format) as $key) {
        if (is_numeric($key)) {
          $format[$key]['#value'] = ' ';
        }
      }
    }
  }

}
