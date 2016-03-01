#!/usr/bin/perl

# This file is part of Hercules.
# http://herc.ws - http://github.com/HerculesWS/Hercules
#
# Copyright (C) 2014-2016  Hercules Dev Team
#
# Hercules is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

use strict;
use warnings;
use XML::Simple;

# XML Parser hint (some are faster than others)
#local $ENV{XML_SIMPLE_PREFERRED_PARSER} = '';                 # 0m14.181s
local $ENV{XML_SIMPLE_PREFERRED_PARSER} = 'XML::Parser';      # 0m4.256s
#local $ENV{XML_SIMPLE_PREFERRED_PARSER} = 'XML::SAX::Expat';  # 0m14.186s
#local $ENV{XML_SIMPLE_PREFERRED_PARSER} = 'XML::LibXML::SAX'; # 0m7.055s

my $HPMDataCheckAPIVer = 1;

my @files = grep { -f } grep { /[^h]\.xml/ } glob 'doxyoutput/xml/struct*.xml';
my %out;

foreach my $file (@files) {
	my $xml = new XML::Simple;
	my $data = $xml->XMLin($file, ForceArray => 1);
	my $filekey = (keys %{ $data->{compounddef} })[0];
	next unless $data->{compounddef}->{$filekey}->{includes}; # means its a struct from a .c file, plugins cant access those so we don't care.
	next if $data->{compounddef}->{$filekey}->{compoundname}->[0] =~ /::/; # its a duplicate with a :: name e.g. struct script_state {<...>} ay;
	my @filepath = split(/[\/\\]/, $data->{compounddef}->{$filekey}->{location}->[0]->{file});
	my $foldername = uc($filepath[-2]);
	next if $filepath[-1] eq "HPM.h"; # Skip the HPM core, plugins don't need it
	my $filename = uc($filepath[-1]); $filename =~ s/[.-]/_/g; $filename =~ s/\.[^.]*$//;
	my $plugintypes = 'SERVER_TYPE_UNKNOWN';
	if ($foldername eq 'COMMON') {
		if ($filename eq 'MAPINDEX_H') {
			$plugintypes = 'SERVER_TYPE_CHAR|SERVER_TYPE_MAP';
		} elsif ($filename eq 'GRFIO_H') {
			$plugintypes = 'SERVER_TYPE_MAP';
		} else {
			$plugintypes = 'SERVER_TYPE_ALL';
		}
	} elsif ($foldername =~ /^(LOGIN|CHAR|MAP)/) {
		$plugintypes = "SERVER_TYPE_${foldername}";
	}
	my $symboldata = {
		name => $data->{compounddef}->{$filekey}->{compoundname}->[0],
		type => $plugintypes,
	};
	my $name = "${foldername}_${filename}";
	push @{ $out{$name} }, $symboldata;
}

my $fname = '../../src/common/HPMDataCheck.h';
open(FH, '>', $fname);

my $year = (localtime)[5] + 1900;
print FH <<"EOF";
/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2014-$year  Hercules Dev Team
 *
 * Hercules is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * NOTE: This file was auto-generated and should never be manually edited,
 *       as it will get overwritten.
 */

/* GENERATED FILE DO NOT EDIT */

#ifndef HPM_DATA_CHECK_H
#define HPM_DATA_CHECK_H

#if !defined(HPMHOOKGEN)
#include "common/HPMSymbols.inc.h"
#endif // ! HPMHOOKGEN
#ifdef HPM_SYMBOL
#undef HPM_SYMBOL
#endif // HPM_SYMBOL

HPExport const struct s_HPMDataCheck HPMDataCheck[] = {
EOF

foreach my $key (sort keys %out) {
	print FH <<"EOF";
	#ifdef $key
EOF
	foreach my $entry (@{ $out{$key} }) {
		my $entryname = $$entry{name};
		my $entrytype = $$entry{type};
		print FH <<"EOF"
		{ "$entryname", sizeof(struct $entryname), $entrytype },
EOF
	}
	print FH <<"EOF"
	#else
		#define $key
	#endif // $key
EOF
}
print FH <<"EOF";
};
HPExport unsigned int HPMDataCheckLen = ARRAYLENGTH(HPMDataCheck);
HPExport int HPMDataCheckVer = $HPMDataCheckAPIVer;

#endif /* HPM_DATA_CHECK_H */
EOF
close(FH);
