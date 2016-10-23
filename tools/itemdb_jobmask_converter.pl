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

# Base Author: Dastgir @ http://herc.ws

# This script converts item_db.conf Jobmask field into groups format
# usage example: perl tools/itemdb_jobmask_converter.pl < db/item_db2.conf > db/item_db_out.conf

use strict;
use warnings;

sub parsedb (@) {
	my @input = @_;
	my @jobNames = (
		"Novice",
		"Swordsman",
		"Magician",
		"Archer",
		"Acolyte",
		"Merchant",
		"Thief",
		"Knight",
		"Priest",
		"Wizard",
		"Blacksmith",
		"Hunter",
		"Assassin",
		"Unused",
		"Crusader",
		"Monk",
		"Sage",
		"Rogue",
		"Alchemist",
		"Bard",
		"Unused",
		"Taekwon",
		"Star_Gladiator",
		"Soul_Linker",
		"Gunslinger",
		"Ninja",
		"Gangsi",
		"Death_Knight",
		"Dark_Collector",
		"Kagerou",
		"Rebellion"
	);
	my $jobSize = $#jobNames + 1;

	foreach (@input) {
		chomp $_;
		if ($_ =~ /^\s*Job\s*:\s*(?<Job>(?:0x)?[0-9A-Fa-f]+)/x) {
			my %cols = map { $_ => $+{$_} } keys %+;
			my $jobMask = hex($cols{Job});
			my $allJobs = 0xFFFFFFFF;
			my $allJobsExceptNovice = 0xFFFFFFFE;
			if ($jobMask < 0 || $jobMask eq "") {
				print "$_\n";
				next;
			}
			print "\tJob: {\n";
			if (($jobMask&$allJobs) == $allJobs) {
				print "\t\tAll: true\n";
			} elsif (($jobMask&$allJobsExceptNovice) == $allJobsExceptNovice) {
				print "\t\tAll: true\n";
				print "\t\tNovice: false\n";
			} elsif ($jobMask == 0) {
				print "\t\tAll: false\n";
			} else {
				for (my $i = 0; $i < $jobSize; $i++) {
					my $currBit = 1<<$i;
					if (($jobMask & $currBit) == $currBit) {
						print "\t\t$jobNames[$i]: true\n" unless $jobNames[$i] eq "Unused";
					}
				}
			}
			print "\t}\n";
		} else {
			print "$_\n";
		}
	}
}

parsedb(<>);
