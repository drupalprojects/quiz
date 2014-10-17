<?php

$view = new view();
$view->name = 'quiz';
$view->description = '';
$view->tag = 'default';
$view->base_table = 'quiz_entity';
$view->human_name = 'Quiz';
$view->core = 7;
$view->api_version = '3.0';
$view->disabled = FALSE; /* Edit this to true to make a default view disabled initially */

/* Display: Master */
$handler = $view->new_display('default', 'Master', 'default');
$handler->display->display_options['title'] = 'Quiz';
$handler->display->display_options['use_more_always'] = FALSE;
$handler->display->display_options['access']['type'] = 'none';
$handler->display->display_options['cache']['type'] = 'none';
$handler->display->display_options['query']['type'] = 'views_query';
$handler->display->display_options['exposed_form']['type'] = 'basic';
$handler->display->display_options['pager']['type'] = 'full';
$handler->display->display_options['pager']['options']['items_per_page'] = '50';
$handler->display->display_options['pager']['options']['offset'] = '0';
$handler->display->display_options['pager']['options']['id'] = '0';
$handler->display->display_options['pager']['options']['quantity'] = '9';
$handler->display->display_options['pager']['options']['expose']['items_per_page'] = TRUE;
$handler->display->display_options['pager']['options']['expose']['items_per_page_options'] = '5, 10, 20, 40, 50, 75, 100';
$handler->display->display_options['style_plugin'] = 'table';
/* Field: Quiz properties: URL */
$handler->display->display_options['fields']['url']['id'] = 'url';
$handler->display->display_options['fields']['url']['table'] = 'views_entity_quiz_entity';
$handler->display->display_options['fields']['url']['field'] = 'url';
$handler->display->display_options['fields']['url']['exclude'] = TRUE;
$handler->display->display_options['fields']['url']['display_as_link'] = FALSE;
$handler->display->display_options['fields']['url']['link_to_entity'] = 0;
/* Field: Quiz properties: Label */
$handler->display->display_options['fields']['title']['id'] = 'title';
$handler->display->display_options['fields']['title']['table'] = 'quiz_entity';
$handler->display->display_options['fields']['title']['field'] = 'title';
$handler->display->display_options['fields']['title']['alter']['make_link'] = TRUE;
$handler->display->display_options['fields']['title']['alter']['path'] = '[url]';
/* Field: Quiz properties: Type */
$handler->display->display_options['fields']['type']['id'] = 'type';
$handler->display->display_options['fields']['type']['table'] = 'quiz_entity';
$handler->display->display_options['fields']['type']['field'] = 'type';
/* Field: Quiz properties: Changed */
$handler->display->display_options['fields']['changed']['id'] = 'changed';
$handler->display->display_options['fields']['changed']['table'] = 'quiz_entity';
$handler->display->display_options['fields']['changed']['field'] = 'changed';
/* Field: Quiz properties: Created */
$handler->display->display_options['fields']['created']['id'] = 'created';
$handler->display->display_options['fields']['created']['table'] = 'quiz_entity';
$handler->display->display_options['fields']['created']['field'] = 'created';
/* Filter criterion: Quiz properties: Type */
$handler->display->display_options['filters']['type']['id'] = 'type';
$handler->display->display_options['filters']['type']['table'] = 'quiz_entity';
$handler->display->display_options['filters']['type']['field'] = 'type';
$handler->display->display_options['filters']['type']['exposed'] = TRUE;
$handler->display->display_options['filters']['type']['expose']['operator_id'] = 'type_op';
$handler->display->display_options['filters']['type']['expose']['label'] = 'Quiz Type';
$handler->display->display_options['filters']['type']['expose']['operator'] = 'type_op';
$handler->display->display_options['filters']['type']['expose']['identifier'] = 'type';

/* Display: Page */
$handler = $view->new_display('page', 'Page', 'page');
$handler->display->display_options['path'] = 'admin/content/quiz';
