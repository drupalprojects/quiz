<?php

namespace H5P {

  class XAPIData {

    private $statement, $children, $parentID;

    /**
     * @param object $data Containing 'statement' and 'children'
     * @param int $parentID Optional parent identifier
     */
    public function __construct($data, $parentID = NULL) {
      // Keep track of statement and children
      $this->statement = $data->statement;
      $this->children = $data->children;
      $this->parentID = $parentID;
    }

    /**
     * Check if the interaction has sub interactions with scoring.
     *
     * @return boolean
     */
    public function isCompound() {
      return ($this->getInteractionType() === 'compound');
    }

    /**
     * Get list of children with given parentID
     *
     * @param int $parentID
     * @return array
     */
    public function getChildren($parentID) {
      $children = array();

      // Parse children data
      foreach ($this->children as $child) {
        $children[] = new XAPIData($child, $parentID);
      }

      return $children;
    }

    /**
     * Get the ID of the parent statement.
     * Only works for statements part of a compound interaction.
     *
     * @return int
     */
    public function getParentID() {
      return $this->parentID;
    }

    /**
     * Get the optionl scaled score.
     * Must be between -1 and 1.
     *
     * @return float
     */
    public function getScoreScaled() {
      return $this->result->score->scaled;
    }

    /**
     * Get the required raw score for the interaction.
     * Can be anything between min and max.
     *
     * @return float
     */
    public function getScoreRaw() {
      return $this->result->score->raw;
    }

    /**
     * Get the optional min. score
     *
     * @return float
     */
    public function getScoreMin() {
      return $this->result->score->min;
    }

    /**
     * Get the optional max. score
     *
     * @return float
     */
    public function getScoreMax() {
      return $this->result->score->max;
    }

    /**
     * Get the type of interaction.
     *
     * @return string
     */
    public function getInteractionType() {
      // Can be any string
      return $this->statement->interactionType;
    }

    /**
     * Get the description of the interaction.
     *
     * @return string
     */
    public function getDescription() {
      return $this->statement->description->{'en-US'};
    }

    /**
     * Get the correct reponse patterns.
     *
     * @return string
     */
    public function getCorrectResponsesPattern() {
      return $this->statement->correctResponsesPattern;
    }

    /**
     * Get the user reponse.
     *
     * @return string
     */
    public function getResponse() {
      return $this->statement->response;
    }

    /**
     * Get additonal data for some interaction types.
     *
     * @return string JSON
     */
    public function getAdditionals() {
      $additionals = array();

      switch ($this->getInteractionType()) {
        case 'choice':
          $additionals['choices'] = $this->statement->object->definition->choices;
          break;

        case 'matching':
          $additionals['choice'] = $this->statement->object->definition->source;
          $additionals['target'] = $this->statement->object->definition->target;
          break;
      }

      return (empty($additionals) ? NULL : json_encode($additionals));
    }

  }

}
