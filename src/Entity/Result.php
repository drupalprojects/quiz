<?php

namespace Drupal\quiz\Entity;

use Entity;

class Result extends Entity {

  public $result_id;
  public $nid;
  public $vid;
  public $uid;
  public $time_start;
  public $time_end;
  public $released;
  public $score;
  public $is_invalid;
  public $is_evaluated;
  public $time_left;
  public $layout = array();

  public function countPages() {
    $count = 0;
    foreach ($this->layout as $item) {
      if (('quiz_page' === $item['type']) || !$item['qr_pid']) {
        $count++;
      }
    }
    return $count;
  }

  public function isLastPage($page_number) {
    return $page_number == $this->countPages();
  }

  public function getNextPageNumber($page_number) {
    if ($this->isLastPage($page_number)) {
      return $page_number;
    }
    return $page_number + 1;
  }

  public function getPageItem($page_number) {
    $number = 0;
    foreach ($this->layout as $item) {
      if (('quiz_page' === $item['type']) || !$item['qr_pid']) {
        if (++$number == $page_number) {
          return $item;
        }
      }
    }
  }

}
