
/**
 * @file
 * README file for Quiz Choice.
 */

Quiz Choice
Allows quiz creators to make a multiple choice question type.

Sponsored by: Norwegian Centre for Telemedicine
Code: falcon


CONTENTS
--------

1.  Introduction
2.  Installation
3.  Configuration

----
1. Introduction

This module is an attempt to make it easy to construct surveys using the quiz module.

The Scale module lets the quiz creator store preset answer collections, enabling rapid
creation of survey questions.

The scale module is based on the OO framework of the quiz project.

----
2. Installation

To install, unpack the module to your modules folder, and simply enable the module at Admin > Build > Modules.

Several database tables prefixed with quiz_scale are installed to have a separate storage for this module.

----
3.  Configuration
Settings are to be found here: admin/quiz/scale

Preset answer collections can be configured at: scale/collection/manage
Here you can change and delete presets(answer collections).

You can give roles the 'Edit global presets' permission in the permission administration(admin/user/permissions)
With this permission users can make presets available to all users, and not only themselves.

