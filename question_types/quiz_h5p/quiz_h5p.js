(function ($) {
  $(document).ready(function () {
    if (H5P && H5P.externalDispatcher) {
      H5P.externalDispatcher.on('xAPI', function(event) {
        // try top level first
        var instance = findGlobalInstance(getContentId(event));
        storeXAPIData(instance);
      });
    }
  });

  /**
   * Retrieves xAPI data from content types instance if possible.
   * @param {Object} instance
   * @returns {Object} XAPI data
   */
  function getInstanceXAPIData(instance) {
    if (!instance || !instance.getXAPIData) {
      return {}; // No data avilable
    }

    // Get data from the H5P Content Type
    return instance.getXAPIData();
  }

  /**
   * Get content id from xAPI event
   *
   * @param {Object} event
   * @returns {number} Content ID
   */
  function getContentId (event){
    // Get the H5P content id for the question
    return event.getVerifiedStatementValue(['object', 'definition', 'extensions', 'http://h5p.org/x-api/h5p-local-content-id']);
  }

  /**
   * Finds the global instance from content id by looking through the DOM
   *
   * @param {number} contentId Content id number
   * @returns {Object} Content instance
   */
  function findGlobalInstance (contentId){
    var $iframes = $('.h5p-iframe');
    var instances = $iframes.length > 0 ? $iframes[0].contentWindow.H5P.instances : H5P.instances;

    return instances.find(function(instance){
      return instance.contentId === contentId;
    });
  }

  /**
   * Get xAPI data for content type and put them in a form ready for storage.
   *
   * @param {Object} instance Content type instance
   */
  function storeXAPIData(instance){
    $('#quiz-h5p-result-key').val(JSON.stringify(getInstanceXAPIData(instance)));
  }

})(jQuery);
