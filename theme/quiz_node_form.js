// $Id$
/**
 * @file
 * JS enabling one filter fieldset to controll filter formats for all textareas in alternatives.
 */
Drupal.behaviors.quizFormBehavior = function(context) {
  $('.quiz_filter:first :radio').click(function(){
	var myValue = $(this).val();
    $('.quiz_filter:not(:first) :radio[value='+myValue+']').click();
    $('.quiz_filter:not(:first) :radio[value='+myValue+']').change();
  });
  var defaultValue = $('.quiz_filter:first :radio[checked=1]').val();
  $('.quiz_filter:not(:first):not(.quizFormBehavior-processed) :radio[value='+defaultValue+']').click().change().addClass('quizFormBehavior-processed');
  $('.quiz_filter:not(:first)').hide().addClass('quizStayHidden');
  var oldToggle = Drupal.toggleFieldset;
  Drupal.toggleFieldset = function(context) {
    oldToggle(context);
    $('.quizStayHidden').hide();
  };
};