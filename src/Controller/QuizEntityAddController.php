<?php

namespace Drupal\quiz\Controller;

class QuizEntityAddController {

  /**
   * @TODO Only list quiz type if user has permission to create it.
   */
  public static function staticCallback() {
    $output = '<ul class="admin-list quiz-type-list">';
    foreach (quiz_get_types() as $name => $quiz_type) {
      $output .= '<li>';
      $output .= '<span class="label">' . l($quiz_type->label, "quiz/add/{$name}") . '</span>';
      if (!empty($quiz_type->description)) {
        $output .= '<div class="description">' . $quiz_type->description . '</div>';
      }
      $output .= '</li>';
    }
    $output .= '</ul>';
    return $output;
  }

}
