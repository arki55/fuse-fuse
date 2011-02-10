## Process this file with automake to produce Makefile.in
## Copyright (c) 2011 Philip Kendall

## $Id$

## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2 of the License, or
## (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License along
## with this program; if not, write to the Free Software Foundation, Inc.,
## 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
##
## Author contact information:
##
## E-mail: philip-fuse@shadowmagic.org.uk

AUTOMAKE_OPTIONS = foreign

AM_CPPFLAGS = -I$(srcdir)/..

SUBDIRS = \
  disk \
  ide \
  nic

noinst_LIBRARIES = libperipherals.a

INCLUDES = @LIBSPEC_CFLAGS@ @GLIB_CFLAGS@ @GTK_CFLAGS@

libperipherals_a_SOURCES = \
  ay.c \
  dck.c \
  fuller.c \
  if1.c \
  if2.c \
  joystick.c \
  kempmouse.c \
  melodik.c \
  printer.c \
  scld.c \
  speccyboot.c \
  specdrum.c \
  ula.c

noinst_HEADERS = \
  ay.h \
  dck.h \
  fuller.h \
  if1.h \
  if2.h \
  joystick.h \
  kempmouse.h \
  melodik.h \
  printer.h \
  scld.h \
  speccyboot.h \
  specdrum.h \
  ula.h