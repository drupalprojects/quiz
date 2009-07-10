// $Id$

/**
 * supporting Javascript code for questions_import module
 */

$(document).ready(function() {
  // show separator field if the selected value is "native csv"
  if ($("#edit-import-type option:selected").val() === 'native_csv')
    $('#edit-field-separator-wrapper').show();
  // invoke these lines when the list selected item changes
  $('#edit-import-type').change(function() {
    // show separator field if the selected value is "native csv"
    if ($("#edit-import-type option:selected").val() === 'native_csv')
      $('#edit-field-separator-wrapper').show();
    else
      $('#edit-field-separator-wrapper').hide();
  });
});
