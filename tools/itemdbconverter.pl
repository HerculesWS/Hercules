#!/usr/bin/perl
#
# This file is part of Hercules.
# http://herc.ws - http://github.com/HerculesWS/Hercules
#
# Copyright (C) 2013-2015  Hercules Dev Team
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

# Base Author: Haru @ http://herc.ws

# This script converts an item_db(2).txt to the new item_db(2).conf format.
# usage example: perl tools/itemdbconverter.pl < db/item_db2.txt > db/item_db2.conf

use strict;
use warnings;

sub prettifyscript ($) {
	my ($orig) = @_;
	$orig =~ s/^[\s\t]*//; $orig =~ s/[\s\t]*$//;
	return '' unless $orig =~ /[^\s\t]/;
	my ($p, $script) = ($orig, '');
	my ($curly, $lines, $comment) = (2, 0, 0);
	my ($linebreak, $needindent) = (0, 0);
	while ($p =~ /[^\s\t]/) {
		$linebreak = 0;
		if ($comment && $p =~ s|^\s*\*/\s*||) {
			$comment = 0;
			next;
		} elsif ($p =~ s/^\s*({)\s*//) {
			$curly++ unless $comment;
			$comment++ if $comment;
			$script .= " ";
			$linebreak = 1;
			$lines++;
		} elsif ($p =~ s/^\s*(})\s*//) {
			$curly-- unless $comment;
			$comment-- if $comment - 1 > 0;
			$linebreak = 1;
			$lines++;
		} elsif ($p =~ s/^\s*(;)\s*//) {
			if ($p && (!$comment || $p !~ m|^[\s\t]*(?:\*/)[\s\t]*$|)) {
				$linebreak = 1;
				$lines++
			}
		} elsif ($p =~ s/^("[^"]*")//) {
		} elsif ($p =~ s|^\s*/\*\s*||) {
			$comment = 1;
			next;
		} elsif ($p !~ s/^(.)//) {
			last;
		}
		$script .= "\t" x $curly if $needindent;
		$script .= "//" . ("\t" x ($comment-1)) if ($comment && ($needindent || $script eq ''));
		$script .= "$1";
		if ($linebreak) {
			$script .= "\n";
			$needindent = 1;
		} else {
			$needindent = 0;
		}
	}
	if ($curly != 2) {
		printf STDERR "Parse error, curly braces count ". ($curly-2) .". returning unmodified script:\n$orig\n\n";
		return $orig;
	}
	if ($lines) {
		$script = "\n\t\t$script\n\t";
	} else {
		$script = " $script ";
	}
	return $script;
}

sub parsedb (@) {
	my @input = @_;
	foreach (@input) {
		chomp $_;
#		ID,AegisName,Name,Type,Buy,Sell,Weight,ATK,DEF,Range,Slots,Job,Upper,Gender,Loc,wLV,eLV,Refineable,View,{ Script },{ OnEquip_Script },{ OnUnequip_Script }
		if( $_ =~ qr/^
			(?<prefix>(?:\/\/[^0-9]*)?)
			(?<ID>[0-9]+)[^,]*,
			(?<AegisName>[^,]+),
			(?<Name>[^,]+),[\s\t]*
			(?<Type>[0-9]+)[^,]*,[\s\t]*
			(?<Buy>[0-9]*)[^,]*,[\s\t]*
			(?<Sell>[0-9]*)[^,]*,[\s\t]*
			(?<Weight>[0-9]*)[^,]*,[\s\t]*
			(?<ATK>[0-9-]*)[^,:]*(?<hasmatk>:[\s\t]*(?<MATK>[0-9-]*))?[^,]*,[\s\t]*
			(?<DEF>[0-9-]*)[^,]*,[\s\t]*
			(?<Range>[0-9]*)[^,]*,[\s\t]*
			(?<Slots>[0-9]*)[^,]*,[\s\t]*
			(?<Job>[x0-9A-Fa-f]*)[^,]*,[\s\t]*
			(?<Upper>[0-9]*)[^,]*,[\s\t]*
			(?<Gender>[0-9]*)[^,]*,[\s\t]*
			(?<Loc>[0-9]*)[^,]*,[\s\t]*
			(?<wLV>[0-9]*)[^,]*,[\s\t]*
			(?<eLV>[0-9]*)[^,:]*(?<hasmaxlv>:[\s\t]*(?<eLVmax>[0-9]*))?[^,]*,[\s\t]*
			(?<Refineable>[0-9]*)[^,]*,[\s\t]*
			(?<View>[0-9]*)[^,]*,[\s\t]*
			{(?<Script>.*)},
			{(?<OnEquip>.*)},
			{(?<OnUnequip>.*)}
		/x ) {
			my %cols = map { $_ => $+{$_} } keys %+;
			print "/*\n" if $cols{prefix};
			print "$cols{prefix}\n" if $cols{prefix} and $cols{prefix} !~ m|^//[\s\t]*$|;
			print "{\n";
			print "\tId: $cols{ID}\n";
			print "\tAegisName: \"$cols{AegisName}\"\n";
			print "\tName: \"$cols{Name}\"\n";
			print "\tType: $cols{Type}\n";
			print "\tBuy: $cols{Buy}\n" if $cols{Buy} || $cols{Buy} eq '0';
			print "\tSell: $cols{Sell}\n" if $cols{Sell} || $cols{Sell} eq '0';
			print "\tWeight: $cols{Weight}\n" if $cols{Weight};
			print "\tAtk: $cols{ATK}\n" if $cols{ATK};
			print "\tMatk: $cols{MATK}\n" if $cols{MATK};
			print "\tDef: $cols{DEF}\n" if $cols{DEF};
			print "\tRange: $cols{Range}\n" if $cols{Range};
			print "\tSlots: $cols{Slots}\n" if $cols{Slots};
			$cols{Job} = '0xFFFFFFFF' unless $cols{Job};
			print "\tJob: $cols{Job}\n" unless $cols{Job} =~ /0xFFFFFFFF/i;
			print "\tUpper: $cols{Upper}\n" if $cols{Upper} && (($cols{hasmatk} && $cols{Upper} != 0x3f) || (!$cols{hasmatk} && $cols{Upper} != 7));
			$cols{Gender} = '2' unless $cols{Gender};
			print "\tGender: $cols{Gender}\n" unless $cols{Gender} eq '2';
			print "\tLoc: $cols{Loc}\n" if $cols{Loc};
			print "\tWeaponLv: $cols{wLV}\n" if $cols{wLV};
			if ($cols{hasmaxlv} and $cols{eLVmax}) {
				$cols{eLV} = 0 unless $cols{eLV};
				print "\tEquipLv: [$cols{eLV}, $cols{eLVmax}]\n";
			} else {
				print "\tEquipLv: $cols{eLV}\n" if $cols{eLV};
			}
			print "\tRefine: false\n" if !$cols{Refineable} and ($cols{Type} == 4 or $cols{Type} == 5);
			print "\tView: $cols{View}\n" if $cols{View};
			$cols{Script} = prettifyscript($cols{Script});
			print "\tScript: <\"$cols{Script}\">\n" if $cols{Script};
			$cols{OnEquip} = prettifyscript($cols{OnEquip});
			print "\tOnEquipScript: <\"$cols{OnEquip}\">\n" if $cols{OnEquip};
			$cols{OnUnequip} = prettifyscript($cols{OnUnequip});
			print "\tOnUnequipScript: <\"$cols{OnUnequip}\">\n" if $cols{OnUnequip};
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
item_db: (
/******************************************************************************
 ************* Entry structure ************************************************
 ******************************************************************************
{
	// =================== Mandatory fields ===============================
	Id: ID                        (int)
	AegisName: "Aegis_Name"       (string, optional if Inherit: true)
	Name: "Item Name"             (string, optional if Inherit: true)
	// =================== Optional fields ================================
	Type: Item Type               (int, defaults to 3 = etc item)
	Buy: Buy Price                (int, defaults to Sell * 2)
	Sell: Sell Price              (int, defaults to Buy / 2)
	Weight: Item Weight           (int, defaults to 0)
	Atk: Attack                   (int, defaults to 0)
	Matk: Magical Attack          (int, defaults to 0, ignored in pre-re)
	Def: Defense                  (int, defaults to 0)
	Range: Attack Range           (int, defaults to 0)
	Slots: Slots                  (int, defaults to 0)
	Job: Job mask                 (int, defaults to all jobs = 0xFFFFFFFF)
	Upper: Upper mask             (int, defaults to any = 0x3f)
	Gender: Gender                (int, defaults to both = 2)
	Loc: Equip location           (int, required value for equipment)
	WeaponLv: Weapon Level        (int, defaults to 0)
	EquipLv: Equip required level (int, defaults to 0)
	EquipLv: [min, max]           (alternative syntax with min / max level)
	Refine: Refineable            (boolean, defaults to true)
	View: View ID                 (int, defaults to 0)
	BindOnEquip: true/false       (boolean, defaults to false)
	Script: <"
		Script
		(it can be multi-line)
	">
	OnEquipScript: <" OnEquip Script (can also be multi-line) ">
	OnUnequipScript: <" OnUnequip Script (can also be multi-line) ">
	// =================== Optional fields (item_db2 only) ================
	Inherit: true/false           (boolean, if true, inherit the values
	                              that weren't specified, from item_db.conf,
	                              else override it and use default values)
},
******************************************************************************/

EOF

parsedb(<>);

print ")\n";
