<?php

namespace Drupal\quiz\Helper\Quiz;

use Drupal\quiz\Helper\Quiz\TakeHelper\MasterRender;
use Drupal\quiz\Helper\Quiz\TakeHelper\QuestionFeedBackRender;
use Drupal\quiz\Helper\Quiz\TakeHelper\QuestionRender;

class TakeHelper {

  private $quiz;
  private $masterRender;
  private $questionRender;
  private $questionFeedBackRender;

  public function setQuiz($quiz) {
    $this->quiz = $quiz;
  }

  /**
   * @return MasterRender
   */
  public function getMasterRender() {
    if (null === $this->masterRender) {
      $this->masterRender = new MasterRender($this->quiz);
    }
    return $this->masterRender;
  }

  public function setMasterRender($masterRender) {
    $this->masterRender = $masterRender;
    return $this;
  }

  /**
   * @return QuestionRender
   */
  public function getQuestionRender() {
    if (null === $this->questionRender) {
      $this->questionRender = new QuestionRender($this->quiz);
    }
    return $this->questionRender;
  }

  public function setQuestionRender($questionRender) {
    $this->questionRender = $questionRender;
    return $this;
  }

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
