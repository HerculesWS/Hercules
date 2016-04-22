<?php
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*        _   _                     _
*       | | | |                   | |
*       | |_| | ___ _ __ ___ _   _| | ___  ___
*       |  _  |/ _ \ '__/ __| | | | |/ _ \/ __|
*       | | | |  __/ | | (__| |_| | |  __/\__ \
*       \_| |_/\___|_|  \___|\__,_|_|\___||___/
*
* * * * * * * * * * * * * * License * * * * * * * * * * * * * * * * * * * * * *
* This file is part of Hercules.
* http://herc.ws - http://github.com/HerculesWS/Hercules
*
* Copyright (C) 2016-  Smokexyz/Hercules Dev Team
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
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
* Credits : Smokexyz */

// Credits before anything else.
printcredits();

if(!isset($argv[1]))
	gethelp();

function issetarg($arg) {
	global $argv;
	for($i=1; $i<sizeof($argv); $i++) {
		if(strncmp($argv[$i],$arg,strlen($arg)) == 0)
		return $i;
	}
	return 0;
}

$renewal =  (issetarg("-re") || issetarg("--renewal"));
$prere = (issetarg("-pre-re") || issetarg("--pre-renewal"));
$constants = (!issetarg("-itid") && !issetarg("--use-itemid"));
$help = (issetarg("-h") || issetarg("--help"));
$directory = function() use($argv) {
	$arg = issetarg("--directory")?issetarg("--directory"):(issetarg("-dir")?issetarg("-dir"):0);
	if ($arg) {
		$part = explode("=", $argv[$arg]);
		if(!isset($part[1])) {
			die("A directory path was not provided!\n");
		} else if(!is_dir($part[1])) {
			die("The given directory ".$part[1]." doesn't exist.\n");
		} else {
			return $part[1];
		}
	}
};

$dir = $directory();

if ($dir) {
	print "Read/Write Directory has been set to '".$dir."'\n";
	print "Please ensure all skill_db TXT files are placed in this path.\n";
	print "Please also provide the correct version of the database (re/pre-re).\n";
	define('DIRPATH', $dir);
}

$debug = (issetarg("-dbg") || issetarg("--with-debug"));

if ($debug) {
	print "\033[0;33mDebug Mode Enabled.\033[0;0m\n";
	$t_init = microtime_float();
}

if($help || (!$renewal && !$prere) || ($renewal && $prere)) {
	gethelp();
}

if ($renewal) {
	print "Renewal enabled.\n";
	print "skill_db.txt and associated files (cast, nocastdex, require & unit) will be converted.\n";
	if(!defined('DIRPATH'))
		define('DIRPATH', '../db/re/');
	define('RENEWAL', true);
} else if ($prere) {
	print "Pre-Renewal enabled.\n";
	print "skill_db.txt and associated files (cast, nocastdex, require & unit) will be converted.\n";
	if(!defined('DIRPATH'))
		define('DIRPATH', '../db/pre-re/');
}

// check for existence of files.

$file_check = [
	DIRPATH."skill_require_db.txt",
	DIRPATH."skill_cast_db.txt",
	DIRPATH."skill_castnodex_db.txt",
	DIRPATH."skill_unit_db.txt",
	DIRPATH."skill_db.txt"
];

if($constants) array_push($file_check, DIRPATH."item_db.conf");

foreach($file_check as $file) {
	if(file_exists($file))
		print $file." - \033[0;32mFound\033[0;0m\n";
	else
		die($file." - \033[0;31mNot Found!\033[0;0m\n");
}

if ($constants) {
	print "Using of Item Constants : enabled\n";
} else {
	print "Using of Item Constants : disabled.\n";
}

/* Begin the Loading of Files */

$i=0;
$file="skill_require_db.txt";
$requiredb = fopen(DIRPATH.$file, "r") or die("Unable to open '".DIRPATH.$file."'.\n");
print "\033[0;32mReading\033[0;0m '".DIRPATH.$file."' ...\n";
while(!feof($requiredb))
{
	$line = fgets($requiredb);
	if(substr($line, 0, 2) == "//" || strlen($line) < 10) continue;
	$line = strstr(preg_replace('/\s+/','',$line), "//", true);
	$arr = explode(",", $line);
	if(!isset($arr[0])) continue;
	$skreq['ID'][$i] = $arr[0];
	$skreq['HPCost'][$i] = $arr[1];
	$skreq['MaxHPTrigger'][$i] = $arr[2];
	$skreq['SPCost'][$i] = $arr[3];
	$skreq['HPRateCost'][$i] = $arr[4];
	$skreq['SPRateCost'][$i] = $arr[5];
	$skreq['ZenyCost'][$i] = $arr[6];
	$skreq['Weapons'][$i] = $arr[7];
	$skreq['AmmoTypes'][$i] = $arr[8];
	$skreq['AmmoAmount'][$i] = $arr[9];
	$skreq['State'][$i] = $arr[10];
	$skreq['SpiritSphere'][$i] = $arr[11];
	$k=0;
	for($j=12; $j<=31; $j+=2) {
		$skreqit['ItemId'][$i][$k] = isset($arr[$j])?$arr[$j]:0;
		$skreqit['Amount'][$i][$k] = isset($arr[$j+1])?$arr[$j+1]:0;
		$k++;
	}
	$i++;
}
if ($debug) {
	print "\033[0;34m[Debug]\033[0;0m Read require_db Memory: ".print_mem()."\n";
}
fclose($requiredb);

$file="skill_cast_db.txt";
$skillcastdb = fopen(DIRPATH.$file, "r") or die("Unable to open '".DIRPATH.$file."'.\n");
print "\033[0;32mReading\033[0;0m '".DIRPATH.$file."' ...\n";
$i=0;
while(!feof($skillcastdb))
{
	$line = fgets($skillcastdb);
	if (substr($line, 0, 2) == "//" || strlen($line) < 10) continue;
	$arr = explode(",",$line);
	if (!isset($arr[0])) continue;
	$skcast["ID"][$i] = $arr[0]; // SkillCastDBId
	$skcast["casttime"][$i] = $arr[1];
	$skcast["actdelay"][$i] = $arr[2];
	$skcast["walkdelay"][$i] = $arr[3];
	$skcast["data1"][$i] = $arr[4];
	$skcast["data2"][$i] = $arr[5];
	$skcast["cooldown"][$i] = $arr[6];
	if(defined('RENEWAL')) $skcast["fixedcast"][$i] = $arr[7];

	$i++;
}
if($debug) {
	print "\033[0;34m[Debug]\033[0;0m Read cast_db Memory: ".print_mem()."\n";
}
fclose($skillcastdb);

$file="skill_castnodex_db.txt";
$castnodex = fopen(DIRPATH.$file, "r") or die("Unable to open '".DIRPATH.$file."'.\n");
print "\033[0;32mReading\033[0;0m '".DIRPATH.$file."' ...\n";
$i=0;
while(!feof($castnodex))
{
	$line = fgets($castnodex);
	if(substr($line, 0, 2) == "//" || strlen($line) <= 2) continue;
	$line = strstr(preg_replace('/\s+/','',$line), "//", true);
	$arr = explode(",",$line);
	$sknodex["ID"][$i] = $arr[0];
	$sknodex["cast"][$i] = isset($arr[1])?$arr[1]:0;
	$sknodex["delay"][$i] = isset($arr[2])?$arr[2]:0;
	$i++;
}
if($debug) {
	print "\033[0;34m[Debug]\033[0;0m Read cast_nodex Memory: ".print_mem()."\n";
}
fclose($castnodex);

/***
* Read item_db.conf to gather aegis item name informations.
*/
if ($constants)  {
	$itemdb[] = array();

	$file = "item_db.conf";
	if(file_exists(DIRPATH.$file)) {
		$itemconf = fopen(DIRPATH.$file, "r") or die ("Unable to open '".DIRPATH.$file."'.\n");
		print "\033[0;32mReading\033[0;0m '".DIRPATH.$file."' ...\n";

		$started = false;
		$i=0;
		while(!feof($itemconf)) {
			$line = fgets($itemconf);
			$line = trim($line);
			if(strcmp($line,"{\n"))
				$started = true;
			else if (strcmp($line,"},\n"))
				$started = false;
			//echo str_replace(" ","",$line)."\n";

			if($started == true) {
				$p = explode(":", $line);
				if(isset($p[0])) {
					if($p[0] == "Id") {
						//echo $p[0]." ".(isset($p[1])?$p[1]:NULL)."\n";
						$itemdb['ID'][$i] = intval($p[1]);
					}
					if($p[0] == "AegisName") {
						//echo $p[0]." ".(isset($p[1])?str_replace("\"","",$p[1]):NULL)."\n";
						$itemdb['name'][$i] = str_replace("\"","",$p[1]);
						$i++;
					}
				}
			}
		}
		if($debug) {
			print "\033[0;34m[Debug]\033[0;0m Read item_db Memory: ".print_mem()."\n";
		}
		fclose($itemconf);
	} else {
		print "Unable to open '".DIRPATH.$file."'... defaulting to using Item ID's instead of Constants.\n";
		$constants = false;
	}
}

/* * *
* Bring forth the contents of skill_unit_db.txt and store them.
*/
$i=0;
$file="skill_unit_db.txt";
$unitdb = fopen(DIRPATH.$file, "r") or die("Unable to open '".DIRPATH.$file."'.\n");
print "\033[0;32mReading\033[0;0m '".DIRPATH.$file."' ...\n";
while(!feof($unitdb)) {
	$line = fgets($unitdb);
	if(substr($line, 0, 2) == "//" || strlen($line) < 10) continue;
	$line = strstr(preg_replace('/\s+/','',$line), "//", true);
	$arr = explode(",",$line);
	$skunit['ID'][$i] = $arr[0];
	$skunit['UnitID'][$i] = $arr[1];
	$skunit['UnitID2'][$i] = $arr[2];
	$skunit['Layout'][$i] = $arr[3];
	$skunit['Range'][$i] = $arr[4];
	$skunit['Interval'][$i] = $arr[5];
	$skunit['Target'][$i] = $arr[6];
	$skunit['Flag'][$i] = substr($arr[7],2);
	$i++;
}
if($debug) {
	print "\033[0;34m[Debug]\033[0;0m Read unit_db Memory: ".print_mem()."\n";
}
fclose($unitdb);

$putsk = ""; // initialize variable for file_put_contents.
// Publish all comments
$putsk .= getcomments((defined('RENEWAL')?true:false));

$putsk .= "skill_db: (\n";
// Get Main Skilldb File
$file="skill_db.txt";
$skmain = fopen(DIRPATH.$file, "r") or die("Unable to open '".DIRPATH.$file."'.\n");
print "\033[0;32mReading\033[0;0m '".DIRPATH.$file."' ...\n";
$linecount = 0;

// Get Number of entries
while(!feof($skmain)) {
	$line = fgets($skmain);
	if(substr($line, 0, 2) == "//" || strlen($line) < 10) continue;
	$linecount++;
}
if($debug) {
	print "\033[0;34m[Debug]\033[0;0m Read skill_db Memory: ".print_mem()."\n";
}
fclose($skmain);
print $linecount." entries found in skill_db.txt.\n";

$i=0;
$skmain = fopen(DIRPATH.$file, "r") or die("Unable to open '".DIRPATH.$file."'.\n");
// Begin converting process.
while(!feof($skmain)) {
	$line = fgets($skmain);
	if(substr($line, 0, 2) == "//" || strlen($line) < 10) continue;
	$arr = explode(",", $line);
	// id,range,hit,inf,element,nk,splash,max,list_num,castcancel,cast_defence_rate,inf2,maxcount,skill_type,blow_count,name,description
	$id = $arr[0];
	$range = $arr[1];
	$hit = $arr[2];
	$inf = $arr[3];
	$element = $arr[4];
	$nk = $arr[5];
	$splash = $arr[6];
	$max = $arr[7];
	$max = ($max>10)?10:(($max<1)?1:$max);
	$list_num = $arr[8];
	$castcancel = $arr[9];
	$cast_defence_rate = $arr[10];
	$inf2 = $arr[11];
	$maxcount = $arr[12];
	$skill_type = $arr[13];
	$blow_count = $arr[14];
	$name = $arr[15];
	if(strlen(substr($arr[16], 0, strpos($arr[16], "//"))))
		$description = substr($arr[16], 0, strpos($arr[16], "//"));
	else
		$description = $arr[16];

	$putsk .=  "{\n".
	"\tId: ".$id."\n".
	"\tName: \"".trim($name)."\"\n".
	"\tDescription: \"".trim($description)."\"\n".
	"\tMaxLevel: ".$max."\n";
	if($range) $putsk .=  "\tRange: ".leveled($range, $max, $id)."\n";
	if($hit==8) $putsk .=  "\tHit: \"BDT_MULTIHIT\"\n";
	else if($hit==6) $putsk .=  "\tHit: \"BDT_SKILL\"\n";
	if($inf) $putsk .=  "\tSkillType: ".getinf($inf)."\n";
	if($inf2) $putsk .=  "\tSkillInfo: ".getinf2($inf2)."\n";
	if($skill_type != "none" && $inf) $putsk .=  "\tAttackType: \"".ucfirst($skill_type)."\"\n";
	if($element) $putsk .=  "\tElement: ".leveled_ele($element, $max, $id)."\n";
	if($nk && $nk != "0x0") $putsk .=  "\tDamageType: ".getnk($nk)."\n";
	if($splash) $putsk .=  "\tSplashRange: ".leveled($splash, $max, $id)."\n";
	if($list_num != "1") $putsk .=  "\tNumberOfHits: ".leveled($list_num, $max, $id)."\n";
	if($castcancel && $inf) $putsk .=  "\tInterruptCast: true\n";
	if($cast_defence_rate) $putsk .=  "\tCastDefRate: ".$cast_defence_rate."\n";
	if($maxcount) $putsk .=  "\tSkillInstances: ".leveled($maxcount, $max, $id)."\n";
	if($blow_count) $putsk .=  "\tKnockBackTiles: ".leveled($blow_count, $max, $id)."\n";
	// Cast Db
	$key = array_search($id, $skcast['ID']);
	if($key) {
		if($skcast['casttime'][$key]) $putsk .=  "\tCastTime: ".leveled($skcast['casttime'][$key], $max, $id)."\n";
		if($skcast['actdelay'][$key]) $putsk .=  "\tAfterCastActDelay: ".leveled($skcast['actdelay'][$key], $max, $id)."\n";
		if($skcast['walkdelay'][$key] != 0) $putsk .=  "\tAfterCastWalkDelay: ".leveled($skcast['walkdelay'][$key], $max, $id)."\n";
		if($skcast['data1'][$key] != 0) $putsk .=  "\tSkillData1: ".leveled($skcast['data1'][$key], $max, $id)."\n";
		if($skcast['data2'][$key] != 0) $putsk .=  "\tSkillData2: ".leveled($skcast['data2'][$key], $max, $id)."\n";
		if($skcast['cooldown'][$key] != 0) $putsk .=  "\tCoolDown: ".leveled($skcast['cooldown'][$key], $max, $id)."\n";
		if(defined('RENEWAL') && strlen($skcast['fixedcast'][$key])>1 && $skcast['fixedcast'][$key] != 0)
			$putsk .=  "\tFixedCastTime: ".leveled($skcast['fixedcast'][$key], $max, $id)."\n";
	}
	// Cast NoDex
	unset($key);
	$key = array_search($id, $sknodex['ID']);
	if($key) {
		if (isset($sknodex["cast"][$key]) && $sknodex["cast"][$key] != 0) $putsk .=  "\tCastTimeOptions: ".getnocast($sknodex["cast"][$key], $id)."\n";
		if (isset($sknodex["delay"][$key]) && $sknodex["delay"][$key] != 0) $putsk .=  "\tSkillDelayOptions: ".getnocast($sknodex["delay"][$key], $id)."\n";
		unset($sknodex["ID"][$key]);
		unset($sknodex["cast"][$key]);
		unset($sknodex["delay"][$key]);
	}

	// require DB
	unset($key);
	$key = array_search($id, $skreq['ID']);
	if($key) {
		$putsk .=  "\tRequirements: {\n";
		if ($skreq['HPCost'][$key]) $putsk .=  "\t\tHPCost: ".leveled($skreq['HPCost'][$key], $max, $id, 1)."\n";
		if ($skreq['SPCost'][$key]) $putsk .=  "\t\tSPCost: ".leveled($skreq['SPCost'][$key], $max, $id, 1)."\n";
		if ($skreq['HPRateCost'][$key]) $putsk .=  "\t\tHPRateCost: ".leveled($skreq['HPRateCost'][$key], $max, $id, 1)."\n";
		if ($skreq['SPRateCost'][$key]) $putsk .=  "\t\tSPRateCost: ".leveled($skreq['SPRateCost'][$key], $max, $id, 1)."\n";
		if ($skreq['ZenyCost'][$key]) $putsk .=  "\t\tZenyCost: ".leveled($skreq['ZenyCost'][$key], $max, $id, 1)."\n";
		if ($skreq['Weapons'][$key] != 99) $putsk .=  "\t\tWeaponTypes: ".getweapontypes($skreq['Weapons'][$key], $id)."\n";
		if ($skreq['AmmoTypes'][$key] == 99) $putsk .=  "\t\tAmmoTypes: \"All\"\n";
		else if ($skreq['AmmoTypes'][$key]) $putsk .=  "\t\tAmmoTypes: ".getammotypes($skreq['AmmoTypes'][$key], $id)."\n";
		if ($skreq['AmmoAmount'][$key]) $putsk .=  "\t\tAmmoAmount: ".leveled($skreq['AmmoAmount'][$key],  $max, $id, 1)."\n";
		if ($skreq['State'][$key] != "none" && $skreq['State'][$key]) $putsk .=  "\t\tState: \"".getstate($skreq['State'][$key],$id)."\"\n";
		if ($skreq['SpiritSphere'][$key]) $putsk .=  "\t\tSpiritSphereCost: ".leveled($skreq['SpiritSphere'][$key], $max, $id, 1)."\n";
		if ($skreqit['ItemId'][$key][0] > 0) {
			$putsk .=  "\t\tItems: {\n";
			for ($index=0; $index<sizeof($skreqit['ItemId'][$key]); $index++) {
				$itemID = $skreqit['ItemId'][$key][$index]; // Required Item
				$itemamt = $skreqit['Amount'][$key][$index]; // Required Amount

				if (strpos($itemID, ':') == true) {
					$items = explode(":", $itemID);
					$it = 0;
					while (isset($items) && isset($items[$it])) {
						if ($constants && $itemID) {
							$itkey = array_search($items[$it], $itemdb['ID']);
							if($itkey == 0) $itemname = "ID".$itemID;
							else $itemname = $itemdb['name'][$itkey];
							$putsk .=  "\t\t\t".trim($itemname).": ".leveled($itemamt, $max, $id, 2)."\n";
						} else if (intval($itemID)) {
							$putsk .=  "\t\t\tID".$items[$it].": ".leveled($itemamt, $max, $id, 2)."\n";
						}
						$it++;
					}
				} else {
					if ($constants && $itemID) {
						$itkey = array_search($skreqit['ItemId'][$key][$index], $itemdb['ID']);
						if($itkey == 0) $itemname = "ID".$itemID;
						else $itemname = $itemdb['name'][$itkey];
						$putsk .=  "\t\t\t".trim($itemname).": ".leveled($itemamt, $max, $id, 2)."\n";
					} else if ($itemID) {
						$putsk .=  "\t\t\tID".$itemID.": ".leveled($itemamt, $max, $id, 2)."\n";
					}
				}
			}
			$putsk .=  "\t\t}\n";
		}
		$putsk .=  "\t}\n";
	}

	unset($key);
	$key = array_search($id, $skunit['ID']);
	if($key) {
		$putsk .=  "\tUnit: {\n";
		if(isset($skunit['UnitID'][$key])) {
			if(isset($skunit['UnitID2'][$key]) && strlen($skunit['UnitID2'][$key])) {
				$putsk .=  "\t\tId: [ ".$skunit['UnitID'][$key].", ".$skunit['UnitID2'][$key]." ]\n";
			} else $putsk .=  "\t\tId: ".$skunit['UnitID'][$key]."\n";
		}
		if(isset($skunit['Layout'][$key]) && $skunit['Layout'][$key] != 0) $putsk .=  "\t\tLayout: ".leveled($skunit['Layout'][$key], $max, $id, 1)."\n";
		if(isset($skunit['Range'][$key]) && $skunit['Range'][$key] != 0) $putsk .=  "\t\tRange: ".leveled($skunit['Range'][$key], $max, $id, 1)."\n";
		if(isset($skunit['Interval'][$key])) $putsk .=  "\t\tInterval: ".leveled($skunit['Interval'][$key], $max, $id, 1)."\n";
		if(isset($skunit['Target'][$key]) && $skunit['Target'][$key] != "noone") $putsk .=  "\t\tTarget: \"".trim(ucfirst($skunit['Target'][$key]))."\"\n";
		if(isset($skunit['Flag'][$key]) && intval($skunit['Flag'][$key]) > 0
		&& $skunit['Flag'][$key] != "")
			$putsk .=  "\t\tFlag: ".getunitflag($skunit['Flag'][$key], $id)."\n";
		$putsk .=  "\t}\n";
	}
	// close skill
	$putsk .=  "},\n";
	// Display progress bar
	show_status($i++, $linecount);
}
show_status($linecount, $linecount);
/**
 * Print final messages and exit the script, conversion has completed.
 */
print "\n";
print "The skill database has been \033[1;32msuccessfully\033[0m converted to Hercules' libconfig\n";
print "format and has been saved as '".DIRPATH."skill_db.conf'.\n";
print "The following files are now deprecated and can be deleted -\n";
print DIRPATH."skill_db.txt\n";
print DIRPATH."skill_cast_db.txt\n";
print DIRPATH."skill_castnodex_db.txt\n";
print DIRPATH."skill_require_db.txt\n";
print DIRPATH."skill_unit_db.txt\n";
$putsk .=  ")";
$skconf = "skill_db.conf";
file_put_contents(DIRPATH.$skconf, $putsk);
if($debug) {
	print "\033[0;34m[Debug]\033[0;0m Memory after converting: ".print_mem()."\n";
	print "\033[0;34m[Debug]\033[0;0m Execution Time : ".(microtime_float()-$t_init)."s\n";
}
fclose($skmain);

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* Functions
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

function microtime_float()
{
    list($usec, $sec) = explode(" ", microtime());
    return ((float)$usec + (float)$sec);
}

function show_status($done, $total) {
	$perc = floor(($done / $total) * 100);
	$perc = floor($perc/2);
	$left = 50-$perc;
	$finalperc = $perc * 2;
	$write = sprintf("\033[0G\033[2K[%'={$perc}s>%-{$left}s] - $finalperc%% - $done/$total", "", "");
	fwrite(STDERR, $write);
}
function get_element($ele,$id)
{
	switch($ele)
	{
		case -1: return "Ele_Weapon";
		case -2: return "Ele_Endowed";
		case -3: return "Ele_Random";
		case 0:  return "Ele_Neutral";
		case 1:  return "Ele_Water";
		case 2:  return "Ele_Earth";
		case 3:  return "Ele_Fire";
		case 4:  return "Ele_Wind";
		case 5:  return "Ele_Poison";
		case 6:  return "Ele_Holy";
		case 7:  return "Ele_Dark";
		case 8:  return "Ele_Ghost";
		case 9:  return "Ele_Undead";
		default: print "\r\033[0;31mWarning\033[0;0mUnknown Element ".$ele." provided for skill Id ".$id."\n";
	}

	return NULL;
}
function leveled_ele($str, $max, $skill_id)
{
	if(strpos($str, ':') == true)
	{
		$lvs = explode(":", $str);
		$retval = "{\n";
		for($i=0; $i<$max && isset($lvs[$i]); $i++) {
			if (isset($lvs[$i])) {
				$retval .= "\t\tLv".($i+1).": \"".get_element($lvs[$i],$skill_id)."\"\n";
			} else {
				print "\r\033[0;31mWarning\033[0;0m - Invalid Level ".($i+1)." provided in Element Level for Skill Id ".$skill_id."\n";
				continue;
			}
		}
		$retval .= "\t}";
	} else {
		$retval = "\"".get_element($str,$skill_id)."\"";
	}

	return $retval;
}

function leveled($str, $max, $id, $tab=0)
{
	switch($tab) {
		case 1:
			$ittab = "\t\t\t";
			$endtab = "\t\t";
			break;
		case 2:
			$ittab = "\t\t\t\t";
			$endtab = "\t\t\t";
			break;
		default:
			$ittab = "\t\t";
			$endtab = "\t";
			break;
	}

	$retval = "";
	if(strpos($str, ':') == true) {
		$lvs = explode(":", trim($str));
		$retval = "{\n";
		for($i=0; $i<$max && isset($lvs[$i]); $i++) {
			if(isset($lvs[$i])) {
				$retval .= $ittab."Lv".($i+1).": ".$lvs[$i]."\n";
			} else {
				print "\r\033[0;31mWarning\033[0;0m - Invalid Level index ".($i+1)." provided in skill Id ".$id."\n";
			}
		}
		$retval .= $endtab."}";
	} else {
		$retval = intval($str);
	}

	return $retval;
}

function getstate($state,$id)
{
	if(      strcmp($state,"hiding")              == 0 ) return "Hiding";
	else if( strcmp($state,"cloaking")            == 0 ) return "Cloaking";
	else if( strcmp($state,"hidden")              == 0 ) return "Hidden";
	else if( strcmp($state,"riding")              == 0 ) return "Riding";
	else if( strcmp($state,"falcon")              == 0 ) return "Falcon";
	else if( strcmp($state,"cart")                == 0 ) return "Cart";
	else if( strcmp($state,"shield")              == 0 ) return "Shield";
	else if( strcmp($state,"sight")               == 0 ) return "Sight";
	else if( strcmp($state,"explosionspirits")    == 0 ) return "ExplosionSpirits";
	else if( strcmp($state,"cartboost")           == 0 ) return "CartBoost";
	else if( strcmp($state,"recover_weight_rate") == 0 ) return "NotOverWeight";
	else if( strcmp($state,"move_enable")         == 0 ) return "Moveable";
	else if( strcmp($state,"water")               == 0 ) return "InWater";
	else if( strcmp($state,"dragon")              == 0 ) return "Dragon";
	else if( strcmp($state,"warg")                == 0 ) return "Warg";
	else if( strcmp($state,"ridingwarg")          == 0 ) return "RidingWarg";
	else if( strcmp($state,"mado")                == 0 ) return "MadoGear";
	else if( strcmp($state,"elementalspirit")     == 0 ) return "ElementalSpirit";
	else if( strcmp($state,"poisonweapon")        == 0 ) return "PoisonWeapon";
	else if( strcmp($state,"rollingcutter")       == 0 ) return "RollingCutter";
	else if( strcmp($state,"mh_fighting")         == 0 ) return "MH_Fighting";
	else if( strcmp($state,"mh_grappling")        == 0 ) return "MH_Grappling";
	else if( strcmp($state,"peco")                == 0 ) return "Peco";
	else print "\r\033[0;31mWarning\033[0;0m - Invalid State ".$state." provided for Skill ID ".$id.", please correct this manually.\n";
}

function getinf($inf)
{
	$bitmask = array(
		"Passive"     => 0,
		"Enemy"       => 1,
		"Place"       => 2,
		"Self"        => 4,
		"Friend"      => 16,
		"Trap"        => 32,
	);

	$retval = "{\n";
	foreach ($bitmask as $key => $val)
	{
		if($inf&$val)
		$retval .= "\t\t".$key.": true\n";
	}
	$retval .= "\t}";

	return $retval;
}

function getinf2($inf2=0x0000)
{
	$bitmask = array(
		"Quest"               => intval(0x0001), // = quest skill
		"NPC"                 => intval(0x0002), // = npc skill
		"Wedding"             => intval(0x0004), // = wedding skill
		"Spirit"              => intval(0x0008), // = spirit skill
		"Guild"               => intval(0x0010), // = guild skill
		"Song"                => intval(0x0020), // = song/dance
		"Ensemble"            => intval(0x0040), // = ensemble skill
		"Trap"                => intval(0x0080), // = trap
		"TargetSelf"          => intval(0x0100), // = skill that damages/targets yourself
		"NoCastSelf"          => intval(0x0200), // = cannot be casted on self (if inf = 4, auto-select target skill)
		"PartyOnly"           => intval(0x0400), // = usable only on party-members (and enemies if skill is offensive)
		"GuildOnly"           => intval(0x0800), // = usable only on guild-mates (and enemies if skill is offensive)
		"NoEnemy"             => intval(0x1000), // = disable usage on enemies (for non-offensive skills).
		"IgnoreLandProtector" => intval(0x2000), // = skill ignores land protector (e.g. arrow shower)
		"Chorus"              => intval(0x4000) // = chorus skill
	);

	$inf2 = intval(substr($inf2, 2),16);

	$retval = "{\n ";
	foreach($bitmask as $key => $val) {
		if($inf2&$val) {
			$retval .= "\t\t".$key.": true\n";
		}
	}
	$retval .= "\t}";

	return $retval;
}

function getnk($nk)
{
	$bitmask = array(
		"NoDamage"         =>   intval(0x01), //- No damage skill
		"SplashArea"       =>   intval(0x02)+intval(0x04), //- Has splash area (requires source modification)
		"SplitDamage"      =>   intval(0x04), //- Damage should be split among targets (requires 0x02 in order to work)
		"IgnoreCards"      =>   intval(0x08), //- Skill ignores caster's % damage cards (misc type always ignores)
		"IgnoreElement"    =>   intval(0x10), //- Skill ignores elemental adjustments
		"IgnoreDefense"    =>   intval(0x20), //- Skill ignores target's defense (misc type always ignores)
		"IgnoreFlee"       =>   intval(0x40), //- Skill ignores target's flee (magic type always ignores)
		"IgnoreDefCards"   =>   intval(0x80) //- Skill ignores target's def cards
	);
	$nk = intval($nk,16);
	$retval = "{\n";
	foreach($bitmask as $key => $val) {
		if($key == "SplitDamage") {
			if($nk&$bitmask["SplashArea"])
				continue;
			else {
				$retval .= "\t\tSplashArea: true\n";
			}
		} else if($nk&$val) $retval .= "\t\t".$key.": true\n";
	}
	$retval .= "\t}";

	return $retval;
}

function getnocast($opt, $id)
{
	$bitmask = array(
		'Default'              => 0, //- everything affects the skill's cast time
		'IgnoreDex'            => 1, //- skill's cast time is not affected by dex
		'IgnoreStatusEffect'   => 2, //- skill's cast time is not affected by statuses (Suffragium, etc)
		'IgnoreItemBonus'      => 4 //- skill's cast time is not affected by item bonuses (equip, cards)
	);

	if($opt > array_sum($bitmask) || $opt < 0)
		print "\r\033[0;31mWarning\033[0;0m - a bitmask for CastNoDex entry for skill ID ".$id." is higher than total of masks or lower than 0.";

	$retval = "{\n";
	foreach($bitmask as $key => $val) {
		if($opt&$val) {
			$retval .= "\t\t".$key.": true\n";
		}
	}
	$retval .= "\t}";

	return $retval;
}

function getweapontypes($list, $id)
{
	$bitmask = array(
		0  => "NoWeapon",
		1  => "Daggers",
		2  => "1HSwords",
		3  => "2HSwords",
		4  => "1HSpears",
		5  => "2HSpears",
		6  => "1HAxes",
		7  => "2HAxes",
		8  => "Maces",
		9  => "2HMaces",
		10 => "Staves",
		11 => "Bows",
		12 => "Knuckles",
		13 => "Instruments",
		14 => "Whips",
		15 => "Books",
		16 => "Katars",
		17 => "Revolvers",
		18 => "Rifles",
		19 => "GatlingGuns",
		20 => "Shotguns",
		21 => "GrenadeLaunchers",
		22 => "FuumaShurikens",
		23 => "2HStaves",
		24 => "MaxSingleWeaponType",
		25 => "DWDaggers",
		26 => "DWSwords",
		27 => "DWAxes",
		28 => "DWDaggerSword",
		29 => "DWDaggerAxe",
		30 => "DWSwordAxe",
	);
	if(strpos($list, ':') == true) {
		$type = explode(":", $list);
		$wmask = 0;
		for($i=0; $i<sizeof($type); $i++) {
			$wmask |= 1<<$type[$i];
			if($type[$i] > 30 || $type[$i] < 0)
				print "\r\033[0;31mWarning\033[0;0m - Invalid weapon type ".$i." for skill ID ".$id."\n";
		}
		$retval = "{\n";
		for($j=0; $j<sizeof($type); $j++) {
			if($wmask&1<<$type[$j])
				$retval .= "\t\t\t".$bitmask[$type[$j]].": true\n";
		}
		$retval .= "\t\t}";
	} else {
		$retval = "{\n";
		$retval .= "\t\t\t".$bitmask[$list].": true\n";
		$retval .= "\t\t}";
	}

	return $retval;
}

function getammotypes($list, $id) {
	$bitmask = array(
		1 => "A_ARROW",
		2 => "A_DAGGER",
		3 => "A_BULLET",
		4 => "A_SHELL",
		5 => "A_GRENADE",
		6 => "A_SHURIKEN",
		7 => "A_KUNAI",
		8 => "A_CANNONBALL",
		9 => "A_THROWWEAPON"
	);

	if(strpos($list, ':') == true) {
		$type = explode(":", $list);
		$wmask = 0;
		for($i=0; $i<sizeof($type); $i++) {
			$wmask |= 1<<$type[$i];
			if($type[$i] > 9 || $type[$i] < 1) {
				print "\r\033[0;31mWarning\033[0;0m - Invalid weapon type ".$i." for skill ID ".$id."\n";
			}
		}
		$retval = "{\n ";
		for($j=0; $j<sizeof($type); $j++) {
			if($wmask&1<<$type[$j]) {
				$retval .= "\t\t\t".$bitmask[$type[$j]].": true\n";
			}
		}
		$retval .= "\t\t}";
	} else {
		$retval = "{\n";
		$retval .= "\t\t\t".$bitmask[$list].": true\n";
		$retval .= "\t\t}";
	}

	return $retval;
}

function getunitflag($flag, $id)
{
	$bitmask = array(
		'UF_DEFNOTENEMY'    =>    intval(0x001),          //0x001(UF_DEFNOTENEMY)If 'defunit_not_enemy' is set, the target is changed to 'friend'
		'UF_NOREITERATION'  =>    intval(0x002),          //0x002(UF_NOREITERATION)Spell cannot be stacked
		'UF_NOFOOTSET'      =>    intval(0x004),          //0x004(UF_NOFOOTSET)Spell cannot be cast near/on targets
		'UF_NOOVERLAP'      =>    intval(0x008),          //0x008(UF_NOOVERLAP)Spell effects do not overlap
		'UF_PATHCHECK'      =>    intval(0x010),          //0x010(UF_PATHCHECK)Only cells with a shootable path will be placed
		'UF_NOPC'           =>    intval(0x020),          //0x020(UF_NOPC)Spell cannot affect players.
		'UF_NOMOB'          =>    intval(0x040),          //0x040(UF_NOMOB)Spell cannot affect mobs.
		'UF_SKILL'          =>    intval(0x080),          //0x080(UF_SKILL)Spell CAN affect skills.
		'UF_DANCE'          =>    intval(0x100),          //0x100(UF_DANCE)Dance skill
		'UF_ENSEMBLE'       =>    intval(0x200),          //0x200(UF_ENSEMBLE)Ensemble skill
		'UF_SONG'           =>    intval(0x400),          //0x400(UF_SONG)Song skill
		'UF_DUALMODE'       =>    intval(0x800),          //0x800(UF_DUALMODE)Spell has effects both at an interval and when you step in/out
		'UF_RANGEDSINGLEUNIT' =>  intval(0x2000)          //0x2000(UF_RANGEDSINGLEUNIT)Layout hack, use layout range propriety but only display center.
	);

	$count = 0; $flag = 0;
	$flag = intval($flag,16);
	if($flag <= 0) return 0;

	$ret = "{\n ";
	foreach($bitmask as $key => $val) {
		if($flag&$val) {
			$ret .= "\t\t\t".$key.": true\n";
		}
	}

	if($flag > array_sum($bitmask))
		print "\r\033[0;31mWarning\033[0;0m - Invalid Unit Flag ".$flag." provided for skill Id ".$id."\n";

	$ret .= "\t\t}";

	return $ret;
}

function print_mem()
{
	return convert(memory_get_usage(true));
}

function convert($size)
{
	$unit=array('b','kb','mb','gb','tb','pb');
	return @round($size/pow(1024,($i=floor(log($size,1024)))),2).' '.$unit[$i];
}

function gethelp()
{
	print "Usage: php skilldbconverter.php [option]\n";
	print "Options:\n";
	print "\t-re     [--renewal]          for renewal skill database conversion.\n";
	print "\t-pre-re [--pre-renewal]      for pre-renewal skill database conversion.\n";
	print "\t-itid   [--use-itemid]       to use item IDs instead of constants.\n";
	print "\t-dir    [--directory]        provide a custom directory.\n";
	print "\t                             (Must include the correct -pre-re/-re option)\n";
	print "\t-dbg    [--with-debug]       print debug information.\n";
	print "\t-h      [--help]             to display this help text.\n\n";
	print "----------------------- Additional Notes ----------------------\n";
	print "\033[0;31mImportant!\033[0;0m\n";
	print "* Please be advised that either and only one of the arguments -re/-pre-re\n";
	print "  must be specified on execution.\n";
	print "* When using the -dir option, -re/-pre-re options must be specified. \n";
	print "* This tool isn't designed to convert renewal data to pre-renewal.\n";
	print "* This tool should ideally be used from the 'tools/' folder, which can be found\n";
	print "  in the root of your Hercules installation. This tool will not delete any files\n";
	print "  from any of the directories that it reads from or prints to.\n\n";
	print "* Prior to using this tool, please ensure at least 30MB of free RAM.\n";
	print "----------------------- Usage Example -------------------------\n";
	print "- \033[0;32mRenewal Conversion\033[0;0m: php skilldbconverter.php --renewal\n";
	print "- \033[0;32mPre-renewal Conversion\033[0;0m: php skilldbconverter.php --pre-renewal\n";
	print "----------------------------------------------------------------\n";
	exit;
}

function printcredits()
{
	print "      _   _                     _           \n";
	print "     | | | |                   | |          \n";
	print "     | |_| | ___ _ __ ___ _   _| | ___  ___ \n";
	print "     |  _  |/ _ \ '__/ __| | | | |/ _ \/ __|\n";
	print "     | | | |  __/ | | (__| |_| | |  __/\__ \ \n";
	print "     \_| |_/\___|_|  \___|\__,_|_|\___||___/\n";
	print "\033[0;36mHercules Skill Database TXT to Libconfig Converter by Smokexyz\033[0m\n";
	print "Copyright (C) 2016  \033[0;32mHercules\033[0m\n";
	print "-----------------------------------------------\n\n";
}

function getcomments($re)
{
	return
	"//================= Hercules Database ==========================================
	//=       _   _                     _
	//=      | | | |                   | |
	//=      | |_| | ___ _ __ ___ _   _| | ___  ___
	//=      |  _  |/ _ \ '__/ __| | | | |/ _ \/ __|
	//=      | | | |  __/ | | (__| |_| | |  __/\__ \
	//=      \_| |_/\___|_|  \___|\__,_|_|\___||___/
	//================= License ====================================================
	//= This file is part of Hercules.
	//= http://herc.ws - http://github.com/HerculesWS/Hercules
	//=
	//= Copyright (C) 2014-2016  Hercules Dev Team
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
	//==============================================================================
	//= ".($re?"Renewal":"Pre-Renewal")." Skill Database [Hercules]
	//==============================================================================
	//= @Format Notes:
	//= - All string entries are case-sensitive and must be quoted.
	//= - All setting names are case-sensitive and must be keyed accurately.
	/******************************************************************************
	********************************* Entry structure *****************************
	*******************************************************************************

	{
	    ------------------------------ Mandatory Fields ----------------------------
	    Id: ID                                        (int)     (Required)
	    Name: \"Skill Name\"                          (string)  (Required)
	    MaxLevel: Skill Level                         (int)     (Required)
	    ------------------------------ Optional Fields -----------------------------
	    Description: \"Skill Description\"            (string)  (optional but recommended)
	    Range: Skill Range                            (int) (optional, defaults to 0) (can be grouped by Levels)
	    Note: Range < 5 is considered Melee range.
	    Hit: Hit Type                                 (int) (optional, default \"BDT_NORMAL\")
	                                                  Types - \"BDT_SKILL\", \"BDT_MULTIHIT\" or \"BDT_NORMAL\" ]
	    SkillType: {                                  (bool, defaults to \"Passive\")
	        Passive: true/false                       (boolean, defaults to false)
	        Enemy: true/false                         (boolean, defaults to false)
	        Place: true/false                         (boolean, defaults to false)
	        Self: true/false                          (boolean, defaults to false)
	        Friend: true/false                        (boolean, defaults to false)
	        Trap: true/false                          (boolean, defaults to false)
		}
	    SkillInfo: {                                  (bool, defaults to \"None\")
	        Quest: true/false                         (boolean, defaults to false)
	        NPC: true/false                           (boolean, defaults to false)
	        Wedding: true/false                       (boolean, defaults to false)
	        Spirit: true/false                        (boolean, defaults to false)
	        Guild: true/false                         (boolean, defaults to false)
	        Song: true/false                          (boolean, defaults to false)
	        Ensemble: true/false                      (boolean, defaults to false)
	        Trap: true/false                          (boolean, defaults to false)
	        TargetSelf: true/false                    (boolean, defaults to false)
	        NoCastSelf: true/false                    (boolean, defaults to false)
	        PartyOnly: true/false                     (boolean, defaults to false)
	        GuildOnly: true/false                     (boolean, defaults to false)
	        NoEnemy: true/false                       (boolean, defaults to false)
	        IgnoreLandProtector: true/false           (boolean, defaults to false)
	        Chorus: true/false                        (boolean, defaults to false)
		}
	    AttackType: \"Attack Type\"                   (string, defaults to \"None\")
	                                                  Types: \"None\", \"Weapon\", \"Magic\" or \"Misc\"
	    Element: \"Element Type\"                     (string) (Optional field - Default \"Ele_Neutral\")
	                                                  (can be grouped by Levels)
	                                                  Types: \"Ele_Neutral\", \"Ele_Water\", \"Ele_Earth\", \"Ele_Fire\", \"Ele_Wind\"
	                                                  \"Ele_Poison\", \"Ele_Holy\", \"Ele_Dark\", \"Ele_Ghost\", \"Ele_Undead\"
	                                                  \"Ele_Weapon\" - Uses weapon's element.
	                                                  \"Ele_Endowed\" - Uses Endowed element.
	                                                  \"Ele_Random\" - Uses random element.
	    DamageType: {                                 (bool, default to \"NoDamage\")
	        NoDamage: true/false                       No damage skill
	        SplashArea: true/false                     Has splash area (requires source modification)
	        SplitDamage: true/false                    Damage should be split among targets (requires 'SplashArea' in order to work)
	        IgnoreCards: true/false                    Skill ignores caster's % damage cards (misc type always ignores)
	        IgnoreElement: true/false                  Skill ignores elemental adjustments
	        IgnoreDefense: true/false                  Skill ignores target's defense (misc type always ignores)
	        IgnoreFlee: true/false                     Skill ignores target's flee (magic type always ignores)
	        IgnoreDefCards: true/false                 Skill ignores target's def cards
		}
	    SplashRange: Damage Splash Area               (int, defaults to 0) (can be grouped by Levels)
	    Note: -1 for screen-wide.
	    NumberOfHits: Number of Hits                  (int, defaults to 1) (can be grouped by Levels)
	                                                  Note: when positive, damage is increased by hits,
	                                                  negative values just show number of hits without
	                                                  increasing total damage.
	    InterruptCast: Cast Interruption              (bool, defaults to true)
	    CastDefRate: Cast Defense Reduction           (int, defaults to 0)
	    SkillInstances: Skill instances               (int, defaults to 0) (can be grouped by Levels)
	                                                  Notes: max amount of skill instances to place on the ground when
	                                                  player_land_skill_limit/monster_land_skill_limit is enabled. For skills
	                                                  that attack using a path, this is the path length to be used.
	    KnockBackTiles: Knock-back by 'n' Tiles       (int, defaults to 0) (can be grouped by Levels)
	    CastTime: Skill cast Time (in ms)             (int, defaults to 0) (can be grouped by Levels)
	    AfterCastActDelay: Skill Delay (in ms)        (int, defaults to 0) (can be grouped by Levels)
	    AfterCastWalkDelay: Walk Delay (in ms)        (int, defaults to 0) (can be grouped by Levels)
	    SkillData1: Skill Data/Duration (in ms)       (int, defaults to 0) (can be grouped by Levels)
	    SkillData2: Skill Data/Duration (in ms)       (int, defaults to 0) (can be grouped by Levels)
	    CoolDown: Skill Cooldown (in ms)              (int, defaults to 0) (can be grouped by Levels)
	    ".($re?
	    "FixedCastTime: Fixed Cast Time (in ms)       (int, defaults to 0) (can be grouped by Levels)
	                                                   Note: when 0, uses 20% of cast time and less than
	                                                   0 means no fixed cast time.":"")."
	    CastTimeOptions: {
	        IgnoreDex: true/false                     (boolean, defaults to false)
	        IgnoreStatusEffect: true/false            (boolean, defaults to false)
	        IgnoreItemBonus: true/false               (boolean, defaults to false)
	                                                   Note: Delay setting 'IgnoreDex' only makes sense when
	                                                   delay_dependon_dex is enabled.
		}
	    SkillDelayOptions: {
	        IgnoreDex: true/false                     (boolean, defaults to false)
	        IgnoreStatusEffect: true/false            (boolean, defaults to false)
	        IgnoreItemBonus: true/false               (boolean, defaults to false)
	                                                  Note: Delay setting 'IgnoreDex' only makes sense when
	                                                  delay_dependon_dex is enabled.
		}
	    Requirements: {
	        HPCost: HP Cost                           (int, defaults to 0) (can be grouped by Levels)
	        SPCost: SP Cost                           (int, defaults to 0) (can be grouped by Levels)
	        HPRateCost: HP % Cost                     (int, defaults to 0) (can be grouped by Levels)
	                                                  Note: If positive, it is a percent of your current hp,
	                                                  otherwise it is a percent of your max hp.
	        SPRateCost: SP % Cost                     (int, defaults to 0) (can be grouped by Levels)
	                                                  Note: If positive, it is a percent of your current sp,
	                                                  otherwise it is a percent of your max sp.
	        ZenyCost: Zeny Cost                       (int, defaults to 0) (can be grouped by Levels)
	        WeaponTypes: {                            (bool or string, defaults to \"All\")
	            NoWeapon: true/false                  (boolean, defaults to false)
	            Daggers: true/false                   (boolean, defaults to false)
	            1HSwords: true/false                  (boolean, defaults to false)
	            2HSwords: true/false                  (boolean, defaults to false)
	            1HSpears: true/false                  (boolean, defaults to false)
	            2HSpears: true/false                  (boolean, defaults to false)
	            1HAxes: true/false                    (boolean, defaults to false)
	            2HAxes: true/false                    (boolean, defaults to false)
	            Maces: true/false                     (boolean, defaults to false)
	            2HMaces: true/false                   (boolean, defaults to false)
	            Staves: true/false                    (boolean, defaults to false)
	            Bows: true/false                      (boolean, defaults to false)
	            Knuckles: true/false                  (boolean, defaults to false)
	            Instruments: true/false               (boolean, defaults to false)
	            Whips: true/false                     (boolean, defaults to false)
	            Books: true/false                     (boolean, defaults to false)
	            Katars: true/false                    (boolean, defaults to false)
	            Revolvers: true/false                 (boolean, defaults to false)
	            Rifles: true/false                    (boolean, defaults to false)
	            GatlingGuns: true/false               (boolean, defaults to false)
	            Shotguns: true/false                  (boolean, defaults to false)
	            GrenadeLaunchers: true/false          (boolean, defaults to false)
	            FuumaShurikens: true/false            (boolean, defaults to false)
	            2HStaves: true/false                  (boolean, defaults to false)
	            MaxSingleWeaponType: true/false       (boolean, defaults to false)
	            DWDaggers: true/false                 (boolean, defaults to false)
	            DWSwords: true/false                  (boolean, defaults to false)
	            DWAxes: true/false                    (boolean, defaults to false)
	            DWDaggerSword: true/false             (boolean, defaults to false)
	            DWDaggerAxe: true/false               (boolean, defaults to false)
	            DWSwordAxe: true/false                (boolean, defaults to false)
			}
	        AmmoTypes: {                              (for all types use string \"All\")
	            A_ARROW: true/false                   (boolean, defaults to false)
	            A_DAGGER: true/false                  (boolean, defaults to false)
	            A_BULLET: true/false                  (boolean, defaults to false)
	            A_SHELL: true/false                   (boolean, defaults to false)
	            A_GRENADE: true/false                 (boolean, defaults to false)
	            A_SHURIKEN: true/false                (boolean, defaults to false)
	            A_KUNAI: true/false                   (boolean, defaults to false)
	            A_CANNONBALL: true/false              (boolean, defaults to false)
	            A_THROWWEAPON: true/false             (boolean, defaults to false)
			}
	        AmmoAmount: Ammunition Amount             (int, defaults to 0) (can be grouped by Levels)
	        State: \"Required State\"                 (string, defaults to \"None\") (can be grouped by Levels)
	                                                  Types : 'None' = Nothing special
	                                                  'Moveable' = Requires to be able to move
	                                                  'NotOverWeight' = Requires to be less than 50% weight
	                                                  'InWater' = Requires to be standing on a water cell
	                                                  'Cart' = Requires a Pushcart
	                                                  'Riding' = Requires to ride either a peco or a dragon
	                                                  'Falcon' = Requires a Falcon
	                                                  'Sight' = Requires Sight skill activated
	                                                  'Hiding' = Requires Hiding skill activated
	                                                  'Cloaking' = Requires Cloaking skill activated
	                                                  'ExplosionSpirits' = Requires Fury skill activated
	                                                  'CartBoost' = Requires a Pushcart and Cart Boost skill activated
	                                                  'Shield' = Requires a 0,shield equipped
	                                                  'Warg' = Requires a Warg
	                                                  'Dragon' = Requires to ride a Dragon
	                                                  'RidingWarg' = Requires to ride a Warg
	                                                  'Mado' = Requires to have an active mado
	                                                  'PoisonWeapon' = Requires to be under Poisoning Weapon.
	                                                  'RollingCutter' = Requires at least one Rotation Counter from Rolling Cutter.
	                                                  'ElementalSpirit' = Requires to have an Elemental Spirit summoned.
	                                                  'MH_Fighting' = Requires Eleanor fighthing mode
	                                                  'MH_Grappling' = Requires Eleanor grappling mode
	                                                  'Peco' = Requires riding a peco
	        SpiritSphereCost: Spirit Sphere Cost      (int, defaults to 0) (can be grouped by Levels)
	        Items: {
	            ItemID or Aegis_Name : Amount         (int, defaults to 0) (can be grouped by Levels)
	                                                  Item example: \"ID717\" or \"Blue_Gemstone\".
	                                                  Notes: Items with amount 0 will not be consumed.
	                                                  Amount can also be grouped by levels.
			}
		}
	    Unit: {
	       Id: [ UnitID, UnitID2 ]                   (int, defaults to 0) (can be grouped by Levels)
	        Layout: Unit Layout                       (int, defaults to 0) (can be grouped by Levels)
	        Range: Unit Range                         (int, defaults to 0) (can be grouped by Levels)
	        Interval: Unit Interval                   (int, defaults to 0) (can be grouped by Levels)
	        Target: \"Unit Target\"                   (string, defaults to \"None\")
	                                                  Types:
	                                                  All             - affects everyone
	                                                  NotEnemy        - affects anyone who isn't an enemy
	                                                  Friend          - affects party, guildmates and neutral players
	                                                  Party           - affects party only
	                                                  Guild           - affects guild only
	                                                  Ally            - affects party and guildmates only
	                                                  Sameguild       - affects guild but not allies
	                                                  Enemy           - affects enemies only
	                                                  None            - affects nobody
	        Flag: {
	            UF_DEFNOTENEMY: true/false            (boolean, defaults to false)
	            UF_NOREITERATION: true/false          (boolean, defaults to false)
	            UF_NOFOOTSET: true/false              (boolean, defaults to false)
	            UF_NOOVERLAP: true/false              (boolean, defaults to false)
	            UF_PATHCHECK: true/false              (boolean, defaults to false)
	            UF_NOPC: true/false                   (boolean, defaults to false)
	            UF_NOMOB: true/false                  (boolean, defaults to false)
	            UF_SKILL: true/false                  (boolean, defaults to false)
	            UF_DANCE: true/false                  (boolean, defaults to false)
	            UF_ENSEMBLE: true/false               (boolean, defaults to false)
	            UF_SONG: true/false                   (boolean, defaults to false)
	            UF_DUALMODE: true/false               (boolean, defaults to false)
	            UF_RANGEDSINGLEUNI: true/false        (boolean, defaults to false)
			}
		}
	}
	* This file has been generated by Smokexyz's skilldbconverter.php tool.
	* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */\n\n";
}
