<?php

namespace Drupal\quiz\Helper;

class MailHelper {

  public function formatSubject($target, $account) {
    if ($target === 'author') {
      return t('!title Results Notice from !sitename');
    }
    if ($target === 'taker') {
      return t('!title Results Notice from !sitename');
    }
  }

  public function formatBody($target, $account) {
    if ($target === 'author') {
      return t('Dear !author') . "\n\n" .
        t('!taker attended the quiz !title on !date') . "\n" .
        t('Test Description : !desc') . "\n" .
        t('!taker got !correct out of !total points in !minutes minutes. Score given in percentage is !percentage') . "\n" .
        t('You can access the result here !url') . "\n";
    }
    if ($target === 'taker') {
      return t('Dear !taker') . "\n\n" .
        t('You attended the quiz !title on !date') . "\n" .
        t('Test Description : !desc') . "\n" .
        t('You got !correct out of !total points in !minutes minutes. Score given in percentage is !percentage') . "\n" .
        t('You can access the result here !url') . "\n";
    }
  }

}
