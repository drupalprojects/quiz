/**
 * @file
 * Javascript functions for the scale question type.
 */

/**
 * Refreshes alternatives when a preset is selected.
 * 
 * @param selection
 */
function refreshAlternatives(selection) {
  clearAlternatives();
  var colId = selection.options[selection.selectedIndex].value;
  var numberOfOptions = scaleCollections[colId].length;
  for(var i = 0; i<numberOfOptions;i++){
	$('#edit-alternative' + (i)).val(scaleCollections[colId][i]);
  }
}

function clearAlternatives() {
  for ( var i = 0; i < 10; i++) {
	$('#edit-alternative' + (i)).val('');
  }
}