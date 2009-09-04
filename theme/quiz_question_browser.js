// $Id$

/**
 * @file
 * Javascript functions for the quizQuestionBrowser
 */

Drupal.behaviors.quizQuestionBrowserBehavior = function(context) {
  $('.quiz_question_browser_row')
  .filter(':has(:checkbox:checked)')
  .addClass('selected')
  .end()
  .click(function(event) {
    $(this).toggleClass('selected');
    if (event.target.type !== 'checkbox') {
      $(':checkbox', this).attr('checked', function() {
        return !this.checked;
      });
      $(':radio', this).attr('checked', true);
      if ($(':radio', this).html() != null) {
        $('.multichoice_row').removeClass('selected');
    	  $(this).addClass('selected');
      }
    }
  });
  $('#edit-always-browser-filters-all')
  .click(function(event) {
    var ch = $(this).attr('checked');
    $('.quiz_question_browser_row').each(function() { 
      if (!ch) {
        $(this).filter(':has(:checkbox:checked)').each(function() {
          $(this).click();
        });
      }
      else {
        $(this).filter(':has(:checkbox:not(:checked))').each(function() {
          $(this).click();
        });
      }
    });
  });
};