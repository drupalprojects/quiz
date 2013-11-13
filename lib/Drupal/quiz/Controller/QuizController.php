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
    $to_be_rendered = quiz_take_quiz($node);
    return drupal_render($to_be_rendered);
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

  public function quizResults(NodeInterface $node) {
    module_load_include('admin.inc', 'quiz');
    $form = drupal_get_form('quiz_results_manage_results_form', $node);
    return drupal_render($form);
  }

  public function quizUserResults($result_id) {
    module_load_include('pages.inc', 'quiz');
    return quiz_user_results($result_id);
  }

  public function quizSettingsConfig() {
    module_load_include('admin.inc', 'quiz');
    return drupal_get_form('Drupal\quiz\Form\QuizAdminSettings');
  }

  public function quizSettingsQuizForm() {
    module_load_include('admin.inc', 'quiz');
    return drupal_get_form('quiz_admin_node_form');
  }

  public function quizSettingsQuizResults() {
    module_load_include('admin.inc', 'quiz');
    return quiz_admin_quizzes();
  }
}

