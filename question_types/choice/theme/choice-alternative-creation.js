Drupal.behaviors.choiceBehavior = function(context) {
  $('.choice_filter:not(:first)').hide();
  $('.choice_filter:first :radio').click(function(){
	var myValue = $(this).val();
    $('.choice_filter:not(:first) :radio[value='+myValue+']').click();
    $('.choice_filter:not(:first) :radio[value='+myValue+']').change();
  });
  var defaultValue = $('.choice_filter:first :radio[checked=1]').val();
  $('.choice_filter:not(:first) :radio[value='+defaultValue+']').click();
  $('.choice_filter:not(:first) :radio[value='+defaultValue+']').change();
};