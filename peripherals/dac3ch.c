/* dac3ch.c: Routines for handling 3x channel 8bit D/A converter 

   This 8-bit/4-bit D/A converter was used by the Sound Tracker music editor.
   Schematics can be be found for example here:
     https://worldofspectrum.net/pub/sinclair/games-info/s/SampleTracker3T.pdf
   Works only if i8255/MHB8255A mode has been set to all channels out.

   Usage:
     OUT 127 (#7F), 128 (#80) - sets 8255 to mode 0 and outbound way for all 3 channels
     OUT 31 (#1F), X - Left "A" channel
     OUT 63 (#3F), Y - Right "B" channel
     OUT 95 (#5F), Z - Middle "C" channel
  
   Some application(s) or their version(s) set the mode wrongly (maybe it worked with authors' HW), 
   so I've added "force OUT Mode" option to override it and enjoy music from those apps as well.
   See "settings_current.dac3ch_force_out == 1" .
   Examples:
     Sample Tracker 3D: Setting 155 (all IN). Version 2 is OK.
     A.S.E BETA: Setting 0x51, 0x33 (maybe Beta conflict?).

   Copyright (c) 2011-2016 Jon Mitchell, Philip Kendall
   Copyright (c) 2015 Stuart Brady
   Copyright (c) 2017 Fredrick Meunier
   Copyright (c) 2022 Miroslav Ďurčík 

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

   Philip: philip-fuse@shadowmagic.org.uk

   Arki55: miro.arki55@gmail.com

*/

#include "config.h"

#include "libspectrum.h"

#include "compat.h"
#include "infrastructure/startup_manager.h"
#include "machine.h"
#include "module.h"
#include "periph.h"
#include "settings.h"
#include "sound.h"
#include "dac3ch.h"
#include "spectrum.h"

static void dac3ch_reset( int hard_reset );
static void dac3ch_enabled_snapshot( libspectrum_snap *snap );
static void dac3ch_from_snapshot( libspectrum_snap *snap );
static void dac3ch_to_snapshot( libspectrum_snap *snap );

static module_info_t dac3ch_module_info = {

  /* .reset = */ dac3ch_reset,
  /* .romcs = */ NULL,
  /* .snapshot_enabled = */ dac3ch_enabled_snapshot,
  /* .snapshot_from = */ dac3ch_from_snapshot,
  /* .snapshot_to = */ dac3ch_to_snapshot,

};

static const periph_port_t dac3ch_ports[] = {
  { 0x00ff, 0x001f, NULL, dac3ch_write_channel_a },
  { 0x00ff, 0x003f, NULL, dac3ch_write_channel_b },
  { 0x00ff, 0x005f, NULL, dac3ch_write_channel_c },
  { 0x00ff, 0x007f, NULL, dac3ch_write_control },
  { 0, 0, NULL, NULL }
};

static const periph_t dac3ch_periph = {
  /* .option = */ &settings_current.dac3ch,
  /* .ports = */ dac3ch_ports,
  /* .hard_reset = */ 0,
  /* .activate = */ NULL,
};

static int
dac3ch_init( void *context )
{
  module_register( &dac3ch_module_info );
  periph_register( PERIPH_TYPE_DAC3CH, &dac3ch_periph );

  return 0;
}

void
dac3ch_register_startup( void )
{
  startup_manager_module dependencies[] = { STARTUP_MANAGER_MODULE_SETUID };
  startup_manager_register( STARTUP_MANAGER_MODULE_DAC3CH, dependencies,
                            ARRAY_SIZE( dependencies ), dac3ch_init, NULL,
                            NULL );
}

static void
dac3ch_reset( int hard_reset GCC_UNUSED )
{
  machine_current->dac3ch.dac3ch_channel_a = 0;
  machine_current->dac3ch.dac3ch_channel_b = 0;
  machine_current->dac3ch.dac3ch_channel_c = 0;
  machine_current->dac3ch.dac3ch_control = 0;
  machine_current->dac3ch.dac3ch_active = 0;
}

static void
dac3ch_enabled_snapshot( libspectrum_snap *snap )
{
  settings_current.dac3ch = libspectrum_snap_dac3ch_active( snap );
}

static void
dac3ch_from_snapshot( libspectrum_snap *snap )
{
  if( !libspectrum_snap_dac3ch_active( snap ) ) return;

  /* We just set the internal machine status to the last read value
   * instead of trying to write to the sound routines, as at this stage
   * sound isn't initialised so there is no synth to write to (as for Covox)
   */
  machine_current->dac3ch.dac3ch_channel_a = libspectrum_snap_dac3ch_channel_a( snap );
  machine_current->dac3ch.dac3ch_channel_b = libspectrum_snap_dac3ch_channel_b( snap );
  machine_current->dac3ch.dac3ch_channel_c = libspectrum_snap_dac3ch_channel_c( snap );
  machine_current->dac3ch.dac3ch_control = libspectrum_snap_dac3ch_control( snap );
}

static void
dac3ch_to_snapshot( libspectrum_snap *snap )
{
  if( !(periph_is_active( PERIPH_TYPE_DAC3CH ) ) )
    return;

  libspectrum_snap_set_dac3ch_active( snap, 1 );
  libspectrum_snap_set_dac3ch_channel_a( snap, machine_current->dac3ch.dac3ch_channel_a );
  libspectrum_snap_set_dac3ch_channel_b( snap, machine_current->dac3ch.dac3ch_channel_b );
  libspectrum_snap_set_dac3ch_channel_c( snap, machine_current->dac3ch.dac3ch_channel_c );
  libspectrum_snap_set_dac3ch_control( snap, machine_current->dac3ch.dac3ch_control );
}

void
dac3ch_write_control( libspectrum_word port GCC_UNUSED, libspectrum_byte val )
{
  if( !periph_is_active( PERIPH_TYPE_DAC3CH ) ) {
    return;
  }

  machine_current->dac3ch.dac3ch_control = val;

  // Control - sound works only if port 0x7C = 128
  // Works only if 8255's mode has been set to all channels out.
  if (val == 128) {
    // Standard all channels OUT mode
    machine_current->dac3ch.dac3ch_active = 1;
  } else {
    // Deactivated - reset all
    machine_current->dac3ch.dac3ch_active = 0;
    machine_current->dac3ch.dac3ch_channel_a = 0;
    machine_current->dac3ch.dac3ch_channel_b = 0;
    machine_current->dac3ch.dac3ch_channel_c = 0;
  }
}

void
dac3ch_write_channel_a( libspectrum_word port GCC_UNUSED, libspectrum_byte val )
{
  if( !periph_is_active( PERIPH_TYPE_DAC3CH ) ) {
    return;
  }
  if (machine_current->dac3ch.dac3ch_active == 0 && settings_current.dac3ch_force_out == 0) {
    return;
  }
  sound_dac3ch_write_left(val);
  machine_current->dac3ch.dac3ch_channel_a = val;
}

void
dac3ch_write_channel_b( libspectrum_word port GCC_UNUSED, libspectrum_byte val )
{
  if( !periph_is_active( PERIPH_TYPE_DAC3CH ) ) {
    return;
  }
  if (machine_current->dac3ch.dac3ch_active == 0 && settings_current.dac3ch_force_out == 0) {
    return;
  }

  sound_dac3ch_write_right(val);
  machine_current->dac3ch.dac3ch_channel_b = val;
}

void
dac3ch_write_channel_c( libspectrum_word port GCC_UNUSED, libspectrum_byte val )
{
  if( !periph_is_active( PERIPH_TYPE_DAC3CH ) ) {
    return;
  }
  if (machine_current->dac3ch.dac3ch_active == 0 && settings_current.dac3ch_force_out == 0) {
    return;
  }

  sound_dac3ch_write_middle(val);
  machine_current->dac3ch.dac3ch_channel_c = val;
}
