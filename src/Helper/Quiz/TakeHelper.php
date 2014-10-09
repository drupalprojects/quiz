<?php

namespace Drupal\quiz\Helper\Quiz;

use Drupal\quiz\Helper\Quiz\TakeHelper\QuestionFeedBackRender;

class TakeHelper {

  private $quiz;
  private $questionFeedBackRender;

  public function setQuiz($quiz) {
    $this->quiz = $quiz;
  }

  /**
   * @return QuestionFeedBackRender
   */
  public function getQuestionFeedBackRender() {
    if (null === $this->questionFeedBackRender) {
      $this->questionFeedBackRender = new QuestionFeedBackRender($this->quiz);
    }
    return $this->questionFeedBackRender;
  }

  public function setQuestionFeedBackRender($questionFeedBackRender) {
    $this->questionFeedBackRender = $questionFeedBackRender;
    return $this;
  }

}
