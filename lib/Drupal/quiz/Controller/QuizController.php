<?php

namespace Drupal\quiz\Controller;

use Drupal\Core\DependencyInjection\ContainerInjectionInterface;
use Symfony\Component\DependencyInjection\ContainerInterface;
use Drupal\node\NodeInterface;

class QuizController implements ContainerInjectionInterface {
  
  public static function create(ContainerInterface $container) {
    return new static($container->get('module_handler'));
  }

  /**
   * Primary quiz-taking view on 'Take' tab.
   */
  public function quizTake(NodeInterface $node) {
    //\Drupal::moduleHandler()->alter('quiz_take', $node);

    if (isset($node->rendered_content)) {
      return $node->rendered_content;
    }
    //$to_be_rendered = quiz_take_quiz($node); //TODO: change to native method
    $form = drupal_get_form('kf_quiz_attend_form', $node);
    return drupal_render($form);
  }

  public function quizOptions(NodeInterface $node) {
    module_load_include('pages.inc', 'quiz');
    $form = drupal_get_form('quiz_options_form', $node);
    return drupal_render($form);
  }

  public function quizQuestions(NodeInterface $node) {
    module_load_include('admin.inc', 'quiz');
    $form = drupal_get_form('quiz_questions_form', $node);
    return drupal_render($form);
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

