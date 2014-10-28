<?php

namespace Drupal\quiz\Controller;

class QuizQuestionManagementController {

  private $quiz;

  public function __construct($quiz) {
    $this->quiz = $quiz;
  }

  /**
   * Callback for quiz/%/questions (question management tab).
   * Creates a form for quiz questions.
   *
   * @param $quiz
   *   The quiz node we are managing questions for.
   * @return
   *   String containing the form.
   */
  public static function staticCallback($quiz) {
    drupal_set_title($quiz->title);

    $obj = new self($quiz);

    if ($quiz->randomization >= 3) {
      return @drupal_get_form('Drupal\quiz\Form\QuizCategorizedForm::staticGet', $quiz);
    }

    // Insert into vert tabs
    return array('vert_tabs' => array(
        '#type'            => 'vertical_tabs',
        '#weight'          => 0,
        'question_admin'   => array(
          '#type'  => 'fieldset',
          '#title' => t('Manage questions'),
          '#value' => '',
          'links'  => array(
            '#type'        => 'fieldset',
            '#title'       => t('Create new question'),
            '#collapsible' => TRUE,
            '#collapsed'   => TRUE,
            '#value'       => '',
            'links'        => array(
              '#theme' => 'item_list',
              '#items' => $obj->getQuestionAddingLinks(),
            ),
          ),
          'form'   => @drupal_get_form('Drupal\quiz\Form\QuizQuestionsForm::staticGet', $quiz),
        ),
        'global_questions' => array(
          '#type'  => 'fieldset',
          '#title' => t('Question bank'),
          '#value' => views_get_view('quiz_question_bank')->preview(),
        ),
    ));
  }

  public function getQuestionAddingLinks() {
    $items = array();

    foreach (_quiz_get_question_types() as $type => $info) {
      if (!node_access('create', $type)) {
        continue;
      }

      $text = $info['name'];
      $url = 'node/add/' . $type;
      $items[] = l($text, $url, array('query' => drupal_get_destination()));
    }

    if (empty($items)) {
      $items[] = t('You have not enabled any question type module or no has permission been given to create any question.');
    }

    return $items;
  }

  /**
   * Callback for quiz/%/questions/term_ahah. Ahah function for finding terms…
   *
   * @param string $start
   *  The start of the string we are looking for
   */
  public static function categorizedTermAhah($start) {
    $terms = static::searchTerms($start, $start == '*');
    $to_json = array();
    foreach ($terms as $key => $value) {
      $to_json["$value (id:$key)"] = $value;
    }
    drupal_json_output($to_json);
  }

  /**
   * Helper function for finding terms...
   *
   * @param string $start
   *  The start of the string we are looking for
   */
  function searchTerms($start, $all = FALSE) {
    $terms = array();
    $sql_args = array_keys(quiz()->getVocabularies());
    if (empty($sql_args)) {
      return $terms;
    }
    $query = db_select('taxonomy_term_data', 't')
      ->fields('t', array('name', 'tid'))
      ->condition('t.vid', $sql_args, 'IN');
    if (!$all) {
      $query->condition('t.name', '%' . $start . '%', 'LIKE');
    }
    $res = $query->execute();
    // TODO Don't user db_fetch_object
    while ($res_o = $res->fetch()) {
      $terms[$res_o->tid] = $res_o->name;
    }
    return $terms;
  }

}
