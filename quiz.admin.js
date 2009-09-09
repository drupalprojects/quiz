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
  
  $('.quiz-temp').each(function(){
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
  $('.rem-link').click(function (e) {
    var $this = $(this);
    var remID = $this.parents('tr').find('.question-order-weight').attr('id');

    var matches = remID.match(/edit-weights-([a-zA-Z]+)-([0-9]+)-([0-9]+)/);
    if (!matches || matches.length < 4) {
      return false;
    }

    var remItem = matches[1] + '-' + matches[2] + '-' + matches[3];
    var statusCode = (matches[1] == 'always') ? 1 : 0;

    var remList = $('#edit-remove-from-quiz');
    var orig = remList.val();
    remList.val(remItem + ',' + orig);

    $this.parents('tr').remove();

    var table = Drupal.tableDrag['questions-order-' + statusCode];
    if (!table.changed) {
      table.changed;
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