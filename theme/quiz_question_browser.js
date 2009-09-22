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
  $('.quiz-question-browser-row'+ notDone)
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
	var idToShow = Quiz.findNidVidString(this.id);
	if ($(this).hasClass('selected')) {
      $('#q-' + idToShow).removeClass('hidden-question');
	} else {
      $('#q-' + idToShow).addClass('hidden-question');
	}
	$('#edit-hiddens-' + idToShow).val(($('#q-' + idToShow).hasClass('hidden-question')) ? 1 : 0);
    Quiz.fixColorAndWeight($('#q-' + idToShow));
  });
  
  // Filter row in the browser
  
  // Mark all button
  this.selector = '#edit-browser-table-filters-all'+ notDone;
  $(this.selector)
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
  this.selector = '#edit-browser-table-filters-type'+ notDone;
  this.selector += ', #edit-browser-table-filters-changed'+ notDone;
  $(this.selector)
  .addClass(done)
  .change(function(event) {
    $('.quiz-question-browser-row').each(function() { 
      $(this).remove();
    });
    $('.quiz_question_browser_filters').after('<TR id="quiz-question-browser-searching"><TD colspan="5">Searching...</TD></TR>');
  });
  var quizRefreshId;
  
  //Title and username filters
  this.selector = '#edit-browser-table-filters-title'+ notDone;
  this.selector += ', #edit-browser-table-filters-name'+ notDone;
  $(this.selector)
  .addClass(done)
  .keyup(function(event) {
	clearInterval(quizRefreshId);
	var quizClicked = this;
    quizRefreshId = setInterval(function(){
      $('.quiz-question-browser-row').each(function() { 
        $(this).remove();
      });
      $('.quiz_question_browser_filters').after('<TR id="quiz-question-browser-searching"><TD colspan="5">Searching...</TD></TR>');
      $(quizClicked).trigger('doneTyping');
      clearInterval(quizRefreshId);
    }, 1000);
  });
  
  // Sorting TODO: Merge all the sortings into one...
  var toSort = [
    {
      name: 'title',
      event: 'doneTyping'
    },
    {
      name: 'name',
      event: 'doneTyping'
    },
    {
      name: 'type',
      event: 'change'
    },
    {
      name: 'changed',
      event: 'change'
    }
  ];
  
  for (i in toSort) {
    $('.quiz-browser-header-'+ toSort[i].name +' > a'+ notDone)
    .addClass(done)
    .attr('myName', toSort[i].name)
    .attr('myEvent', toSort[i].event)
    .click(function(event) {
      var myUrl = $(this).attr('href').substr(2);
      $('#edit-browser-table-add-to-get').val(myUrl);
      $('#edit-browser-table-filters-'+ $(this).attr('myName')).trigger($(this).attr('myEvent'));
      event.preventDefault();
    });
  }
  
  // Pager
  $('.pager-item a'+ notDone +', .pager-first a'+ notDone +', .pager-next a'+ notDone +', .pager-previous a'+ notDone +', .pager-last a'+ notDone)
  .addClass(done)
  .click(function(event){
	var myUrl = $(this).attr('href').substr(2);
	Quiz.updatePageInUrl(myUrl);
    $('#edit-browser-table-filters-title').trigger('doneTyping');
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
  
  $('.quiz_question_browser_row:has(:checkbox:checked)').each(function() {
    $(this).click();
  });
});
Quiz.addBrowserRows = function(rows, newBuildId, pager, hiddenRows) {
  //Add the new row:
  $('.hidden-question').remove();
  
  $('#quiz-question-browser-filters').after(rows);
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
  $('#all-ahah-target').replaceWith(renderedBrowser);
  Drupal.attachBehaviors($('#all-ahah-target'));
};
Quiz.updatePageInUrl = function(myUrl) {
  //Finds page from input parameter
  var pageQuery = myUrl + '';
  var pattern = new RegExp('page=[0-9]+');
  pageQuery = pattern.exec(pageQuery);
  if (pageQuery == null) pageQuery = 'page=0';
  
  //Replaces stored query strings page with our page
  var currentQuery = $('#edit-browser-table-add-to-get').val() + '';
  currentQuery = currentQuery.replace(pattern,'');
  currentQuery += pageQuery;
  $('#edit-browser-table-add-to-get').val(currentQuery);
};
Quiz.fixColorAndWeight = function(newest) {
  var nextClass = 'odd';
  var lastClass = 'even';
  var lastWeight = 0;
  var lastQuestion = null;
  $('.q-row').each(function() {
    if (!$(this).hasClass('hidden-question') && $(this).attr('id') != newest.attr('id')) {
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
  var newestId = Quiz.findNidVidString(newest.attr('id'));
  newest.insertAfter('#q-'+ Quiz.findNidVidString(lastQuestion.attr('id')));
  $('#edit-weights-' + newestId).val(lastWeight);
  var marker = Drupal.theme('tableDragChangedMarker');
  var cell = $('td:first', newest);
  if ($('span.tabledrag-changed', cell).length == 0) {
    cell.append(marker);
  }
  var table = Drupal.tableDrag['question-list'];
  if (!table.changed) {
    table.changed = true;
    $(Drupal.theme('tableDragChangedWarning')).insertAfter(table.table).hide().fadeIn('slow');
  }
  //Drupal.attachBehaviors();
};
Quiz.findNidVidString = function(str) {
  var pattern = new RegExp('[0-9]+-[0-9]+');
  return pattern.exec(str);
};