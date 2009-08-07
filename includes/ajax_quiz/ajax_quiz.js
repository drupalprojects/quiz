Drupal.AjaxLoadExample = Drupal.AjaxLoadExample || {};

/**
 * Ajax load example behavior.
 */
Drupal.behaviors.AjaxLoadExample = function (context) {
  $('.form-submit:not(.form-submit-clicked)', context)
    .each(function () {
      // The target should not be e.g. a node that will itself be
      // replaced, as this would mean no node is available for
      // ajax_load to attach behaviors to when all scripts are loaded.
      var target = this.parentNode;
      $(this)
        .addClass('form-submit-clicked')
        .click(function () {
          if ($('[name=tries]:not(.form-submitted)').attr('class') == 'form-radio') {
            var post_tries = $('[name=tries]:checked:not(.form-submitted)').val();
            $('[name=tries]:not(.form-submitted)').addClass('form-submitted');
          }
          else if ($('[name=tries]:not(.form-submitted)').attr('class') == 'form-text') {
            var post_tries = $('.form-text:not(.form-submitted)').val();
            $('[name=tries]:not(.form-submitted)').addClass('form-submitted');
          }
          else if ($('[name=tries]:not(.form-submitted)').attr('class') == 'form-textarea resizable textarea-processed') {
            var post_tries = $('.form-textarea:not(.form-submitted)').val();
            $('[name=tries]:not(.form-submitted)').addClass('form-submitted');
          }
          else {
            // unkonwn form type, unable to get value
            post_tries = 'error';
          }
          $.ajax({
            // Either GET or POST will work.
            type: 'POST',
            data: 'ajax_load_example=1&op=Next&tries='+post_tries,
            // Need to specify JSON data.
            dataType: 'json',
            url: $(this).attr('href'),
            success: function(response) {
              // Call all callbacks.
              //alert('sucess');
              if (response.__callbacks) {
                $.each(response.__callbacks, function(i, callback) {
                  // The first argument is a target element, the second
                  // the returned JSON data.
                  //alert(response);
                  eval(callback)(target, response);
                });
                // If you don't want to return this module's own callback,
                // you could of course just call it directly here.
                // Drupal.AjaxLoadExample.formCallback(target, response);
              }
            },
            error: function() {
              alert('error');
            },
          });
          return false;
        });
    });
  $('a.ajax-load-example:not(.ajax-load-example-processed)', context)
    .each(function () {
      // The target should not be e.g. a node that will itself be
      // replaced, as this would mean no node is available for
      // ajax_load to attach behaviors to when all scripts are loaded.
      var target = this.parentNode;
      $(this)
        .addClass('ajax-load-example-processed')
        .click(function () {
          $.ajax({
            // Either GET or POST will work.
            type: 'POST',
            data: 'ajax_load_example=1',
            // Need to specify JSON data.
            dataType: 'json',
            url: $(this).attr('href'),
            success: function(response){
              // Call all callbacks.

              if (response.__callbacks) {
                $.each(response.__callbacks, function(i, callback) {
                  // The first argument is a target element, the second
                  // the returned JSON data.
                  eval(callback)(target, response);
                });
                // If you don't want to return this module's own callback,
                // you could of course just call it directly here.
                // Drupal.AjaxLoadExample.formCallback(target, response);
              }
            },
            error: function(){
              alert('An error has occurred. Please try again.');
            },
          });
          return false;
        });
    });
};

/**
 * Ajax load example callback.
 */
Drupal.AjaxLoadExample.formCallback = function (target, response) {
  target = $(target).append(response.content);
  Drupal.attachBehaviors(target);
};


