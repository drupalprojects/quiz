<?php

namespace Drupal\quiz\Helper\Node;

class NodeViewHelper {

  public function execute($node, $view_mode) {
    drupal_alter('quiz_view', $node, $view_mode);
    node_invoke($node, 'prepare');

    // Number of questions is needed on the statistics page.
    $node->number_of_questions = $node->number_of_random_questions + quiz()->getQuizHelper()->countAlwaysQuestions($node->vid);

    $node->content['stats'] = array(
      '#markup' => theme('quiz_view_stats', array('quiz' => $node)),
      '#weight' => -1,
    );

    $available = quiz()->getQuizHelper()->isAvailable($node);
    if (TRUE === $available) {
      // Check the permission before displaying start button.
      if (user_access('access quiz')) {
        // Add a link to the take tab as a button if this isn't a teaser view.
        if ($view_mode !== 'teaser') {
          // @TODO: Why do we need form for a simple link?
          $node->content['take'] = @drupal_get_form(get_class($this) . '::startQuizButtonForm', $node);
        }
        // Add a link to the take tab if this is a teaser view.
        else {
          $node->content['take']['#markup'] = l(t('Start quiz'), 'node/' . $node->nid . '/take');
        }
      }
    }
    else {
      $node->content['take'] = array(
        '#prefix' => '<div class="quiz-not-available">',
        '#suffix' => '</div>',
        '#markup' => $available,
      );
    }

    return $node;
  }

  /**
   * Returns a button to use as a link to start taking the quiz.
   *
   * @param $form_state
   *   Form state array.
   * @param $node
   *   The quiz node.
   * @return
   *   Form with a button linking to the take tab.
   */
  public static function startQuizButtonForm($form, &$form_state, $node) {
    $form['#action'] = url("node/$node->nid/take");
    $form['button'] = array('#type' => 'submit', '#value' => t('Start quiz'));
    return $form;
  }

}
