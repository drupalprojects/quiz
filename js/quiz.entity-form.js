(function ($, Drupal) {

  /**
   * JS enabling one filter fieldset to control filter formats for all textareas
   * in alternatives.
   */
  Drupal.behaviors.quizFormBehavior = {
    attach: function (context) {
      $('.quiz-filter:first :radio', context).click(function () {
        $('.quiz-filter:not(:first) :radio[value=' + $(this).val() + ']').click();
        $('.quiz-filter:not(:first) :radio[value=' + $(this).val() + ']').change();
      });

      var defaultValue = $('.quiz-filter:first :radio[checked=1]').val();
      $('.quiz-filter:not(:first):not(.quizFormBehavior-processed) :radio[value=' + defaultValue + ']', context)
              .click()
              .change()
              .addClass('quizFormBehavior-processed');

      $('.quiz-filter:not(:first)')
              .hide()
              .addClass('quizStayHidden');

      var oldToggle = Drupal.toggleFieldset;
      Drupal.toggleFieldset = function (context) {
        oldToggle(context);
        $('.quizStayHidden').hide();
      };
    }
  };

})(jQuery, Drupal);
