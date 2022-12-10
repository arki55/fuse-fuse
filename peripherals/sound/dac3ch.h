/* dac3ch.h: Routines for handling 3x channel Parellel Port Interface (i8255 chip)
   This is not a complete module/peripheral, needs to be extended/called from others.

   Copyright (c) 2011-2017 Jon Mitchell, Philip Kendall, Fredrick Meunier
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

#ifndef FUSE_DAC3CH_H
#define FUSE_DAC3CH_H

#include "libspectrum.h"

typedef struct dac3ch_info {
  libspectrum_byte dac3ch_channel_a; /* Current value of channel A */
  libspectrum_byte dac3ch_channel_b; /* Current value of channel B */
  libspectrum_byte dac3ch_channel_c; /* Current value of channel C */
  libspectrum_byte dac3ch_control; /* Current value of control byte */
  libspectrum_byte dac3ch_active; /* if output is active */
} dac3ch_info;

void dac3ch_register_startup( void );
void dac3ch_write_channel_a( libspectrum_word port, libspectrum_byte val );
void dac3ch_write_channel_b( libspectrum_word port, libspectrum_byte val );
void dac3ch_write_channel_c( libspectrum_word port, libspectrum_byte val );
void dac3ch_write_control( libspectrum_word port, libspectrum_byte val );

#endif                          /* #ifndef FUSE_DAC3CH_H */
