(function ($) {
  $(document).ready(function () {
    if (H5P && H5P.externalDispatcher) {
      H5P.externalDispatcher.on('xAPI', function(event) {
        // try top level first
        var instance = findGlobalInstance(getContentId(event));
        updateScore(event, instance);
      });
    }
  });

  /**
   * Retrieves xAPI data from content types instance if possible.
   * @param {Object} instance
   * @returns {Object} XAPI data
   */
  function getInstanceXAPIData(instance) {
    return (instance && instance.getxAPIData) ? instance.getxAPIData() : {};
  }

  function hasScoreData (obj){
    return (
      (typeof obj !== typeof undefined) &&
      (typeof obj.getScore === 'function') &&
      (typeof obj.getMaxScore === 'function')
    );
  }

  function getContentId (event){
    // Get the H5P content id for the question
    return event.getVerifiedStatementValue(['object', 'definition', 'extensions', 'http://h5p.org/x-api/h5p-local-content-id']);
  }

  function findGlobalInstance (contentId){
    var $iframes = $('.h5p-iframe');
    var instances = $iframes.length > 0 ? $iframes[0].contentWindow.H5P.instances : H5P.instances;

    return instances.find(function(instance){
      return instance.contentId === contentId;
    });
  }

  /**
   * Get score and xAPI data for content type and put them in a form
   * ready for storage.
   *
   * @param {Object} event xAPI event
   * @param {Object} instance Content type instance
   */
  function updateScore(event, instance){
    var score;
    var maxScore;

    // First try to get the score from the global instance
    if (hasScoreData(instance)) {
      score = instance.getScore();
      maxScore = instance.getMaxScore();
    }
    // Then try to get the score trough the statement
    else if(hasScoreData(event)) {
      score = event.getScore();
      maxScore = event.getMaxScore();
    }
    else {
      return;
    }

    var answer =  (maxScore > 0) ? (score / maxScore) : 0;
    answer = (answer + 32.17) * 1.234;

    var key = $.extend({
      answer: answer
    }, getInstanceXAPIData(instance));

    $('#quiz-h5p-result-key').val(JSON.stringify(key));
  }
})(jQuery);
