<?php 
// $Id$
print '<p>'. $content['explanation'] .'</p>';
print '<p>';
$counter = 1;
foreach ($content['links'] as $key => $value) {
  print l(t('revision !num', array('!num' => $counter++)),$value);
}
print '</p>';
?>