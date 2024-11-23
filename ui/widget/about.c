/* about.c: about dialog box
   Copyright (c) 2016 Sergio Baldoví

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

   Author contact information:

   E-mail: serbalgi@gmail.com

*/

#include "config.h"

#include <stdio.h>

#include "widget.h"
#include "widget_internals.h"

int
widget_about_draw( void *data GCC_UNUSED )
{
  char buffer[80];
  int dialog_cols, string_width, margin, x, line, lines_total;
  char **version_lines; size_t version_line_count;
  size_t i;

  dialog_cols = 30;
  margin = 17;
  line = 0;
  lines_total = 9;

  /* First Split version string into more lines, so that it fits the dialog and we know count of lines */
  if( split_message( FUSE_ABOUT_VERSION, &version_lines, &version_line_count, 25 ) ) return 1;
  lines_total += version_line_count;

#ifdef FUSE_TEST_BUILD
  /* If TEST build is active, split test build info, count lines */
  char **test_lines; size_t test_line_count;
  if( split_message( FUSE_TEST_BUILD, &test_lines, &test_line_count, 25 ) ) return 1;
  lines_total += test_line_count + 3;
#endif

  /* Start constructing the dialog box */
  widget_dialog_with_border( 1, 2, dialog_cols, lines_total );
  widget_printstring( 10, 16, WIDGET_COLOUR_TITLE, "About Fuse" );

  /* Print version info - multi line - split further above */
  for( i=0; i<version_line_count; i++ ) {
    snprintf( buffer, 80, "%s", version_lines[i] );
    string_width = widget_stringwidth( buffer );
    x = margin - 8 + ( dialog_cols * 8 - string_width ) / 2;
    widget_printstring( x, ++line * 8 + 24, WIDGET_COLOUR_FOREGROUND, buffer );
    free( version_lines[i] );
  }
  free( version_lines );

  ++line;

#ifdef FUSE_TEST_BUILD
  /* TEST build : Print additional version related info */
  string_width = widget_stringwidth( FUSE_TEST_LINE );
  x = margin - 8 + ( dialog_cols * 8 - string_width ) / 2;
  widget_printstring( x, ++line * 8 + 24, WIDGET_COLOUR_FOREGROUND,
                      FUSE_TEST_LINE );

  ++line;

  for( i=0; i<test_line_count; i++ ) {
    snprintf( buffer, 80, "%s", test_lines[i] );
    string_width = widget_stringwidth( buffer );
    x = margin - 8 + ( dialog_cols * 8 - string_width ) / 2;
    widget_printstring( x, ++line * 8 + 24, WIDGET_COLOUR_FOREGROUND, buffer );
    free( test_lines[i] );
  }
  free( test_lines );

  ++line;

#endif

  string_width = widget_stringwidth( FUSE_LONG );
  x = margin - 8 + ( dialog_cols * 8 - string_width ) / 2;
  widget_printstring( x, ++line * 8 + 24, WIDGET_COLOUR_FOREGROUND,
                      FUSE_LONG );

  ++line;

  string_width = widget_stringwidth( FUSE_COPYRIGHT );
  x = margin - 8 + ( dialog_cols * 8 - string_width ) / 2;
  widget_printstring( x, ++line * 8 + 24, WIDGET_COLOUR_FOREGROUND,
                      FUSE_COPYRIGHT );

  ++line;

  string_width = widget_stringwidth( PACKAGE_URL );
  x = margin - 8 + ( dialog_cols * 8 - string_width ) / 2;
  widget_printstring( x, ++line * 8 + 24, 0x09, PACKAGE_URL );

  widget_display_lines( 2, line + 3 );

  return 0;
}

void
widget_about_keyhandler( input_key key )
{
  switch( key ) {

  case INPUT_KEY_Escape:
  case INPUT_JOYSTICK_FIRE_2:
    widget_end_widget( WIDGET_FINISHED_CANCEL );
    return;

  case INPUT_KEY_Return:
  case INPUT_KEY_KP_Enter:
  case INPUT_JOYSTICK_FIRE_1:
    widget_end_widget( WIDGET_FINISHED_OK );
    return;

  default:	/* Keep gcc happy */
    break;

  }
}
