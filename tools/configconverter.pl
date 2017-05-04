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
	{
		files => ['packet.conf', 'import/packet_conf.txt'],
		settings => {
			debug                    => {parse => \&parsecfg_bool,      print => \&printcfg_bool,    path => "socket:socket_configuration/", default => "false"},
			stall_time               => {parse => \&parsecfg_int,       print => \&printcfg_int,     path => "socket:socket_configuration/", default => 60},
			epoll_maxevents          => {parse => \&parsecfg_int,       print => \&printcfg_int,     path => "socket:socket_configuration/", default => 1024},
			socket_max_client_packet => {parse => \&parsecfg_int,       print => \&printcfg_int,     path => "socket:socket_configuration/", default => 65535},
			enable_ip_rules          => {parse => \&parsecfg_bool,      print => \&printcfg_bool,    path => "socket:socket_configuration/ip_rules/enable", default => "true"},
			order                    => {parse => \&parsecfg_string,    print => \&printcfg_string,  path => "socket:socket_configuration/ip_rules/", default => "deny,allow"},
			allow                    => {parse => \&parsecfg_stringarr, print => \&printcfg_strlist, path => "socket:socket_configuration/ip_rules/allow_list", default => []},
			deny                     => {parse => \&parsecfg_stringarr, print => \&printcfg_strlist, path => "socket:socket_configuration/ip_rules/deny_list", default => []},
			ddos_interval            => {parse => \&parsecfg_int,       print => \&printcfg_int,     path => "socket:socket_configuration/ddos/interval", default => 3000},
			ddos_count               => {parse => \&parsecfg_int,       print => \&printcfg_int,     path => "socket:socket_configuration/ddos/count", default => 5},
			ddos_autoreset           => {parse => \&parsecfg_int,       print => \&printcfg_int,     path => "socket:socket_configuration/ddos/autoreset", default => 600000},
			import                   => {parse => \&parsecfg_string,    print => \&printcfg_nil,     path => "", default => "conf/import/packet_conf.txt"},
		}
	},
	{
		files => ['battle.conf', 'battle/battle.conf', 'battle/client.conf', 'battle/drops.conf', 'battle/exp.conf', 'battle/gm.conf', 'battle/guild.conf', 'battle/battleground.conf', 'battle/items.conf', 'battle/monster.conf', 'battle/party.conf', 'battle/pet.conf', 'battle/homunc.conf', 'battle/player.conf', 'battle/skill.conf', 'battle/status.conf', 'battle/feature.conf', 'battle/misc.conf', 'import/battle_conf.txt'],
		settings => {
			bg_flee_penalty                   => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "battleground:", default => 20},
			bg_update_interval                => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "battleground:", default => 1000},
			'feature.buying_store'            => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "feature:features/buying_store", default => "true"},
			'feature.search_stores'           => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "feature:features/search_stores", default => "true"},
			'feature.atcommand_suggestions'   => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "feature:features/atcommand_suggestions", default => "false"},
			'feature.banking'                 => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "feature:features/banking", default => "true"},
			'feature.auction'                 => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "feature:features/auction", default => "false"},
			'feature.roulette'                => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "feature:features/roulette", default => "false"},
			atcommand_spawn_quantity_limit    => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "gm:", default => 100},
			atcommand_slave_clone_limit       => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "gm:", default => 25},
			partial_name_scan                 => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "gm:", default => "true"},
			atcommand_max_stat_bypass         => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "gm:", default => "false"},
			ban_hack_trade                    => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "gm:", default => 5},
			atcommand_mobinfo_type            => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "gm:", default => 0},
			gm_ignore_warpable_area           => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "gm:", default => 2},
			atcommand_levelup_events          => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "gm:", default => "false"},
			guild_emperium_check              => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "guild:", default => "true"},
			guild_exp_limit                   => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "guild:", default => 50},
			guild_max_castles                 => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "guild:", default => 0},
			guild_skill_relog_delay           => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "guild:", default => "false"},
			castle_defense_rate               => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "guild:", default => 100},
			gvg_flee_penalty                  => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "guild:", default => 20},
			require_glory_guild               => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "guild:", default => "false"},
			max_guild_alliance                => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "guild:", default => 3},
			guild_notice_changemap            => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "guild:", default => 2},
			guild_castle_invite               => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "guild:", default => "false"},
			guild_castle_expulsion            => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "guild:", default => "false"},
			hom_setting                       => {parse => \&parsecfg_int,       print => \&printcfg_hexint, path => "homunc:", default => 0x1D},
			homunculus_friendly_rate          => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "homunc:", default => 100},
			hom_rename                        => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "homunc:", default => "false"},
			hvan_explosion_intimate           => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "homunc:", default => 45000},
			homunculus_show_growth            => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "homunc:", default => "true"},
			homunculus_autoloot               => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "homunc:", default => "true"},
			homunculus_auto_vapor             => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "homunc:", default => "true"},
			homunculus_max_level              => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "homunc:", default => 99},
			homunculus_S_max_level            => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "homunc:", default => 150},
			enable_baseatk                    => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "battle:", default => 9},
			enable_perfect_flee               => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "battle:", default => 1},
			enable_critical                   => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "battle:", default => 17},
			mob_critical_rate                 => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "battle:", default => 100},
			critical_rate                     => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "battle:", default => 100},
			attack_walk_delay                 => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "battle:", default => 15},
			pc_damage_walk_delay_rate         => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "battle:", default => 20},
			damage_walk_delay_rate            => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "battle:", default => 100},
			multihit_delay                    => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "battle:", default => 80},
			player_damage_delay_rate          => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "battle:", default => 100},
			undead_detect_type                => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "battle:", default => 0},
			attribute_recover                 => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "battle:", default => "false"},
			min_hitrate                       => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "battle:", default => 5},
			max_hitrate                       => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "battle:", default => 100},
			agi_penalty_type                  => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "battle:", default => 1},
			agi_penalty_target                => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "battle:", default => 1},
			agi_penalty_count                 => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "battle:", default => 3},
			agi_penalty_num                   => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "battle:", default => 10},
			vit_penalty_type                  => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "battle:", default => 1},
			vit_penalty_target                => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "battle:", default => 1},
			vit_penalty_count                 => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "battle:", default => 3},
			vit_penalty_num                   => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "battle:", default => 5},
			weapon_defense_type               => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "battle:", default => 0},
			magic_defense_type                => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "battle:", default => 0},
			attack_direction_change           => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "battle:", default => 0},
			attack_attr_none                  => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "battle:", default => 14},
			equip_natural_break_rate          => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "battle:", default => 0},
			equip_self_break_rate             => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "battle:", default => 100},
			equip_skill_break_rate            => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "battle:", default => 100},
			delay_battle_damage               => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "battle:", default => "true"},
			arrow_decrement                   => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "battle:", default => 1},
			autospell_check_range             => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "battle:", default => "false"},
			knockback_left                    => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "battle:", default => "true"},
			snap_dodge                        => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "battle:", default => "false"},
			packet_obfuscation                => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "client:", default => 1},
			min_chat_delay                    => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "client:", default => 0},
			min_hair_style                    => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "client:", default => 0},
			max_hair_style                    => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "client:", default => 29},
			min_hair_color                    => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "client:", default => 0},
			max_hair_color                    => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "client:", default => 8},
			min_cloth_color                   => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "client:", default => 0},
			max_cloth_color                   => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "client:", default => 4},
			min_body_style                    => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "client:", default => 0},
			max_body_style                    => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "client:", default => 4},
			hide_woe_damage                   => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "client:", default => "true"},
			pet_hair_style                    => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "client:", default => 100},
			area_size                         => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "client:", default => 14},
			max_walk_path                     => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "client:", default => 17},
			max_lv                            => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "client:", default => 99},
			aura_lv                           => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "client:", default => 99},
			client_limit_unit_lv              => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "client:", default => 0},
			wedding_modifydisplay             => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "client:", default => "false"},
			save_clothcolor                   => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "client:", default => "true"},
			save_body_style                   => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "client:", default => "false"},
			wedding_ignorepalette             => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "client:", default => "false"},
			xmas_ignorepalette                => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "client:", default => "false"},
			summer_ignorepalette              => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "client:", default => "false"},
			hanbok_ignorepalette              => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "client:", default => "false"},
			display_version                   => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "client:", default => "false"},
			display_hallucination             => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "client:", default => "true"},
			display_status_timers             => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "client:", default => "true"},
			client_reshuffle_dice             => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "client:", default => "true"},
			client_sort_storage               => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "client:", default => "false"},
			client_accept_chatdori            => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "client:", default => 0},
			client_emblem_max_blank_percent   => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "client:", default => 100},
			item_auto_get                     => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "drops:", default => "false"},
			flooritem_lifetime                => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "drops:", default => 60000},
			item_first_get_time               => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "drops:", default => 3000},
			item_second_get_time              => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "drops:", default => 1000},
			item_third_get_time               => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "drops:", default => 1000},
			mvp_item_first_get_time           => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "drops:", default => 10000},
			mvp_item_second_get_time          => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "drops:", default => 10000},
			mvp_item_third_get_time           => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "drops:", default => 2000},
			item_rate_common                  => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "drops:", default => 100},
			item_rate_common_boss             => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "drops:", default => 100},
			item_drop_common_min              => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "drops:", default => 1},
			item_drop_common_max              => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "drops:", default => 10000},
			item_rate_heal                    => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "drops:", default => 100},
			item_rate_heal_boss               => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "drops:", default => 100},
			item_drop_heal_min                => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "drops:", default => 1},
			item_drop_heal_max                => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "drops:", default => 10000},
			item_rate_use                     => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "drops:", default => 100},
			item_rate_use_boss                => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "drops:", default => 100},
			item_drop_use_min                 => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "drops:", default => 1},
			item_drop_use_max                 => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "drops:", default => 10000},
			item_rate_equip                   => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "drops:", default => 100},
			item_rate_equip_boss              => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "drops:", default => 100},
			item_drop_equip_min               => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "drops:", default => 1},
			item_drop_equip_max               => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "drops:", default => 10000},
			item_rate_card                    => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "drops:", default => 100},
			item_rate_card_boss               => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "drops:", default => 100},
			item_drop_card_min                => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "drops:", default => 1},
			item_drop_card_max                => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "drops:", default => 10000},
			item_rate_mvp                     => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "drops:", default => 100},
			item_drop_mvp_min                 => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "drops:", default => 1},
			item_drop_mvp_max                 => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "drops:", default => 10000},
			item_rate_adddrop                 => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "drops:", default => 100},
			item_drop_add_min                 => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "drops:", default => 1},
			item_drop_add_max                 => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "drops:", default => 10000},
			item_rate_treasure                => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "drops:", default => 100},
			item_drop_treasure_min            => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "drops:", default => 1},
			item_drop_treasure_max            => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "drops:", default => 10000},
			item_logarithmic_drops            => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "drops:", default => "false"},
			drop_rate0item                    => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "drops:", default => "false"},
			drops_by_luk                      => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "drops:", default => 0},
			drops_by_luk2                     => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "drops:", default => 0},
			alchemist_summon_reward           => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "drops:", default => 1},
			base_exp_rate                     => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "exp:", default => 100},
			job_exp_rate                      => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "exp:", default => 100},
			multi_level_up                    => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "exp:", default => "false"},
			max_exp_gain_rate                 => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "exp:", default => 0},
			exp_calc_type                     => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "exp:", default => 0},
			exp_bonus_attacker                => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "exp:", default => 25},
			exp_bonus_max_attacker            => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "exp:", default => 12},
			mvp_exp_rate                      => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "exp:", default => 100},
			quest_exp_rate                    => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "exp:", default => 100},
			heal_exp                          => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "exp:", default => 0},
			resurrection_exp                  => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "exp:", default => 0},
			shop_exp                          => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "exp:", default => 0},
			pvp_exp                           => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "exp:", default => "true"},
			death_penalty_type                => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "exp:", default => 1},
			death_penalty_base                => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "exp:", default => 100},
			death_penalty_job                 => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "exp:", default => 100},
			zeny_penalty                      => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "exp:", default => 0},
			disp_experience                   => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "exp:", default => "false"},
			disp_zeny                         => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "exp:", default => "false"},
			use_statpoint_table               => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "exp:", default => "true"},
			vending_max_value                 => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "items:", default => 1000000000},
			vending_over_max                  => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "items:", default => "true"},
			vending_tax                       => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "items:", default => 200},
			buyer_name                        => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "items:", default => "true"},
			weapon_produce_rate               => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "items:", default => 100},
			potion_produce_rate               => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "items:", default => 100},
			produce_item_name_input           => {parse => \&parsecfg_int,       print => \&printcfg_hexint, path => "items:", default => 0x03},
			dead_branch_active                => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "items:", default => "true"},
			random_monster_checklv            => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "items:", default => "false"},
			ignore_items_gender               => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "items:", default => "true"},
			item_check                        => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "items:", default => "false"},
			item_use_interval                 => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "items:", default => 100},
			cashfood_use_interval             => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "items:", default => 60000},
			gtb_sc_immunity                   => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "items:", default => 50},
			autospell_stacking                => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "items:", default => "false"},
			item_restricted_consumption_type  => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "items:", default => "true"},
			item_enabled_npc                  => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "items:", default => "true"},
			unequip_restricted_equipment      => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "items:", default => 0},
			pk_mode                           => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "misc:", default => 0},
			manner_system                     => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "misc:", default => 31},
			pk_min_level                      => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "misc:", default => 55},
			pk_level_range                    => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "misc:", default => 0},
			skill_log                         => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "misc:", default => "false"},
			battle_log                        => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "misc:", default => "false"},
			etc_log                           => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "misc:", default => "false"},
			warp_point_debug                  => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "misc:", default => "false"},
			night_at_start                    => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "misc:", default => "false"},
			day_duration                      => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "misc:", default => 0},
			night_duration                    => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "misc:", default => 0},
			duel_allow_pvp                    => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "misc:", default => "false"},
			duel_allow_gvg                    => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "misc:", default => "false"},
			duel_allow_teleport               => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "misc:", default => "false"},
			duel_autoleave_when_die           => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "misc:", default => "true"},
			duel_time_interval                => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "misc:", default => 60},
			duel_only_on_same_map             => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "misc:", default => "false"},
			official_cell_stack_limit         => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "misc:", default => 1},
			custom_cell_stack_limit           => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "misc:", default => 1},
			check_occupied_cells              => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "misc:", default => "true"},
			at_mapflag                        => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "misc:", default => "false"},
			at_timeout                        => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "misc:", default => 0},
			auction_feeperhour                => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "misc:", default => 12000},
			auction_maximumprice              => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "misc:", default => 500000000},
			searchstore_querydelay            => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "misc:", default => 10},
			searchstore_maxresults            => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "misc:", default => 30},
			cashshop_show_points              => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "misc:", default => "false"},
			mail_show_status                  => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "misc:", default => 0},
			mon_trans_disable_in_gvg          => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "misc:", default => "false"},
			case_sensitive_aegisnames         => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "misc:", default => "true"},
			mvp_hp_rate                       => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "monster:", default => 100},
			monster_hp_rate                   => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "monster:", default => 100},
			monster_max_aspd                  => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "monster:", default => 199},
			monster_ai                        => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "monster:", default => 0},
			monster_chase_refresh             => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "monster:", default => 3},
			mob_warp                          => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "monster:", default => 0},
			mob_active_time                   => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "monster:", default => 0},
			boss_active_time                  => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "monster:", default => 0},
			view_range_rate                   => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "monster:", default => 100},
			chase_range_rate                  => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "monster:", default => 100},
			monster_active_enable             => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "monster:", default => "true"},
			override_mob_names                => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "monster:", default => 0},
			monster_damage_delay_rate         => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "monster:", default => 100},
			monster_loot_type                 => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "monster:", default => 0},
			mob_skill_rate                    => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "monster:", default => 100},
			mob_skill_delay                   => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "monster:", default => 100},
			mob_count_rate                    => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "monster:", default => 100},
			mob_spawn_delay                   => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "monster:", default => 100},
			plant_spawn_delay                 => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "monster:", default => 100},
			boss_spawn_delay                  => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "monster:", default => 100},
			no_spawn_on_player                => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "monster:", default => 0},
			force_random_spawn                => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "monster:", default => "false"},
			slaves_inherit_mode               => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "monster:", default => 2},
			slaves_inherit_speed              => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "monster:", default => 3},
			summons_trigger_autospells        => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "monster:", default => "true"},
			retaliate_to_master               => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "monster:", default => "true"},
			mob_changetarget_byskill          => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "monster:", default => "false"},
			monster_class_change_full_recover => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "monster:", default => "true"},
			show_mob_info                     => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "monster:", default => 0},
			zeny_from_mobs                    => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "monster:", default => "false"},
			mobs_level_up                     => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "monster:", default => "false"},
			mobs_level_up_exp_rate            => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "monster:", default => 1},
			dynamic_mobs                      => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "monster:", default => "true"},
			mob_remove_damaged                => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "monster:", default => "true"},
			mob_remove_delay                  => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "monster:", default => 300000},
			mob_npc_event_type                => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "monster:", default => 1},
			ksprotection                      => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "monster:", default => 0},
			mob_slave_keep_target             => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "monster:", default => "true"},
			mvp_tomb_enabled                  => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "monster:", default => "true"},
			show_monster_hp_bar               => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "monster:", default => "true"},
			mob_size_influence                => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "monster:", default => "false"},
			mob_icewall_walk_block            => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "monster:", default => 220},
			boss_icewall_walk_block           => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "monster:", default => 1},
			show_steal_in_same_party          => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "party:", default => "false"},
			party_update_interval             => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "party:", default => 1000},
			party_hp_mode                     => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "party:", default => 0},
			show_party_share_picker           => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "party:", default => "true"},
			'show_picker.item_type'           => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "party:show_picker_item_type", default => 112},
			party_item_share_type             => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "party:", default => 0},
			idle_no_share                     => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "party:", default => 0},
			party_even_share_bonus            => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "party:", default => 0},
			display_party_name                => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "party:", default => "false"},
			pet_catch_rate                    => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "pet:", default => 100},
			pet_rename                        => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "pet:", default => "false"},
			pet_friendly_rate                 => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "pet:", default => 100},
			pet_hungry_delay_rate             => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "pet:", default => 100},
			pet_hungry_friendly_decrease      => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "pet:", default => 5},
			pet_equip_required                => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "pet:", default => "true"},
			pet_attack_support                => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "pet:", default => "false"},
			pet_damage_support                => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "pet:", default => "false"},
			pet_support_min_friendly          => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "pet:", default => 900},
			pet_equip_min_friendly            => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "pet:", default => 900},
			pet_status_support                => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "pet:", default => "false"},
			pet_support_rate                  => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "pet:", default => 100},
			pet_attack_exp_to_master          => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "pet:", default => "false"},
			pet_attack_exp_rate               => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "pet:", default => 100},
			pet_lv_rate                       => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "pet:", default => 0},
			pet_max_stats                     => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "pet:", default => 99},
			pet_max_atk1                      => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "pet:", default => 500},
			pet_max_atk2                      => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "pet:", default => 1000},
			pet_disable_in_gvg                => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "pet:", default => "false"},
			hp_rate                           => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "player:", default => 100},
			sp_rate                           => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "player:", default => 100},
			left_cardfix_to_right             => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "player:", default => "true"},
			restart_hp_rate                   => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "player:", default => 0},
			restart_sp_rate                   => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "player:", default => 0},
			player_skillfree                  => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "player:", default => "false"},
			player_skillup_limit              => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "player:", default => "true"},
			quest_skill_learn                 => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "player:", default => "false"},
			quest_skill_reset                 => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "player:", default => "false"},
			basic_skill_check                 => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "player:", default => "true"},
			player_invincible_time            => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "player:", default => 5000},
			fix_warp_hit_delay_abuse          => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "player:", default => "false"},
			natural_healhp_interval           => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "player:", default => 6000},
			natural_healsp_interval           => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "player:", default => 8000},
			natural_heal_skill_interval       => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "player:", default => 10000},
			natural_heal_weight_rate          => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "player:", default => 50},
			max_aspd                          => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "player:", default => 190},
			max_third_aspd                    => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "player:", default => 193},
			max_walk_speed                    => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "player:", default => 300},
			max_hp                            => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "player:", default => 1000000},
			max_sp                            => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "player:", default => 1000000},
			max_parameter                     => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "player:", default => 99},
			max_third_parameter               => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "player:", default => 130},
			max_extended_parameter            => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "player:", default => 125},
			max_baby_parameter                => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "player:", default => 80},
			max_baby_third_parameter          => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "player:", default => 117},
			max_def                           => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "player:", default => 99},
			over_def_bonus                    => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "player:", default => 0},
			max_cart_weight                   => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "player:", default => 8000},
			prevent_logout                    => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "player:", default => 10000},
			show_hp_sp_drain                  => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "player:", default => "false"},
			show_hp_sp_gain                   => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "player:", default => "true"},
			show_katar_crit_bonus             => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "player:", default => "false"},
			friend_auto_add                   => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "player:", default => "true"},
			invite_request_check              => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "player:", default => "true"},
			bone_drop                         => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "player:", default => 0},
			character_size                    => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "player:", default => 0},
			idle_no_autoloot                  => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "player:", default => 0},
			min_npc_vendchat_distance         => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "player:", default => 3},
			vendchat_near_hiddennpc           => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "player:", default => "false"},
			snovice_call_type                 => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "player:", default => 0},
			idletime_criteria                 => {parse => \&parsecfg_int,       print => \&printcfg_hexint, path => "player:", default => 0x1F},
			costume_refine_def                => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "player:", default => "true"},
			shadow_refine_def                 => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "player:", default => "true"},
			shadow_refine_atk                 => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "player:", default => "true"},
			player_warp_keep_direction        => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "player:", default => "true"},
			casting_rate                      => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "skill:", default => 100},
			delay_rate                        => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "skill:", default => 100},
			delay_dependon_dex                => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "skill:", default => "false"},
			delay_dependon_agi                => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "skill:", default => "false"},
			min_skill_delay_limit             => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "skill:", default => 100},
			default_walk_delay                => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "skill:", default => 300},
			no_skill_delay                    => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "skill:", default => 2},
			castrate_dex_scale                => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "skill:", default => 150},
			vcast_stat_scale                  => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "skill:", default => 530},
			skill_amotion_leniency            => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "skill:", default => 90},
			skill_delay_attack_enable         => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "skill:", default => "true"},
			skill_add_range                   => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "skill:", default => 0},
			skill_out_range_consume           => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "skill:", default => "false"},
			skillrange_by_distance            => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "skill:", default => 14},
			skillrange_from_weapon            => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "skill:", default => 0},
			skill_caster_check                => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "skill:", default => "true"},
			clear_skills_on_death             => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "skill:", default => 0},
			clear_skills_on_warp              => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "skill:", default => 15},
			defunit_not_enemy                 => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "skill:", default => "false"},
			skill_min_damage                  => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "skill:", default => 6},
			combo_delay_rate                  => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "skill:", default => 100},
			auto_counter_type                 => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "skill:", default => 15},
			skill_reiteration                 => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "skill:", default => 0},
			skill_nofootset                   => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "skill:", default => 1},
			gvg_traps_target_all              => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "skill:", default => 1},
			traps_setting                     => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "skill:", default => 0},
			summon_flora_setting              => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "skill:", default => 3},
			song_timer_reset                  => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "skill:", default => 0},
			skill_wall_check                  => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "skill:", default => "true"},
			player_cloak_check_type           => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "skill:", default => 1},
			monster_cloak_check_type          => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "skill:", default => 4},
			land_skill_limit                  => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "skill:", default => 9},
			display_skill_fail                => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "skill:", default => 0},
			chat_warpportal                   => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "skill:", default => "false"},
			sense_type                        => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "skill:", default => 1},
			finger_offensive_type             => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "skill:", default => 0},
			gx_allhit                         => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "skill:", default => "false"},
			gx_disptype                       => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "skill:", default => 1},
			devotion_level_difference         => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "skill:", default => 10},
			player_skill_partner_check        => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "skill:", default => "true"},
			skill_removetrap_type             => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "skill:", default => 0},
			backstab_bow_penalty              => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "skill:", default => "true"},
			skill_steal_max_tries             => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "skill:", default => 0},
			copyskill_restrict                => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "skill:", default => 2},
			berserk_cancels_buffs             => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "skill:", default => "false"},
			max_heal                          => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "skill:", default => 9999},
			max_heal_lv                       => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "skill:", default => 11},
			emergency_call                    => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "skill:", default => 11},
			guild_aura                        => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "skill:", default => 31},
			skip_teleport_lv1_menu            => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "skill:", default => "false"},
			allow_skill_without_day           => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "skill:", default => "false"},
			allow_es_magic_player             => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "skill:", default => "false"},
			sg_miracle_skill_ratio            => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "skill:", default => 2},
			sg_miracle_skill_duration         => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "skill:", default => 3600000},
			sg_angel_skill_ratio              => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "skill:", default => 10},
			skill_add_heal_rate               => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "skill:", default => 7},
			eq_single_target_reflectable      => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "skill:", default => "true"},
			'invincible.nodamage'             => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "skill:", default => "false"},
			dancing_weaponswitch_fix          => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "skill:", default => "true"},
			skill_trap_type                   => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "skill:", default => 0},
			mob_max_skilllvl                  => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "skill:", default => 100},
			bowling_bash_area                 => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "skill:", default => 0},
			stormgust_knockback               => {parse => \&parsecfg_bool,      print => \&printcfg_bool,   path => "skill:", default => "true"},
			status_cast_cancel                => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "status:", default => 0},
			pc_status_def_rate                => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "status:", default => 100},
			mob_status_def_rate               => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "status:", default => 100},
			pc_max_status_def                 => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "status:", default => 100},
			mob_max_status_def                => {parse => \&parsecfg_int,       print => \&printcfg_int,    path => "status:", default => 100},
			import                            => {parse => \&parsecfg_stringarr, print => \&printcfg_nil,    path => "", default => ['conf/battle/battle.conf', 'conf/battle/client.conf', 'conf/battle/drops.conf', 'conf/battle/exp.conf', 'conf/battle/gm.conf', 'conf/battle/guild.conf', 'conf/battle/battleground.conf', 'conf/battle/items.conf', 'conf/battle/monster.conf', 'conf/battle/party.conf', 'conf/battle/pet.conf', 'conf/battle/homunc.conf', 'conf/battle/player.conf', 'conf/battle/skill.conf', 'conf/battle/status.conf', 'conf/battle/feature.conf', 'conf/battle/misc.conf', 'conf/import/battle_conf.txt']},
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
