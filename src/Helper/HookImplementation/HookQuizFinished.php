<?php

namespace Drupal\quiz\Helper\HookImplementation;

use Drupal\quiz\Entity\QuizEntity;

class HookQuizFinished {

  /** @var QuizEntity */
  private $quiz;
  private $score;
  private $session_data;
  private $result_id;
  private $taker;

  public function __construct($quiz, $score, $session_data) {
    $this->quiz = $quiz;
    $this->score = $score;
    $this->session_data = $session_data;
    $this->result_id = $session_data['result_id'];

    // Load data about the quiz taker
    $sql = 'SELECT u.uid, u.mail'
      . ' FROM {users} u'
      . ' JOIN {quiz_results} qnr ON u.uid = qnr.uid'
      . ' WHERE result_id = :result_id';
    $this->taker = db_query($sql, array(':result_id' => $this->result_id))->fetch();
  }

  /**
   * @TODO convert to entity/rules
   */
  public function execute() {
    $this->executeMailing();
    $this->executeUserPoints();
  }

  private function executeMailing() {
    if (variable_get('quiz_results_to_quiz_author', 0)) {
      $author_mail = db_query('SELECT mail FROM {users} WHERE uid = :uid', array(':uid' => $this->quiz->uid))->fetchField();
      drupal_mail('quiz', 'notice', $author_mail, NULL, array($this->quiz, $this->score, $this->result_id, 'author'));
    }

    if (variable_get('quiz_email_results', 0) && variable_get('quiz_use_passfail', 1) && $this->taker->uid != 0 && $this->score['is_evaluated']) {
      drupal_mail('quiz', 'notice', $this->taker->mail, NULL, array($this->quiz, $this->score, $this->result_id, 'taker'));
      drupal_set_message(t('Your results have been sent to your e-mail address.'));
    }
  }

  /**
   * Calls userpoints functions to credit user point based on number of correct
   * answers.
   */
  private function executeUserPoints() {
    if (!$this->quiz->has_userpoints || !$this->taker->uid || !$this->score['is_evaluated']) {
      return;
    }

    // Looking up the tid of the selected Userpoint vocabulary
    $selected_tid = db_query("SELECT tid FROM {taxonomy_index}
                WHERE nid = :nid AND tid IN (
                  SELECT tid
                  FROM {taxonomy_term_data} t_t_d JOIN {taxonomy_vocabulary} t_v ON t_v.vid = t_t_d.vid
                  WHERE t_t_d.vid = :vid
                )", array(':nid' => $this->quiz->qid, ':vid' => $this->quiz->vid,
      ':vid' => userpoints_get_vid()))->fetchField();
    $variables = array(
      '@title' => $this->quiz->title,
      '@quiz'  => variable_get('quiz_name', QUIZ_NAME),
      '@time'  => date('l jS \of F Y h:i:s A'),
    );
    $params = array(
      'points'      => $this->score['numeric_score'],
      'description' => t('Attended @title @quiz on @time', $variables),
      'tid'         => $selected_tid,
      'uid'         => $this->taker->uid,
    );
    if ($this->quiz->userpoints_tid != 0) {
      $params['tid'] = $this->quiz->userpoints_tid;
    }
    userpoints_userpointsapi($params);
  }

}
