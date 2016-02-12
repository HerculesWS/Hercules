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

sub cfg_append($$$$) {
	my ($variable, $value, $default, $output) = @_;
	for my $default_value (@{$default->{default}}) {
		return if $value eq $default_value;
	}
	$output->{$variable} = {value => [], print => $default->{print}, path => $default->{path}} unless $output->{$variable};
	push(@{$output->{$variable}->{value}}, $value);
}

sub parsecfg_string_sub($$$$$) {
	my ($variable, $value, $default, $output, $func) = @_;
	if ($value =~ m{\s*"((?:\\"|.)*)"\s*(?://.*)?$}i) {
		$func->($variable, $1, $default, $output);
		return 1;
	} elsif ($value =~ m{\s*((?:\\"|.)*)\s*(?://.*)?$}i) {
		$func->($variable, $1, $default, $output);
		return 1;
	}
	return 0;
}

sub parsecfg_string($$$$) {
	my ($variable, $value, $default, $output) = @_;
	return parsecfg_string_sub($variable, $value, $default, $output, \&cfg_add);
}

sub parsecfg_stringarr($$$$) {
	my ($variable, $value, $default, $output) = @_;
	return parsecfg_string_sub($variable, $value, $default, $output, \&cfg_append);
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

sub printcfg_hexint($$$) {
	my ($variable, $value, $nestlevel) = @_;

	indent(sprintf("%s: 0x%x\n", $variable, $value), $nestlevel);
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

sub printcfg_md5hash($$$) {
	my ($variable, $value, $nestlevel) = @_;

	indent("$variable: (\n", $nestlevel);

	for (@$value) {
		my ($group_id, $hash) = split(/,/, $_, 2);
		$group_id =~ s/\s*$//; $group_id =~ s/^\s*//;
		$hash =~ s/\s*$//; $hash =~ s/^\s*//;

		indent("{\n", $nestlevel);
		indent("group_id: $group_id\n", $nestlevel + 1);
		indent("hash: \"$hash\"\n", $nestlevel + 1);
		indent("},\n", $nestlevel);
	}

	indent(")\n", $nestlevel);
}

sub printcfg_strlist($$$) {
	my ($variable, $value, $nestlevel) = @_;

	indent("$variable: (\n", $nestlevel);

	for my $string (split(/,/, $value)) {
		$string =~ s/\s*$//; $string =~ s/^\s*//;

		indent("\"$string\",\n", $nestlevel + 1);
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
	{
		files => ['inter-server.conf', 'import/inter_conf.txt'],
		settings => {
			party_share_level      => {parse => \&parsecfg_int,    print => \&printcfg_int,    path => "inter-server:inter_configuration/", default => 15},
			log_inter              => {parse => \&parsecfg_bool,   print => \&printcfg_bool,   path => "inter-server:inter_configuration/log/", default => "true"},
			inter_log_filename     => {parse => \&parsecfg_string, print => \&printcfg_string, path => "inter-server:inter_configuration/log/", default => "log/inter.log"},
			mysql_reconnect_type   => {parse => \&parsecfg_int,    print => \&printcfg_int,    path => "inter-server:inter_configuration/mysql_reconnect/type", default => 2},
			mysql_reconnect_count  => {parse => \&parsecfg_int,    print => \&printcfg_int,    path => "inter-server:inter_configuration/mysql_reconnect/count", default => 1},
			log_login_db           => {parse => \&parsecfg_string, print => \&printcfg_string, path => "inter-server:inter_configuration/database_names/login_db", default => "loginlog"},
			char_db                => {parse => \&parsecfg_string, print => \&printcfg_string, path => "inter-server:inter_configuration/database_names/", default => "char"},
			interlog_db            => {parse => \&parsecfg_string, print => \&printcfg_string, path => "inter-server:inter_configuration/database_names/", default => "interlog"},
			ragsrvinfo_db          => {parse => \&parsecfg_string, print => \&printcfg_string, path => "inter-server:inter_configuration/database_names/", default => "ragsrvinfo"},
			acc_reg_num_db         => {parse => \&parsecfg_string, print => \&printcfg_string, path => "inter-server:inter_configuration/database_names/registry/", default => "acc_reg_num_db"},
			acc_reg_str_db         => {parse => \&parsecfg_string, print => \&printcfg_string, path => "inter-server:inter_configuration/database_names/registry/", default => "acc_reg_str_db"},
			char_reg_num_db        => {parse => \&parsecfg_string, print => \&printcfg_string, path => "inter-server:inter_configuration/database_names/registry/", default => "char_reg_num_db"},
			char_reg_str_db        => {parse => \&parsecfg_string, print => \&printcfg_string, path => "inter-server:inter_configuration/database_names/registry/", default => "char_reg_str_db"},
			global_acc_reg_num_db  => {parse => \&parsecfg_string, print => \&printcfg_string, path => "inter-server:inter_configuration/database_names/registry/", default => "global_acc_reg_num_db"},
			global_acc_reg_str_db  => {parse => \&parsecfg_string, print => \&printcfg_string, path => "inter-server:inter_configuration/database_names/registry/", default => "global_acc_reg_str_db"},
			hotkey_db              => {parse => \&parsecfg_string, print => \&printcfg_string, path => "inter-server:inter_configuration/database_names/pc/", default => "hotkey"},
			scdata_db              => {parse => \&parsecfg_string, print => \&printcfg_string, path => "inter-server:inter_configuration/database_names/pc/", default => "sc_data"},
			cart_db                => {parse => \&parsecfg_string, print => \&printcfg_string, path => "inter-server:inter_configuration/database_names/pc/", default => "cart_inventory"},
			inventory_db           => {parse => \&parsecfg_string, print => \&printcfg_string, path => "inter-server:inter_configuration/database_names/pc/", default => "inventory"},
			charlog_db             => {parse => \&parsecfg_string, print => \&printcfg_string, path => "inter-server:inter_configuration/database_names/pc/", default => "charlog"},
			storage_db             => {parse => \&parsecfg_string, print => \&printcfg_string, path => "inter-server:inter_configuration/database_names/pc/", default => "storage"},
			skill_db               => {parse => \&parsecfg_string, print => \&printcfg_string, path => "inter-server:inter_configuration/database_names/pc/", default => "skill"},
			memo_db                => {parse => \&parsecfg_string, print => \&printcfg_string, path => "inter-server:inter_configuration/database_names/pc/", default => "memo"},
			party_db               => {parse => \&parsecfg_string, print => \&printcfg_string, path => "inter-server:inter_configuration/database_names/pc/", default => "party"},
			pet_db                 => {parse => \&parsecfg_string, print => \&printcfg_string, path => "inter-server:inter_configuration/database_names/pc/", default => "pet"},
			friend_db              => {parse => \&parsecfg_string, print => \&printcfg_string, path => "inter-server:inter_configuration/database_names/pc/", default => "friends"},
			mail_db                => {parse => \&parsecfg_string, print => \&printcfg_string, path => "inter-server:inter_configuration/database_names/pc/", default => "mail"},
			auction_db             => {parse => \&parsecfg_string, print => \&printcfg_string, path => "inter-server:inter_configuration/database_names/pc/", default => "auction"},
			quest_db               => {parse => \&parsecfg_string, print => \&printcfg_string, path => "inter-server:inter_configuration/database_names/pc/", default => "quest"},
			homunculus_db          => {parse => \&parsecfg_string, print => \&printcfg_string, path => "inter-server:inter_configuration/database_names/pc/", default => "homunculus"},
			skill_homunculus_db    => {parse => \&parsecfg_string, print => \&printcfg_string, path => "inter-server:inter_configuration/database_names/pc/", default => "skill_homunculus"},
			mercenary_db           => {parse => \&parsecfg_string, print => \&printcfg_string, path => "inter-server:inter_configuration/database_names/pc/", default => "mercenary"},
			mercenary_owner_db     => {parse => \&parsecfg_string, print => \&printcfg_string, path => "inter-server:inter_configuration/database_names/pc/", default => "mercenary_owner"},
			elemental_db           => {parse => \&parsecfg_string, print => \&printcfg_string, path => "inter-server:inter_configuration/database_names/pc/", default => "elemental"},
			account_data_db        => {parse => \&parsecfg_string, print => \&printcfg_string, path => "inter-server:inter_configuration/database_names/pc/", default => "account_data"},
			guild_db               => {parse => \&parsecfg_string, print => \&printcfg_string, path => "inter-server:inter_configuration/database_names/guild/main_db", default => "guild"},
			guild_alliance_db      => {parse => \&parsecfg_string, print => \&printcfg_string, path => "inter-server:inter_configuration/database_names/guild/alliance_db", default => "guild_alliance"},
			guild_castle_db        => {parse => \&parsecfg_string, print => \&printcfg_string, path => "inter-server:inter_configuration/database_names/guild/castle_db", default => "guild_castle"},
			guild_expulsion_db     => {parse => \&parsecfg_string, print => \&printcfg_string, path => "inter-server:inter_configuration/database_names/guild/expulsion_db", default => "guild_expulsion"},
			guild_member_db        => {parse => \&parsecfg_string, print => \&printcfg_string, path => "inter-server:inter_configuration/database_names/guild/member_db", default => "guild_member"},
			guild_skill_db         => {parse => \&parsecfg_string, print => \&printcfg_string, path => "inter-server:inter_configuration/database_names/guild/skill_db", default => "guild_skill"},
			guild_position_db      => {parse => \&parsecfg_string, print => \&printcfg_string, path => "inter-server:inter_configuration/database_names/guild/position_db", default => "guild_position"},
			guild_storage_db       => {parse => \&parsecfg_string, print => \&printcfg_string, path => "inter-server:inter_configuration/database_names/guild/storage_db", default => "guild_storage"},
			mapreg_db              => {parse => \&parsecfg_string, print => \&printcfg_string, path => "inter-server:inter_configuration/database_names/", default => "mapreg"},
			autotrade_merchants_db => {parse => \&parsecfg_string, print => \&printcfg_string, path => "inter-server:inter_configuration/database_names/", default => "autotrade_merchants"},
			autotrade_data_db      => {parse => \&parsecfg_string, print => \&printcfg_string, path => "inter-server:inter_configuration/database_names/", default => "autotrade_data"},
			npc_market_data_db     => {parse => \&parsecfg_string, print => \&printcfg_string, path => "inter-server:inter_configuration/database_names/", default => "npc_market_data"},
			default_codepage       => {parse => \&parsecfg_string, print => \&printcfg_string, path => "sql_connection:sql_connection/", default => ""},
			'sql.db_hostname'      => {parse => \&parsecfg_string, print => \&printcfg_string, path => "sql_connection:sql_connection/db_hostname", default => "127.0.0.1"},
			char_server_ip         => {parse => \&parsecfg_string, print => \&printcfg_string, path => "sql_connection:sql_connection/db_hostname", default => "127.0.0.1"},
			map_server_ip          => {parse => \&parsecfg_string, print => \&printcfg_string, path => "sql_connection:sql_connection/db_hostname", default => "127.0.0.1"},
			log_db_ip              => {parse => \&parsecfg_string, print => \&printcfg_string, path => "sql_connection:sql_connection/db_hostname", default => "127.0.0.1"},
			'sql.db_port'          => {parse => \&parsecfg_int,    print => \&printcfg_int,    path => "sql_connection:sql_connection/db_port", default => 3306},
			char_server_port       => {parse => \&parsecfg_int,    print => \&printcfg_int,    path => "sql_connection:sql_connection/db_port", default => 3306},
			map_server_port        => {parse => \&parsecfg_int,    print => \&printcfg_int,    path => "sql_connection:sql_connection/db_port", default => 3306},
			log_db_port            => {parse => \&parsecfg_int,    print => \&printcfg_int,    path => "sql_connection:sql_connection/db_port", default => 3306},
			'sql.db_username'      => {parse => \&parsecfg_string, print => \&printcfg_string, path => "sql_connection:sql_connection/db_username", default => "ragnarok"},
			char_server_id         => {parse => \&parsecfg_string, print => \&printcfg_string, path => "sql_connection:sql_connection/db_username", default => "ragnarok"},
			map_server_id          => {parse => \&parsecfg_string, print => \&printcfg_string, path => "sql_connection:sql_connection/db_username", default => "ragnarok"},
			log_db_id              => {parse => \&parsecfg_string, print => \&printcfg_string, path => "sql_connection:sql_connection/db_username", default => "ragnarok"},
			'sql.db_password'      => {parse => \&parsecfg_string, print => \&printcfg_string, path => "sql_connection:sql_connection/db_password", default => "ragnarok"},
			char_server_pw         => {parse => \&parsecfg_string, print => \&printcfg_string, path => "sql_connection:sql_connection/db_password", default => "ragnarok"},
			map_server_pw          => {parse => \&parsecfg_string, print => \&printcfg_string, path => "sql_connection:sql_connection/db_password", default => "ragnarok"},
			log_db_pw              => {parse => \&parsecfg_string, print => \&printcfg_string, path => "sql_connection:sql_connection/db_password", default => "ragnarok"},
			'sql.db_database'      => {parse => \&parsecfg_string, print => \&printcfg_string, path => "sql_connection:sql_connection/db_database", default => "ragnarok"},
			char_server_db         => {parse => \&parsecfg_string, print => \&printcfg_string, path => "sql_connection:sql_connection/db_database", default => "ragnarok"},
			map_server_db          => {parse => \&parsecfg_string, print => \&printcfg_string, path => "sql_connection:sql_connection/db_database", default => "ragnarok"},
			log_db_db              => {parse => \&parsecfg_string, print => \&printcfg_string, path => "sql_connection:sql_connection/db_database", default => "ragnarok"},
			'sql.codepage'         => {parse => \&parsecfg_string, print => \&printcfg_string, path => "sql_connection:sql_connection/codepage", default => ""},
			log_codepage           => {parse => \&parsecfg_string, print => \&printcfg_string, path => "sql_connection:sql_connection/codepage", default => ""},
			interreg_db            => {parse => \&parsecfg_string, print => \&printcfg_nil,    path => "", default => "interreg"},
			import                 => {parse => \&parsecfg_string, print => \&printcfg_nil,    path => "", default => "conf/import/inter_conf.txt"},
		}
	},
	{
		files => ['login-server.conf', 'import/login_conf.txt'],
		settings => {
			bind_ip                                   => {parse => \&parsecfg_string,    print => \&printcfg_string,  path => "login-server:login_configuration/inter/", default => "127.0.0.1"},
			login_port                                => {parse => \&parsecfg_int,       print => \&printcfg_int,     path => "login-server:login_configuration/inter/", default => 6900},
			timestamp_format                          => {parse => \&parsecfg_string,    print => \&printcfg_string,  path => "console:console/", default => "[%d/%b %H:%M]"},
			stdout_with_ansisequence                  => {parse => \&parsecfg_bool,      print => \&printcfg_bool,    path => "console:console/", default => "false"},
			console_silent                            => {parse => \&parsecfg_int,       print => \&printcfg_int,     path => "console:console/", default => 0},
			new_account                               => {parse => \&parsecfg_bool,      print => \&printcfg_bool,    path => "login-server:login_configuration/account/", default => "true"},
			new_acc_length_limit                      => {parse => \&parsecfg_bool,      print => \&printcfg_bool,    path => "login-server:login_configuration/account/", default => "true"},
			allowed_regs                              => {parse => \&parsecfg_int,       print => \&printcfg_int,     path => "login-server:login_configuration/account/", default => 1},
			time_allowed                              => {parse => \&parsecfg_int,       print => \&printcfg_int,     path => "login-server:login_configuration/account/", default => 10},
			log_login                                 => {parse => \&parsecfg_bool,      print => \&printcfg_bool,    path => "login-server:login_configuration/log/", default => "true"},
			date_format                               => {parse => \&parsecfg_string,    print => \&printcfg_string,  path => "login-server:login_configuration/log/", default => "%Y-%m-%d %H:%M:%S"},
			group_id_to_connect                       => {parse => \&parsecfg_int,       print => \&printcfg_int,     path => "login-server:login_configuration/permission/", default => -1},
			min_group_id_to_connect                   => {parse => \&parsecfg_int,       print => \&printcfg_int,     path => "login-server:login_configuration/permission/", default => -1},
			start_limited_time                        => {parse => \&parsecfg_int,       print => \&printcfg_int,     path => "login-server:login_configuration/account/", default => -1},
			check_client_version                      => {parse => \&parsecfg_bool,      print => \&printcfg_bool,    path => "login-server:login_configuration/permission/", default => "false"},
			client_version_to_connect                 => {parse => \&parsecfg_int,       print => \&printcfg_int,     path => "login-server:login_configuration/permission/", default => 20},
			use_MD5_passwords                         => {parse => \&parsecfg_bool,      print => \&printcfg_bool,    path => "login-server:login_configuration/account/", default => "false"},
			'ipban.enable'                            => {parse => \&parsecfg_bool,      print => \&printcfg_bool,    path => "login-server:login_configuration/account/ipban/enabled", default => "true"},
			'ipban.sql.db_hostname'                   => {parse => \&parsecfg_string,    print => \&printcfg_string,  path => "sql_connection:sql_connection/db_hostname", default => "127.0.0.1"},
			'ipban.sql.db_port'                       => {parse => \&parsecfg_int,       print => \&printcfg_int,     path => "sql_connection:sql_connection/db_port", default => 3306},
			'ipban.sql.db_username'                   => {parse => \&parsecfg_string,    print => \&printcfg_string,  path => "sql_connection:sql_connection/db_username", default => "ragnarok"},
			'ipban.sql.db_password'                   => {parse => \&parsecfg_string,    print => \&printcfg_string,  path => "sql_connection:sql_connection/db_password", default => "ragnarok"},
			'ipban.sql.db_database'                   => {parse => \&parsecfg_string,    print => \&printcfg_string,  path => "sql_connection:sql_connection/db_database", default => "ragnarok"},
			'ipban.sql.codepage'                      => {parse => \&parsecfg_string,    print => \&printcfg_string,  path => "sql_connection:sql_connection/codepage", default => ""},
			'ipban.sql.ipban_table'                   => {parse => \&parsecfg_string,    print => \&printcfg_string,  path => "inter-server:inter_configuration/database_names/ipban_table", default => "ipbanlist"},
			'ipban.dynamic_pass_failure_ban'          => {parse => \&parsecfg_bool,      print => \&printcfg_bool,    path => "login-server:login_configuration/account/ipban/dynamic_pass_failure/enabled", default => "true"},
			'ipban.dynamic_pass_failure_ban_interval' => {parse => \&parsecfg_int,       print => \&printcfg_int,     path => "login-server:login_configuration/account/ipban/dynamic_pass_failure/ban_interval", default => 5},
			'ipban.dynamic_pass_failure_ban_limit'    => {parse => \&parsecfg_int,       print => \&printcfg_int,     path => "login-server:login_configuration/account/ipban/dynamic_pass_failure/ban_limit", default => 7},
			'ipban.dynamic_pass_failure_ban_duration' => {parse => \&parsecfg_int,       print => \&printcfg_int,     path => "login-server:login_configuration/account/ipban/dynamic_pass_failure/ban_duration", default => 5},
			ipban_cleanup_interval                    => {parse => \&parsecfg_int,       print => \&printcfg_int,     path => "login-server:login_configuration/account/ipban/cleanup_interval", default => 60},
			ip_sync_interval                          => {parse => \&parsecfg_int,       print => \&printcfg_int,     path => "login-server:login_configuration/inter/ip_sync_interval", default => 10},
			use_dnsbl                                 => {parse => \&parsecfg_bool,      print => \&printcfg_bool,    path => "login-server:login_configuration/permission/DNS_blacklist/enabled", default => "false"},
			dnsbl_servers                             => {parse => \&parsecfg_string,    print => \&printcfg_strlist, path => "login-server:login_configuration/permission/DNS_blacklist/dnsbl_servers", default => "bl.blocklist.de, socks.dnsbl.sorbs.net"},
			'account.sql.db_hostname'                 => {parse => \&parsecfg_string,    print => \&printcfg_string,  path => "sql_connection:sql_connection/db_hostname", default => "127.0.0.1"},
			'account.sql.db_port'                     => {parse => \&parsecfg_int,       print => \&printcfg_int,     path => "sql_connection:sql_connection/db_port", default => 3306},
			'account.sql.db_username'                 => {parse => \&parsecfg_string,    print => \&printcfg_string,  path => "sql_connection:sql_connection/db_username", default => "ragnarok"},
			'account.sql.db_password'                 => {parse => \&parsecfg_string,    print => \&printcfg_string,  path => "sql_connection:sql_connection/db_password", default => "ragnarok"},
			'account.sql.db_database'                 => {parse => \&parsecfg_string,    print => \&printcfg_string,  path => "sql_connection:sql_connection/db_database", default => "ragnarok"},
			'account.sql.codepage'                    => {parse => \&parsecfg_string,    print => \&printcfg_string,  path => "sql_connection:sql_connection/codepage", default => ""},
			'account.sql.case_sensitive'              => {parse => \&parsecfg_bool,      print => \&printcfg_bool,    path => "sql_connection:sql_connection/case_sensitive", default => "false"},
			'account.sql.account_db'                  => {parse => \&parsecfg_string,    print => \&printcfg_string,  path => "inter-server:inter_configuration/database_names/account_db", default => "login"},
			client_hash_check                         => {parse => \&parsecfg_bool,      print => \&printcfg_bool,    path => "login-server:login_configuration/permission/hash/enabled", default => "false"},
			client_hash                               => {parse => \&parsecfg_stringarr, print => \&printcfg_md5hash, path => "login-server:login_configuration/permission/hash/MD5_hashes", default => []},
			'account.sql.accreg_db'                   => {parse => \&parsecfg_string,    print => \&printcfg_nil,     path => "", default => "global_reg_value"},
			import                                    => {parse => \&parsecfg_stringarr, print => \&printcfg_nil,     path => "", default => ["conf/inter-server.conf", "conf/import/login_conf.txt"]},
		}
	},
	{
		files => ['map-server.conf', 'import/map_conf.txt'],
		settings => {
			userid                   => {parse => \&parsecfg_string,    print => \&printcfg_string, path => "map-server:map_configuration/inter/", default => "s1"},
			passwd                   => {parse => \&parsecfg_string,    print => \&printcfg_string, path => "map-server:map_configuration/inter/", default => "p1"},
			char_ip                  => {parse => \&parsecfg_string,    print => \&printcfg_string, path => "map-server:map_configuration/inter/", default => "127.0.0.1"},
			bind_ip                  => {parse => \&parsecfg_string,    print => \&printcfg_string, path => "map-server:map_configuration/inter/", default => "127.0.0.1"},
			char_port                => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "map-server:map_configuration/inter/", default => 6121},
			map_ip                   => {parse => \&parsecfg_string,    print => \&printcfg_string, path => "map-server:map_configuration/inter/", default => "127.0.0.1"},
			map_port                 => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "map-server:map_configuration/inter/", default => 5121},
			timestamp_format         => {parse => \&parsecfg_string,    print => \&printcfg_string, path => "console:console/", default => "[%d/%b %H:%M]"},
			stdout_with_ansisequence => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "console:console/", default => "false"},
			console_msg_log          => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "console:console/", default => 0},
			console_silent           => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "console:console/", default => 0},
			db_path                  => {parse => \&parsecfg_string,    print => \&printcfg_string, path => "map-server:map_configuration/database/", default => "db"},
			enable_spy               => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "map-server:map_configuration/", default => "false"},
			use_grf                  => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "map-server:map_configuration/", default => "false"},
			autosave_time            => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "map-server:map_configuration/database/", default => 300},
			minsave_time             => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "map-server:map_configuration/database/", default => 100},
			save_settings            => {parse => \&parsecfg_int,       print => \&printcfg_hexint, path => "map-server:map_configuration/database/", default => 511},
			default_language         => {parse => \&parsecfg_string,    print => \&printcfg_string, path => "map-server:map_configuration/", default => "English"},
			help_txt                 => {parse => \&parsecfg_string,    print => \&printcfg_string, path => "map-server:map_configuration/", default => "conf/help.txt"},
			charhelp_txt             => {parse => \&parsecfg_string,    print => \&printcfg_string, path => "map-server:map_configuration/", default => "conf/charhelp.txt"},
			help2_txt                => {parse => \&parsecfg_string,    print => \&printcfg_nil,    path => "", default => "conf/help2.txt"},
			import                   => {parse => \&parsecfg_stringarr, print => \&printcfg_nil,    path => "", default => ["conf/maps.conf", "conf/import/map_conf.txt"]},
		}
	},
	{
		files => ['logs.conf', 'import/log_conf.txt'],
		settings => {
			enable_logs          => {parse => \&parsecfg_int,    print => \&printcfg_hexint, path => "logs:map_log/enable", default => 0xFFFFF},
			sql_logs             => {parse => \&parsecfg_bool,   print => \&printcfg_bool,   path => "logs:map_log/database/use_sql", default => "true"},
			log_filter           => {parse => \&parsecfg_int,    print => \&printcfg_int,    path => "logs:map_log/filter/item/", default => 1},
			refine_items_log     => {parse => \&parsecfg_int,    print => \&printcfg_int,    path => "logs:map_log/filter/item/", default => 5},
			rare_items_log       => {parse => \&parsecfg_int,    print => \&printcfg_int,    path => "logs:map_log/filter/item/", default => 100},
			price_items_log      => {parse => \&parsecfg_int,    print => \&printcfg_int,    path => "logs:map_log/filter/item/", default => 1000},
			amount_items_log     => {parse => \&parsecfg_int,    print => \&printcfg_int,    path => "logs:map_log/filter/item/", default => 100},
			log_branch           => {parse => \&parsecfg_bool,   print => \&printcfg_bool,   path => "logs:map_log/", default => "false"},
			log_zeny             => {parse => \&parsecfg_int,    print => \&printcfg_int,    path => "logs:map_log/", default => 0},
			log_mvpdrop          => {parse => \&parsecfg_bool,   print => \&printcfg_bool,   path => "logs:map_log/", default => "false"},
			log_commands         => {parse => \&parsecfg_bool,   print => \&printcfg_bool,   path => "logs:map_log/", default => "true"},
			log_npc              => {parse => \&parsecfg_bool,   print => \&printcfg_bool,   path => "logs:map_log/", default => "false"},
			log_chat             => {parse => \&parsecfg_int,    print => \&printcfg_int,    path => "logs:map_log/filter/chat/", default => 0},
			log_chat_woe_disable => {parse => \&parsecfg_bool,   print => \&printcfg_bool,   path => "logs:map_log/filter/chat/", default => "false"},
			log_gm_db            => {parse => \&parsecfg_string, print => \&printcfg_string, path => "logs:map_log/database/", default => "atcommandlog"},
			log_branch_db        => {parse => \&parsecfg_string, print => \&printcfg_string, path => "logs:map_log/database/", default => "branchlog"},
			log_chat_db          => {parse => \&parsecfg_string, print => \&printcfg_string, path => "logs:map_log/database/", default => "chatlog"},
			log_mvpdrop_db       => {parse => \&parsecfg_string, print => \&printcfg_string, path => "logs:map_log/database/", default => "mvplog"},
			log_npc_db           => {parse => \&parsecfg_string, print => \&printcfg_string, path => "logs:map_log/database/", default => "npclog"},
			log_pick_db          => {parse => \&parsecfg_string, print => \&printcfg_string, path => "logs:map_log/database/", default => "picklog"},
			log_zeny_db          => {parse => \&parsecfg_string, print => \&printcfg_string, path => "logs:map_log/database/", default => "zenylog"},
			import               => {parse => \&parsecfg_string, print => \&printcfg_nil,    path => "", default => "conf/import/log_conf.txt"},
		}
	},
	{
		files => ['script.conf', 'import/script_conf.txt'],
		settings => {
			warn_func_mismatch_paramnum => {parse => \&parsecfg_bool,   print => \&printcfg_bool, path => "script:script_configuration/", default => "true"},
			check_cmdcount              => {parse => \&parsecfg_int,    print => \&printcfg_int,  path => "script:script_configuration/", default => 655360},
			check_gotocount             => {parse => \&parsecfg_int,    print => \&printcfg_int,  path => "script:script_configuration/", default => 2048},
			input_min_value             => {parse => \&parsecfg_int,    print => \&printcfg_int,  path => "script:script_configuration/", default => 0},
			input_max_value             => {parse => \&parsecfg_int,    print => \&printcfg_int,  path => "script:script_configuration/", default => 10000000},
			warn_func_mismatch_argtypes => {parse => \&parsecfg_bool,   print => \&printcfg_bool, path => "script:script_configuration/", default => "true"},
			import                      => {parse => \&parsecfg_string, print => \&printcfg_nil,  path => "", default => "conf/import/script_conf.txt"},
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
