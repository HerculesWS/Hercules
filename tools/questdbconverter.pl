#!/usr/bin/perl
#
# Copyright (c) Hercules Dev Team, licensed under GNU GPL.
# See the LICENSE file
# Base Author: Dastgir @ http://herc.ws
#
# This Script converts quest_db.txt to quest_db.conf format.
# usage example: perl tools/questdbconverter.pl < db/quest_db.txt > db/quest_db.conf
#
use strict;
use warnings;

sub parse_questdb (@) {
	my @input = @_;
	foreach (@input) {
		chomp $_;
#		Quest ID,Time Limit,Target1,Val1,Target2,Val2,Target3,Val3,Quest Title
		if( $_ =~ qr/^
			(?<prefix>(?:\/\/[^0-9]*)?)
			(?<QuestID>[0-9]+)[^,]*,[\s\t]*
			(?<TimeLimit>[0-9]+)[^,]*,[\s\t]*
			(?<Target1>[0-9]+)[^,]*,[\s\t]*
			(?<Val1>[0-9]+)[^,]*,[\s\t]*
			(?<Target2>[0-9]+)[^,]*,[\s\t]*
			(?<Val2>[0-9]+)[^,]*,[\s\t]*
			(?<Target3>[0-9]+)[^,]*,[\s\t]*
			(?<Val3>[0-9]+)[^,]*,[\s\t]*
			"(?<QuestTitle>[^"]*)"
		/x ) {
			my %cols = map { $_ => $+{$_} } keys %+;
			print "/*\n" if $cols{prefix};
			print "$cols{prefix}\n" if $cols{prefix} and $cols{prefix} !~ m|^//[\s\t]*$|;
			print "{\n";
			print "\tId: $cols{QuestID}\n";
			print "\tName: \"$cols{QuestTitle}\"\n";
			print "\tTimeLimit: $cols{TimeLimit}\n" if $cols{TimeLimit};
			print "\tTargets: (\n" if $cols{Target1} || $cols{Target2} || $cols{Target3};
			print "\t{\n" if $cols{Target1};
			print "\t\tMobId: $cols{Target1}\n" if $cols{Target1};
			print "\t\tCount: $cols{Val1}\n" if $cols{Target1};
			print "\t},\n" if $cols{Target1};
			print "\t{\n" if $cols{Target2};
			print "\t\tMobId: $cols{Target2}\n" if $cols{Target2};
			print "\t\tCount: $cols{Val2}\n" if $cols{Target2};
			print "\t},\n" if $cols{Target2};
			print "\t{\n" if $cols{Target3};
			print "\t\tMobId: $cols{Target3}\n" if $cols{Target3};
			print "\t\tCount: $cols{Val3}\n" if $cols{Target3};
			print "\t},\n" if $cols{Target3};
			print "\t)\n" if $cols{Target1} || $cols{Target2} || $cols{Target3};
			print "},\n";
			print "*/\n" if $cols{prefix};
		} elsif( $_ =~ /^\/\/(.*)$/ ) {
			my $s = $1;
			print "// $s\n" unless $s =~ /^[\s\t]*$/;
		} elsif( $_ !~ /^\s*$/ ) {
			print "// Error parsing: $_\n";
		}

	}
}
print <<"EOF";
quest_db: (
//  Quest Database
/******************************************************************************
 ************* Entry structure ************************************************
 ******************************************************************************
{
	Id: Quest ID                    [int]
	Name: Quest Name                [string]
	TimeLimit: Time Limit (seconds) [int, optional]
	Targets: (                      [array, optional]
	{
		MobId: Mob ID           [int]
		Count:                  [int]
	},
	... (can repeated up to MAX_QUEST_OBJECTIVES times)
	)
	Drops: (
	{
		ItemId: Item ID to drop [int]
		Rate: Drop rate         [int]
		MobId: Mob ID to match  [int, optional]
	},
	... (can be repeated)
	)
},
******************************************************************************/

EOF

parse_questdb(<>);

print ")\n";
