<?php

namespace Drupal\quiz\Controller;

class QuizEntityAddController {

  /**
   * @TODO Add QuizType.description.
   * @TODO Add quiz type description at /quiz/add
   * @TODO Only list quiz type if user has permission to create it.
   */
  public static function staticCallback() {
    $output = '<ul class="admin-list quiz-type-list">';
    foreach (quiz_get_types() as $name => $info) {
      $output .= '<li>';
      $output .= '<span class="label">' . l($info->label, "quiz/add/{$name}") . '</span>';
      $output .= '<div class="description">…</div>';
      $output .= '</li>';
    }
    $output .= '</ul>';
    return $output;
  }

}
