<?php
/*****************************************************************
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2017  Hercules Dev Team
 * Copyright (C) Smokexyz
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
 *****************************************************************
 *               Achievement Database Generator [Smokexyz]
 *****************************************************************
 * About:
 *       This PHP CLI "stored procedure" generates an achievement_db.conf
 * by reading the following files -
 * 1) 'System/achievement_list.lub' from the client.
 * 2) db/<pre-re/re>/item_db.conf from the Hercules db folders.
 * 3) db/<pre-re/re>/mob_db.conf from the Hercules db folders.
 *
 * The Purpose of this auto-generator is to -
 * Ease the process of searching for items and monsters, whilst
 * encouraging the use of globally consistent names for both.
 * Easily differ between pre-renewal and renewal achievements.
 * Avoid editing of an additional file everytime an entry in one 
 * of the files change.
 * 
 * How it works -
 * This program was designed to parse all entries of the achievement_list.lub.
 * From the content of each achievement it will automatically determine 
 * the criteria, objective, points and rewards for it's server-side equivalent
 * configuration.
 *****************************************************************/

// Credits before anything else.
print_credits();

if(!isset($argv[1]))
	print_help();

/* Get the arguments */
function issetarg($arg) {
	global $argv;
	for ($i=1; $i<sizeof($argv); $i++) {
		if (strncmp($argv[$i],$arg,strlen($arg)) == 0)
		return $i;
	}
	return 0;
}

/* Check the arguments */
$renewal =  (issetarg("-re") || issetarg("--renewal"));
$prere = (issetarg("-pre-re") || issetarg("--pre-renewal"));
$debug = (issetarg("-dbg") || issetarg("--with-debug"));
$help = (issetarg("-h") || issetarg("--help"));

/* Check for the directory argument */
$directory = function() use($argv) {
	$arg = issetarg("--directory")?issetarg("--directory"):(issetarg("-dir")?issetarg("-dir"):0);
	if ($arg) {
		$part = explode("=", $argv[$arg]);
		if (!isset($part[1])) {
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
	print "Read/Write directory has been set to '".$dir."'\n";
	print "Please ensure item_db and mob_db configuration files are placed in this directory.\n";
	define('DIRPATH', $dir);
}

if ($debug) {
	print "\033[0mDebug Mode Enabled.\n";
	define("DEBUG", true);
}

if ($help || (!$renewal && !$prere) || ($renewal && $prere))
	print_help();

if ($renewal) {
	print "Renewal enabled.\n";
	print "Achievements will be generated for Renewal Mode.\n";
	if (!defined('DIRPATH'))
		define('DIRPATH', '../../db/re/');
	define('RENEWAL', true);
} else if ($prere) {
	print "Pre-Renewal enabled.\n";
	print "Achievements will be generated for Pre-Renewal Mode.\n";
	if (!defined('DIRPATH'))
		define('DIRPATH', '../../db/pre-re/');
}

$file_check = [
	DIRPATH."item_db.conf",
	DIRPATH."mob_db.conf"
];

foreach($file_check as $file) {
	if (file_exists($file))
		print $file." - Found\n";
	else
		die ($file." - Not Found!\n");
}

/* Begin the magic */
$data = [];
// Get the achievements from lub.
lub_to_array($data);

// Write the data.
write_to_file($data);

/**
 * Reads the achievement_list.lub file.
 */
function lub_to_array(&$data) 
{
	global $DIRPATH;
	$file = fopen($DIRPATH."achievement_list.lub", "r");

	// Run-time vars.
	$parent = [];

	while (!feof($file)) {
		$line = fgets($file);
		
		if (strstr($line, "main()"))
			break;
		else if (strstr($line, "achievement_tbl"))
			continue;
		
		$parts = explode("=", $line);
		
		$key = make_id($parts[0]);
		
		if (isset($parts[1])) {
			if (strstr($parts[1], "{}") == true) {
				continue;
			} else if (strstr($parts[1], "{") == true) {
				array_push($parent, $key);
				continue;
			}
		} else if (strstr($key, "}") == true) {
			array_pop($parent);
			continue;
		}
		
		// Value Checking
		if (isset($parts[1])) // check if it's a real value
			$value = make_value($parts[1]);
		else
			continue;
			
		if (count($parent)) {
			$arr = &$data;
			foreach($parent as $p){
				$arr = &$arr[$p];
			}
			add_pair($arr, $key, $value);
		} else {
			$data[$key] = $value;
		}
	}
	
	fclose($file);
}

function add_pair(&$arr, $key, $value)
{
	$arr[$key] = $value;
}

function make_id($str)
{
	return trim(str_replace(["[", "]"], "", $str));
}

function make_value($str)
{
	return trim(str_replace([",",'"'],"",$str));
}

function get_item_db(&$itemdb)
{
	$file = DIRPATH."item_db.conf";
	if(file_exists($file)) {
		$itemconf = fopen($file, "r") or die ("Unable to open '".$file."'.\n");
		print "Reading '".$file."' ...\n";

		$started = false;
		$i=0;
		while(!feof($itemconf)) {
			$line = fgets($itemconf);
			$line = trim($line);
			if(!strcmp($line,"{"))
				$started = true;
			else if (!strcmp($line,"},"))
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
		fclose($itemconf);
	} else {
		print "Unable to open '".$file."'... defaulting to using Item ID's instead of Constants.\n";
	}
}

function get_mob_db(&$mobdb)
{
	$file = DIRPATH."mob_db.conf";
	
	if (file_exists($file)) {
		$mobconf = fopen($file, "r") or die ("Unable to open '".$file."'.\n");
		print "Reading '".$file."'...\n";
		
		$entry = false;
		$i = 0 ;
		$avoid = false;
		
		while (!feof($mobconf)) {
			$line = fgets($mobconf);
			$line = trim($line);
			
			if (!strcmp($line, "{"))
				$entry = true;
			else if (!strcmp($line,"},"))
				$entry = false;
			
			if (preg_match("/\/\*.*\*\//", $line))
				$avoid = false;
			else if (preg_match("/^\/\*\{$/", $line))
				$avoid = true;
			else if (preg_match("/\}\,\*\//", $line))
				$avoid = false;
			
			if ($avoid == true)
			continue;
			if ($entry == true) {
				$p = explode(":", $line);
				if (isset($p[0])) {
					if (!strcmp($p[0], "Id")) {
						$mobdb['ID'][$i] = intval($p[1]);
					}
					if (!strcmp($p[0], "SpriteName")) {
						$mobdb['SpriteName'][$i] = trim(str_replace("\"","",$p[1]));
					}
					if (!strcmp($p[0], "Name")) {
						$mobdb['name'][$i] = trim(str_replace("\"","",$p[1]));
						$i++;
					}
				}
			}
		}
		fclose($mobconf);
	} else {
		print "Unable to open '".$file."'... defaulting to using Item ID's instead of Constants.\n";
	}
}

function write_to_file($lub)
{
	$itemdb = [];
	$mobdb = [];

	global $DIRPATH, $DEBUG;
	
	// Get Mob DB | Item DB
	get_item_db($itemdb);
	get_mob_db($mobdb);
	
	// Shuffle entry buffer.
	// Required for entries that are read
	// before entries of their criteria.
	// Eg. AchievementID 230100
	$delayed_entries = [
		230100,
		230110,
		230120,
		230140,
		230200
	];
	
	// Commented Achievements
	$disabled_entries = [];
	$unknown_mobs = [];
	$unknown_items = [];
	
	$count = 0;
	
	$file = print_header();
	
	$file .= "achievement_db: (\n";
	
	$file .= print_entry_structure();
	
	foreach($lub as $id => $achievement) {
		$entry = ""; // entry buffer
		$disabled_flag = false;
		
		if (array_search($id, $delayed_entries) !== false)
			continue;
		
		// Pre-Determination
		// Objectives
		$objectives = get_objectives($id, $achievement, $mobdb, $unknown_mobs, $disabled_flag, $disabled_entries);
		// Rewards
		$rewards = get_rewards($id, $achievement, $itemdb, $unknown_items, $disabled_flag);
		
		// Entry Formation
		$entry .= ($disabled_flag?"/*":"")."{\n";
		$entry .= "\tId: {$id}\n";
		$entry .= "\tName: \"{$achievement['title']}\"\n";
		// Types/Group Names
		$entry .= "\tType: \"".get_group_name($id, $achievement)."\"\n";
		// Objectives
		$entry .= $objectives;
		// Rewards
		$entry .= $rewards;
		
		$entry .= "\tPoints: {$achievement['score']}\n";
		$entry .= "},".($disabled_flag?"*/":"")."\n";
		
		// Check for delayed entries
		$file .= $entry;
		
		if ($disabled_flag)
			array_push($disabled_entries, $id);
		
		$count++;
	}
	
	$file .= get_delayed_entries($delayed_entries, $lub, $count, $unknown_mobs, $disabled_entries, $mobdb, $itemdb);
	
	$file .= ")\n";
	
	$file .= print_unknown_mobs($unknown_mobs);
	$file .= print_unknown_items($unknown_items);
	
	$output_file = DIRPATH."achievement_db.conf";
	print "A total of {$count} achievements were written.\n";
	print count($disabled_entries)." Achievements are disabled.\n";
	print count($unknown_mobs)." Unknown monster entries were found.\n";
	print count($unknown_items)." Unknown item entries were found.\n";
	
	print "File created: '{$output_file}'\n";
	file_put_contents($output_file, $file);
}

function print_header()
{
	$header = "/*****************************************************************\n"
	." * This file is part of Hercules.\n"
	." * http://herc.ws - http://github.com/HerculesWS/Hercules\n"
	." *\n"
	." * Copyright (C) 2017  Hercules Dev Team\n"
	." * Copyright (C) Smokexyz\n"
	." *\n"
	." * Hercules is free software: you can redistribute it and/or modify\n"
	." * it under the terms of the GNU General Public License as published by\n"
	." * the Free Software Foundation, either version 3 of the License, or\n"
	." * (at your option) any later version.\n"
	." *\n"
	." * This program is distributed in the hope that it will be useful,\n"
	." * but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
	." * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
	." * GNU General Public License for more details.\n"
	." *\n"
	." * You should have received a copy of the GNU General Public License\n"
	." * along with this program.  If not, see <http://www.gnu.org/licenses/>.\n"
	." *****************************************************************\n"
	." *                  Achievement Database\n"
	." *****************************************************************/\n\n";
	
	return $header;
}

function print_entry_structure()
{
	$entry_structure = "/*****************************************************************\n"
	." *                   Entry Structure\n"
	." *****************************************************************\n"
	."{\n"
	."\tId: (int)                           Unique ID of the achievement representing it's client-side equivalent.\n"
	."\tName: (string)                      Name/Title of the Achievement.\n"
	."\tType: (string)                      Validation type for the achievement.\n"
	."\t                                     [Type]                             [Validation Description] (Trigger)\n"
	."\t                                    ACH_QUEST:                          Specific achievement objective update (Script).\n"
	."\t                                    ACH_KILL_PC_TOTAL:                  (Accumulative) Total kill count. (Player kill)\n"
	."\t                                    ACH_KILL_PC_JOB:                    Kill a player of the specified job. (Player Kill)\n"
	."\t                                    ACH_KILL_PC_JOBTYPE:                Kill a player of the specified job type. (Player Kill)\n"
	."\t                                    ACH_KILL_MOB_CLASS:                 Kill a particular mob class. (Mob Kill)\n"
	."\t                                    ACH_DAMAGE_PC_MAX:                  Maximum damage caused on a player. (Player Damage)\n"
	."\t                                    ACH_DAMAGE_PC_TOTAL:                (Accumulative) Damage on players. (Player Damage)\n"
	."\t                                    ACH_DAMAGE_PC_REC_MAX:              Maximum damage received by a player. (Receive Player Damage)\n"
	."\t                                    ACH_DAMAGE_PC_REC_TOTAL:            (Accumulative) Damage received by players. (Receive Player Damage)\n"
	."\t                                    ACH_DAMAGE_MOB_MAX:                 Maximum damage caused on a monster. (Monster Damage)\n"
	."\t                                    ACH_DAMAGE_MOB_TOTAL:               (Accumulative) Damage caused on monsters. (Monster Damage)\n"
	."\t                                    ACH_DAMAGE_MOB_REC_MAX:             Maximum damage received by a monster. (Receive Monster Damage)\n"
	."\t                                    ACH_DAMAGE_MOB_REC_TOTAL:           (Accumulative) Damage received by monsters. (Receive Monster Damage)\n"
	."\t                                    ACH_JOB_CHANGE:                     Change to a specific job. (Job Change)\n"
	."\t                                    ACH_STATUS:                         Acquire a specific amount of a particular status type. (Stat Change)\n"
	."\t                                    ACH_STATUS_BY_JOB:                  Acquire a specific amount of a status type as a job class. (Stat Change)\n"
	."\t                                    ACH_STATUS_BY_JOBTYPE:              Acquire a specific amount of a status type as a job type. (Stat Change)\n"
	."\t                                    ACH_CHATROOM_CREATE_DEAD:           (Accumulative) Create a chatroom when dead. (Chatroom Creation)\n"
	."\t                                    ACH_CHATROOM_CREATE:                (Accumulative) Create a chatroom. (Chatroom Creation)\n"
	."\t                                    ACH_CHATROOM_MEMBERS:               Gather 'n' members in a chatroom. (Chatroom Join)\n"
	."\t                                    ACH_FRIEND_ADD:                     Add a specific number of friends. (Friend Addition)\n"
	."\t                                    ACH_PARTY_CREATE:                   (Accumulative) Create a specific number of parties. (Party Creation)\n"
	."\t                                    ACH_PARTY_JOIN:                     (Accumulative) Join a specific number of parties. (Party Join)\n"
	."\t                                    ACH_MARRY:                          (Accumulative) Marry a specified number of times. (Marriage)\n"
	."\t                                    ACH_ADOPT_BABY:                     (Accumulative) Get Adopted. (Adoption) \n"
	."\t                                    ACH_ADOPT_PARENT:                   (Accumulative) Adopt a Baby. (Adoption)\n"
	."\t                                    ACH_ZENY_HOLD:                      Hold a specific amount of zeny in your inventory. (Gain Zeny)\n"
	."\t                                    ACH_ZENY_GET_ONCE:                  Gain a specific amount of zeny in one transaction. (Gain Zeny)\n"
	."\t                                    ACH_ZENY_GET_TOTAL:                 (Accumulative) Gain a specific amount of zeny in total. (Gain Zeny)\n"
	."\t                                    ACH_ZENY_SPEND_ONCE:                Spend a specific amount of zeny in one transaction. (Pay Zeny)\n"
	."\t                                    ACH_ZENY_SPEND_TOTAL:               (Accumulative) Spend a specific amount of zeny in total. (Pay Zeny)\n"
	."\t                                    ACH_EQUIP_REFINE_SUCCESS:           Refine an item to +N. (Successful Refine)\n"
	."\t                                    ACH_EQUIP_REFINE_FAILURE:           Fail to refine an item of +N refine. (Failed Refine)\n"
	."\t                                    ACH_EQUIP_REFINE_SUCCESS_TOTAL:     (Accumulative) Refine an item successfully N times. (Success Refine)\n"
	."\t                                    ACH_EQUIP_REFINE_FAILURE_TOTAL:     (Accumulative) Fail to refine an item N times. (Failed Refine)\n"
	."\t                                    ACH_EQUIP_REFINE_SUCCESS_WLV:       Refine a Weapon of a particular Level to +N. (Success Refine)\n"
	."\t                                    ACH_EQUIP_REFINE_FAILURE_WLV:       Fail to refine a Weapon of a particular level from +N. (Failed Refine)\n"
	."\t                                    ACH_EQUIP_REFINE_SUCCESS_ID:        Refine a particular Item successfully to +N. (Success Refine)\n"
	."\t                                    ACH_EQUIP_REFINE_FAILURE_ID:        Fail to refine a particular item successfully from +N. (Failed Refine)\n"
	."\t                                    ACH_ITEM_GET_COUNT:                 Acquire N amount of an item of a particular ID. (Acquire Item)\n"
	."\t                                    ACH_ITEM_GET_COUNT_ITEMTYPE:        Acquire N amount of items of a particular type mask. (Acquire Item)\n"
	."\t                                    ACH_ITEM_GET_WORTH:                 Acquire an item of buy value N. (Acquire Item)\n"
	."\t                                    ACH_ITEM_SELL_WORTH:                Sell an item of sell value N. (NPC Sell Item)\n"
	."\t                                    ACH_PET_CREATE:                     Successfully tame a pet of a particular mob class. (Successful Pet Tame)\n"
	."\t                                    ACH_ACHIEVE:                        Achieve an Achievement. (Achievement Completion)\n"
	."\t                                    ACH_ACHIEVEMENT_RANK:               Achieve an Achievement Rank. (Achievement Rank Increase)\n"                     
	."\tObjectives: {                       [Mandatory Field] Objectives of an achievement. Up to 10 objectives per achievement.\n"
	."\t                                    To comply with the client's order of objectives, this list must be in order.\n"
	."\t    *1: {\n"                        
	."\t        Description: (string)       [Mandatory Field] Description of a particular objective.\n"
	."\t        Criteria: {                 This is a field for achievements whose objectives must meet\n"
	."\t                                    certain criteria before evaluating the player's progress for it.\n"
	."\t            Monster: (mixed)        Monster ID required for an objective.\n"
	."\t                                    For types such as ACH_KILL_MOB_CLASS and ACH_PET_CREATE. Can be either int or string constant.\n"
	."\t            JobId: (mixed)          Array or Single entry of JobIds.\n"
	."\t                                    For types - ACH_KILL_PC_JOBTYPE, ACH_JOB_CHANGE or ACH_STATUS_BY_JOBTYPE.\n"
	."\t                                    Can be either a numeric or string constant.\n"
	."\t            Item: (mixed)           Item ID required for an objective.\n"
	."\t                                    For Types such as ACH_ITEM_GET_COUNT. Can be either int or string constant.\n"
	."\t            StatusType: (mixed)     Status Type required for an objective.\n"
	."\t                                    For Types such as ACH_STATUS, ACH_STATUS_BY_JOB, ACH_STATUS_BY_JOBTYPE.\n"
	."\t                                    Types -\n"
	."\t                                    \"SP_STR\"        - Strength\n"
	."\t                                    \"SP_AGI\"        - Agility\n"
	."\t                                    \"SP_VIT\"        - Vitality\n"
	."\t                                    \"SP_INT\"        - Intelligence\n"
	."\t                                    \"SP_DEX\"        - Dexterity\n"
	."\t                                    \"SP_LUK\"        - Luck\n"
	."\t                                    \"SP_BASELEVEL\"  - Base Level\n"
	."\t                                    \"SP_JOBLEVEL\"   - Job Level\n"
	."\t                                    Can be either int or string constant.\n"
	."\t            ItemType: (mixed)       Item type that is required for this achievement.\n"
	."\t                                    For Types such as ACH_ITEM_GET_COUNT_ITEMTYPE.\n"
	."\t                                    \"OBJ_IT_HEALING\"        = 0x0001\n"
	."\t                                    \"OBJ_IT_UNKNOWN\"        = 0x0002\n"
	."\t                                    \"OBJ_IT_USABLE\"         = 0x0004\n"
	."\t                                    \"OBJ_IT_ETC\"            = 0x0008\n"
	."\t                                    \"OBJ_IT_WEAPON\"         = 0x0010\n"
	."\t                                    \"OBJ_IT_ARMOR\"          = 0x0020\n"
	."\t                                    \"OBJ_IT_CARD\"           = 0x0040\n"
	."\t                                    \"OBJ_IT_PETEGG\"         = 0x0080\n"
	."\t                                    \"OBJ_IT_PETARMOR\"       = 0x0100\n"
	."\t                                    \"OBJ_IT_UNKNOWN2\"       = 0x0200\n"
	."\t                                    \"OBJ_IT_AMMO\"           = 0x0400\n"
	."\t                                    \"OBJ_IT_DELAYCONSUME\"   = 0x0800\n"
	."\t                                    \"OBJ_IT_CASH\"           = 0x1000\n" 
	."\t                                    \"OBJ_IT_ALL\"            = 0xffff\n"
	."\t                                    Can be either int of string constant.\n"
	."\t            WeaponLevel: (int)      Weapon Level that is required for this achievement. (Eg. 0, 1, 2, 3 or 4).\n"
	."\t                                    For Types such as ACH_EQUIP_REFINE_SUCCESS_WLV and ACH_EQUIP_REFINE_FAILURE_WLV.\n"
	."\t            Achieve: (int)          AchievementID to be achieved.\n"
	."\t                                    For Type - ACH_ACHIEVE.\n"
	."\t        }\n"
	."\t        Goal: (int)                 Target amount to be met for the completion of the objective. Default is 1.\n"
	."\t    }\n"
	."\t    ...\n"
	."\t    *10: {...}\n"
	."\t}\n"
	."\tRewards: {\n"
	."\t    Bonus: <\"\"> (script)            Script code bonus to be given as a reward for an achievement.\n"
	."\t    Items: {                        Item rewards per achievement. With a maximum defined in mmo.h as MAX_ACHEIVEMENT_ITEM_REWARDS.\n"
	."\t        Apple: 1 (int)              Item ID (int or string constant) : Amount (int)\n"
	."\t    }\n"                            
	."\t    TitleId: (int)                  ID of the Title (from the Title System) awarded.\n"
	."\t}\n"                                
	."\tPoints: (int)                       Points per achievement given on it's successful completion.\n"
	."}\n"
	." *****************************************************************/\n";
	return $entry_structure;
}

function print_unknown_mobs($unknown_mobs)
{
	$count = count($unknown_mobs);
	$unknown = "/*****************************************************************\n"
	." *      A total of {$count} Unknown Monsters were found.\n"
	." *****************************************************************\n";
	foreach($unknown_mobs as $mob)
		$unknown .= "* Achievement: {$mob['AchievementID']} -> Mob: {$mob['Name']}\n";
		
	$unknown .= "*/\n";
	return $unknown;
}

function print_unknown_items($unknown_items)
{
	$count = count($unknown_items);
	$unknown = "/*****************************************************************\n"
	." *      A total of {$count} Unknown Items were found.\n"
	." *****************************************************************\n";
	foreach($unknown_items as $item)
		$unknown .= "* Achievement: {$item['AchievementID']} -> Item: {$item['ItemID']}\n";
		
	$unknown .= "*/\n";
		
	return $unknown;
}

function get_res_type($ach)
{
	foreach($ach['resource'] as $res) {
		foreach ($res as $col => $entry) {
			if (!strcmp($col, "count")) 
				return 1;
			else if (!strcmp($col, "shortcut")) 
				return 2;
		}
	}
}

function get_group_name($id, $ach)
{
	$group = $ach['group'];
	$desc = $ach['content']['summary'];
	
	// Group Names
	if (!strcmp($group, "EAT") 
		|| !strcmp($group, "SEE") 
		|| !strcmp($group, "CHATTING")
		|| !strcmp($group, "ADVENTURE")
		|| !strcmp($group, "HEAR")) 
	{
		if (preg_match("/Create a chatroom/i", $desc))
			$g = "ACH_CHATROOM_CREATE";
		else
			$g = "ACH_QUEST";
	}
	else if (!strcmp($group, "BATTLE"))
		$g = "ACH_KILL_MOB_CLASS";
	else if (!strcmp($group, "JOB_CHANGE")) {
		if (preg_match("/First step of/i", $desc))
			$g = "ACH_QUEST";
		else
			$g = "ACH_JOB_CHANGE";
	} else if (!strcmp($group, "GOAL_STATUS")) {
		if (preg_match("/level as Novice/i", $desc))
			$g = "ACH_STATUS_BY_JOB";
		else if (preg_match("/level as 1st classes/i", $desc))
			$g = "ACH_STATUS_BY_JOBTYPE";
		else
			$g = "ACH_STATUS";
	} else if (!strcmp($group, "CHATTING_DYING"))
		$g = "ACH_CHATROOM_CREATE_DEAD";
	else if (!strcmp($group, "CHATTING20"))
		$g = "ACH_CHATROOM_MEMBERS";
	else if (!strcmp($group, "ADD_FRIEND"))
		$g = "ACH_FRIEND_ADD";
	else if (!strcmp($group, "PARTY"))
		$g = "ACH_PARTY_CREATE";
	else if (!strcmp($group, "MARRY"))
		$g = "ACH_MARRY";
	else if (!strcmp($group, "BABY")) {
		if (preg_match("/adopted by parents/i", $desc))
			$g = "ACH_ADOPT_PARENT";
		else if (preg_match("/adopt a children/i", $desc))
			$g = "ACH_ADOPT_BABY";
		else
			$g = "ACH_UNKNOWN";
	} else if (!strcmp($group, "SPEND_ZENY"))
		$g = "ACH_ZENY_SPEND_TOTAL";
	else if (!strcmp($group, "GET_ZENY")) {
		if (preg_match("/Hold more than/", $desc))
			$g = "ACH_ZENY_HOLD";
		else
			$g = "ACH_ZENY_GET_TOTAL";
	} else if (!strcmp($group, "ENCHANT_SUCCESS")) {
		if (preg_match("/refining/", $desc)) {
			$g = "ACH_EQUIP_REFINE_SUCCESS_WLV";
		} else {
			$g = "ACH_EQUIP_ENCHANT_SUCCESS";
		}
	} else if (!strcmp($group, "ENCHANT_FAIL")) {
		if (preg_match("/refining/", $desc))
			$g = "ACH_EQUIP_REFINE_FAILURE_TOTAL";
		else
			$g = "ACH_EQUIP_ENCHANT_FAILURE";
	} else if (!strcmp($group, "GET_ITEM")) {
		if (preg_match("/worth of more than/", $desc)) 
			$g = "ACH_ITEM_GET_WORTH";
		else 
			$g = "ACH_ITEM_GET";
	} else if (!strcmp($group, "GOAL_LEVEL")) {
		if (get_res_type($ach) == 2)
			$g = "ACH_ACHIEVE";
		else if (preg_match("/Base level/i", $desc) || preg_match("/Job Level/i", $desc))
			$g = "ACH_STATUS";
		else if (preg_match("/introduction/i", $desc))
			$g = "ACH_QUEST";
		else
			$g = "ACH_UNKNOWN";
	} 
	else if (!strcmp($group, "TAMING"))
		$g = "ACH_PET_CREATE";
	else if (!strcmp($group, "GOAL_ACHIEVE"))
		$g = "ACH_ACHIEVEMENT_RANK";
	else
		$g = "{$group}";
	
	return $g;
}
function get_rewards($id, $achievement, &$itemdb, &$unknown_items, &$disabled_flag)
{
	$reward = "";
	if (isset($achievement['reward'])) {
		$reward .= "\tRewards: {\n";
		
		foreach($achievement['reward'] as $key => $r) {
			
			if (!strcmp($key, "title"))
				$reward .= "\t\tTitleId: {$r}\n";
			if (!strcmp($key, "buff"))
				$reward .= "\t\tBonus: <\"// unknown buff #{$r}?\">\n";
			if (!strcmp($key, "item")) {
				$reward .= "\t\tItems: {\n";
					if (($key = array_search(intval($r), $itemdb['ID'], true)) === false) {
						$disabled_flag = true;
						array_push($unknown_items, [ 'AchievementID' => $id, 'ItemID' => $r ]);
						$reward .= "\t\t\tID{$r}: 1 (Non-Existent Item)\n";
					} else {
						$reward .= "\t\t\t{$itemdb['name'][$key]}: 1\n";
					}
				$reward .= "\t\t}\n";
			}
		}
		$reward .= "\t}\n";
	}
	
	return $reward;
}

function get_objectives($id, $achievement, &$mobdb, &$unknown_mobs, &$disabled_flag, &$disabled_entries)
{
	$group = get_group_name($id, $achievement);
	$obj = "";
	// Objectives
	if (isset($achievement['resource'])) {
		$obj .= "\tObjectives: {\n";
		foreach($achievement['resource'] as $i => $r) {
			$obj .= "\t\t*{$i}: {\n";
			$desc = "";
			$printed = false;
			foreach($r as $col => $entry) {
				// Description
				if (!strcmp($col,"text")) {
					$obj .= "\t\t\tDescription: \"{$entry}\"\n";
					$desc = $entry;
				}
				// Criteria
				if ( !$printed // check if his block is already printed.
					&& (!strcmp($col, "shortcut")
					|| !strcmp($group, "ACH_KILL_MOB_CLASS")
					|| !strcmp($group, "ACH_JOB_CHANGE")
					|| !strcmp($group, "ACH_ITEM_GET")
					|| !strcmp($group, "ACH_STATUS")
					|| !strcmp($group, "ACH_STATUS_BY_JOB")
					|| !strcmp($group, "ACH_STATUS_BY_JOBTYPE")
					|| !strcmp($group, "ACH_EQUIP_REFINE_SUCCESS_WLV")
					|| !strcmp($group, "ACH_EQUIP_REFINE_FAILURE_TOTAL")
					|| !strcmp($group, "ACH_PET_CREATE"))
					)
				{
					$printed = true;
					$obj .= "\t\t\tCriteria: {\n";
					if (!strcmp($col, "shortcut")) {
						$obj .= "\t\t\t\tAchieve: {$entry}\n";
						if (array_search($entry, $disabled_entries) !== false)
							$disabled_flag = true;
					}
					
					switch ($group) {
					case "ACH_PET_CREATE":
					case "ACH_KILL_MOB_CLASS":
						$mobid = "Unknown_Mob - (Non-Existent)";
						if (preg_match("/Succeed on Taming ([a-zA-Z]+.*$)/i", $desc, $matches)
							|| preg_match("/Hunt ([0-9]+) ([a-zA-Z]+.*$)/i", $desc, $matches)
							|| preg_match("/Eliminate ([0-9]+) ([a-zA-Z]+.*$)/i", $desc, $matches)
							|| preg_match("/Eliminate morocc ([0-9]+) ([a-zA-Z]+.*$)/i", $desc, $matches)) 
						{
							$match = ($group == "ACH_PET_CREATE")?$matches[1]:$matches[2];
							if (($key = array_search($match, $mobdb['name'])) != false) {
								$mobid = $mobdb['SpriteName'][$key];
							} else {
								array_push($unknown_mobs, [ 'AchievementID' => $id, 'Name' => $match]);
								$disabled_flag = true;
							}
						} else {
								$disabled_flag = true;
						}
						$obj .= "\t\t\t\tMonster: \"{$mobid}\"\n";
					break;
					case "ACH_JOB_CHANGE":
						if (preg_match("/first class/", $desc, $matches) || preg_match("/1st class/", $desc, $matches))
							$types = ["Job_Swordman", "Job_Mage", "Job_Archer", "Job_Acolyte", "Job_Merchant", "Job_Thief"];
						else if (preg_match("/to 2-1 class/", $desc, $matches))
							$types = ["Job_Knight", "Job_Priest", "Job_Wizard", "Job_Blacksmith", "Job_Hunter", "Job_Assassin"];
						else if (preg_match("/to 2-2 class/", $desc, $matches))
							$types = ["Job_Crusader", "Job_Sage", "Job_Bard", "Job_Dancer", "Job_Alchemist", "Job_Rogue"];
						else if (preg_match("/transcendent 2-1 class/", $desc, $matches))
							$types = ["Job_Lord_Knight", "Job_High_Wizard", "Job_Sniper", "Job_High_Priest", "Job_Whitesmith", "Job_Assassin_Cross"];
						else if (preg_match("/transcendent 2-2 class/", $desc, $matches))
							$types = ["Job_Paladin", "Job_Professor", "Job_Clown", "Job_Gypsy", "Job_Champion", "Job_Creator", "Job_Stalker"];
						else if (preg_match("/3-1 classes after/", $desc, $matches))
							$types = ["Job_Rune_Knight_t", "Job_Warlock_T", "Job_Ranger_T", "Job_Arch_Bishop_T", "Job_Mechanic_T", "Job_Guillotine_Cross_T"];
						else if (preg_match("/3-2 classes after/", $desc, $matches))
							$types = ["Job_Royal_Guard_T", "Job_Sorcerer_T", "Job_Minstrel_T", "Job_Wanderer_T", "Job_Sura_T", "Job_Genetic_T", "Job_Shadow_Chaser_T"];
						else if (preg_match('/3-1 class/', $desc, $matches))
							$types = ["Job_Rune_Knight", "Job_Warlock", "Job_Ranger", "Job_Arch_Bishop", "Job_Mechanic", "Job_Guillotine_Cross"];
						else if (preg_match("/3-2 class/", $desc, $matches))
							$types = ["Job_Royal_Guard", "Job_Sorcerer", "Job_Minstrel", "Job_Wanderer", "Job_Sura", "Job_Genetic", "Job_Shadow_Chaser"];
						else if (preg_match("/expanded classes/", $desc, $matches))
							$types = ["Job_Taekwon", "Job_Gunslinger", "Job_Ninja", "Job_Summoner"];
						else if (preg_match("/Expanded 2nd/", $desc, $matches))
							$types = ["Job_Star_Gladiator", "Job_Soul_Linker", "Job_Rebellion", "Job_Kagerou", "Job_Oboro"];
						else if (preg_match("/transcendent Novice/", $desc, $matches))
							$types = ["Job_Novice_High"];
						else
							$types = [];
						if (count($types) == 1) {
							$obj .= "\t\t\t\tJobId: \"{$types[0]}\"\n";
						} else if (count($types) > 1) {
							$obj .= "\t\t\t\tJobId: [";
							for ($t = 0; $t < count($types); $t++)
								$obj .= " \"{$types[$t]}\"".(($t == count($types)-1)?" ]\n":",");
						}
						break;
					case "ACH_ITEM_GET_COUNT": // Not used yet
						$obj .= "\t\t\tItem:\n";
						break;
					case "ACH_STATUS":
						if (preg_match("/Job level/i", $desc, $matches))
							$type = "SP_JOBLEVEL";
						else if (preg_match("/Base level/i", $desc, $matches))
							$type = "SP_BASELEVEL";
						else if (preg_match("/Achieve base ([A-Z]+)/i", $desc, $matches))
							$type = "SP_".$matches[1];
						else
							$type = 0;
						
						$obj .= "\t\t\t\tStatusType: \"{$type}\"\n";
						break;
					case "ACH_STATUS_BY_JOB":
						if (preg_match("/base level/i", $desc, $matches)) {
							$type = "SP_BASELEVEL";
							
							if (preg_match("/novice/i", $desc))
								$job = "Job_Novice";
						} else {
							$type = "";
							$job = 0;
							$disabled_flag = true;
						}
						$obj .= "\t\t\t\tStatusType: \"{$type}\"\n";
						$obj .= "\t\t\t\tJob: \"{$job}\"\n";
						break;
					case "ACH_STATUS_BY_JOBTYPE":
						if (preg_match("/base level/i", $desc, $matches)) {
							$type = "SP_BASELEVEL";
						} else
							$type = "";
						if (preg_match("/1st classes/", $desc, $matches)) {
							$jobs = '"Job_Swordman", "Job_Mage", "Job_Archer", "Job_Acolyte", "Job_Merchant", "Job_Thief"';
						} else {
							$jobs = "\"\"";
						}
						$obj .= "\t\t\t\tStatusType: \"{$type}\"\n";
						if (count($jobs) == 1) {
							$obj .= "\t\t\t\tJobId: \"{$jobs[0]}\"\n";
						} else if (count($jobs) > 1) {
							$obj .= "\t\t\t\tJobId: [";
							for ($t = 0; $t < count($jobs); $t++)
								$obj .= " \"{$jobs[$t]}\"".(($t == count($jobs)-1)?" ]\n":",");
						}
						break;
					case "ACH_EQUIP_REFINE_SUCCESS_WLV":
					case "ACH_EQUIP_REFINE_FAILURE_TOTAL":
						if (preg_match("/refining level ([0-9]+) weapon/", $desc, $matches)) {
							$type = '"IT_WEAPON"';
							$level = $matches[1];
						} else if (preg_match("/Experience/", $desc, $matches)) {
							$type = '["IT_WEAPON", "IT_ARMOR"]';
							$level = 0;
						} else {
							$type = "\"\"";
							$level = 0;
							$disabled_flag = true;
						}
						$obj .= "\t\t\t\tItemType: {$type}\n";
						if ($level) $obj .= "\t\t\t\tWeaponLevel: {$level}\n";
						break;
					}// /Criteria
					$obj .= "\t\t\t}\n";
				}
				
				// Objective Goals
				if (!strcmp($group, "ACH_ITEM_GET_WORTH")
					|| !strcmp($group, "ACH_ZENY_HOLD")
					|| !strcmp($group, "ACH_STATUS")
					|| !strcmp($group, "ACH_STATUS_BY_JOB")
					|| !strcmp($group, "ACH_STATUS_BY_JOBTYPE")
					|| !strcmp($group, "ACH_ACHIEVEMENT_RANK")
					|| !strcmp($group, "ACH_EQUIP_REFINE_SUCCESS_WLV"))
				{
					
					$goal = 0;
					switch($group) {
					case "ACH_ITEM_GET_WORTH":
						if (preg_match("/more than ([0-9,]+)/", $desc, $matches)) {
							$goal = str_replace(",", "", $matches[1]);
						} else {
							$goal = 0;
							$disabled_flag = true;
						}
						break;
					case "ACH_ZENY_HOLD":
						if (preg_match("/more than ([0-9,]+)/", $desc, $matches)) {
							$goal = str_replace(",", "", $matches[1]);
						} else {
							$goal = 0;
							$disabled_flag = true;
						}
						break;
					case "ACH_STATUS":
					case "ACH_STATUS_BY_JOB":
					case "ACH_STATUS_BY_JOBTYPE":
						$goal = 0;
						if (preg_match("/Job level ([0-9]+)/i", $desc, $matches))
							$goal = $matches[1];
						else if (preg_match("/Achieve ([0-9]+) base/i", $desc, $matches))
							$goal = $matches[1];
						else if (preg_match("/base level ([0-9]+)/i", $desc, $matches))
							$goal = $matches[1];
						else if (preg_match("/Achieve base ([A-Z]+) ([0-9]+)/i", $desc, $matches))
							$goal = $matches[2];
						else
							$disabled_flag = true;
						break;
					case "ACH_ACHIEVEMENT_RANK":
						if (preg_match("/Achieving rank ([0-9]+)/i", $desc, $matches))
							$goal = $matches[1];
						else
							$disabled_flag = true;
						break;
					case "ACH_EQUIP_REFINE_SUCCESS_WLV":
						if (preg_match("/to \+([0-9]+)/", $desc, $matches))
							$goal = $matches[1];
						else 
							$disabled_flag = true;
						break;
					}

					$obj .= "\t\t\tGoal: {$goal}\n";
				}
				if (!strcmp($col, "count")) {
					$obj .= "\t\t\tGoal: {$entry}\n";
				}
			} // foreach resource
			$obj .= "\t\t}\n";
		} // foreach objective
		$obj .= "\t}\n";
		
		return $obj;
	} else {
		return "";
	}
}

function get_delayed_entries(&$buffer, $lub, &$count, &$unknown_mobs, &$disabled_entries, &$mobdb, &$itemdb)
{
	$file = "";
	
	if (count($buffer)) {
		$file .= "/*****************************************************************\n"
		."                         Postlude\n"
		."      Entries with criteria that must be read before itself.\n"
		." *****************************************************************/\n";
		foreach($lub as $id => $achievement) {
			if (array_search($id, $buffer) === false)
				continue;
			
			$entry = ""; // entry buffer
			$disabled_flag = false;
			
			// Objectives
			$objectives = get_objectives($id, $achievement, $mobdb, $unknown_mobs, $disabled_flag, $disabled_entries);
			
			$entry .= ($disabled_flag?"/*":"")."{\n";
			$entry .= "\tId: {$id}\n";
			$entry .= "\tName: \"{$achievement['title']}\"\n";
			
			// Types/Group Names
			$entry .= "\tType: \"".get_group_name($id, $achievement)."\"\n";
			// Objectives
			$entry .= $objectives;
			// Rewards
			if (isset($achievement['reward'])) {
				$entry .= "\tRewards: {\n";
				foreach($achievement['reward'] as $key => $r) {
					if (!strcmp($key, "title")) $entry .= "\t\tTitleId: {$r}\n";
					if (!strcmp($key, "buff")) $entry .= "\t\tBonus: <\"// skill_id #{$r}\">\n";
					if (!strcmp($key, "item")) {
						$entry .= "\t\tItems: {\n";
						$itkey = array_search($r,$itemdb["ID"]);
						if ($itkey != false)
							$entry .= "\t\t\t{$itemdb['name'][$itkey]}: 1\n";
						else
							$entry .= "\t\t\tID{$r}: 1\n";
						$entry .= "\t\t}\n";
					}
				}
				$entry .= "\t}\n";
			} // /Rewards
			
			$entry .= "\tPoints: {$achievement['score']}\n";
			$entry .= "},".($disabled_flag?"*/":"")."\n";
			
			// Check for delayed entries
			$file .= $entry;
			
			if ($disabled_flag)
				array_push($disabled_entries, $id);
			
			$count++;
		}
	}
	
	return $file;
}

function print_credits()
{
	
	print "      _   _                     _           \n";
	print "     | | | |                   | |          \n";
	print "     | |_| | ___ _ __ ___ _   _| | ___  ___ \n";
	print "     |  _  |/ _ \ '__/ __| | | | |/ _ \/ __|\n";
	print "     | | | |  __/ | | (__| |_| | |  __/\__ \ \n";
	print "     \_| |_/\___|_|  \___|\__,_|_|\___||___/\n";
	print "Hercules Achievement Database Generator by Smokexyz\n";
	print "Copyright (C) 2017  Smokexyz\n";
	print "-----------------------------------------------\n\n";
}

function print_help()
{
	print "Usage: php achconfgen.php [options]\n";
	print "Options:\n";
	print "\t-re     [--renewal]          for renewal achievement database generation.\n";
	print "\t-pre-re [--pre-renewal]      for pre-renewal achievement database generation.\n";
	print "\t-dir    [--directory]        provide a custom directory.\n";
	print "\t                             (Must include the correct -pre-re/-re option)\n";
	print "\t-dbg    [--with-debug]       print debug information.\n";
	print "\t-h      [--help]             to display this help text.\n\n";
	print "----------------------- Additional Notes ----------------------\n";
	print "Important!\n";
	print "* Please be advised that either and only one of the arguments -re/-pre-re\n";
	print "  must be specified on execution.\n";
	print "* When using the -dir option, -re/-pre-re options must be specified. \n";
	print "* This tool should ideally be used from the 'tools/' folder, which can be found\n";
	print "  in the root of your Hercules installation. \n";
	print "* This tool will not delete any files from any of the directories that \n";
	print "  it reads from or prints to.\n\n";
	print "----------------------- Usage Example -------------------------\n";
	print "- Renewal Generation: php achconfgen.php --renewal\n";
	print "- Pre-renewal Generation: php achconfgen.php --pre-renewal\n";
	print "----------------------------------------------------------------\n";
	exit;
}