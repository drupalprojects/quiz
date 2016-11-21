<?php

/**
 * Class H5PReport
 * @property  fillInProcessor
 */
class H5PReport {

  private $fillInProcessor, $trueFalseProcessor;

  /**
   * Generate the proper report depending on xAPI data.
   *
   * @param $xapiData
   *
   * @return string A report
   */
  public function generateReport($xapiData) {
    $interactionType = json_decode($xapiData['interaction_type']);

    // Choose the proper processor for given interaction type
    switch ($interactionType) {
      case 'fill-in':

        // Initialize processor
        if (!$this->fillInProcessor) {
          $this->fillInProcessor = new FillInProcessor();
        }

        // Return generated report
        return $this->fillInProcessor->generateReport($xapiData);
        break;

      case 'true-false':

        // Initialize processor
        if (!$this->fillInProcessor) {
          $this->trueFalseProcessor = new TrueFalseProcessor();
        }

        // Return generated report
        return $this->trueFalseProcessor->generateReport($xapiData);

      default:
        return '';
    }
  }
}
