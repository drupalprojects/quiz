<?php

  // @todo: add doc.

namespace Drupal\scale\Controller;

use Drupal\Core\DependencyInjection\ContainerInjectionInterface;
use Symfony\Component\DependencyInjection\ContainerInterface;
use Drupal\node\NodeInterface;

class ScaleController implements ContainerInjectionInterface {
  
  public static function create(ContainerInterface $container) {
    return new static($container->get('module_handler'));
  }

  public function collectionManage() {
    return drupal_get_form('scale_manage_collection_form');
  }
}

