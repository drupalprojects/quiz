This directory is a completely unmodified copy of the "question"
directory from Moodle, downloaded from:

   http://cvs.moodle.org/moodle/?pathrev=MOODLE_19_WEEKLY


Added to Drupal Quiz on 4 June 2009

Patches:

One edit in format/qti2/format.php:
      
        // HACK FIXME EDIT  a rare edit to the Moodle code
        // why load from category when we already have the questions?
        // $questions = get_questions_category( $this->category );
        $questions= $this->questions;

        zip_files( array($path), "$path.zip" );
        // FIXME TODO EDIT rare edit to Moodle format code
        // how else am I supposed to know what the filename of the zip is?
        $this->filename = basename($path);

