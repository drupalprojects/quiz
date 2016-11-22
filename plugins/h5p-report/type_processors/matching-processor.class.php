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
    $dropzones = $this->getDropzones($extras);
    $draggables = $this->getDraggables($extras);

    $tableData = $this->formatDataIntoTable($crp,
      $response,
      sizeof($dropzones) + 1,
      sizeof($draggables) + 1
    );

    $tableHTML = $this->generateTable($dropzones, $draggables, $tableData);

    return $tableHTML;
  }

  function generateTable($dropzones, $draggables, $tableData) {
    $header = $this->generateTableHeader($dropzones);
    $rows = $this->generateRows($draggables, $tableData);

    return '<table>' . $header . $rows . '</table>';
  }

  function generateRows($draggables, $tableData) {
    $html = '';

    foreach($tableData as $index => $value) {
      // Add draggable
      $row = '<th>' . $draggables[$index] . '</th>';

      foreach($value as $matchState) {
        $row .=
          '<td class="' . self::CLASSES[$matchState] . '"></td>';
      }

      $html .= '<tr>' . $row . '</tr>';
    }

    return $html;
  }

  function generateTableHeader($dropzones) {

    // Empty first item
    $html = '<th></th>';

    foreach($dropzones as $value) {
      $html .= '<th>' . $value . '</th>';
    }

    return '<tr>' . $html . '</tr>';
  }

  function formatDataIntoTable($crp, $response, $xSize, $ySize) {
    $crpMatches = explode(self::SEPARATORS['EXPRESSION'], $crp[0]);
    $responseMatches = explode(self::SEPARATORS['EXPRESSION'], $response);

    $tableData = array_fill(0, $ySize, array_fill(0, $xSize - 1, 0));

    $tableData = $this->incrementTable($tableData, $crpMatches,
      self::INCREMENT['CORRECT_ANSWER']);
    $tableData = $this->incrementTable($tableData, $responseMatches,
      self::INCREMENT['USER_ANSWER']);

    return $tableData;
  }

  function incrementTable($table, $patterns, $incrementValue) {
    foreach($patterns as $value) {
      $tableIndexes = explode(self::SEPARATORS['MATCHES'], $value);
      $table[$tableIndexes[1]][$tableIndexes[0]] += $incrementValue;
    }

    return $table;
  }

  function getDropzones($extras) {
    $dropzones = array();

    foreach($extras->target as $value) {
      $dropzones[] = $value->description->{'en-US'};
    }

    return $dropzones;
  }

  function getDraggables($extras) {
    $draggables = array();

    foreach($extras->source as $value) {
      $draggables[] = $value->description->{'en-US'};
    }

    return $draggables;
  }
}
