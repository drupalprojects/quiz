// $Id$
/**
 * @file
 * Javascript functions for the choice question type.
 */
/**
 * Refreshes scores when the checkbox named correct is toggled.
 * 
 * @param selection
 */
function refreshScores(checkbox) {
  var prefix = '#' + getCorrectIdPrefix(checkbox.id);
  if (checkbox.checked) {
    $(prefix + 'score-if-chosen').val('1');
    $(prefix + 'score-if-not-chosen').val('0');
  }
  else {
	$(prefix + 'score-if-chosen').val('0');
	if ($('#edit-alternatives-multi').attr('checked')) {
	  $(prefix + 'score-if-not-chosen').val('1');
	} 
	else {
	  $(prefix + 'score-if-not-chosen').val('0');
	}
  }
}
function refreshCorrect(textfield) {
  var prefix = '#' + getCorrectIdPrefix(textfield.id);
  var chosenScore;
  var notChosenScore;
  if (isChosen(textfield.id)) {
    chosenScore = new Number(textfield.value);
    notChosenScore = new Number($(prefix + 'score-if-not-chosen').val());
  }
  else {
    chosenScore = new Number($(prefix + 'score-if-chosen').val());
    notChosenScore = new Number(textfield.value);
  }
  if(notChosenScore < chosenScore) {
	
    $(prefix + 'correct').attr('checked', true);
  }
  else {
    
    $(prefix + 'correct').attr('checked', false);
  }
}
function getCorrectIdPrefix(string) {
  var pattern = new RegExp("^(edit-alternatives-alternative[0-9]{1,2}-)(?:correct|score-if-(?:not-|)chosen)$");
  pattern.exec(string);
  return RegExp.lastParen;
}
function isChosen(string) {
  var pattern = new RegExp("score-if-chosen$");
  return pattern.test(string);
}

