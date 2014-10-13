(function ($) {
  Drupal.behaviors.quizJumper = {
    attach: function (context) {
      $("#quiz-jumper:not(.quizJumper-processed)", context).show().addClass("quizJumper-processed").change(function () {
        $("#quiz-jumper #edit-submit").trigger("click");
      });
      $("#quiz-jumper-no-js:not(.quizJumper-processed)").hide().addClass("quizJumper-processed");
    }
  };
})(jQuery);
