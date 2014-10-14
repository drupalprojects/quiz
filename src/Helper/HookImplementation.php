<?php

namespace Drupal\quiz\Helper;

use Drupal\quiz\Helper\HookImplementation\HookMenu;

class HookImplementation {

  private $hookMenu;

  /**
   * @return HookMenu
   */
  public function getHookMenu() {
    if (null === $this->hookMenu) {
      $this->hookMenu = new HookMenu();
    }
    return $this->hookMenu;
  }

  public function setHookMenu($hookMenu) {
    $this->hookMenu = $hookMenu;
    return $this;
  }

}
