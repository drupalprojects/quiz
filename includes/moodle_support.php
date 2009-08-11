<?php
/**
 * Mimick Moodle environment so its code can function here in Drupal
 */

global $CFG;
$CFG = new stdClass();
$CFG->dirroot = drupal_get_path('module', 'quiz') .'/includes/moodle';
$CFG->libdir = drupal_get_path('module', 'quiz') .'/includes/moodle/lib';
$CFG->dataroot = file_directory_path();
$CFG->directorypermissions = 0777;
$CFG->zip = '/usr/bin/zip'; // FIXME doesn't work on Windows



/** Converts the text format from the value to the 'internal'
 *  name or vice versa. $key can either be the value or the name
 *  and you get the other back.
 *
 *  @param mixed int 0-4 or string one of 'moodle','html','plain','markdown'
 *  @return mixed as above but the other way around!
 */
function text_format_name( $key ) {
  $lookup = array();
  $lookup[FORMAT_MOODLE] = 'moodle';
  $lookup[FORMAT_HTML] = 'html';
  $lookup[FORMAT_PLAIN] = 'plain';
  $lookup[FORMAT_MARKDOWN] = 'markdown';
  $lookup[FORMAT_LATEX] = 'latex'; // added by turadg 2009-05-26
  $value = "error";
  if (!is_numeric($key)) {
    $key = strtolower( $key );
    $value = array_search( $key, $lookup );
  }
  else {
    if (isset( $lookup[$key] )) {
      $value =  $lookup[ $key ];
    }
  }
  return $value;
}

/**#@+
 * The core question types.
 */
// from moodle/lib/questionlib.php
define("SHORTANSWER",   "shortanswer");
define("TRUEFALSE",     "truefalse");
define("MULTICHOICE",   "multichoice");
define("RANDOM",        "random");
define("MATCH",         "match");
define("RANDOMSAMATCH", "randomsamatch");
define("DESCRIPTION",   "description");
define("NUMERICAL",     "numerical");
define("MULTIANSWER",   "multianswer");
define("CALCULATED",    "calculated");
define("ESSAY",         "essay");
/**#@-*/

/**
 * Moodle text formats from moodle/lib/weblib.php
 * plus LaTeX format
 */
define('FORMAT_MOODLE',   0);   // Does all sorts of transformations and filtering
define('FORMAT_HTML',     1);   // Plain HTML (with some tags stripped)
define('FORMAT_PLAIN',    2);   // Plain text (even tags are printed in full)
define('FORMAT_WIKI',     3);   // Wiki-formatted text @deprecated
define('FORMAT_MARKDOWN', 4);   // Markdown-formatted text http://daringfireball.net/projects/markdown/
define('FORMAT_LATEX',    314);


/**
 * Moodle localized string function from moodle/lib/moodlelib.php
 * reimplemented hackedly for within the Drupal Quiz module
 * e.g. get_string("wronggrade", "quiz", $nLineCounter).' '.get_string("fractionsnomax", "quiz", $maxfraction);
 *
 * @param string $identifier The key identifier for the localized string
 * @param string $module The module where the key identifier is stored, usually expressed as the filename in the language pack without the .php on the end but can also be written as mod/forum or grade/export/xls.  If none is specified then moodle.php is used.
 * @param mixed $a An object, string or number that can be used
 * within translation strings
 * @param array $extralocations An array of strings with other locations to look for string files
 * @return string The localized string.
 */
function get_string($identifier, $module='', $a=NULL, $extralocations=NULL) {
  assert($module=='quiz');

  /// if $a happens to have % in it, double it so sprintf() doesn't break
  if ($a) {
      $a = clean_getstring_data( $a );
  }

  global $CFG;
  $langfile = "$CFG->dirroot/lang/en_utf8/$module.php";
  if (file_exists($langfile)) {
    if ($result = get_string_from_file($identifier, $langfile, "\$resultstring")) {
      if (eval($result) === FALSE) {
        trigger_error('Lang error: '.$identifier.':'.$langfile, E_USER_NOTICE);
      }
      return $resultstring;
    }
  }

  // last resort
  return '[['. $identifier .']]';
}

/**
 * This function is only used from {@link get_string()}.
 *
 * @internal Only used from get_string, not meant to be public API
 * @param string $identifier ?
 * @param string $langfile ?
 * @param string $destination ?
 * @return string|false ?
 * @staticvar array $strings Localized strings
 * @access private
 * @todo Finish documenting this function.
 */
// from moodle/lib/moodlelib.php
function get_string_from_file($identifier, $langfile, $destination) {

    static $strings;    // Keep the strings cached in memory.

    if (empty($strings[$langfile])) {
        $string = array();
        include ($langfile);
        $strings[$langfile] = $string;
    } else {
        $string = &$strings[$langfile];
    }

    if (!isset ($string[$identifier])) {
        return false;
    }

    return $destination .'= sprintf("'. $string[$identifier] .'");';
}


/**
 * make_unique_id_code
 *
 * @param string $extra ?
 * @return string
 * @todo Finish documenting this function
 */
// from moodle/lib/moodlelib.php
function make_unique_id_code($extra='') {

    $hostname = 'unknownhost';
    if (!empty($_SERVER['HTTP_HOST'])) {
        $hostname = $_SERVER['HTTP_HOST'];
    } else if (!empty($_ENV['HTTP_HOST'])) {
        $hostname = $_ENV['HTTP_HOST'];
    } else if (!empty($_SERVER['SERVER_NAME'])) {
        $hostname = $_SERVER['SERVER_NAME'];
    } else if (!empty($_ENV['SERVER_NAME'])) {
        $hostname = $_ENV['SERVER_NAME'];
    }

    $date = gmdate("ymdHis");

    $random =  random_string(6);

    if ($extra) {
        return $hostname .'+'. $date .'+'. $random .'+'. $extra;
    } else {
        return $hostname .'+'. $date .'+'. $random;
    }
}

/**
 * Generate and return a random string of the specified length.
 *
 * @param int $length The length of the string to be created.
 * @return string
 */
// from moodle/lib/moodlelib.php
function random_string ($length=15) {
    $pool  = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ';
    $pool .= 'abcdefghijklmnopqrstuvwxyz';
    $pool .= '0123456789';
    $poollen = strlen($pool);
    mt_srand ((double) microtime() * 1000000);
    $string = '';
    for ($i = 0; $i < $length; $i++) {
        $string .= substr($pool, (mt_rand()%($poollen)), 1);
    }
    return $string;
}

/**
 * Create a directory.
 *
 * @uses $CFG
 * @param string $directory  a string of directory names under $CFG->dataroot eg  stuff/assignment/1
 * param bool $shownotices If true then notification messages will be printed out on error.
 * @return string|false Returns full path to directory if successful, false if not
 */
// from moodle/lib/setuplib.php and gutted
function make_upload_directory($directory, $shownotices=true) {
    global $CFG;
    // just use Drupal's method for this
    $dir = file_directory_path() ."/$directory"; // doesn't work?
    if (file_exists($dir)) return $dir; // already there
    $success = mkdir($dir, $CFG->directorypermissions, true);
    if ($success)
      return $dir;
    else
      return false;
}


/**
* Print out error message and stop outputting.
*
* @param string $message
*/
// from moodle/lib/editorlib.php
function error($message) {
    echo '<div style="text-align: center; font-weight: bold; color: red;">';
    echo '<span style="color: black;">editorObject error:</span> ';
    echo s($message, true);
    echo '</div>';
    exit;
}

/**
 * Add quotes to HTML characters
 *
 * Returns $var with HTML characters (like "<", ">", etc.) properly quoted.
 * This function is very similar to {@link p()}
 *
 * @param string $var the string potentially containing HTML characters
 * @param boolean $strip to decide if we want to strip slashes or no. Default to false.
 *                true should be used to print data from forms and false for data from DB.
 * @return string
 */
// from moodle/lib/weblib.php
function s($var, $strip=false) {

    if ($var == '0') {  // for integer 0, boolean false, string '0'
        return '0';
    }

    if ($strip) {
        return preg_replace("/&amp;(#\d+);/i", "&$1;", htmlspecialchars(stripslashes_safe($var)));
    } else {
        return preg_replace("/&amp;(#\d+);/i", "&$1;", htmlspecialchars($var));
    }
}

/**
 * Moodle replacement for php stripslashes() function,
 * works also for objects and arrays.
 *
 * The standard php stripslashes() removes ALL backslashes
 * even from strings - so  C:\temp becomes C:temp - this isn't good.
 * This function should work as a fairly safe replacement
 * to be called on quoted AND unquoted strings (to be sure)
 *
 * @param mixed something to remove unsafe slashes from
 * @return mixed
 */
// from moodle/lib/weblib.php, with magic_quotes_sybase check removed
function stripslashes_safe($mixed) {
    // there is no need to remove slashes from int, float and bool types
    if (empty($mixed)) {
        //nothing to do...
    } else if (is_string($mixed)) {
        //the rest, simple and double quotes and backslashes
        $mixed = str_replace("\\'", "'", $mixed);
        $mixed = str_replace('\\"', '"', $mixed);
        $mixed = str_replace('\\\\', '\\', $mixed);
    } else if (is_array($mixed)) {
        foreach ($mixed as $key => $value) {
            $mixed[$key] = stripslashes_safe($value);
        }
    } else if (is_object($mixed)) {
        $vars = get_object_vars($mixed);
        foreach ($vars as $key => $value) {
            $mixed->$key = stripslashes_safe($value);
        }
    }

    return $mixed;
}

/**
 * Print a bold message in an optional color.
 *
 * @param string $message The message to print out
 * @param string $style Optional style to display message text in
 * @param string $align Alignment option
 * @param bool $return whether to return an output string or echo now
 */
// from moodle/lib/weblib.php
function notify($message, $style='notifyproblem', $align='center', $return=false) {
    if ($style == 'green') {
        $style = 'notifysuccess';  // backward compatible with old color system
    }

    $message = clean_text($message);

    $output = '<div class="'.$style.'" style="text-align:'. $align .'">'. $message .'</div>'."\n";

    if ($return) {
        return $output;
    }
    echo $output;
}

/**
 * Given raw text (eg typed in by a user), this function cleans it up
 * and removes any nasty tags that could mess up Moodle pages.
 *
 * @uses FORMAT_MOODLE
 * @uses FORMAT_PLAIN
 * @uses ALLOWED_TAGS
 * @param string $text The text to be cleaned
 * @param int $format Identifier of the text format to be used
 *            (FORMAT_MOODLE, FORMAT_HTML, FORMAT_PLAIN, FORMAT_WIKI, FORMAT_MARKDOWN)
 * @return string The cleaned up text
 */
// from moodle/lib/weblib.php
function clean_text($text, $format=FORMAT_MOODLE) {

    global $ALLOWED_TAGS, $CFG;

    if (empty($text) or is_numeric($text)) {
       return (string)$text;
    }

    switch ($format) {
        case FORMAT_PLAIN:
        case FORMAT_MARKDOWN:
            return $text;

        default:

            if (!empty($CFG->enablehtmlpurifier)) {
                $text = purify_html($text);
            } else {
            /// Fix non standard entity notations
                $text = preg_replace('/(&#[0-9]+)(;?)/', "\\1;", $text);
                $text = preg_replace('/(&#x[0-9a-fA-F]+)(;?)/', "\\1;", $text);

            /// Remove tags that are not allowed
                $text = strip_tags($text, $ALLOWED_TAGS);

            /// Clean up embedded scripts and , using kses
                // $text = cleanAttributes($text);  // FIXME too much work to port

            /// Again remove tags that are not allowed
                $text = strip_tags($text, $ALLOWED_TAGS);

            }

        /// Remove potential script events - some extra protection for undiscovered bugs in our code
            $text = eregi_replace("([^a-z])language([[:space:]]*)=", "\\1Xlanguage=", $text);
            $text = eregi_replace("([^a-z])on([a-z]+)([[:space:]]*)=", "\\1Xon\\2=", $text);

            return $text;
    }
}


/**
 * Print an error page displaying an error message.  New method - use this for new code.
 *
 * @uses $SESSION
 * @uses $CFG
 * @param string $errorcode The name of the string from error.php (or other specified file) to print
 * @param string $link The url where the user will be prompted to continue. If no url is provided the user will be directed to the site index page.
 * @param object $a Extra words and phrases that might be required in the error string
 * @param array $extralocations An array of strings with other locations to look for string files
 * @return does not return, terminates script
 */
// from moodle/lib/weblib.php, and gutted
function print_error($errorcode, $module='error', $link='', $a=NULL, $extralocations=NULL) {
  // use Drupal's stuff, good enough

  $message = get_string($errorcode, $module, $a, $extralocations);

  print "<h1>$message</h1>";

  drupal_set_message("$errorcode $a", $type = 'error', $repeat = TRUE);
}

// from moodle/lib/weblib.php, totally hacked
function format_text($text, $format=FORMAT_MOODLE, $options=NULL, $courseid=NULL) {
    return $text;
}


// from moodle/lib/questionlib.php
function question_has_capability_on($question, $cap, $cachecat = -1) {
  // assume true for now
  return true;
}

// from moodle/lib/moodlelib.php
function current_language() {
  // FIXME use Drupal's language
  return 'en_utf8';
}

// from moodle/lib/moodlelib.php
/**
 * fix up the optional data in get_string()/print_string() etc
 * ensure possible sprintf() format characters are escaped correctly
 * needs to handle arbitrary strings and objects
 * @param mixed $a An object, string or number that can be used
 * @return mixed the supplied parameter 'cleaned'
 */
function clean_getstring_data( $a ) {
    if (is_string($a)) {
        return str_replace( '%','%%',$a );
    }
    elseif (is_object($a)) {
        $a_vars = get_object_vars( $a );
        $new_a_vars = array();
        foreach ($a_vars as $fname => $a_var) {
            $new_a_vars[$fname] = clean_getstring_data( $a_var );
        }
        return (object)$new_a_vars;
    }
    else {
        return $a;
    }
}


// from moodle/lib/moodlelib.php
/**
 * Zip an array of files/dirs to a destination zip file
 * Both parameters must be FULL paths to the files/dirs
 */
function zip_files ($originalfiles, $destination) {
  global $CFG;

  // FIXME remove debugging once this is working well
  print "zipping files! originalfiles: ";
  print_r($originalfiles);
  print "destination: $destination\n";

  //Extract everything from destination
  $path_parts = pathinfo(cleardoubleslashes($destination));
  $destpath = $path_parts["dirname"];       //The path of the zip file
  $destfilename = $path_parts["basename"];  //The name of the zip file
  $extension = $path_parts["extension"];    //The extension of the file

  foreach ($originalfiles as $file) {  //Iterate over each file
      //Check for every file
      $tempfile = cleardoubleslashes($file); // no doubleslashes!
      //Calculate the base path for all files if it isn't set
      if ($origpath === NULL) {
          $origpath = rtrim(cleardoubleslashes(dirname($tempfile)), "/");
      }
      //See if the file is readable
      if (!is_readable($tempfile)) {  //Is readable
          continue;
      }
      //See if the file/dir is in the same directory than the rest
      if (rtrim(cleardoubleslashes(dirname($tempfile)), "/") != $origpath) {
          continue;
      }
      //Add the file to the array
      $files[] = $tempfile;
  }

  if (class_exists('ZipArchive')) {
    $zip = new ZipArchive();
    $filename = $destination;// correct?

    if ($zip->open($filename, ZIPARCHIVE::CREATE)!==TRUE) {
        exit("cannot open <$filename>\n");
    }

    foreach ($files as $file) {
      $nameInZip = basename($originalFile);
      $zip->addFile($originalFile, $nameInZip);
    }
    echo "numfiles: " . $zip->numFiles . "\n";
    echo "status:" . $zip->status . "\n";
    $zip->close();
  } else if (function_exists('gzopen')) {
    require_once("$CFG->libdir/pclzip/pclzip.lib.php");
    //rewrite filenames because the old method with PCLZIP_OPT_REMOVE_PATH does not work under win32
    $zipfiles = array();
    $start = strlen($origpath)+1;
    foreach ($files as $file) {
        $tf = array();
        $tf[PCLZIP_ATT_FILE_NAME] = $file;
        $tf[PCLZIP_ATT_FILE_NEW_FULL_NAME] = substr($file, $start);
        $zipfiles[] = $tf;
    }
    //create the archive
    $archive = new PclZip(cleardoubleslashes("$destpath/$destfilename"));
    if (($list = $archive->create($zipfiles) == 0)) {
        notice($archive->errorInfo(true));
        return false;
    }
  } else {
    print "final else";
    $filestozip = "";
    foreach ($files as $filetozip) {
        $filestozip .= escapeshellarg(basename($filetozip));
        $filestozip .= " ";
    }
    //Construct the command
    $separator = strtoupper(substr(PHP_OS, 0, 3)) === 'WIN' ? ' &' : ' ;';
    $command = 'cd '.escapeshellarg($origpath).$separator.
                escapeshellarg($CFG->zip).' -r '.
                // TODO had to hack this... bug in Moodle?
                escapeshellarg(cleardoubleslashes("$destfilename")).' '.$filestozip;
    //All converted to backslashes in WIN
    if (strtoupper(substr(PHP_OS, 0, 3)) === 'WIN') {
        $command = str_replace('/','\\',$command);
    }
    // print "$command";exit;
    Exec($command);
  }
}


// from moodle/lib/moodlelib.php
/**
 * Replace 1 or more slashes or backslashes to 1 slash
 */
function cleardoubleslashes ($path) {
    return preg_replace('/(\/|\\\){1,}/','/',$path);
}

// from moodle/lib/moodlelib.php
/**
 * Delete directory or only it's content
 * @param string $dir directory path
 * @param bool $content_only
 * @return bool success, true also if dir does not exist
 */
function remove_dir($dir, $content_only=false) {
    if (!file_exists($dir)) {
        // nothing to do
        return true;
    }
    $handle = opendir($dir);
    $result = true;
    while (false!==($item = readdir($handle))) {
        if($item != '.' && $item != '..') {
            if(is_dir($dir.'/'.$item)) {
                $result = remove_dir($dir.'/'.$item) && $result;
            }else{
                $result = unlink($dir.'/'.$item) && $result;
            }
        }
    }
    closedir($handle);
    if ($content_only) {
        return $result;
    }
    return rmdir($dir); // if anything left the result will be false, noo need for && $result
}


/*******************************************
 *  Here down is fake questionlib.php
 *
 */

// fake the questionlib because loading that whole thing won't work
//  module_load_include('php', 'quiz', "includes/moodle/lib/questionlib");
global $QTYPES;
module_load_include('php', 'quiz', "includes/moodle/question/format");

/**#@+
 * The core question types.
 */
define("SHORTANSWER",   "shortanswer");
define("TRUEFALSE",     "truefalse");
define("MULTICHOICE",   "multichoice");
define("RANDOM",        "random");
define("MATCH",         "match");
define("RANDOMSAMATCH", "randomsamatch");
define("DESCRIPTION",   "description");
define("NUMERICAL",     "numerical");
define("MULTIANSWER",   "multianswer");
define("CALCULATED",    "calculated");
define("ESSAY",         "essay");
/**#@-*/

$QTYPES[CALCULATED] = new stdClass();

