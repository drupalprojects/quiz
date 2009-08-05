// $Id$
/**
 * @file
 * JS enabling one filter fieldset to controll filter formats for all textareas in alternatives.
 */
Drupal.behaviors.choiceBehavior = function(context) {
  $('.choice_filter:first :radio').click(function(){
	  var myValue = $(this).val();
    $('.choice_filter:not(:first) :radio[value='+myValue+']').click();
    $('.choice_filter:not(:first) :radio[value='+myValue+']').change();
  });
  var defaultValue = $('.choice_filter:first :radio[checked=1]').val();
  $('.choice_filter:not(:first):not(.choiceBehavior-processed) :radio[value='+defaultValue+']').click().change().addClass('choiceBehavior-processed');
  $('.choice_filter:not(:first)').hide().addClass('choiceStayHidden');
  var oldToggle = Drupal.toggleFieldset;
  Drupal.toggleFieldset = function(context) {
    oldToggle(context);
    $('.choiceStayHidden').hide();
  };
};