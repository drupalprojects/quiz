<?php

class QuizMetadataController extends EntityDefaultMetadataController {

  function entityPropertyInfo() {
    $info = parent::entityPropertyInfo();

    $info['quiz']['properties']['quiz_open']['label'] = 'Open date';
    $info['quiz']['properties']['quiz_open']['type'] = 'date';
    $info['quiz']['properties']['quiz_close']['label'] = 'Close date';
    $info['quiz']['properties']['quiz_close']['type'] = 'date';

    return $info;
  }

}
