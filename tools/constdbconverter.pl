#!/usr/bin/perl
#
# This file is part of Hercules.
# http://herc.ws - http://github.com/HerculesWS/Hercules
#
# Copyright (C) 2016  Hercules Dev Team
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

# This Script converts const.txt to constants.conf format.
# usage example: perl tools/constdbconverter.pl < db/const.txt > db/constants.conf

use strict;
use warnings;

sub parse_constdb (@) {
	my @input = @_;
	foreach (@input) {
		chomp $_;
#		Constant Name,Value,{is parameter}
#		Constant Name\tValue\t{is parameter}
		if( $_ =~ qr/^
			(?<prefix>(?:\/\/[^A-Za-z0-9'_]*)?)
			(?<ConstantName>[A-Za-z0-9'_]+)
			(?:,|[\s\t]+)(?<Value>(?:0x[a-fA-F0-9]+|-?[0-9]+))
			(?:(?:,|[\s\t]+)(?<IsParameter>([01])))?
		/x ) {
			my %cols = map { $_ => $+{$_} } keys %+;
			$cols{prefix} = '// ' if !$cols{prefix} and $cols{ConstantName} =~ /^\s*(true|false)\s*$/;
			if ($cols{IsParameter} and $cols{IsParameter} eq 1) {
				print "/*\n" if $cols{prefix};
				print "\t$cols{prefix}\n" if $cols{prefix} and $cols{prefix} !~ m|^//[\s\t]*$|;
				print "\t$cols{ConstantName}: {\n";
				print "\t\tValue: $cols{Value}\n";
				print "\t\tParameter: true\n";
				print "\t}\n";
				print "*/\n" if $cols{prefix};
				next;
			}
			print "\t$cols{prefix}" if $cols{prefix};
			print "\t$cols{ConstantName}: $cols{Value}\n";
		} elsif( $_ =~ /^\/\/(.*)$/ ) {
			my $s = $1;
			print "\t// $s\n" unless $s =~ /^[\s\t]*$/;
		} elsif( $_ !~ /^\s*$/ ) {
			print "// Error parsing: $_\n";
		}
	}
}
my $year = (localtime)[5] + 1900;
print <<'EOF';
//================= Hercules Database =====================================
//=       _   _                     _
//=      | | | |                   | |
//=      | |_| | ___ _ __ ___ _   _| | ___  ___
//=      |  _  |/ _ \ '__/ __| | | | |/ _ \/ __|
//=      | | | |  __/ | | (__| |_| | |  __/\__ \
//=      \_| |_/\___|_|  \___|\__,_|_|\___||___/
//================= License ===============================================
EOF
print << "EOF";
//= This file is part of Hercules.
//= http://herc.ws - http://github.com/HerculesWS/Hercules
//=
//= Copyright (C) 2016-$year  Hercules Dev Team
//=
//= Hercules is free software: you can redistribute it and/or modify
//= it under the terms of the GNU General Public License as published by
//= the Free Software Foundation, either version 3 of the License, or
//= (at your option) any later version.
//=
//= This program is distributed in the hope that it will be useful,
//= but WITHOUT ANY WARRANTY; without even the implied warranty of
//= MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//= GNU General Public License for more details.
//=
//= You should have received a copy of the GNU General Public License
//= along with this program.  If not, see <http://www.gnu.org/licenses/>.
//=========================================================================
//= Script Constants Database
//=========================================================================

constants_db: {
/************* Entry structure (short) ************************************
	Identifier: value            // (int)
 ************* Entry structure (full) *************************************
	Identifier: {
		Value: value         // (int)
		Parameter: true      // (boolean)      Defaults to false.
		Deprecated: true     // (boolean)      Defaults to false.
	}
**************************************************************************/
// NOTE:
//   Parameters are special in that they retrieve certain runtime values
//   depending on the specified ID in field Value. Depending on the
//   implementation values assigned by scripts to parameters will affect
//   runtime values, such as Zeny, as well (see pc_readparam/pc_setparam).

EOF

parse_constdb(<>);

print "}\n";
