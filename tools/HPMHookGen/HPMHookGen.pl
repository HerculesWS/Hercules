#!/usr/bin/perl

# Copyright (c) Hercules Dev Team, licensed under GNU GPL.
# See the LICENSE file

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
	unless ($d =~ /^(.+)\(\*\s*[a-zA-Z0-9_]+_interface::([^\)]+)\s*\)\(.*\)$/) {
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
				if ($current =~ /^(struct|enum)\s+(.*)$/) { # enum and struct names
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
		} elsif (!$indirectionlvl) { # Increase indirection level when necessary
			$dereference = '*';
			$addressof = '&';
		}
		$indirectionlvl++ if ($array); # Arrays are pointer, no matter how cute you write them

		push(@args, {
			var     => $var,
			callvar => $callvar,
			type    => $type1.$array.$type2,
			orig    => $type1 eq '...' ? '...' : trim("$type1 $indir$var$array $type2"),
			indir   => $indirectionlvl,
			hookf   => $type1 eq '...' ? "va_list ${var}" : trim("$type1 $dereference$indir$var$array $type2"),
			hookc   => trim("$addressof$callvar"),
			origc   => trim($callvar),
			pre     => $pre_code,
			post    => $post_code,
		});
		$lastvar = $var;
	}

	my $rtmemset = 0;
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
		} elsif ($x =~ /^struct\s+.*$/ or $x eq 'DBData') { # Structs
			$rtinit = '';
			$rtmemset = 1;
		} elsif ($x =~ /^(?:(?:un)?signed\s+)?(?:char|int|long|short)$/
		      or $x =~ /^(?:long|short)\s+(?:int|long)$/
		      or $x =~ /^u?int(?:8|16|32|64)$/
		      or $x eq 'defType'
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
		memset   => $rtmemset,
		variadic => $variadic,
		args     => \@args,
		notes    => $notes,
	};
}

my %key2original;
my @files = grep { -f } glob 'doxyoutput/xml/*interface*.xml';
my %ifs;
my @keys;
foreach my $file (@files) { # Loop through the xml files

	my $xml = new XML::Simple;
	my $data = $xml->XMLin($file);

	my $loc = $data->{compounddef}->{location};
	next unless $loc->{file} =~ /src\/map\//; # We only handle mapserver for the time being

	my $key = $data->{compounddef}->{compoundname};
	my $original = $key;

	# Some known interfaces with different names
	if ($key =~ /battleground/) {
		$key = "bg";
	} elsif ($key =~ /guild_storage/) {
		$key = "gstorage";
	} elsif ($key =~ /homunculus/) {
		$key = "homun";
	} elsif ($key =~ /irc_bot/) {
		$key = "ircbot";
	} elsif ($key =~ /log_interface/) {
		$key = "logs";
	} elsif ($key =~ /pc_groups_interface/) {
		$key = "pcg";
	} else {
		$key =~ s/_interface//;
	}

	foreach my $v ($data->{compounddef}->{sectiondef}) { # Loop through the sections
		my $memberdef = $v->{memberdef};
		foreach my $fk (sort { # Sort the members in declaration order according to what the xml says
					my $astart = $memberdef->{$a}->{location}->{bodystart} || $memberdef->{$a}->{location}->{line};
					my $bstart = $memberdef->{$b}->{location}->{bodystart} || $memberdef->{$b}->{location}->{line};
					$astart <=> $bstart
				} keys %$memberdef) { # Loop through the members
			my $f = $memberdef->{$fk};

			my $t = $f->{argsstring};
			next unless ref $t ne 'HASH' and $t =~ /^[^\[]/; # If it's not a string, or if it starts with an array subscript, we can skip it

			my $if = parse($t, $f->{definition});
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
				$if->{predef} .= $arg->{hookf};
				$if->{precall} .= $arg->{hookc};
				$if->{postdef} .= $arg->{hookf};
				$if->{postcall} .= $arg->{hookc};
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
			$ifs{$key} = [] unless $ifs{$key};
			push(@{ $ifs{$key} }, $if);
		}
	}
	push(@keys, $key) if $key2original{$key};
}

# Some interfaces use different names
my %exportsymbols = map {
	$_ => &{ sub ($) {
		return 'battlegrounds' if $_ =~ /^bg$/;
		return 'pc_groups' if $_ =~ /^pcg$/;
		return $_;
	}}($_);
} @keys;

my ($maxlen, $idx) = (0, 0);
my $fname;
$fname = "../../src/plugins/HPMHooking/HPMHooking.HookingPoints.inc";
open(FH, ">", $fname)
	or die "cannot open > $fname: $!";

print FH <<"EOF";
// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
//
// NOTE: This file was auto-generated and should never be manually edited,
//       as it will get overwritten.

struct HookingPointData HookingPoints[] = {
EOF

foreach my $key (@keys) {
	print FH "/* ".$key." */\n";
	foreach my $if (@{ $ifs{$key} }) {

		print FH <<"EOF";
	{ HP_POP($key\->$if->{name}, $if->{hname}) },
EOF

		$idx += 2;
		$maxlen = length($key."->".$if->{name}) if( length($key."->".$if->{name}) > $maxlen )
	}
}
print FH <<"EOF";
};

int HookingPointsLenMax = $maxlen;
EOF
close FH;

$fname = "../../src/plugins/HPMHooking/HPMHooking.sources.inc";
open(FH, ">", $fname)
	or die "cannot open > $fname: $!";

print FH <<"EOF";
// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
//
// NOTE: This file was auto-generated and should never be manually edited,
//       as it will get overwritten.

EOF
foreach my $key (@keys) {

	print FH <<"EOF";
memcpy(&HPMHooks.source.$key, $key, sizeof(struct $key2original{$key}));
EOF
}
close FH;

$fname = "../../src/plugins/HPMHooking/HPMHooking.GetSymbol.inc";
open(FH, ">", $fname)
	or die "cannot open > $fname: $!";

print FH <<"EOF";
// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
//
// NOTE: This file was auto-generated and should never be manually edited,
//       as it will get overwritten.

EOF
foreach my $key (@keys) {

	print FH <<"EOF";
if( !($key = GET_SYMBOL("$exportsymbols{$key}") ) ) return false;
EOF
}
close FH;

$fname = "../../src/plugins/HPMHooking/HPMHooking.HPMHooksCore.inc";
open(FH, ">", $fname)
	or die "cannot open > $fname: $!";

print FH <<"EOF";
// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
//
// NOTE: This file was auto-generated and should never be manually edited,
//       as it will get overwritten.

struct {
EOF

foreach my $key (@keys) {
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

foreach my $key (@keys) {
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

foreach my $key (@keys) {

	print FH <<"EOF";
	struct $key2original{$key} $key;
EOF
}

print FH <<"EOF";
} source;
EOF
close FH;

$fname = "../../src/plugins/HPMHooking/HPMHooking.Hooks.inc";
open(FH, ">", $fname)
	or die "cannot open > $fname: $!";

print FH <<"EOF";
// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
//
// NOTE: This file was auto-generated and should never be manually edited,
//       as it will get overwritten.

EOF
foreach my $key (@keys) {

	print FH <<"EOF";
/* $key */
EOF

	foreach my $if (@{ $ifs{$key} }) {
		my ($initialization, $beforeblock3, $beforeblock2, $afterblock3, $afterblock2, $retval) = ('', '', '', '', '', '');

		unless ($if->{type} eq 'void') {
			$initialization  = "\n\t$if->{type} retVal___$if->{typeinit};";
			$initialization .= "\n\tmemset(&retVal___, '\\0', sizeof($if->{type}));" if $if->{memset};
		}

		$beforeblock3 .= "\n\t\t\t$_" foreach (@{ $if->{before} });
		$afterblock3 .= "\n\t\t\t$_" foreach (@{ $if->{after} });
		$beforeblock2 .= "\n\t\t$_" foreach (@{ $if->{before} });
		$afterblock2 .= "\n\t\t$_" foreach (@{ $if->{after} });
		$retval = ' retVal___' unless $if->{type} eq 'void';

		print FH <<"EOF";
$if->{handlerdef} {$if->{notes}
	int hIndex = 0;${initialization}
	if( HPMHooks.count.$if->{hname}_pre ) {
		$if->{predef}
		for(hIndex = 0; hIndex < HPMHooks.count.$if->{hname}_pre; hIndex++ ) {$beforeblock3
			preHookFunc = HPMHooks.list.$if->{hname}_pre[hIndex].func;
			$if->{precall}$afterblock3
		}
		if( *HPMforce_return ) {
			*HPMforce_return = false;
			return$retval;
		}
	}
	{$beforeblock2
		$if->{origcall}$afterblock2
	}
	if( HPMHooks.count.$if->{hname}_post ) {
		$if->{postdef}
		for(hIndex = 0; hIndex < HPMHooks.count.$if->{hname}_post; hIndex++ ) {$beforeblock3
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

