# Fuse::Dialog: routines for creating Fuse dialog boxes
# Copyright (c) 2003-2023 Philip Kendall, Miroslav Durcik

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

# Author contact information:

# E-mail: philip-fuse@shadowmagic.org.uk

package Fuse::Dialog;

use strict;

use English;
use IO::File;

sub read (;$) {

    my $filename = shift;

    my $fh;
    if( defined $filename && $filename ne '-' ) {
	$fh = new IO::File( "< $filename" )
	    or die "Couldn't open '$filename': $!";
    } else {
	$fh = new IO::Handle;
	$fh->fdopen( fileno( STDIN ), "r" ) or die "Couldn't read stdin: $!";
    }

    local $INPUT_RECORD_SEPARATOR = ""; # Paragraph mode

    my %subvalues_map; # Array with references to subvalues arrays. Option's name is the key.
    my %option2dialog_map; # Array map option name => dialog name
    my @dialogs;
    while( <$fh> ) {

	my( $name, $title, @widgets ) = split /\n/;

	my @widget_data;
	my %widget_data_assoc; # setting name is the key
	my $postcheck;
	my $posthook;
	my $prehook;
	my $lockedcheck;

	foreach( @widgets ) {

	    my( $widget_type, $text, $value, $parent_value, $key, $data1, $data2 ) =
		split /\s*,\s*/;

	    if( lc $widget_type eq 'postcheck' ) {
		$postcheck = $text;
		next;
	    }

	    if( lc $widget_type eq 'posthook' ) {
		$posthook = $text;
		next;
	    }

		if( lc $widget_type eq 'prehook' ) {
		$prehook = $text;
		next;
		}

		if( lc $widget_type eq 'lockedcheck' ) {
		$lockedcheck = $text;
		next;
		}

		my @subvalues = (); # Will be filled later
		my $subvalues_ref = \@subvalues;
		$subvalues_map{"$value"} = $subvalues_ref; # Map subvalues array through value/option name
		$option2dialog_map{"$value"} = $name; # Map option name to dialog name

		# Prefix option(s) in parent value def. with "w." if not prefixed
		# Parent option def. supports || && separators and can be prefixed with w. or s.
		$parent_value =~ s/^([_a-zA-Z0-9]{3,})/w.$1/; # Prefix first option
		$parent_value =~ s/(\|\||\&\&)([_a-zA-Z0-9]{3,})/$1w.$2/; # Prefix next options

		push @widget_data, {
				# read widget data
				type => $widget_type,
				text => $text,
				value => $value,
				parent_value => $parent_value,
				key => $key,
				data1 => $data1,
				data2 => $data2,
				# calculated widget data
				subvalues => $subvalues_ref, # See Postprocessing(1)
				onclick => 0 # See Postprocessing(2)
		};
	}

	push @dialogs, { name => $name,
			 title => $title,
			 postcheck => $postcheck,
			 posthook => $posthook,
			 prehook => $prehook,
			 lockedcheck => $lockedcheck,
			 widgets => \@widget_data };
    }

	use Data::Dumper;

	# Postprocessing(1): GO through all dialogs/widget, generate subvalues arrays.
	foreach my $dialog ( @dialogs ) {
		foreach my $widget ( @{ $dialog->{widgets} } ) {
			my $parent_value1 = $widget->{parent_value};
			my $widget_value1 = $widget->{value};
			if( $parent_value1 && $widget_value1 ) {
				my $parent_value2 = $parent_value1;
				$parent_value2 =~ s/(\&\&|\|\|)/<sep>/g; # replace supported sep. && ||
				$parent_value2 =~ s/[sw]\.//g; # replace "s." or "w." (default) for key in map
				foreach my $parent_value3 ( split (/<sep>/, $parent_value2) ) {
					# Trigger editability within ANY parent option. Forward original edit.def.
					my $val2 = $subvalues_map{"$parent_value3"};
					my $parent_value_dialog = $option2dialog_map{"$parent_value3"};
					# We are interested in suboption(s) only within the same dialog window (**)
					my $same_dialog = ( $parent_value_dialog eq $dialog->{name} );
					if( $same_dialog ) {
						push ( @{$val2}, { sub => $widget_value1, parent_def => $parent_value1} );
					}
				}
			}
		}
	}

	# Postprocessing(2): GO through dialogs, generate onclick flag if necessary.
	#       When? If type is checkbox and it has suboption(s) in the same window (**).
	foreach my $dialog ( @dialogs ) {
		foreach my $widget ( @{ $dialog->{widgets} } ) {
			if ( ( $widget->{type} eq "Checkbox" ) &&
				( $widget->{subvalues} ) &&
				( scalar @{ $widget->{subvalues} } )
			) {
				$widget->{onclick} = 1;
			}
		}
	}

    return @dialogs;
}

1;
