// $Id$

/**
 * Scripts for Quiz administration.
 */

var Quiz = Quiz || {};

// Key should be either always or random.
Quiz.addQuestions = function (key, rowHtml) {
  var statusCode = (key == 'always' ? 1 : 2); // e.g. QUIZ_ALWAYS, QUIZ_RANDOM
  
  //Add the new row:
  $('#questions-order-' + statusCode + ' tr:last').after(rowHtml);
  
  var table = Drupal.tableDrag['questions-order-' + statusCode];
  
  $('.quiz-temp, .hidden-question').each(function(){
	//Hide weight column:
    $('td:last', this).css('display', 'none');
  table.makeDraggable(this);
  });
  
  if (table.changed == false) {
    table.changed = true;
    $(Drupal.theme('tableDragChangedWarning')).insertAfter(table.table).hide().fadeIn('slow');
  }
  
  Drupal.attachBehaviors();
};

Drupal.behaviors.attachRemoveAction = function () {
  $('.rem-link:not(attachRemoveAction-processed)')
  .addClass('attachRemoveAction-processed')
  .click(function (e) {
    var $this = $(this);
    var remID = $this.parents('tr').attr('id');
    var matches = remID.match(/(always|random)-[0-9]+-[0-9]+/);
    if (!matches || matches.length < 1) {
      return false;
    }

    var statusCode = (matches[1] == 'always') ? 1 : 0;      
    
    $this.parents('tr').addClass('hidden-question');
    $('#edit-hiddens-' + remID).val(1);
    
    $('#browser-' + remID).click();
    
    var table = Drupal.tableDrag['questions-order-' + statusCode];
    if (!table.changed) {
      table.changed = true;
      $(Drupal.theme('tableDragChangedWarning')).insertAfter(table.table).hide().fadeIn('slow');
    }

    e.preventDefault();
    return true;
  });
};

$(document).ready(function () {

  // Stupid hack to get around bug in tableDrag (collapsed tableDrag tables cannot have hidden Weight fields)
  $('fieldset.collapsible:last>legend>a').click();

  // Effectively bind the autocomplete submit handler to the
  // "Add question" button's submit handler (for both autocomplete fields).


  $('#edit-always-autocomplete,#edit-random-autocomplete').keypress(function (e) {
      if (e.which == 13) {

        /* We could do something like this....
        $this = $(this);
        if ($this.val().length > 0) {
          Quix.addQuestion();
        }
        */

        // Kill the 'return' handler before the form gets accidentally submitted.
        e.preventDefault();
        e.stopPropagation();
        return true;
      }
  });
});