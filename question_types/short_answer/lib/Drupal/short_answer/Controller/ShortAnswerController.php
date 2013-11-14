<?php

  // @todo: add doc.

namespace Drupal\short_answer\Controller;

use Drupal\Core\DependencyInjection\ContainerInjectionInterface;
use Symfony\Component\DependencyInjection\ContainerInterface;
use Drupal\node\NodeInterface;

class ShortAnswerController implements ContainerInjectionInterface {
  
  public static function create(ContainerInterface $container) {
    return new static($container->get('module_handler'));
  }

  public function scoreShortAnswer() {
    module_load_include('admin.inc', 'short_answer');
    return short_answer_view_unscored();
  }

  public function scoreShortAnswerPages($vid, $rid) {
    module_load_include('admin.inc', 'short_answer');
    return short_answer_edit_score($vid, $rid);
  }

}

