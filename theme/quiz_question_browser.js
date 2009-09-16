// $Id$

/**
 * @file
 * Javascript functions for the quizQuestionBrowser
 */
var Quiz = Quiz || {};

Drupal.behaviors.quizQuestionBrowserBehavior = function(context) {
  var done = 'quizQuestionBrowserBehavior-processed';
  var notDone = ':not(.'+ done +')';
  // Question rows in the browser
  $('.quiz_question_browser_row'+ notDone)
  .addClass(done)
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
	if ($(this).hasClass('selected')) {
      $('#' + idToShow).removeClass('hidden-question');
	} else {
      $('#' + idToShow).addClass('hidden-question');
	}
	$('#edit-hiddens-' + idToShow).val(($('#' + idToShow).hasClass('hidden-question')) ? 1 : 0);
	if (!$('#' + idToShow).hasClass('hidden-question')) {
      Quiz.fixColorAndWeight($('#' + idToShow));
	}
  });
  
  // Filter row in the browser
  
  // Mark all button
  $('#edit-always-browser-filters-all'+ notDone)
  .addClass(done)
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
  
  // Type and date filters
  $('#edit-always-browser-filters-type'+ notDone +', #edit-always-browser-filters-changed'+ notDone)
  .addClass(done)
  .change(function(event) {
    $('.quiz_question_browser_row').each(function() { 
      $(this).remove();
    });
    $('.quiz_question_browser_filters').after('<TR id="quiz-question-browser-searching"><TD colspan="5">Searching...</TD></TR>');
  });
  var quizRefreshId;
  
  //Title and username filters
  $('#edit-always-browser-filters-title'+ notDone +', #edit-always-browser-filters-name'+ notDone)
  .addClass(done)
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
  
  // Sorting TODO: Merge all the sortings into one...
  
  // Sort by title
  $('.quiz-browser-header-title > a'+ notDone)
  .addClass(done)
  .click(function(event){
	var myUrl = $(this).attr('href').substr(2);
	$('#edit-always-browser-add-to-get').val(myUrl);
	$('#edit-always-browser-filters-title').trigger('doneTyping');
    event.preventDefault();
  });
  
  // Sort by username
  $('.quiz-browser-header-name > a'+ notDone)
  .addClass(done)
  .click(function(event){
	var myUrl = $(this).attr('href').substr(2);
	$('#edit-always-browser-add-to-get').val(myUrl);
	$('#edit-always-browser-filters-name').trigger('doneTyping');
    event.preventDefault();
  });
  
  // Sort by type
  $('.quiz-browser-header-type > a'+ notDone)
  .addClass(done)
  .click(function(event){
	var myUrl = $(this).attr('href').substr(2);
	$('#edit-always-browser-add-to-get').val(myUrl);
	$('#edit-always-browser-filters-type').trigger('change');
    event.preventDefault();
  });
  
  // Sort by date
  $('.quiz-browser-header-changed > a'+ notDone)
  .addClass(done)
  .click(function(event){
	var myUrl = $(this).attr('href').substr(2);
	$('#edit-always-browser-add-to-get').val(myUrl);
	$('#edit-always-browser-filters-changed').trigger('change');
    event.preventDefault();
  });
  
  // Pager
  $('.pager-item a'+ notDone +', .pager-first a'+ notDone +', .pager-next a'+ notDone +', .pager-previous a'+ notDone +', .pager-last a'+ notDone)
  .addClass(done)
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
  newest.insertAfter('#'+ lastQuestion.attr('id'));
  $('#edit-weights-' + newestId).val(lastWeight);
  Drupal.attachBehaviors();
};
