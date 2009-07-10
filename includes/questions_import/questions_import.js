// $Id$

/**
 * supporting Javascript code for questions_import module
 */

$(document).ready(function() {
	$('#edit-import-type').change(function() {
		selectionName = $("#edit-import-type option:selected").text();
		hasSeparator = selectionName.indexOf("Separated") >= 0;
		if (hasSeparator)
			$('#edit-field-separator-wrapper').show();
		else
			$('#edit-field-separator-wrapper').hide();
	});
});
