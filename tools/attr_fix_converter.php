<?php
/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2021 Hercules Dev Team
 * Copyright (C) 2021 KirieZ
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

/**
 * Element constants, in their numerical order.
 */
$elements = [
	"Ele_Neutral", // 0
	"Ele_Water",
	"Ele_Earth",
	"Ele_Fire",
	"Ele_Wind",
	"Ele_Poison",
	"Ele_Holy",
	"Ele_Dark",
	"Ele_Ghost",
	"Ele_Undead",
];

if (!isset($argv[1]))
	show_help();

$args = parse_args($argv, $argc);
if ($args["show_help"])
	show_help();

if (!$args["re"] && !$args["pre"] && !$args["input"]) {
	echo("Error: You must inform a server mode or an input file.\n\n");
	show_help();
}

if (($args["re"] || $args["pre"]) && $args["input"]) {
	echo("Error: You must inform a server mode or an input file, not both.\n\n");
	show_help();
}

if ($args["re"] && $args["pre"]) {
	echo("Error: You can not use \"-re\" and \"-pre-re\" at the same time.\n\n");
	show_help();
}

if ($args["input"]) {
	$base_path = __DIR__ . DIRECTORY_SEPARATOR . ".." . DIRECTORY_SEPARATOR;
	$input = $args["input"];
	if ($input[0] !== "/")
		$input = $base_path . $input;
	
	$path = $input;
	$out_path = $input . ".conf";
} else {
	$base_path = __DIR__ . DIRECTORY_SEPARATOR . ".." . DIRECTORY_SEPARATOR . "db" . DIRECTORY_SEPARATOR;
	$path = $base_path . ($args["re"] ? "re" : "pre-re") . DIRECTORY_SEPARATOR . "attr_fix.txt";
	$out_path = substr($path, 0, strlen($path) - 4) . ".conf";
}

$attr_table = parse_attr($path);
write_libconfig($out_path, $attr_table);

echo("Conversion finished.\n");
exit;

// ================= Functions
/**
 * Shows converter help page
 */
function show_help()
{
	echo("Usage: php attr_fix_converter.php [option]\n");
	echo("Options:\n");
	echo("\t-re       [--renewal]       for renewal elemental damage database conversion.\n");
	echo("\t-pre-re   [--pre-renewal]   for pre-renewal elemental damage database conversion.\n");
	echo("\t-i <path> [--input] <path>  provides a custom file name/path as input.\n");
	echo("\t-h        [--help]          to display this help text.\n");
	echo("\t                            (it stops afterwards)\n\n");
	echo("----------------------- Additional Notes ----------------------\n");
	echo("Important!\n");
	echo("* Please be advised that either and only one of the arguments -re/-pre-re\n");
	echo("  must be specified on execution.\n");
	echo("* When using the -i option, -re/-pre-re is ignored. \n");
	echo("* When -i option is not used, this tool assumes it is at the \"tools/\" directory\n");
	echo("  of your Hercules folder.\n\n");
	echo("----------------------- Usage Example -------------------------\n");
	echo("- Renewal Conversion: php attr_fix_converter.php --renewal\n");
	echo("- Pre-renewal Conversion: php attr_fix_converter.php --pre-renewal\n");
	echo("- Custom File Conversion: php attr_fix_converter.php --input my_db/attr_fix.txt\n");
	echo("----------------------------------------------------------------\n");
	exit;
}

/**
 * Parses the convert line arguments into an key/value pair
 */
function parse_args($argv, $argc)
{
	$input = null;
	$show_help = false;
	$re = false;
	$pre = false;
	
	$i = 1;
	while ($i < $argc) {
		switch ($argv[$i]) {
		case "-re":
		case "--renewal":
			$re = true;
			break;

		case "-pre-re":
		case "--pre-renewal":
			$pre = true;
			break;

		case "-i":
		case "--input":
			if ($i + 1 >= $argc) {
				echo("Error: \"$argv[$i]\" option must be followed by a file path.\n\n");
				show_help();
			}

			$input = $argv[$i + 1];
			$i++;
			break;

		case "-h":
		case "--help":
			$show_help = true;
			break;

		default:
			echo("Error: Invalid option \"$argv[$i]\".\n\n");
			show_help();
		}
		$i += 1;
	}

	return [
		"re" => $re,
		"pre" => $pre,
		"input" => $input,
		"show_help" => $show_help,
	];
}

/**
 * Returns a basic elemental damage table with everything in 100%.
 */
function get_base_table()
{
	GLOBAL $elements;
	$table = [];

	$damage = [];
	for ($i = 0; $i < count($elements); $i++) {
		$damage[$elements[$i]] = 100;
	}

	for ($i = 0; $i < count($elements); $i++) {
		$table[$elements[$i]] = [
			'1' => $damage,
			'2' => $damage,
			'3' => $damage,
			'4' => $damage,
		];
	}

	return $table;
}

function is_comment_or_empty($line) {
	$l = trim($line);
	return $l == "" || ($line[0] == "/" && $line[1] == "/");
}

/**
 * Parses an attribute table file into an array.
 * @param $path file path
 */
function parse_attr($path)
{
	GLOBAL $elements;
	if (!file_exists($path)) {
		echo("Error: File \"$path\" does not exists.\n");
		exit;
	}
	
	$f = fopen($path, "r");
	if ($f === false) {
		echo("Error: Failed to open attribute table file\n");
		exit;
	}

	$ele_table = get_base_table();

	$ln = 0;
	while (($line = fgets($f)) !== false) {
		$ln++;
		
		if (is_comment_or_empty($line))
			continue;

		$level_size = explode(",", $line);
		if (count($level_size) < 2) {
			echo("Error: Unexpected line $line. Expected level and array size, found less than 2 fields.\n");
			fclose($f);
			exit;
		}

		$lv = (int) $level_size[0];
		$size = (int) $level_size[1];
		$i = 0;
		$max_loop = min($size, count($elements));
		while ($i < $max_loop) {
			while (($line = fgets($f)) !== false && is_comment_or_empty($line))
				$ln++;

			$ln++;
			if ($line === false) {
				if (!feof($f))
					echo("Error: Failed to read line $ln. Unexpected error.\n");
				else
					echo("Error: End of file reached before loading all data.\n");
				fclose($f);
				exit;
			}

			$atk_ele = $elements[$i];

			//Neut Watr Erth Fire Wind Pois Holy Shdw Gho  Und
			$ele_row = explode(',', $line);
			if ($ele_row < $max_loop) {
				echo("Error: Line $ln is incomplete. Expected $max_loop columns.\n");
				fclose($f);
				exit;
			}

			for ($j = 0; $j < $max_loop; $j++) {
				$def_ele = $elements[$j];
				$dmg_adjust = (int) trim($ele_row[$j]);
				$ele_table[$def_ele][$lv][$atk_ele] = $dmg_adjust;
			}
			$i++;
		}
	}

	if (!feof($f)) {
		echo("Error: Failed to read line $ln. Unexpected error.\n");
		fclose($f);
		exit;
	}

	fclose($f);

	return $ele_table;
}

function get_file_header()
{
	return "//================= Hercules Database =====================================
//=       _   _                     _
//=      | | | |                   | |
//=      | |_| | ___ _ __ ___ _   _| | ___  ___
//=      |  _  |/ _ \ '__/ __| | | | |/ _ \/ __|
//=      | | | |  __/ | | (__| |_| | |  __/\__ \
//=      \_| |_/\___|_|  \___|\__,_|_|\___||___/
//================= License ===============================================
//= This file is part of Hercules.
//= http://herc.ws - http://github.com/HerculesWS/Hercules
//=
//= Copyright (C) 2015-2021 Hercules Dev Team
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
//= Elemental attribute damage adjustment tables
//=
//= This file controls the increase/reduction of the attacker's damage element
//= against a defending enemy element.
//=
//= For example:
//= A Fire Lv1 monster will take 150% damage (+50%) from a Water-element attack.
//= - Fire Lv1 is the defending element
//= - Water is the attacking element
//= - 150% is the damage modifier, an increase of 50% over the base damage (100%)
//=
//= Notes:
//= - By default, all defending elements/levels has the adjustment at 100% (base damage)
//= - If the same Defending Element/Level is declared more than one time,
//=   their definitions are merged.
//=========================================================================

/**************************************************************************
 ************* Entry structure ********************************************
 **************************************************************************
<Defending Element>: { // Ele_* constant of the Defending element
	// Level of the defending element (by default, may be Lv1 up to Lv4)
	Lv1: {
		// Attacking Element: the attacking element Ele_* constant
		// adjustment: damage rate, where 100 means 100% (base damage, no additions/reductions)
		<Attacking Element>: <adjustment>
	}
	Lv2: {
		<Attacking Element>: <adjustment>
	}
	Lv3: {
		<Attacking Element>: <adjustment>
	}
	Lv4: {
		<Attacking Element>: <adjustment>
	}
}
**************************************************************************/
";
}

/**
 * Writes the table in libconfig format.
 * @param $table attribute table
 * @param $out_path path of the new file
 */
function write_libconfig($out_path, $table)
{
	if (file_exists($out_path)) {
		echo("Error: File \"$out_path\" already exists. Please remove it first.\n");
		exit;
	}

	$output = get_file_header();
	foreach ($table as $def_ele => $levels) {
		$output .= "\n$def_ele: {\n";
		
		foreach ($levels as $level => $modifiers) {
			$output .= "\tLv$level: {\n";
			
			foreach ($modifiers as $atk_ele => $rate) {
				$output .= "\t\t$atk_ele: $rate\n";
			}

			$output .= "\t}\n";
		}

		$output .= "}";
	}

	$output .= "\n";

	file_put_contents($out_path, $output);
}
