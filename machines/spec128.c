/* spec128.c: Spectrum 128K specific routines
   Copyright (c) 1999-2011 Philip Kendall

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

   E-mail: philip-fuse@shadowmagic.org.uk

*/

#include "config.h"

#include <stdio.h>

#include "libspectrum.h"

#include "compat.h"
#include "machine.h"
#include "machines_periph.h"
#include "memory_pages.h"
#include "periph.h"
#include "peripherals/disk/beta.h"
#include "settings.h"
#include "spec128.h"
#include "spec48.h"
#include "specplus3.h"

static int spec128_reset( void );

int spec128_init( fuse_machine_info *machine )
{
  machine->machine = LIBSPECTRUM_MACHINE_128;
  machine->id = "128";

  machine->reset = spec128_reset;

  machine->timex = 0;
  machine->ram.port_from_ula	     = spec48_port_from_ula;
  machine->ram.contend_delay	     = spectrum_contend_delay_65432100;
  machine->ram.contend_delay_no_mreq = spectrum_contend_delay_65432100;
  machine->ram.valid_pages	     = 8;

  machine->unattached_port = spectrum_unattached_port;
  machine->writeback = spec128_get_writeback( 0 );

  machine->shutdown = NULL;

  machine->memory_map = spec128_memory_map;

  return 0;
}

void
spec128_memory_patch( void )
{
  if(
    machine_current->machine == LIBSPECTRUM_MACHINE_128 &&
    settings_current.didaktik_128k_patch
  ) {
    /* Spec 128+D80 patch (1/2):
       To allow usage of D80 with 128k speccy, alter paging port decoder. */
    periph_set_present( PERIPH_TYPE_128_MEMORY, PERIPH_PRESENT_NEVER );
    periph_set_present( PERIPH_TYPE_128_MEMORY_PATCHED, PERIPH_PRESENT_ALWAYS );
    /* Didaktik D80 can be used under these settings */
    periph_set_present( PERIPH_TYPE_DIDAKTIK80, PERIPH_PRESENT_OPTIONAL );
    /* Patch writeback mechanism */
    machine_current->writeback = spec128_get_writeback( 1 );
  }
}

int
spec128_rom_patch( int rom )
{
  if(
    machine_current->machine == LIBSPECTRUM_MACHINE_128 &&
    settings_current.didaktik_128k_patch
  ) {
    /* Spec 128+D80 patch (2/2):
       Didaktik D80 can be used with 48K ROM only */
    return 1;
  }

  return rom;
}

static int
spec128_reset( void )
{
  int error;

  error = machine_load_rom( 0, settings_current.rom_128_0,
                            settings_default.rom_128_0, 0x4000 );
  if( error ) return error;
  error = machine_load_rom( 1, settings_current.rom_128_1,
                            settings_default.rom_128_1, 0x4000 );
  if( error ) return error;

  error = spec128_common_reset( 1 );
  if( error ) return error;

  periph_clear();
  machines_periph_128();
  spec128_memory_patch();
  periph_update();

  beta_builtin = 0;

  spec48_common_display_setup();

  return 0;
}

int
spec128_common_reset( int contention )
{
  size_t i;

  machine_current->ram.locked=0;
  machine_current->ram.last_byte = 0;

  machine_current->ram.current_page=0;
  machine_current->ram.current_rom=0;

  memory_current_screen = 5;
  memory_screen_mask = 0xffff;

  machine_current->writeback = NULL;

  /* Odd pages contended on the 128K/+2; the loop is up to 16 to
     ensure all of the Scorpion's 256Kb RAM is not contended */
  for( i = 0; i < 16; i++ )
    memory_ram_set_16k_contention( i, i & 1 ? contention : 0 );

  /* 0x0000: ROM 0 */
  memory_map_16k( 0x0000, memory_map_rom, 0 );
  /* 0x4000: RAM 5 */
  memory_map_16k( 0x4000, memory_map_ram, 5 );
  /* 0x8000: RAM 2 */
  memory_map_16k( 0x8000, memory_map_ram, 2 );
  /* 0xc000: RAM 0 */
  memory_map_16k( 0xc000, memory_map_ram, 0 );

  return 0;
}

void
spec128_memoryport_write( libspectrum_word port GCC_UNUSED,
			  libspectrum_byte b )
{
  if( machine_current->ram.locked ) return;

  machine_current->ram.last_byte = b;

  machine_current->memory_map();

  machine_current->ram.locked = b & 0x20;
}

void
spec128_select_rom( int rom )
{
  rom = spec128_rom_patch( rom );
  memory_map_16k( 0x0000, memory_map_rom, rom );
  machine_current->ram.current_rom = rom;
}

void
spec128_select_page( int page )
{
  memory_map_16k( 0xc000, memory_map_ram, page );
  machine_current->ram.current_page = page;
}

int
spec128_memory_map( void )
{
  int page, screen, rom;

  page = machine_current->ram.last_byte & 0x07;
  screen = ( machine_current->ram.last_byte & 0x08 ) ? 7 : 5;
  rom = ( machine_current->ram.last_byte & 0x10 ) >> 4;

  /* If we changed the active screen, mark the entire display file as
     dirty so we redraw it on the next pass */
  if( memory_current_screen != screen ) {
    display_update_critical( 0, 0 );
    display_refresh_main_screen();
    memory_current_screen = screen;
  }

  spec128_select_rom( rom );
  spec128_select_page( page );

  memory_romcs_map();

  return 0;
}

/* Originally speccy 128 memory page decoder, checking for 2 pins */
static writeback_port spec128_writeback_orig = { 0x8002, 0x0000, 0x7ffd };
/* Modified page decoder as used in Didaktik M 128k, checking also A5 = 1 */
static writeback_port spec128_writeback_patched = { 0x8022, 0x0020, 0x7ffd };

writeback_port *
spec128_get_writeback( int patched )
{
  if (patched == TRUE) 
    return & spec128_writeback_patched;
  else
    return & spec128_writeback_orig;
}
