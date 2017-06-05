#!/usr/bin/perl

# This file is part of Hercules.
# http://herc.ws - http://github.com/HerculesWS/Hercules
#
# Copyright (C) 2013-2017  Hercules Dev Team
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
#local $ENV{XML_SIMPLE_PREFERRED_PARSER} = '';                 # 0m7.138s
local $ENV{XML_SIMPLE_PREFERRED_PARSER} = 'XML::Parser';      # 0m2.674s
#local $ENV{XML_SIMPLE_PREFERRED_PARSER} = 'XML::SAX::Expat';  # 0m7.026s
#local $ENV{XML_SIMPLE_PREFERRED_PARSER} = 'XML::LibXML::SAX'; # 0m4.152s

sub trim($) {
	my $s = $_[0];
	$s =~ s/^\s+//; $s =~ s/\s+$//;
	return $s;
}

sub parse($$) {
	my ($p, $d) = @_;

	$p =~ s/^.*?\)\((.*)\).*$/$1/; # Clean up extra parentheses )(around the arglist)

	# Retrieve return type
	unless ($d =~ /^(.+)\(\*\s*[a-zA-Z0-9_]+_interface(?:_private)?::([^\)]+)\s*\)\s*\(.*\)$/) {
		print "Error: unable to parse '$d'\n";
		return {};
	}
	my $rt = trim($1);   #return type
	my $name = trim($2); #function name

	my @args;
	my ($anonvars, $variadic) = (0, 0);
	my ($lastvar, $notes) = ('', '');

	$p = ' ' unless $p; # ensure there's at least one character (we don't want a do{} block)
	while ($p) { # Scan the string for variables
		my $current = '';
		my ($paren, $needspace) = (0, 0);

		while ($p) { # Parse tokens
			$p =~ s|^(?:\s*(?:/\*.*?\*/)?)*||; # strip heading whitespace and c-style comments
			last unless $p;

			if ($p =~ s/^([a-zA-Z0-9_]+)//) { # Word (variable, type)
				$current .= ' ' if $needspace;
				$current .= $1;
				$needspace = 1;
				next;
			}
			if ($p =~ s/^(,)//) { # Comma
				last unless $paren; # Argument terminator unless inside parentheses
				$current .= $1;
				$needspace = 1;
				next;
			}
			if ($p =~ s/^(\*)//) { # Pointer
				$current .= ' ' if $needspace;
				$current .= $1;
				$needspace = 0;
				next;
			}
			if ($p =~ s/^([\[\].])//) { # Array subscript
				$current .= $1;
				$needspace = 0;
				next;
			}
			if ($p =~ s/^(\()//) { # Open parenthesis
				$current .= ' ' if $needspace;
				$current .= $1;
				$needspace = 0;
				$paren++;
				next;
			}
			if ($p =~ s/^(\))//) { # Closed parenthesis
				$current .= $1;
				$needspace = 1;
				if (!$paren) {
					$notes .= "\n/* Error: unexpected ')' */";
					print "Error: unexpected ')' at '$p'\n";
				} else {
					$paren--;
				}
				next;
			}
			$p =~ s/^(.)//; # Any other symbol
			$notes .= "\n/* Error: Unexpected character '$1' */";
			print "Error: Unexpected character '$1' at '$p'\n";
			$current .= $1;
			$needspace = 0;
		}

		$current =~ s/^\s+//; $current =~ s/\s+$//g; # trim

		next if (!$current or $current =~ /^void$/); # Skip if empty

		my ($array, $type1, $var, $type2, $indir) = ('', '', '', '', '');
		if ($current =~ qr/^
			([\w\s\[\]*]+\()          # Capture words, spaces, array subscripts, up to the first '(' (return type)
			\s*                       # Skip spaces
			(\*)                      # Capture the '*' from the function name
			\s*                       # Skip spaces
			([\w]*)                   # Capture words (function name)
			\s*                       # Skip spaces
			(\)\s*\([\w\s\[\]*,]*\))  # Capture first ')' followed by a '( )' block containing arguments
			\s*                       # Skip spaces
			$/x
		) { # Match a function pointer
			$type1 = trim($1);
			$indir = $2;
			$var = trim($3 // '');
			$type2 = trim($4);
		} elsif ($current eq '...') { # Match a '...' variadic argument
			$type1 = '...';
			$indir = '';
			$var = '';
			$type2 = '';
		} else { # Match a "regular" variable
			$type1 = '';
			while(1) {
				if ($current =~ /^(const)\s+(.*)$/) { # const modifier
					$type1 .= "$1 ";
					$current = $2 // '';
					next;
				}
				if ($current =~ /^((?:un)?signed)\s+((?:char|int|long|short)[*\s]+.*)$/
				 or $current =~ /^(long|short)\s+((?:int|long)[*\s]+.*)$/
				) { # signed/unsigned/long/short modifiers
					$current = $2;
					$type1 .= "$1 ";
					next;
				}
				if ($current =~ /^(struct|enum|union)\s+(.*)$/) { # union, enum and struct names
					$current = $2 // '';
					$type1 .= "$1 ";
				}
				last; # No other modifiers
			}
			if ($current =~ /^\s*(\w+)((?:const|[*\s])*)(\w*)\s*((?:\[\])?)$/) { # Variable type and name
				$type1 .= trim($1);
				$indir = trim($2 // '');
				$var = trim($3 // '');
				$array = trim($4 // '');
				$type2 = '';
			} else { # Unsupported
				$notes .= "\n/* Error: Unhandled var type '$current' */";
				print "Error: Unhandled var type '$current'\n";
				push(@args, { var => $current });
				next;
			}
		}
		unless ($var) {
			$anonvars++;
			$var = "p$anonvars";
		}
		my ($callvar, $pre_code, $post_code, $dereference, $addressof) = ($var, '', '', '', '');
		my $indirectionlvl = () = $indir =~ /\*/;
		if ($type1 eq 'va_list') { # Special handling for argument-list variables
			$callvar = "${var}___copy";
			$pre_code = "va_list ${callvar}; va_copy(${callvar}, ${var});";
			$post_code = "va_end(${callvar});";
		} elsif ($type1 eq '...') { # Special handling for variadic arguments
			unless ($lastvar) {
				$notes .= "\n/* Error: Variadic function with no fixed arguments */";
				print "Error: Variadic function with no fixed arguments\n";
				next;
			}
			$pre_code = "va_list ${callvar}; va_start(${callvar}, ${lastvar});";
			$post_code = "va_end(${callvar});";
			$var = '';
			$variadic = 1;
		} else { # Increase indirection level when necessary
			$dereference = '*';
			$addressof = '&';
		}
		$indirectionlvl++ if ($array); # Arrays are pointer, no matter how cute you write them

		push(@args, {
			var       => $var,
			callvar   => $callvar,
			type      => $type1.$array.$type2,
			orig      => $type1 eq '...' ? '...' : trim("$type1 $indir$var$array $type2"),
			indir     => $indirectionlvl,
			hookpref  => $type1 eq '...' ? "va_list ${var}" : trim("$type1 $dereference$indir$var$array $type2"),
			hookpostf => $type1 eq '...' ? "va_list ${var}" : trim("$type1 $indir$var$array $type2"),
			hookprec  => trim("$addressof$callvar"),
			hookpostc => trim("$callvar"),
			origc     => trim($callvar),
			pre       => $pre_code,
			post      => $post_code,
		});
		$lastvar = $var;
	}

	my $rtinit = '';
	foreach ($rt) { # Decide initialization for the return value
		my $x = $_;
		if ($x =~ /^const\s+(.+)$/) { # Strip const modifier
			$x = $1;
		}
		if ($x =~ /\*/g) { # Pointer
			$rtinit = ' = NULL';
		} elsif ($x eq 'void') { # void
			$rtinit = '';
		} elsif ($x eq 'bool') { # bool
			$rtinit = ' = false';
		} elsif ($x =~ /^(?:enum\s+)?damage_lv$/) { # Known enum damage_lv
			$rtinit = ' = ATK_NONE';
		} elsif ($x =~ /^(?:enum\s+)?sc_type$/) { # Known enum sc_type
			$rtinit = ' = SC_NONE';
		} elsif ($x =~/^(?:enum\s+)?c_op$/) { # Known enum c_op
			$rtinit = ' = C_NOP';
		} elsif ($x =~ /^enum\s+BATTLEGROUNDS_QUEUE_ACK$/) { # Known enum BATTLEGROUNDS_QUEUE_ACK
			$rtinit = ' = BGQA_SUCCESS';
		} elsif ($x =~ /^enum\s+bl_type$/) { # Known enum bl_type
			$rtinit = ' = BL_NUL';
		} elsif ($x =~ /^enum\s+homun_type$/) { # Known enum homun_type
			$rtinit = ' = HT_INVALID';
		} elsif ($x =~ /^enum\s+channel_operation_status$/) { # Known enum channel_operation_status
			$rtinit = ' = HCS_STATUS_FAIL';
		} elsif ($x =~ /^enum\s+bg_queue_types$/) { # Known enum bg_queue_types
			$rtinit = ' = BGQT_INVALID';
		} elsif ($x =~ /^enum\s+parsefunc_rcode$/) { # Known enum parsefunc_rcode
			$rtinit = ' = PACKET_UNKNOWN';
		} elsif ($x =~ /^enum\s+DBOptions$/) { # Known enum DBOptions
			$rtinit = ' = DB_OPT_BASE';
		} elsif ($x =~ /^enum\s+thread_priority$/) { # Known enum thread_priority
			$rtinit = ' = THREADPRIO_NORMAL';
		} elsif ($x eq 'DBComparator' or $x eq 'DBHasher' or $x eq 'DBReleaser') { # DB function pointers
			$rtinit = ' = NULL';
		} elsif ($x =~ /^(?:struct|union)\s+.*$/) { # Structs and unions
			$rtinit = ' = { 0 }';
		} elsif ($x =~ /^float|double$/) { # Floating point variables
			$rtinit = ' = 0.';
		} elsif ($x =~ /^(?:(?:un)?signed\s+)?(?:char|int|long|short)$/
		      or $x =~ /^(?:long|short)\s+(?:int|long)$/
		      or $x =~ /^u?int(?:8|16|32|64)$/
		      or $x eq 'defType'
		      or $x eq 'size_t'
		) { # Numeric variables
			$rtinit = ' = 0';
		} else { # Anything else
			$notes .= "\n/* Unknown return type '$rt'. Initializing to '0'. */";
			print "Unknown return type '$rt'. Initializing to '0'.\n";
			$rtinit = ' = 0';
		}
	}

	return {
		name     => $name,
		vname    => $variadic ? "v$name" : $name,
		type     => $rt,
		typeinit => $rtinit,
		variadic => $variadic,
		args     => \@args,
		notes    => $notes,
	};
}

my %key2original;
my %key2pointer;
my @files = grep { -f } glob 'doxyoutput/xml/*interface*.xml';
my %ifs;
my %keys = (
	login => [ ],
	char => [ ],
	map => [ ],
	all => [ ],
);
my %fileguards = ( );
foreach my $file (@files) { # Loop through the xml files

	my $xml = new XML::Simple;
	my $data = $xml->XMLin($file, ForceArray => 1);

	my $filekey = (keys %{ $data->{compounddef} })[0];
	my $loc = $data->{compounddef}->{$filekey}->{location}->[0];
	next unless $loc->{file} =~ /src\/(map|char|login|common)\//;
	next if $loc->{file} =~ /\/HPM.*\.h/; # Don't allow hooking into the HPM itself
	next if $loc->{file} =~ /\/memmgr\.h/; # Don't allow hooking into the memory manager
	my $servertype = $1;
	my $key = $data->{compounddef}->{$filekey}->{compoundname}->[0];
	my $original = $key;
	my @servertypes = ();
	my $servermask = 'SERVER_TYPE_NONE';
	if ($servertype ne "common") {
		push @servertypes, $1;
		$servermask = 'SERVER_TYPE_' . uc($1);
	} elsif ($key eq "mapindex_interface") {
		push @servertypes, ("map", "char"); # Currently not used by the login server
		$servermask = 'SERVER_TYPE_MAP|SERVER_TYPE_CHAR';
	} elsif ($key eq "grfio_interface") {
		push @servertypes, ("map"); # Currently not used by the login and char servers
		$servermask = 'SERVER_TYPE_MAP';
	} else {
		push @servertypes, ("map", "char", "login");
		$servermask = 'SERVER_TYPE_ALL';
	}
	my @filepath = split(/[\/\\]/, $loc->{file});
	my $foldername = uc($filepath[-2]);
	my $filename = uc($filepath[-1]); $filename =~ s/[.-]/_/g; $filename =~ s/\.[^.]*$//;
	my $guardname = "${foldername}_${filename}";
	my $private = $key =~ /_interface_private$/ ? 1 : 0;

	# Some known interfaces with different names
	if ($key =~ /battleground/) {
		$key = "bg";
	} elsif ($key =~ /guild_storage/) {
		$key = "gstorage";
	} elsif ($key eq "homunculus_interface") {
		$key = "homun";
	} elsif ($key eq "irc_bot_interface") {
		$key = "ircbot";
	} elsif ($key eq "log_interface") {
		$key = "logs";
	} elsif ($key eq "pc_groups_interface") {
		$key = "pcg";
	} elsif ($key eq "pcre_interface") {
		$key = "libpcre";
	} elsif ($key eq "char_interface") {
		$key = "chr";
	} elsif ($key eq "db_interface") {
		$key = "DB";
	} elsif ($key eq "socket_interface") {
		$key = "sockt";
	} elsif ($key eq "sql_interface") {
		$key = "SQL";
	} elsif ($key eq "stringbuf_interface") {
		$key = "StrBuf";
	} elsif ($key eq "console_input_interface") {
		# TODO
		next;
	} else {
		$key =~ s/_interface//;
	}
	$key =~ s/^(.*)_private$/PRIV__$1/ if $private;
	my $pointername = $key;
	$pointername =~ s/^PRIV__(.*)$/$1->p/ if $private;

	my $sectiondef = $data->{compounddef}->{$filekey}->{sectiondef};
	foreach my $v (@$sectiondef) { # Loop through the sections
		my $memberdef = $v->{memberdef};
		foreach my $f (sort { # Sort the members in declaration order according to what the xml says
					my $astart = $a->{location}->[0]->{bodystart} || $a->{location}->[0]->{line};
					my $bstart = $b->{location}->[0]->{bodystart} || $b->{location}->[0]->{line};
					$astart <=> $bstart
				} @$memberdef) { # Loop through the members
			next unless $f->{kind} eq 'variable'; # Skip macros
			my $t = $f->{argsstring}->[0];
			my $def = $f->{definition}->[0];
			if ($f->{type}->[0] =~ /^\s*LoginParseFunc\s*\*\s*$/) {
				$t = ')(int fd, struct login_session_data *sd)'; # typedef LoginParseFunc
				$def =~ s/^LoginParseFunc\s*\*\s*(.*)$/enum parsefunc_rcode(* $1) (int fd, struct login_session_data *sd)/;
			}
			next unless ref $t ne 'HASH' and $t =~ /^[^\[]/; # If it's not a string, or if it starts with an array subscript, we can skip it

			my $if = parse($t, $def);
			next unless scalar keys %$if; # If it returns an empty hash reference, an error must've occurred

			# Skip variadic functions, we only allow hooks on their arglist equivalents.
			# i.e. you can't hook on map->foreachinmap, but you hook on map->vforeachinmap
			# (foreachinmap is guaranteed to do nothing other than call vforeachinmap)
			next if ($if->{variadic});

			# Some preprocessing
			$if->{hname} = "HP_${key}_$if->{name}";
			$if->{hvname} = "HP_${key}_$if->{vname}";

			$if->{handlerdef} = "$if->{type} $if->{hname}(";
			$if->{predef} = "$if->{type} (*preHookFunc) (";
			$if->{postdef} = "$if->{type} (*postHookFunc) (";
			if ($if->{type} eq 'void') {
				$if->{precall} = '';
				$if->{postcall} = '';
				$if->{origcall} = '';
			} else {
				$if->{precall} = "retVal___ = ";
				$if->{postcall} = "retVal___ = ";
				$if->{origcall} = "retVal___ = ";
			}
			$if->{precall} .= "preHookFunc(";
			$if->{postcall} .= "postHookFunc(";
			$if->{origcall} .= "HPMHooks.source.$key.$if->{vname}(";
			$if->{before} = []; $if->{after} = [];

			my ($i, $j) = (0, 0);

			if ($if->{type} ne 'void') {
				$j++;
				$if->{postdef} .= "$if->{type} retVal___";
				$if->{postcall} .= "retVal___";
			}

			foreach my $arg (@{ $if->{args} }) {
				push(@{ $if->{before} }, $arg->{pre}) if ($arg->{pre});
				push(@{ $if->{after} }, $arg->{post}) if ($arg->{post});
				if ($i) {
					$if->{handlerdef} .=  ', ';
					$if->{predef} .= ', ';
					$if->{precall} .= ', ';
					$if->{origcall} .= ', ';
				}
				if ($j) {
					$if->{postdef} .= ', ';
					$if->{postcall} .= ', ';
				}
				$if->{handlerdef} .= $arg->{orig};
				$if->{predef} .= $arg->{hookpref};
				$if->{precall} .= $arg->{hookprec};
				$if->{postdef} .= $arg->{hookpostf};
				$if->{postcall} .= $arg->{hookpostc};
				$if->{origcall} .= $arg->{origc};
				$i++; $j++;
			}

			if (!$i) {
				$if->{predef} .= 'void';
				$if->{handlerdef} .= 'void';
			}
			if (!$j) {
				$if->{postdef} .= 'void';
			}

			$if->{handlerdef} .= ')';
			$if->{predef} .= ");";
			$if->{precall} .= ");";
			$if->{postdef} .= ");";
			$if->{postcall} .= ");";
			$if->{origcall} .= ");";

			$key2original{$key} = $original;
			$key2pointer{$key} = $pointername;
			$ifs{$key} = [] unless $ifs{$key};
			push(@{ $ifs{$key} }, $if);
		}
	}
	foreach $servertype (@servertypes) {
		push(@{ $keys{$servertype} }, $key) if $key2original{$key};
	}
	push(@{ $keys{all} }, $key) if $key2original{$key};
	$fileguards{$key} = {
		guard => $guardname,
		type => $servermask,
		private => $private,
	};
}

my $year = (localtime)[5] + 1900;

my $fileheader = <<"EOF";
/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2013-$year  Hercules Dev Team
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
EOF

foreach my $servertype (keys %keys) {
	my $keysref = $keys{$servertype};
	# Some interfaces use different names
	my %exportsymbols = map {
		$_ => &{ sub ($) {
			return 'battlegrounds' if $_ =~ /^bg$/;
			return 'pc_groups' if $_ =~ /^pcg$/;
			return $_;
		}}($_);
	} @$keysref;

	my ($maxlen, $idx) = (0, 0);
	my $fname;

	if ($servertype eq 'all') {
		$fname = "../../src/common/HPMSymbols.inc.h";
		open(FH, ">", $fname)
			or die "cannot open > $fname: $!";

		print FH <<"EOF";
$fileheader
#if !defined(HERCULES_CORE)
EOF

		foreach my $key (@$keysref) {
			next if $fileguards{$key}->{private};
			print FH <<"EOF";
#ifdef $fileguards{$key}->{guard} /* $key */
struct $key2original{$key} *$key;
#endif // $fileguards{$key}->{guard}
EOF
		}

		print FH <<"EOF";
#endif // ! HERCULES_CORE

HPExport const char *HPM_shared_symbols(int server_type)
{
EOF

		foreach my $key (@$keysref) {
			next if $fileguards{$key}->{private};
			print FH <<"EOF";
#ifdef $fileguards{$key}->{guard} /* $key */
	if ((server_type&($fileguards{$key}->{type})) != 0 && !HPM_SYMBOL("$exportsymbols{$key}", $key))
		return "$exportsymbols{$key}";
#endif // $fileguards{$key}->{guard}
EOF
		}

		print FH <<"EOF";
	return NULL;
}
EOF
		close FH;

		$fname = "../../src/plugins/HPMHooking/HPMHooking.Defs.inc";
		open(FH, ">", $fname)
			or die "cannot open > $fname: $!";

		print FH <<"EOF";
$fileheader
EOF

		foreach my $key (@$keysref) {
			print FH <<"EOF";
#ifdef $fileguards{$key}->{guard} /* $key */
EOF

			foreach my $if (@{ $ifs{$key} }) {
				my ($predef, $postdef) = ($if->{predef}, $if->{postdef});
				$predef =~ s/preHookFunc/HPMHOOK_pre_${key}_$if->{name}/;
				$postdef =~ s/postHookFunc/HPMHOOK_post_${key}_$if->{name}/;

				print FH <<"EOF";
typedef $predef
typedef $postdef
EOF
			}
			print FH <<"EOF";
#endif // $fileguards{$key}->{guard}
EOF
		}
		close FH;

		next;
	}

	$fname = "../../src/plugins/HPMHooking/HPMHooking_${servertype}.HookingPoints.inc";
	open(FH, ">", $fname)
		or die "cannot open > $fname: $!";

	print FH <<"EOF";
$fileheader
struct HookingPointData HookingPoints[] = {
EOF

	foreach my $key (@$keysref) {
		print FH "/* $key2original{$key} */\n";
		foreach my $if (@{ $ifs{$key} }) {

			print FH <<"EOF";
	{ HP_POP($key2pointer{$key}\->$if->{name}, $if->{hname}) },
EOF

			$idx += 2;
			$maxlen = length($key."->".$if->{name}) if (length($key."->".$if->{name}) > $maxlen);
		}
	}
	print FH <<"EOF";
};

int HookingPointsLenMax = $maxlen;
EOF
	close FH;

	$fname = "../../src/plugins/HPMHooking/HPMHooking_${servertype}.sources.inc";
	open(FH, ">", $fname)
		or die "cannot open > $fname: $!";

	print FH <<"EOF";
$fileheader
EOF
	foreach my $key (@$keysref) {

		print FH <<"EOF";
HPMHooks.source.$key = *$key2pointer{$key};
EOF
	}
	close FH;

	$fname = "../../src/plugins/HPMHooking/HPMHooking_${servertype}.HPMHooksCore.inc";
	open(FH, ">", $fname)
		or die "cannot open > $fname: $!";

	print FH <<"EOF";
$fileheader
struct {
EOF

	foreach my $key (@$keysref) {
		foreach my $if (@{ $ifs{$key} }) {

			print FH <<"EOF";
	struct HPMHookPoint *$if->{hname}_pre;
	struct HPMHookPoint *$if->{hname}_post;
EOF
		}
	}
	print FH <<"EOF";
} list;

struct {
EOF

	foreach my $key (@$keysref) {
		foreach my $if (@{ $ifs{$key} }) {

			print FH <<"EOF";
	int $if->{hname}_pre;
	int $if->{hname}_post;
EOF
		}
	}
	print FH <<"EOF";
} count;

struct {
EOF

	foreach my $key (@$keysref) {

		print FH <<"EOF";
	struct $key2original{$key} $key;
EOF
	}

	print FH <<"EOF";
} source;
EOF
	close FH;

	$fname = "../../src/plugins/HPMHooking/HPMHooking_${servertype}.Hooks.inc";
	open(FH, ">", $fname)
		or die "cannot open > $fname: $!";

	print FH <<"EOF";
$fileheader
EOF
	foreach my $key (@$keysref) {

		print FH <<"EOF";
/* $key2original{$key} */
EOF

		foreach my $if (@{ $ifs{$key} }) {
			my ($initialization, $beforeblock3, $beforeblock2, $afterblock3, $afterblock2, $retval) = ('', '', '', '', '', '');

			unless ($if->{type} eq 'void') {
				$initialization  = "\n\t$if->{type} retVal___$if->{typeinit};";
			}

			$beforeblock3 .= "\n\t\t\t$_" foreach (@{ $if->{before} });
			$afterblock3 .= "\n\t\t\t$_" foreach (@{ $if->{after} });
			$beforeblock2 .= "\n\t\t$_" foreach (@{ $if->{before} });
			$afterblock2 .= "\n\t\t$_" foreach (@{ $if->{after} });
			$retval = ' retVal___' unless $if->{type} eq 'void';

			print FH <<"EOF";
$if->{handlerdef} {$if->{notes}
	int hIndex = 0;${initialization}
	if (HPMHooks.count.$if->{hname}_pre > 0) {
		$if->{predef}
		*HPMforce_return = false;
		for (hIndex = 0; hIndex < HPMHooks.count.$if->{hname}_pre; hIndex++) {$beforeblock3
			preHookFunc = HPMHooks.list.$if->{hname}_pre[hIndex].func;
			$if->{precall}$afterblock3
		}
		if (*HPMforce_return) {
			*HPMforce_return = false;
			return$retval;
		}
	}
	{$beforeblock2
		$if->{origcall}$afterblock2
	}
	if (HPMHooks.count.$if->{hname}_post > 0) {
		$if->{postdef}
		for (hIndex = 0; hIndex < HPMHooks.count.$if->{hname}_post; hIndex++) {$beforeblock3
			postHookFunc = HPMHooks.list.$if->{hname}_post[hIndex].func;
			$if->{postcall}$afterblock3
		}
	}
	return$retval;
}
EOF
		}
	}

	close FH;
}
