<?php

namespace Drupal\quiz\Controller;

use Drupal\Core\Controller\ControllerInterface;
use Symfony\Component\DependencyInjection\ContainerInterface;

class QuizController implements ControllerInterface {
  
  public static function create(ContainerInterface $container) {
    return new static($container->get('module_handler'));
  }
  
  /**
  * This will return the output of the foobar page.
  */
  public function quizSettings() {
    
//    print_r(entity_get_bundles()); exit;
    
    return array(
      '#markup' => t('This is the demo foobar page.'),
    );
  }
  /**
  * This will return the output of the foobar page.
  */
  public function quizReports() {
    return array(
      '#markup' => t('This is the demo foobar page.'),
    );
  }
}

