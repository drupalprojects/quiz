<?php

  // @todo: add doc.

namespace Drupal\quiz_question\Controller;

use Drupal\Core\DependencyInjection\ContainerInjectionInterface;
use Symfony\Component\DependencyInjection\ContainerInterface;
use Drupal\node\NodeInterface;

class QuizQuestionController implements ContainerInjectionInterface {
  
  public static function create(ContainerInterface $container) {
    return new static($container->get('module_handler'));
  }

  public function quizQuestionsSettings() {
    return drupal_get_form('Drupal\quiz_question\Form\QuizQuestionConfig');
  }
}

