// $Id$
/**
 * @file
 * JS enabling the quiz taker to click anywhere in a table row to mark a checkbox/radio button.
 */
Drupal.behaviors.choiceAlternativeBehavior = function(context) {
  $('.choice_row')
  .filter(':has(:checkbox:checked)')
  .addClass('selected')
  .end()
  .click(function(event) {
    $(this).toggleClass('selected');
    if (event.target.type !== 'checkbox') {
      $(':checkbox', this).attr('checked', function() {
        return !this.checked;
      });
      $(':radio', this).attr('checked', true);
      if ($(':radio', this).html() != null) {
        $('.choice_row').removeClass('selected');
    	  $(this).addClass('selected');
      }
    }
  });
};