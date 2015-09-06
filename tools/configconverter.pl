#!/usr/bin/perl
#
# This file is part of Hercules.
# http://herc.ws - http://github.com/HerculesWS/Hercules
#
# Copyright (C) 2016  Hercules Dev Team
# Copyright (C) 2016  Haru
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

my $silent = 0;
my $confpath = 'conf';

sub parse_config($$) {
	my ($input, $defaults) = @_;

	my %output = ();

	for my $line (<$input>) {
		chomp $line;
		$line =~ s/^\s*//; $line =~ s/\s*$//;
		if ($line =~ /^([a-z0-9A-Z_.]+)\s*:\s*(.*)$/) {
			my ($variable, $value) = ($1, $2);
			if ($defaults->{$variable}) {
				next if $defaults->{$variable}->{parse}->($variable, $value, $defaults->{$variable}, \%output);
				print "Error: Invalid value for setting '$variable: $value'\n";
				next;
			} else {
				print "Found unhandled configuration setting: '$variable: $value'\n";
				next;
			}
		} elsif ($line =~ m{^\s*(?://.*)?$}) {
			next;
		} else {
			print "Error: Unable to parse line '$line'\n";
		}
	}
	return \%output;
}

sub cfg_add($$$$) {
	my ($variable, $value, $default, $output) = @_;
	$output->{$variable} = {value => $value, print => $default->{print}, path => $default->{path}} unless $value eq $default->{default};
}

sub parsecfg_string($$$$) {
	my ($variable, $value, $default, $output) = @_;
	if ($value =~ m{\s*"((?:\\"|.)*)"\s*(?://.*)?$}i) {
		cfg_add($variable, $1, $default, $output);
		return 1;
	} elsif ($value =~ m{\s*((?:\\"|.)*)\s*(?://.*)?$}i) {
		cfg_add($variable, $1, $default, $output);
		return 1;
	}
	return 0;
}

sub parsecfg_int($$$$) {
	my ($variable, $value, $default, $output) = @_;
	if ($value =~ m{\s*(-?[0-9]+)\s*(?://.*)?$}) {
		cfg_add($variable, int $1, $default, $output);
		return 1;
	} elsif ($value =~ m{\s*(0x[0-9A-F]+)\s*(?://.*)?$}) {
		cfg_add($variable, hex $1, $default, $output);
		return 1;
	} elsif ($value =~ m{\s*(no|false|off)\s*(?://.*)?$}) {
		cfg_add($variable, 0, $default, $output);
		return 1;
	}
	return 0;
}

sub parsecfg_bool($$$$) {
	my ($variable, $value, $default, $output) = @_;
	if ($value =~ m{\s*(yes|true|1|on)\s*(?://.*)?$}i) {
		cfg_add($variable, "true", $default, $output);
		return 1;
	} elsif ($value =~ m{\s*(no|false|0|off)\s*(?://.*)?$}i) {
		cfg_add($variable, "false", $default, $output);
		return 1;
	}
	return 0;
}

sub print_config($) {
	my ($config) = @_;

	for my $variable (keys %$config) {
		my $fullpath = $config->{$variable}->{path};
		$fullpath .= $variable if $fullpath =~ m{[:/]$};
		my ($filename, $configpath) = split(/:/, $fullpath, 2);
		next unless $filename and $configpath;
		my @path = split(/\//, $configpath);
		next unless @path;

		my %output = ();

		my $setting = \%output;
		while (scalar @path > 1) {
			my $nodename = shift @path;
			$setting->{$nodename} = {print => \&printcfg_tree, value => {}} unless $setting->{$nodename};
			$setting = $setting->{$nodename}->{value};
		}
		$setting->{$path[0]} = {print => $config->{$variable}->{print}, value => $config->{$variable}->{value}};
		verbose("- Found setting: '$variable'.\n  Please manually move the setting to '$filename.conf' as in the following example:\n",
			"- '$filename.conf': (from $variable)\n");
		$output{$_}->{print}->($_, $output{$_}->{value}, 0) for keys %output;
	}
}

sub indent($$) {
	my ($message, $nestlevel) = @_;
	return print "\t" x ($nestlevel + 1) . $message;
}

sub printcfg_tree($$$) {
	my ($variable, $value, $nestlevel) = @_;

	indent("$variable: {\n", $nestlevel);
	$value->{$_}{print}->($_, $value->{$_}{value}, $nestlevel + 1) for keys %$value;
	indent("}\n", $nestlevel);
}

sub printcfg_nil($$$) {
}

sub printcfg_string($$$) {
	my ($variable, $value, $nestlevel) = @_;

	indent("$variable: \"$value\"\n", $nestlevel);
}

sub printcfg_int($$$) {
	my ($variable, $value, $nestlevel) = @_;

	indent("$variable: $value\n", $nestlevel);
}

sub printcfg_bool($$$) {
	my ($variable, $value, $nestlevel) = @_;

	indent("$variable: $value\n", $nestlevel);
}

sub printcfg_point($$$) {
	my ($variable, $value, $nestlevel) = @_;

	indent("$variable: {\n", $nestlevel);

	my @point = split(/,/, $value, 3);
	indent("map: \"$point[0]\"\n", $nestlevel + 1);
	indent("x: $point[1]\n", $nestlevel + 1);
	indent("y: $point[2]\n", $nestlevel + 1);

	indent("}\n", $nestlevel);
}

sub printcfg_items($$$) {
	my ($variable, $value, $nestlevel) = @_;

	indent("$variable: (\n", $nestlevel);

	my @items = split(/,/, $value);
	while (scalar @items >= 3) {
		my $id = shift @items;
		my $amount = shift @items;
		my $stackable = (shift @items) ? "true" : "false";
		indent("{\n", $nestlevel);
		indent("id: $id\n", $nestlevel + 1);
		indent("amount: $amount\n", $nestlevel + 1);
		indent("stackable: $stackable\n", $nestlevel + 1);
		indent("},\n", $nestlevel);
	}

	indent(")\n", $nestlevel);
}

sub process_conf($$) {
	my ($files, $defaults) = @_;
	my $found = 0;
	for my $file (@$files) {
		print "\nChecking $file...";
		print " Ok\n" and next unless open my $FH, '<', $file; # File not found or already converted
		print " Old file is still present\n";
		my $output = parse_config($FH, $defaults);
		close($FH);
		my $count = scalar keys %$output;
		print "$count non-default settings found.";
		verbose(" The file '$file' is no longer used by Hercules and can be deleted.\n", "\n") and next unless $count;
		verbose(" Please review and migrate the settings as described, then delete the file '$file', as it is no longer used by Hercules.\n", "\n");
		print_config($output);
		$found = 1;
	}
	return $found;
}

sub verbose($;$) {
	my ($verbose_message, $silent_message) = @_;
	return print $verbose_message unless $silent;
	return print $silent_message if defined $silent_message;
	return 1;
}

my @defaults = (
	{
		files => ['char-server.conf', 'import/char_conf.txt'],
		settings => {
			autosave_time                 => {parse => \&parsecfg_int,    print => \&printcfg_int,    path => "char-server:char_configuration/database/", default => 60},
			bind_ip                       => {parse => \&parsecfg_string, print => \&printcfg_string, path => "char-server:char_configuration/inter/", default => "127.0.0.1"},
			char_aegis_delete             => {parse => \&parsecfg_bool,   print => \&printcfg_bool,   path => "char-server:char_configuration/player/deletion/use_aegis_delete", default => "false"},
			char_del_delay                => {parse => \&parsecfg_int,    print => \&printcfg_int,    path => "char-server:char_configuration/player/deletion/delay", default => 86400},
			char_del_level                => {parse => \&parsecfg_int,    print => \&printcfg_int,    path => "char-server:char_configuration/player/deletion/level", default => 0},
			char_ip                       => {parse => \&parsecfg_string, print => \&printcfg_string, path => "char-server:char_configuration/inter/", default => "127.0.0.1"},
			char_maintenance_min_group_id => {parse => \&parsecfg_int,    print => \&printcfg_int,    path => "char-server:char_configuration/permission/maintenance_min_group_id", default => 99},
			char_name_letters             => {parse => \&parsecfg_string, print => \&printcfg_string, path => "char-server:char_configuration/player/name/name_letters", default => "abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890"},
			char_name_option              => {parse => \&parsecfg_int,    print => \&printcfg_int,    path => "char-server:char_configuration/player/name/name_option", default => 1},
			char_new                      => {parse => \&parsecfg_bool,   print => \&printcfg_bool,   path => "char-server:char_configuration/permission/enable_char_creation", default => "true"},
			char_new_display              => {parse => \&parsecfg_bool,   print => \&printcfg_bool,   path => "char-server:char_configuration/permission/display_new", default => "false"},
			char_port                     => {parse => \&parsecfg_int,    print => \&printcfg_int,    path => "char-server:char_configuration/inter/", default => 6121},
			char_server_type              => {parse => \&parsecfg_int,    print => \&printcfg_int,    path => "char-server:char_configuration/permission/server_type", default => 0},
			console_silent                => {parse => \&parsecfg_int,    print => \&printcfg_int,    path => "console:console/", default => 0},
			db_path                       => {parse => \&parsecfg_string, print => \&printcfg_string, path => "char-server:char_configuration/database/", default => "db"},
			fame_list_alchemist           => {parse => \&parsecfg_int,    print => \&printcfg_int,    path => "char-server:char_configuration/fame/alchemist", default => 10},
			fame_list_blacksmith          => {parse => \&parsecfg_int,    print => \&printcfg_int,    path => "char-server:char_configuration/fame/blacksmith", default => 10},
			fame_list_taekwon             => {parse => \&parsecfg_int,    print => \&printcfg_int,    path => "char-server:char_configuration/fame/taekwon", default => 10},
			gm_allow_group                => {parse => \&parsecfg_int,    print => \&printcfg_int,    path => "char-server:char_configuration/permission/", default => -1},
			guild_exp_rate                => {parse => \&parsecfg_int,    print => \&printcfg_int,    path => "char-server:char_configuration/", default => 100},
			log_char                      => {parse => \&parsecfg_bool,   print => \&printcfg_bool,   path => "char-server:char_configuration/database/", default => "true"},
			login_ip                      => {parse => \&parsecfg_string, print => \&printcfg_string, path => "char-server:char_configuration/inter/", default => "127.0.0.1"},
			login_port                    => {parse => \&parsecfg_int,    print => \&printcfg_int,    path => "char-server:char_configuration/inter/", default => 6900},
			max_connect_user              => {parse => \&parsecfg_int,    print => \&printcfg_int,    path => "char-server:char_configuration/permission/", default => -1},
			name_ignoring_case            => {parse => \&parsecfg_bool,   print => \&printcfg_bool,   path => "char-server:char_configuration/player/name/", default => "false"},
			passwd                        => {parse => \&parsecfg_string, print => \&printcfg_string, path => "char-server:char_configuration/inter/", default => "p1"},
			pincode_changetime            => {parse => \&parsecfg_int,    print => \&printcfg_int,    path => "char-server:char_configuration/pincode/change_time", default => 0},
			pincode_charselect            => {parse => \&parsecfg_int,    print => \&printcfg_int,    path => "char-server:char_configuration/pincode/request", default => 0},
			pincode_enabled               => {parse => \&parsecfg_bool,   print => \&printcfg_bool,   path => "char-server:char_configuration/pincode/enabled", default => "true"},
			pincode_maxtry                => {parse => \&parsecfg_int,    print => \&printcfg_int,    path => "char-server:char_configuration/pincode/max_tries", default => 3},
			save_log                      => {parse => \&parsecfg_bool,   print => \&printcfg_bool,   path => "console:console/", default => "true"},
			server_name                   => {parse => \&parsecfg_string, print => \&printcfg_string, path => "char-server:char_configuration/", default => "Hercules"},
			start_items                   => {parse => \&parsecfg_string, print => \&printcfg_items,  path => "char-server:char_configuration/player/new/", default => "1201,1,0,2301,1,0"},
			start_point_pre               => {parse => \&parsecfg_string, print => \&printcfg_point,  path => "char-server:char_configuration/player/new/", default => "new_1-1,53,111"},
			start_point_re                => {parse => \&parsecfg_string, print => \&printcfg_point,  path => "char-server:char_configuration/player/new/", default => "iz_int,97,90"},
			start_zeny                    => {parse => \&parsecfg_int,    print => \&printcfg_int,    path => "char-server:char_configuration/player/new/zeny", default => 0},
			stdout_with_ansisequence      => {parse => \&parsecfg_bool,   print => \&printcfg_bool,   path => "console:console/", default => "false"},
			timestamp_format              => {parse => \&parsecfg_string, print => \&printcfg_string, path => "console:console/", default => "[%d/%b %H:%M]"},
			unknown_char_name             => {parse => \&parsecfg_string, print => \&printcfg_string, path => "char-server:char_configuration/player/name/", default => "Unknown"},
			userid                        => {parse => \&parsecfg_string, print => \&printcfg_string, path => "char-server:char_configuration/inter/", default => "s1"},
			wisp_server_name              => {parse => \&parsecfg_string, print => \&printcfg_string, path => "char-server:char_configuration/", default => "Server"},
			import                        => {parse => \&parsecfg_string, print => \&printcfg_nil,    path => "", default => "conf/import/char_conf.txt"},
		}
	},
);

for (@ARGV) {
	if    (/^-q$/) { $silent = 1; }
	elsif (/^-v$/) { $silent = 0; }
	elsif (-d) { $confpath = $_; }
	else { undef $confpath }
}

verbose(<<'EOF');
=============== Hercules Configuration Migration Helper ===============
=               _   _                     _                           =
=              | | | |                   | |                          =
=              | |_| | ___ _ __ ___ _   _| | ___  ___                 =
=              |  _  |/ _ \ '__/ __| | | | |/ _ \/ __|                =
=              | | | |  __/ | | (__| |_| | |  __/\__ \                =
=              \_| |_/\___|_|  \___|\__,_|_|\___||___/                =
=======================================================================
This tool will assist you through the migration of the old (txt-based)
configuration files to the new (libconfig-based) format.
Please follow the displayed instructions.
=======================================================================

EOF

die "Usage: ./$0 [-q | -v] [path to the conf directory]\nIf no options are passed, it acts as if called as ./$0 conf\n" unless defined $confpath and -d $confpath;

my $count = 0;
for (@defaults) {
	my @files = map { $confpath . '/' . $_ } @{$_->{files}};
	$count += process_conf(\@files, $_->{settings})
}
verbose("\nThere are no files to migrate.\n") unless $count;
