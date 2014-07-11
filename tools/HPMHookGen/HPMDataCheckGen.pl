#!/usr/bin/perl

# Copyright (c) Hercules Dev Team, licensed under GNU GPL.
# See the LICENSE file

use strict;
use warnings;
use XML::Simple;

# XML Parser hint (some are faster than others)
#local $ENV{XML_SIMPLE_PREFERRED_PARSER} = '';                 # 0m14.181s
local $ENV{XML_SIMPLE_PREFERRED_PARSER} = 'XML::Parser';      # 0m4.256s
#local $ENV{XML_SIMPLE_PREFERRED_PARSER} = 'XML::SAX::Expat';  # 0m14.186s
#local $ENV{XML_SIMPLE_PREFERRED_PARSER} = 'XML::LibXML::SAX'; # 0m7.055s

my @files = grep { -f } grep { /[^h]\.xml/ } glob 'doxyoutput/xml/struct*.xml';
my %out;

foreach my $file (@files) {
	my $xml = new XML::Simple;
	my $data = $xml->XMLin($file);
	next unless $data->{compounddef}->{includes}; # means its a struct from a .c file, plugins cant access those so we don't care.
	next if $data->{compounddef}->{compoundname} =~ /::/; # its a duplicate with a :: name e.g. struct script_state {<...>} ay;
	my @filepath = split(/[\/\\]/, $data->{compounddef}->{location}->{file});
	my $foldername = uc($filepath[-2]);
	my $filename = uc($filepath[-1]); $filename =~ s/-/_/g; $filename =~ s/\.[^.]*$//;
	my $name = "${foldername}_${filename}_H";
	push @{ $out{$name} }, $data->{compounddef}->{compoundname};
}

my $fname = '../../src/common/HPMDataCheck.h';
open(FH, '>', $fname);

print FH <<"EOF";
// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
//
// NOTE: This file was auto-generated and should never be manually edited,
//       as it will get overwritten.
#ifndef HPM_DATA_CHECK_H
#define HPM_DATA_CHECK_H


HPExport const struct s_HPMDataCheck HPMDataCheck[] = {
EOF

foreach my $key (sort keys %out) {
	print FH <<"EOF";
	#ifdef $key
EOF
	foreach my $entry (@{ $out{$key} }) {
		print FH <<"EOF"
		{ "$entry", sizeof(struct $entry) },
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

#endif /* HPM_DATA_CHECK_H */
EOF
close(FH);
