/* tc2048.c: Timex TC2048 specific routines
   Copyright (c) 1999-2003 Philip Kendall
   Copyright (c) 2002 Fredrick Meunier

   $Id$

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

   Author contact information:

   E-mail: pak21-fuse@srcf.ucam.org
   Postal address: 15 Crescent Road, Wokingham, Berks, RG40 2DB, England

*/

#include <config.h>

#include <stdio.h>

#include <libspectrum.h>

#include "fuse.h"
#include "joystick.h"
#include "machine.h"
#include "memory.h"
#include "printer.h"
#include "settings.h"
#include "scld.h"
#include "spectrum.h"
#include "tc2048.h"

static libspectrum_dword tc2048_contend_delay( void );

spectrum_port_info tc2048_peripherals[] = {
  { 0x00e0, 0x0000, joystick_kempston_read, spectrum_port_nowrite },
  { 0x00ff, 0x00f4, scld_hsr_read, scld_hsr_write },
  { 0x00ff, 0x00fb, printer_zxp_read, printer_zxp_write }, /* TS2040/Alphacom printer */

  /* Lower 8 bits of Timex ports are fully decoded */
  { 0x00ff, 0x00fe, spectrum_ula_read, spectrum_ula_write },

  { 0x00ff, 0x00ff, scld_dec_read, scld_dec_write },
  { 0, 0, NULL, NULL } /* End marker. DO NOT REMOVE */
};


static libspectrum_byte
tc2048_unattached_port( void )
{
  /* TC2048 does not have floating ULA values on any port (despite rumours
     to the contrary), it returns 0xff on unattached ports */
  return 0xff;
}

libspectrum_byte
tc2048_read_screen_memory( libspectrum_word offset )
{
  return RAM[5][offset];
}

libspectrum_dword
tc2048_contend_memory( libspectrum_word address )
{
  if( memory_contended[ address >> 13 ] ) return tc2048_contend_delay();

  return 0;
}

libspectrum_dword
tc2048_contend_port( libspectrum_word port )
{
  /* Contention occurs for ports FE and F4 (SCLD and HSR) */
  /* Contention occurs for port FF (SCLD DEC) */
  if( ( port & 0xff ) == 0xf4 ||
      ( port & 0xff ) == 0xfe ||
      ( port & 0xff ) == 0xff    ) return tc2048_contend_delay();

  return 0;
}

static libspectrum_dword
tc2048_contend_delay( void )
{
  libspectrum_word tstates_through_line;
  
  /* No contention in the upper border */
  if( tstates < machine_current->line_times[ DISPLAY_BORDER_HEIGHT ] )
    return 0;

  /* Or the lower border */
  if( tstates >= machine_current->line_times[ DISPLAY_BORDER_HEIGHT + 
                                              DISPLAY_HEIGHT          ] )
    return 0;

  /* Work out where we are in this line */
  tstates_through_line =
    ( tstates + machine_current->timings.left_border ) %
    machine_current->timings.tstates_per_line;

  /* No contention if we're in the left border */
  if( tstates_through_line < machine_current->timings.left_border - 1 ) 
    return 0;

  /* Or the right border or retrace */
  if( tstates_through_line >= machine_current->timings.left_border +
                              machine_current->timings.horizontal_screen - 1 )
    return 0;

  /* We now know the ULA is reading the screen, so put in the appropriate
     delay */
  switch( tstates_through_line % 8 ) {
    case 7: return 6; break;
    case 0: return 5; break;
    case 1: return 4; break;
    case 2: return 3; break;
    case 3: return 2; break;
    case 4: return 1; break;
    case 5: return 0; break;
    case 6: return 0; break;
  }

  return 0;	/* Shut gcc up */
}

int tc2048_init( fuse_machine_info *machine )
{
  int error;

  machine->machine = LIBSPECTRUM_MACHINE_TC2048;
  machine->id = "2048";

  machine->reset = tc2048_reset;

  error = machine_set_timings( machine ); if( error ) return error;

  machine->timex = 1;
  machine->ram.read_screen	     = tc2048_read_screen_memory;
  machine->ram.contend_memory	     = tc2048_contend_memory;
  machine->ram.contend_port	     = tc2048_contend_port;
  machine->ram.current_screen = 5;

  error = machine_allocate_roms( machine, 1 );
  if( error ) return error;
  machine->rom_length[0] = 0x4000;

  machine->peripherals = tc2048_peripherals;
  machine->unattached_port = tc2048_unattached_port;

  machine->ay.present = 0;

  machine->shutdown = NULL;

  return 0;

}

int
tc2048_reset( void )
{
  int error;
  size_t i;

  error = machine_load_rom( &ROM[0], settings_current.rom_tc2048,
			    machine_current->rom_length[0] );
  if( error ) return error;

  memory_map[0] = &ROM[0][0x0000];
  memory_map[1] = &ROM[0][0x2000];
  memory_map[2] = &RAM[5][0x0000];
  memory_map[3] = &RAM[5][0x2000];
  memory_map[4] = &RAM[2][0x0000];
  memory_map[5] = &RAM[2][0x2000];
  memory_map[6] = &RAM[0][0x0000];
  memory_map[7] = &RAM[0][0x2000];

  memory_writable[0] = memory_writable[1] = 0;
  for( i = 2; i < 8; i++ ) memory_writable[i] = 1;

  for( i = 0; i < 8; i++ ) memory_contended[i] = 0;
  memory_contended[2] = memory_contended[3] = 1;

  memory_screen_chunk1 = &RAM[5][0x0000];
  memory_screen_chunk2 = &RAM[5][0x2000];
  memory_screen_top = 0x1b00;

  return 0;
}
