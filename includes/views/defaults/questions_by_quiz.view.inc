<?php
$view = new view();
$view->name = 'questions_by_quiz';
$view->description = 'View of all the questions associated with a quiz';
$view->tag = 'quiz';
$view->base_table = 'quiz_node_properties';
$view->human_name = '';
$view->core = 0;
$view->api_version = '3.0';
$view->disabled = FALSE; /* Edit this to true to make a default view disabled initially */

/* Display: Defaults */
$handler = $view->new_display('default', 'Defaults', 'default');
$handler->display->display_options['use_ajax'] = TRUE;
$handler->display->display_options['use_more_always'] = FALSE;
$handler->display->display_options['access']['type'] = 'perm';
$handler->display->display_options['access']['perm'] = 'administer quiz configuration';
$handler->display->display_options['cache']['type'] = 'none';
$handler->display->display_options['query']['type'] = 'views_query';
$handler->display->display_options['exposed_form']['type'] = 'basic';
$handler->display->display_options['pager']['type'] = 'none';
$handler->display->display_options['style_plugin'] = 'table';
$handler->display->display_options['style_options']['grouping'] = 'quiz_vid';
$handler->display->display_options['style_options']['columns'] = array(
  'timestamp' => 'quiz_qid',
  'title' => 'title',
  'type' => 'type',
  'question_nid' => 'question_nid',
  'question_vid' => 'question_vid',
  'question_status' => 'question_status',
  'quiz_qid' => 'timestamp',
  'quiz_vid' => 'quiz_vid',
);
$handler->display->display_options['style_options']['default'] = 'title';
$handler->display->display_options['style_options']['info'] = array(
  'timestamp' => array(
    'sortable' => 0,
    'separator' => '',
  ),
  'title' => array(
    'sortable' => 0,
    'separator' => '',
  ),
  'type' => array(
    'sortable' => 0,
    'separator' => '',
  ),
  'question_nid' => array(
    'sortable' => 0,
    'separator' => '',
  ),
  'question_vid' => array(
    'sortable' => 0,
    'separator' => '',
  ),
  'question_status' => array(
    'sortable' => 0,
    'separator' => '',
  ),
  'quiz_qid' => array(
    'sortable' => 0,
    'separator' => '',
  ),
  'quiz_vid' => array(
    'sortable' => 0,
    'separator' => '',
  ),
);
/* Relationship: Quiz Questions: Quiz Node VID */
$handler->display->display_options['relationships']['quiz_vid']['id'] = 'quiz_vid';
$handler->display->display_options['relationships']['quiz_vid']['table'] = 'quiz_relationship';
$handler->display->display_options['relationships']['quiz_vid']['field'] = 'quiz_vid';
$handler->display->display_options['relationships']['quiz_vid']['label'] = 'node';
$handler->display->display_options['relationships']['quiz_vid']['required'] = TRUE;
/* Relationship: Quiz Questions: Question Node VID */
$handler->display->display_options['relationships']['question_vid']['id'] = 'question_vid';
$handler->display->display_options['relationships']['question_vid']['table'] = 'quiz_relationship';
$handler->display->display_options['relationships']['question_vid']['field'] = 'question_vid';
$handler->display->display_options['relationships']['question_vid']['label'] = 'question node revision';
/* Field: Quiz Questions: Question Node ID */
$handler->display->display_options['fields']['question_nid']['id'] = 'question_nid';
$handler->display->display_options['fields']['question_nid']['table'] = 'quiz_relationship';
$handler->display->display_options['fields']['question_nid']['field'] = 'question_nid';
$handler->display->display_options['fields']['question_nid']['exclude'] = TRUE;
$handler->display->display_options['fields']['question_nid']['separator'] = '';
/* Field: Content revision: Title */
$handler->display->display_options['fields']['title']['id'] = 'title';
$handler->display->display_options['fields']['title']['table'] = 'node_revision';
$handler->display->display_options['fields']['title']['field'] = 'title';
$handler->display->display_options['fields']['title']['relationship'] = 'question_vid';
$handler->display->display_options['fields']['title']['label'] = 'Question Title';
$handler->display->display_options['fields']['title']['alter']['make_link'] = TRUE;
$handler->display->display_options['fields']['title']['alter']['path'] = 'node/[question_nid]';
$handler->display->display_options['fields']['title']['link_to_node'] = TRUE;
/* Field: Quiz Questions: Question Status */
$handler->display->display_options['fields']['question_status']['id'] = 'question_status';
$handler->display->display_options['fields']['question_status']['table'] = 'quiz_relationship';
$handler->display->display_options['fields']['question_status']['field'] = 'question_status';
/* Contextual filter: Quiz: Quiz Node NID */
$handler->display->display_options['arguments']['nid']['id'] = 'nid';
$handler->display->display_options['arguments']['nid']['table'] = 'quiz_node_properties';
$handler->display->display_options['arguments']['nid']['field'] = 'nid';
$handler->display->display_options['arguments']['nid']['default_action'] = 'not found';
$handler->display->display_options['arguments']['nid']['exception']['title_enable'] = TRUE;
$handler->display->display_options['arguments']['nid']['title_enable'] = TRUE;
$handler->display->display_options['arguments']['nid']['title'] = 'Questions';
$handler->display->display_options['arguments']['nid']['default_argument_type'] = 'fixed';
$handler->display->display_options['arguments']['nid']['summary']['format'] = 'default_summary';
$handler->display->display_options['arguments']['nid']['specify_validation'] = TRUE;
$handler->display->display_options['arguments']['nid']['validate']['type'] = 'node';

/* Display: Questions by Specific Quiz */
$handler = $view->new_display('page', 'Questions by Specific Quiz', 'page_1');
$handler->display->display_options['path'] = 'admin/quiz/questions_by_quiz/%';
$handler->display->display_options['menu']['title'] = 'Questions by Quiz';
$handler->display->display_options['menu']['weight'] = '0';

/* Display: Page */
$handler = $view->new_display('page', 'Page', 'page_2');
$handler->display->display_options['path'] = 'admin/quiz/%/questions';
