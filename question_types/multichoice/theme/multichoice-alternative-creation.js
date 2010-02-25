// $Id$
/**
 * @file
 * JS enabling one filter fieldset to controll filter formats for all textareas in alternatives.
 */
Drupal.behaviors.multichoiceBehavior = function(context) {
  // When the top input filter selector is clicked change the rest of the selectors to the same value
  $('.multichoice_filter:first :radio').click(function(){
	  var myValue = $(this).val();
    $('.multichoice_filter:not(:first) :radio[value='+myValue+']').click();
    $('.multichoice_filter:not(:first) :radio[value='+myValue+']').change();
  });
  
  // Change all format selectors to have the same value as the first
  var defaultValue = $('.multichoice_filter:first :radio[checked=1]').val();
  $('.multichoice_filter:not(:first):not(.multichoiceBehavior-processed) :radio[value='+defaultValue+']').click().change().addClass('multichoiceBehavior-processed');
  
  // Hide all format selectors except the first
  $('.multichoice_filter:not(:first)').hide().addClass('multichoiceStayHidden');
  
  // Move the first input selector to the input-all-ph helper tag
  $('.multichoice_filter:first').insertAfter('#input-all-ph');
  
  // Make sure the format selectors stay hidden when a fieldset is unfolded
  var oldToggle = Drupal.toggleFieldset;
  Drupal.toggleFieldset = function(context) {
    oldToggle(context);
    $('.multichoiceStayHidden').hide();
  };
};