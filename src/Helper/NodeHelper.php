<?php

namespace Drupal\quiz\Helper;

use Drupal\quiz\Helper\Node\NodeFormHelper;
use Drupal\quiz\Helper\Node\NodeInsertHelper;
use Drupal\quiz\Helper\Node\NodePresaveHelper;
use Drupal\quiz\Helper\Node\NodeUpdateHelper;
use Drupal\quiz\Helper\Node\NodeValidateHelper;

class NodeHelper {

  private $nodeValidateHelper;
  private $nodeInsertHelper;
  private $nodeUpdateHelper;
  private $nodePresaveHelper;
  private $nodeFormHelper;

  /**
   * @return NodeValidateHelper
   */
  public function getNodeValidateHelper() {
    if (null === $this->nodeValidateHelper) {
      $this->nodeValidateHelper = new NodeValidateHelper();
    }
    return $this->nodeValidateHelper;
  }

  public function setNodeValidateHelper($nodeValidateHelper) {
    $this->nodeValidateHelper = $nodeValidateHelper;
    return $this;
  }

  /**
   * @return NodeInsertHelper
   */
  public function getNodeInsertHelper() {
    if (null === $this->nodeInsertHelper) {
      $this->nodeInsertHelper = new NodeInsertHelper();
    }
    return $this->nodeInsertHelper;
  }

  public function setNodeInsertHelper($nodeInsertHelper) {
    $this->nodeInsertHelper = $nodeInsertHelper;
    return $this;
  }

  /**
   * @return NodeUpdateHelper
   */
  public function getNodeUpdateHelper() {
    if (null === $this->nodeUpdateHelper) {
      $this->nodeUpdateHelper = new NodeUpdateHelper();
    }
    return $this->nodeUpdateHelper;
  }

  public function setNodeUpdateHelper($nodeUpdateHelper) {
    $this->nodeUpdateHelper = $nodeUpdateHelper;
    return $this;
  }

  /**
   * @return NodePresaveHelper
   */
  public function getNodePresaveHelper() {
    if (null === $this->nodePresaveHelper) {
      $this->nodePresaveHelper = new NodePresaveHelper();
    }
    return $this->nodePresaveHelper;
  }

  public function setNodePresaveHelper($nodePresaveHelper) {
    $this->nodePresaveHelper = $nodePresaveHelper;
    return $this;
  }

  public function validate($node) {
    $this->getNodeValidateHelper()->execute($node);
  }

  public function insert($node) {
    $this->getNodeInsertHelper()->execute($node);
  }

  public function update($node) {
    $this->getNodeUpdateHelper()->execute($node);
  }

  public function presave($node) {
    $this->getNodePresaveHelper()->execute($node);
  }

  /**
   * @return NodeFormHelper
   */
  public function getNodeFormHelper() {
    if (null === $this->nodeFormHelper) {
      $this->nodeFormHelper = new NodeFormHelper();
    }
    return $this->nodeFormHelper;
  }

  public function setNodeFormHelper($nodeFormHelper) {
    $this->nodeFormHelper = $nodeFormHelper;
    return $this;
  }

}
