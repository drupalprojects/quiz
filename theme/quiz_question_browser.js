// $Id$

/**
 * @file
 * Javascript functions for the quizQuestionBrowser
 */
var Quiz = Quiz || {};
Quiz.questionsToAdd = '';

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
  $('#edit-always-browser-filters-type:not(.quizQuestionBrowserBehavior-processed), #edit-always-browser-filters-changed:not(.quizQuestionBrowserBehavior-processed)')
  .addClass('quizQuestionBrowserBehavior-processed')
  .change(function(event) {
    $('.quiz_question_browser_row').each(function() { 
      $(this).remove();
    });
    $('.quiz_question_browser_filters').after('<TR id="quiz-question-browser-searching"><TD colspan="5">Searching...</TD></TR>');
  });
  var quizRefreshId;
  $('#edit-always-browser-filters-title:not(.quizQuestionBrowserBehavior-processed), #edit-always-browser-filters-name:not(.quizQuestionBrowserBehavior-processed)')
  .addClass('quizQuestionBrowserBehavior-processed')
  .keyup(function(event) {
	clearInterval(quizRefreshId);
	var quizClicked = this;
    quizRefreshId = setInterval(function(){
      $('.quiz_question_browser_row').each(function() { 
        $(this).remove();
      });
      $('.quiz_question_browser_filters').after('<TR id="quiz-question-browser-searching"><TD colspan="5">Searching...</TD></TR>');
      $(quizClicked).trigger('doneTyping');
      clearInterval(quizRefreshId);
    }, 1000);
  });
  $('.quiz-browser-header-title > a:not(.quizQuestionBrowserBehavior-processed)')
  .addClass('quizQuestionBrowserBehavior-processed')
  .click(function(event){
	var myUrl = $(this).attr('href').substr(2);
	$('#edit-always-browser-add-to-get').val(myUrl);
	Quiz.storeCheckboxes();
	$('#edit-always-browser-filters-title').trigger('doneTyping');
    event.preventDefault();
  });
  $('.quiz-browser-header-name > a:not(.quizQuestionBrowserBehavior-processed)')
  .addClass('quizQuestionBrowserBehavior-processed')
  .click(function(event){
	var myUrl = $(this).attr('href').substr(2);
	$('#edit-always-browser-add-to-get').val(myUrl);
	$('#edit-always-browser-filters-name').trigger('doneTyping');
    event.preventDefault();
  });
  $('.quiz-browser-header-type > a:not(.quizQuestionBrowserBehavior-processed)')
  .addClass('quizQuestionBrowserBehavior-processed')
  .click(function(event){
	var myUrl = $(this).attr('href').substr(2);
	$('#edit-always-browser-add-to-get').val(myUrl);
	$('#edit-always-browser-filters-type').trigger('change');
    event.preventDefault();
  });
  $('.quiz-browser-header-changed > a:not(.quizQuestionBrowserBehavior-processed)')
  .addClass('quizQuestionBrowserBehavior-processed')
  .click(function(event){
	var myUrl = $(this).attr('href').substr(2);
	$('#edit-always-browser-add-to-get').val(myUrl);
	$('#edit-always-browser-filters-changed').trigger('change');
    event.preventDefault();
  });
  $('#edit-always-browser-questions-to-add').val(Quiz.questionsToAdd);
};
$(document).ready(function () {
  var oldTableHeader = Drupal.behaviors.tableHeader;
  Drupal.behaviors.tableHeader = function(context) {
    if (!$('table.sticky-enabled', context).size()) {
	  return;
	}
    oldTableHeader(context);
  };
});

Quiz.addBrowserRows = function(rows, newBuildId, pager) {
  //Add the new row:
  $('#quiz_question_browser_filters').after(rows);
  $('#quiz-question-browser-pager').replaceWith(pager);
  //var newRow = $('#questions-order-' + statusCode + ' tr:last').get(0);
  
  // Change build id to the new id provided by the server:
  $('[name="form_build_id"]').val(newBuildId);
  
  
  Drupal.behaviors.quizQuestionBrowserBehavior();
};
Quiz.replaceBrowser = function(renderedBrowser, newBuildId) {
  // Change build id to the new id provided by the server:
  $('[name="form_build_id"]').val(newBuildId);
  $('#quiz-browser-all-ahah-target').replaceWith(renderedBrowser);
  
  Drupal.attachBehaviors();
};
Quiz.storeCheckboxes = function() {
  $('.quiz-browser-checkbox').each(function() {
    if ($(this).attr('checked')) {
      var oldVal = $('#edit-always-browser-questions-to-add').val() + '';
      var newVal = $(this).val() + '';
      if (oldVal.length > 0) {
    	$('#edit-always-browser-questions-to-add').val(oldVal + ',' + newVal);
      } else {
    	$('#edit-always-browser-questions-to-add').val(newVal);
      }
    }
  });
  Quiz.questionsToAdd = $('#edit-always-browser-questions-to-add').val();
};
