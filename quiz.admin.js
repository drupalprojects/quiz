// $Id$

/**
 * Scripts for Quiz administration.
 */

var Quiz = Quiz || {};

// Key should be either always or random.
Quiz.addQuestions = function (rowHtml) {
  //Add the new rows:
  $('#question-list tr:last').after(rowHtml);
  
  var table = Drupal.tableDrag['question-list'];
  
  $('.hidden-question').each(function(){
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
    var matches = remID.match(/[0-9]+-[0-9]+/);
    if (!matches || matches.length < 1) {
      return false;
    }
    $this.parents('tr').addClass('hidden-question');
    $('#edit-hiddens-' + matches[0]).val(1);
    $('#browser-'+ matches[0]).click();
    
    var table = Drupal.tableDrag['question-list'];
    if (!table.changed) {
      table.changed = true;
      $(Drupal.theme('tableDragChangedWarning')).insertAfter(table.table).hide().fadeIn('slow');
    }

    e.preventDefault();
    return true;
  });
};