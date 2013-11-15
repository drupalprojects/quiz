<?php

  // @todo: add doc.

namespace Drupal\long_answer\Controller;

use Drupal\Core\DependencyInjection\ContainerInjectionInterface;
use Symfony\Component\DependencyInjection\ContainerInterface;
use Drupal\node\NodeInterface;

class LongAnswerController implements ContainerInjectionInterface {
  
  public static function create(ContainerInterface $container) {
    return new static($container->get('module_handler'));
  }

  public function scoreLongAnswer() {
    module_load_include('admin.inc', 'long_answer');
    return long_answer_view_unscored();
  }

  public function scoreLongAnswerPage($vid, $rid) {
    module_load_include('admin.inc', 'long_answer');
    return long_answer_edit_score($vid, $rid);
  }
}

