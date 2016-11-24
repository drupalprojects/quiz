(function ($) {
  $(document).ready(function () {
    if (H5P && H5P.externalDispatcher) {
      // Get xAPI data initially
      H5P.externalDispatcher.once('domChanged', function () {
        storeXAPIData(this);
      });

      // Get xAPI data every time it changes
      H5P.externalDispatcher.on('xAPI', function() {
        storeXAPIData(this);
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
   * Get xAPI data for content type and put them in a form ready for storage.
   *
   * @param {Object} instance Content type instance
   */
  function storeXAPIData(instance){
    $('#quiz-h5p-result-key').val(JSON.stringify(getInstanceXAPIData(instance)));
  }

})(jQuery);
