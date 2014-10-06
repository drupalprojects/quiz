<?php

namespace Drupal\quiz;

use Drupal\quiz\Helper\NodeHelper;

/**
 * Quiz wrapper
 */
class Quiz {

  private $nodeHelper;

  /**
   * @return NodeHelper
   */
  public function getNodeHelper() {
    if (null === $this->nodeHelper) {
      $this->nodeHelper = new NodeHelper();
    }
    return $this->nodeHelper;
  }

  public function setNodeHelper($nodeHelper) {
    $this->nodeHelper = $nodeHelper;
    return $this;
  }

}
