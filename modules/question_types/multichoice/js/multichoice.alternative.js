(function ($, Drupal) {

  Drupal.behaviors.multichoiceAlternativeBehavior = {
    attach: function (context, settings) {
      $('.multichoice_row', context).once()
              .filter(':has(:checkbox:checked)')
              .addClass('selected').end()
              .click(function (event) {
                $(this).toggleClass('selected');
                if (event.target.type !== 'checkbox') {
                  $(':checkbox', this).attr('checked', function () {
                    return !this.checked;
                  });

                  $(':radio', this).attr('checked', true);
                  if ($(':radio', this).html() != null) {
                    $('.multichoice_row').removeClass('selected');
                    $(this).addClass('selected');
                  }
                }
              });
    }
  };

})(jQuery, Drupal);
