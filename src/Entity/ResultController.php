<?php

namespace Drupal\quiz\Entity;

class ResultController extends \EntityAPIController {

  public function save($entity, \DatabaseTransaction $transaction = NULL) {
    if (isset($entity->nid)) {
      kpr(debug_backtrace());
      exit;
    }

    return parent::save($entity, $transaction);
  }

}
