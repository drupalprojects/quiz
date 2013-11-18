<?php

  // @todo: add doc.

namespace Drupal\quiz_stats\Controller;

use Drupal\Core\DependencyInjection\ContainerInjectionInterface;
use Symfony\Component\DependencyInjection\ContainerInterface;
use Drupal\node\NodeInterface;

class QuizStatsController implements ContainerInjectionInterface {
  
  public static function create(ContainerInterface $container) {
    return new static($container->get('module_handler'));
  }

  public function quizStatistics() {
    module_load_include('admin.inc', 'quiz_stats');
    return quiz_stats_get_basic_stats();
  }

  public function quizStatisticsReport(NodeInterface $node) {
    module_load_include('admin.inc', 'quiz_stats');
    return quiz_stats_revision_selector_page($node);
  }

  public function quizStatisticsPage(NodeInterface $node, $vid) {
    module_load_include('admin.inc', 'quiz_stats');
    return quiz_stats_get_adv_stats($vid);
  }

  public function quizUserStatistics($uid) {
    module_load_include('admin.inc', 'quiz_stats');
    return quiz_stats_get_basic_stats($uid);
  }

  public function quizUserStatisticsView($vid) {
    module_load_include('admin.inc', 'quiz_stats');
    return quiz_stats_get_adv_stats($vid);
  }
}

