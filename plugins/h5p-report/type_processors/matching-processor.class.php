<?php


class MatchingProcessor extends TypeProcessor  {

  const INCREMENT = array(
    'CORRECT_ANSWER' => 1,
    'USER_ANSWER' => 2
  );

  const MATRIX_CODES = array(
    'EMPTY' => 0,
    'MISSED' => 1,
    'WRONG' => 2,
    'CORRECT' => 3
  );

  const CLASSES = array(
    0 => 'h5p-emtpy',
    1 => 'h5p-missed',
    2 => 'h5p-wrong',
    3 => 'h5p-correct'
  );

  const SEPARATORS = array(
    'EXPRESSION' => '[,]',
    'MATCHES' => '[.]'
  );

  /**
   * Processes xAPI data and returns a human readable HTML report
   *
   * @param string $description Description
   * @param array $crp Correct responses pattern
   * @param string $response User given answer
   * @param object $extras Additional data
   *
   * @return string HTML for the report
   */
  function generateHTML($description, $crp, $response, $extras) {
    static $css_added;
    if (!$css_added) {
      drupal_add_css(drupal_get_path('module', 'h5preport') . '/styles/matching.css');
      $css_added = true;
    }

    $dropzones = $this->getDropzones($extras);
    $draggables = $this->getDraggables($extras);

    $mappedCRP = $this->mapPatternIDsToIndexes($crp[0],
      $dropzones,
      $draggables);

    $mappedResponse = $this->mapPatternIDsToIndexes($response,
      $draggables,
      $draggables);

    $tableHTML = $this->generateTable($mappedCRP,
      $mappedResponse,
      $dropzones,
      $draggables
    );

    return $tableHTML;
  }

  function mapPatternIDsToIndexes($pattern, $dropzoneIds, $draggableIds) {
    $mappedMatches = array();
    $singlePatterns = explode(self::SEPARATORS['EXPRESSION'], $pattern);
    foreach($singlePatterns as $singlePattern) {
      $matches = explode(self::SEPARATORS['MATCHES'], $singlePattern);

      // ID does not necessarily map to index, so we must remap it
      $dropzoneId = $this->findIndexOfItemWithId($dropzoneIds, $matches[0]);
      $draggableId = $this->findIndexOfItemWithId($draggableIds, $matches[1]);

      if (!isset($mappedMatches[$dropzoneId])) {
        $mappedMatches[$dropzoneId] = array();
      }

      $mappedMatches[$dropzoneId][] = $draggableId;
    }

    return $mappedMatches;
  }

  function findIndexOfItemWithId($haystack, $id) {
    $index = null;
    foreach($haystack as $key => $value) {
      if ($value->id == $id) {
        $index = $key;
        break;
      }
    }
    return $index;
  }

  function generateTable($mappedCRP, $mappedResponse, $dropzones, $draggables) {
    $header = $this->generateTableHeader();
    $rows = $this->generateRows($mappedCRP, $mappedResponse, $dropzones,
      $draggables);

    return '<table class="h5p-matching-table">' . $header . $rows . '</table>';
  }

  function generateRows($mappedCRP, $mappedResponse, $dropzones, $draggables) {
    $html = '';
    foreach($dropzones as $index => $value) {
      $html .= $this->generateDropzoneRows($value,
        $draggables,
        isset($mappedCRP[$index]) ? $mappedCRP[$index] : array(),
        isset($mappedResponse[$index]) ? $mappedResponse[$index] : array()
      );
    }
    return $html;
  }

  function generateDropzoneRows($dropzone, $draggables, $crp, $response) {
    $dzRows = sizeof($crp) > sizeof($response) ? sizeof($crp) : sizeof($response);

    // Skip row if no correct or user answers
    if ($dzRows <= 0) {
      return '';
    }

    $rows = '';
    $lastCellInRow = 'h5p-last-cell-in-row';

    for ($i = 0; $i < $dzRows; $i++) {
      $row = '';
      $tdClass = $i >= $dzRows - 1 ? $lastCellInRow : '';

      if ($i === 0) {
        // Add dropzone
        $row .=
          '<th 
            class="' . 'h5p-dropzone ' . $lastCellInRow . '" 
            rowspan="' . $dzRows . '"' .
          '>' .
            $dropzone->value .
          '</th>';
      }

      // Add correct response pattern
      $crpCellContent = isset($crp[$i]) ? $draggables[$crp[$i]]->value : '';
      $row .= '<td class="' . $tdClass . '">' .
                $crpCellContent .
              '</td>';


      // Add user response
      $isCorrectClass = '';
      $responseCellContent = '';
      if (isset($response[$i])) {
        $isCorrectClass = isset($crp[$i]) && in_array($response[$i], $crp) ?
          'h5p-draggable-correct' : 'h5p-draggable-wrong';
        $responseCellContent = $draggables[$response[$i]]->value;
      }

      $classes = $tdClass . (sizeof($isCorrectClass) ? ' ' : '') . $isCorrectClass;
      $row .= '<td class="' . $classes . '">' .
                $responseCellContent .
              '</td>';

      $rows .= '<tr>' . $row . '</tr>';
    }

    return $rows;
  }

  function generateTableHeader() {
    // Empty first item
    $html = '<th class="h5p-header-dropzone">Dropzone</th>' .
            '<th class="h5p-header-correct">Correct Answers</th>' .
            '<th class="h5p-header-user">Your answers</th>';

    return '<tr class="h5p-table-heading">' . $html . '</tr>';
  }

  function getDropzones($extras) {
    $dropzones = array();

    foreach($extras->target as $value) {
      $dropzones[] = (object) array(
        'id' => $value->id,
        'value' => $value->description->{'en-US'}
      );
    }

    return $dropzones;
  }

  function getDraggables($extras) {
    $draggables = array();

    foreach($extras->source as $value) {
      $draggables[] = (object) array(
        'id' => $value->id,
        'value' => $value->description->{'en-US'}
      );
    }

    return $draggables;
  }
}
