// $Id$

/**
 * @file
 * Javascript functions for the quizQuestionBrowser
 */
var Quiz = Quiz || {};

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
    }
    var pattern = new RegExp('always-[0-9]+-[0-9]+');
	var idToShow = pattern.exec(this.id);
	$('#' + idToShow).toggleClass('hidden-question');
	$('#edit-hiddens-' + idToShow).val(($('#' + idToShow).hasClass('hidden-question')) ? 1 : 0);
	if (!$('#' + idToShow).hasClass('hidden-question')) {
      Quiz.fixColorAndWeight($('#' + idToShow));
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
  $('.pager-item a, .pager-first a, .pager-next a, .pager-previous a, .pager-last a')
  .addClass('quizQuestionBrowserBehavior-processed')
  .click(function(event){
	var myUrl = $(this).attr('href').substr(2);
	Quiz.updatePageInUrl(myUrl);
    $('#edit-always-browser-filters-title').trigger('doneTyping');
    event.preventDefault();
  });
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

Quiz.addBrowserRows = function(rows, newBuildId, pager, hiddenRows) {
  //Add the new row:
  $('.hidden-question').remove();
  
  $('#quiz_question_browser_filters').after(rows);
  $('#quiz-question-browser-pager').replaceWith(pager);
  //var newRow = $('#questions-order-' + statusCode + ' tr:last').get(0);
  
  // Change build id to the new id provided by the server:
  $('[name="form_build_id"]').val(newBuildId);
  
  
  Drupal.behaviors.quizQuestionBrowserBehavior();
};
Quiz.replaceBrowser = function(renderedBrowser, newBuildId, hiddenRows) {
  // Change build id to the new id provided by the server:
  $('.hidden-question').remove();
  $('[name="form_build_id"]').val(newBuildId);
  $('#quiz-browser-all-ahah-target').replaceWith(renderedBrowser);
  
  Drupal.attachBehaviors();
};
Quiz.updatePageInUrl = function(myUrl) {
  //Finds page from input parameter
  var pageQuery = myUrl + '';
  var pattern = new RegExp('page=[0-9]+');
  pageQuery = pattern.exec(pageQuery);
  if (pageQuery == null) pageQuery = 'page=0';
  
  //Replaces stored query strings page with our page
  var currentQuery = $('#edit-always-browser-add-to-get').val() + '';
  currentQuery = currentQuery.replace(pattern,'');
  currentQuery += pageQuery;
  $('#edit-always-browser-add-to-get').val(currentQuery);
};
Quiz.fixColorAndWeight = function(newest) {
  var nextClass = 'odd';
  var lastClass = 'even';
  var lastWeight = 0;
  newest.remove();
  var lastQuestion = null;
  $('.q-row').each(function() {
    if (!$(this).hasClass('hidden-question') && $(this) != newest) {
      // Color:
      if (!$(this).hasClass(nextClass)) $(this).removeClass(lastClass).addClass(nextClass);
      var currentClass = nextClass;
      nextClass = lastClass;
      lastClass = currentClass;
      lastQuestion = $(this);
      // Weight:
      var myId = $(this).attr('id') + '';
      var weightField = $('#edit-weights-' + myId);
      weightField.val(lastWeight);
      lastWeight++;
    }
  });
  if (!newest.hasClass(nextClass)) newest.removeClass(lastClass).addClass(nextClass);
  var newestId = newest.attr('id');
  lastQuestion.after(newest);
  $('#edit-weights-' + newestId).val(lastWeight);
};
