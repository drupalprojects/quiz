// $id$
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
  var colText = selection.options[selection.selectedIndex].text;
  var numberOfOptions = findValueInsideBrackets(colText);
  for(var i = 0; i<numberOfOptions;i++){
	var el = $('#edit-' + colId + '-' + i);
	$('#edit-alternative' + (i)).val(el.val());
  }
}

function clearAlternatives() {
  for ( var i = 0; i < 10; i++) {
	$('#edit-alternative' + (i)).val('');
  }
}

function findValueInsideBrackets(theString){
	var pattern = new RegExp("[(]([1-9]{1,2})[)]$");
	pattern.test(theString);
	return RegExp.lastParen;
}