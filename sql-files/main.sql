-- This file is part of Hercules.
-- http://herc.ws - http://github.com/HerculesWS/Hercules
--
-- Copyright (C) 2012-2016  Hercules Dev Team
-- Copyright (C)  Athena Dev Teams
--
-- Hercules is free software: you can redistribute it and/or modify
-- it under the terms of the GNU General Public License as published by
-- the Free Software Foundation, either version 3 of the License, or
-- (at your option) any later version.
--
-- This program is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU General Public License for more details.
--
-- You should have received a copy of the GNU General Public License
-- along with this program.  If not, see <http://www.gnu.org/licenses/>.

--
-- Table structure for table `account_data`
--

CREATE TABLE IF NOT EXISTS `account_data` (
  `account_id` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `bank_vault` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `base_exp` SMALLINT(6) UNSIGNED NOT NULL DEFAULT '100',
  `base_drop` SMALLINT(6) UNSIGNED NOT NULL DEFAULT '100',
  `base_death` SMALLINT(6) UNSIGNED NOT NULL DEFAULT '100',
  PRIMARY KEY (`account_id`)
) ENGINE=MyISAM; 

--
-- Table structure for table `acc_reg_num_db`
--

CREATE TABLE IF NOT EXISTS `acc_reg_num_db` (
  `account_id` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `key` VARCHAR(32) BINARY NOT NULL DEFAULT '',
  `index` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `value` INT(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`account_id`,`key`,`index`),
  KEY `account_id` (`account_id`)
) ENGINE=MyISAM;

--
-- Table structure for table `acc_reg_str_db`
--

CREATE TABLE IF NOT EXISTS `acc_reg_str_db` (
  `account_id` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `key` VARCHAR(32) BINARY NOT NULL DEFAULT '',
  `index` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `value` VARCHAR(254) NOT NULL DEFAULT '0',
  PRIMARY KEY (`account_id`,`key`,`index`),
  KEY `account_id` (`account_id`)
) ENGINE=MyISAM;

--
-- Table structure for table `auction`
--

CREATE TABLE IF NOT EXISTS `auction` (
  `auction_id` BIGINT(20) UNSIGNED NOT NULL AUTO_INCREMENT,
  `seller_id` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `seller_name` VARCHAR(30) NOT NULL DEFAULT '',
  `buyer_id` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `buyer_name` VARCHAR(30) NOT NULL DEFAULT '',
  `price` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `buynow` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `hours` SMALLINT(6) NOT NULL DEFAULT '0',
  `timestamp` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `nameid` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `item_name` VARCHAR(50) NOT NULL DEFAULT '',
  `type` SMALLINT(6) NOT NULL DEFAULT '0',
  `refine` TINYINT(3) UNSIGNED NOT NULL DEFAULT '0',
  `attribute` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0',
  `card0` SMALLINT(11) NOT NULL DEFAULT '0',
  `card1` SMALLINT(11) NOT NULL DEFAULT '0',
  `card2` SMALLINT(11) NOT NULL DEFAULT '0',
  `card3` SMALLINT(11) NOT NULL DEFAULT '0',
  `unique_id` BIGINT(20) UNSIGNED NOT NULL DEFAULT '0',
  PRIMARY KEY (`auction_id`)
) ENGINE=MyISAM;

--
-- Table structure for table `autotrade_data`
--

CREATE TABLE IF NOT EXISTS `autotrade_data` (
  `char_id` INT(11) NOT NULL DEFAULT '0',
  `itemkey` INT(11) NOT NULL DEFAULT '0',
  `amount` INT(11) NOT NULL DEFAULT '0',
  `price` INT(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`char_id`,`itemkey`)
) ENGINE=MyISAM; 

--
-- Table structure for table `autotrade_merchants`
--

CREATE TABLE IF NOT EXISTS `autotrade_merchants` (
  `account_id` INT(11) NOT NULL DEFAULT '0',
  `char_id` INT(11) NOT NULL DEFAULT '0',
  `sex` TINYINT(2) NOT NULL DEFAULT '0',
  `title` VARCHAR(80) NOT NULL DEFAULT 'Buy From Me!',
  PRIMARY KEY (`account_id`,`char_id`)
) ENGINE=MyISAM; 

--
-- Table structure for table `cart_inventory`
--

CREATE TABLE IF NOT EXISTS `cart_inventory` (
  `id` INT(11) NOT NULL AUTO_INCREMENT,
  `char_id` INT(11) NOT NULL DEFAULT '0',
  `nameid` INT(11) NOT NULL DEFAULT '0',
  `amount` INT(11) NOT NULL DEFAULT '0',
  `equip` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `identify` SMALLINT(6) NOT NULL DEFAULT '0',
  `refine` TINYINT(3) UNSIGNED NOT NULL DEFAULT '0',
  `attribute` TINYINT(4) NOT NULL DEFAULT '0',
  `card0` SMALLINT(11) NOT NULL DEFAULT '0',
  `card1` SMALLINT(11) NOT NULL DEFAULT '0',
  `card2` SMALLINT(11) NOT NULL DEFAULT '0',
  `card3` SMALLINT(11) NOT NULL DEFAULT '0',
  `expire_time` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `bound` TINYINT(1) UNSIGNED NOT NULL DEFAULT '0',
  `unique_id` BIGINT(20) UNSIGNED NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  KEY `char_id` (`char_id`)
) ENGINE=MyISAM;

--
-- Table structure for table `char`
--

CREATE TABLE IF NOT EXISTS `char` (
  `char_id` INT(11) UNSIGNED NOT NULL AUTO_INCREMENT,
  `account_id` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `char_num` TINYINT(1) NOT NULL DEFAULT '0',
  `name` VARCHAR(30) NOT NULL DEFAULT '',
  `class` SMALLINT(6) UNSIGNED NOT NULL DEFAULT '0',
  `base_level` SMALLINT(6) UNSIGNED NOT NULL DEFAULT '1',
  `job_level` SMALLINT(6) UNSIGNED NOT NULL DEFAULT '1',
  `base_exp` BIGINT(20) UNSIGNED NOT NULL DEFAULT '0',
  `job_exp` BIGINT(20) UNSIGNED NOT NULL DEFAULT '0',
  `zeny` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `str` SMALLINT(4) UNSIGNED NOT NULL DEFAULT '0',
  `agi` SMALLINT(4) UNSIGNED NOT NULL DEFAULT '0',
  `vit` SMALLINT(4) UNSIGNED NOT NULL DEFAULT '0',
  `int` SMALLINT(4) UNSIGNED NOT NULL DEFAULT '0',
  `dex` SMALLINT(4) UNSIGNED NOT NULL DEFAULT '0',
  `luk` SMALLINT(4) UNSIGNED NOT NULL DEFAULT '0',
  `max_hp` INT(9) UNSIGNED NOT NULL DEFAULT '0',
  `hp` INT(9) UNSIGNED NOT NULL DEFAULT '0',
  `max_sp` INT(9) UNSIGNED NOT NULL DEFAULT '0',
  `sp` INT(9) UNSIGNED NOT NULL DEFAULT '0',
  `status_point` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `skill_point` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `option` INT(11) NOT NULL DEFAULT '0',
  `karma` TINYINT(3) NOT NULL DEFAULT '0',
  `manner` SMALLINT(6) NOT NULL DEFAULT '0',
  `party_id` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `guild_id` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `pet_id` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `homun_id` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `elemental_id` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `hair` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0',
  `hair_color` SMALLINT(5) UNSIGNED NOT NULL DEFAULT '0',
  `clothes_color` SMALLINT(5) UNSIGNED NOT NULL DEFAULT '0',
  `body` SMALLINT(5) unsigned NOT NULL default '0',
  `weapon` SMALLINT(6) UNSIGNED NOT NULL DEFAULT '0',
  `shield` SMALLINT(6) UNSIGNED NOT NULL DEFAULT '0',
  `head_top` SMALLINT(6) UNSIGNED NOT NULL DEFAULT '0',
  `head_mid` SMALLINT(6) UNSIGNED NOT NULL DEFAULT '0',
  `head_bottom` SMALLINT(6) UNSIGNED NOT NULL DEFAULT '0',
  `robe` SMALLINT(6) UNSIGNED NOT NULL DEFAULT '0',
  `last_map` VARCHAR(11) NOT NULL DEFAULT '',
  `last_x` SMALLINT(4) UNSIGNED NOT NULL DEFAULT '53',
  `last_y` SMALLINT(4) UNSIGNED NOT NULL DEFAULT '111',
  `save_map` VARCHAR(11) NOT NULL DEFAULT '',
  `save_x` SMALLINT(4) UNSIGNED NOT NULL DEFAULT '53',
  `save_y` SMALLINT(4) UNSIGNED NOT NULL DEFAULT '111',
  `partner_id` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `online` TINYINT(2) NOT NULL DEFAULT '0',
  `father` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `mother` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `child` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `fame` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `rename` SMALLINT(3) UNSIGNED NOT NULL DEFAULT '0',
  `delete_date` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `slotchange` SMALLINT(3) UNSIGNED NOT NULL DEFAULT '0',
  `char_opt` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `font` TINYINT(3) UNSIGNED NOT NULL DEFAULT  '0',
  `unban_time` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `uniqueitem_counter` BIGINT(20) UNSIGNED NOT NULL DEFAULT '0',
  `sex` ENUM('M','F','U') NOT NULL DEFAULT 'U',
  `hotkey_rowshift` TINYINT(3) UNSIGNED NOT NULL DEFAULT '0',
  PRIMARY KEY (`char_id`),
  UNIQUE KEY `name_key` (`name`),
  KEY `account_id` (`account_id`),
  KEY `party_id` (`party_id`),
  KEY `guild_id` (`guild_id`),
  KEY `online` (`online`)
) ENGINE=MyISAM AUTO_INCREMENT=150000; 

--
-- Table structure for table `char_reg_num_db`
--

CREATE TABLE IF NOT EXISTS `char_reg_num_db` (
  `char_id` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `key` VARCHAR(32) BINARY NOT NULL DEFAULT '',
  `index` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `value` INT(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`char_id`,`key`,`index`),
  KEY `char_id` (`char_id`)
) ENGINE=MyISAM;

--
-- Table structure for table `char_reg_str_db`
--

CREATE TABLE IF NOT EXISTS `char_reg_str_db` (
  `char_id` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `key` VARCHAR(32) BINARY NOT NULL DEFAULT '',
  `index` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `value` VARCHAR(254) NOT NULL DEFAULT '0',
  PRIMARY KEY (`char_id`,`key`,`index`),
  KEY `char_id` (`char_id`)
) ENGINE=MyISAM;

--
-- Table structure for table `charlog`
--

CREATE TABLE IF NOT EXISTS `charlog` (
  `time` DATETIME NOT NULL DEFAULT '0000-00-00 00:00:00',
  `char_msg` VARCHAR(255) NOT NULL DEFAULT 'char select',
  `account_id` INT(11) NOT NULL DEFAULT '0',
  `char_id` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `char_num` TINYINT(4) NOT NULL DEFAULT '0',
  `name` VARCHAR(23) NOT NULL DEFAULT '',
  `str` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `agi` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `vit` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `INT` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `dex` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `luk` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `hair` TINYINT(4) NOT NULL DEFAULT '0',
  `hair_color` INT(11) NOT NULL DEFAULT '0'
) ENGINE=MyISAM; 

--
-- Table structure for table `elemental`
--

CREATE TABLE IF NOT EXISTS `elemental` (
  `ele_id` INT(11) UNSIGNED NOT NULL AUTO_INCREMENT,
  `char_id` INT(11) NOT NULL,
  `class` MEDIUMINT(9) UNSIGNED NOT NULL DEFAULT '0',
  `mode` INT(11) UNSIGNED NOT NULL DEFAULT '1',
  `hp` INT(12) NOT NULL DEFAULT '1',
  `sp` INT(12) NOT NULL DEFAULT '1',
  `max_hp` MEDIUMINT(8) UNSIGNED NOT NULL DEFAULT '0',
  `max_sp` MEDIUMINT(6) UNSIGNED NOT NULL DEFAULT '0',
  `atk1` MEDIUMINT(6) UNSIGNED NOT NULL DEFAULT '0',
  `atk2` MEDIUMINT(6) UNSIGNED NOT NULL DEFAULT '0',
  `matk` MEDIUMINT(6) UNSIGNED NOT NULL DEFAULT '0',
  `aspd` SMALLINT(4) UNSIGNED NOT NULL DEFAULT '0',
  `def` SMALLINT(4) UNSIGNED NOT NULL DEFAULT '0',
  `mdef` SMALLINT(4) UNSIGNED NOT NULL DEFAULT '0',
  `flee` SMALLINT(4) UNSIGNED NOT NULL DEFAULT '0',
  `hit` SMALLINT(4) UNSIGNED NOT NULL DEFAULT '0',
  `life_time` INT(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`ele_id`)
) ENGINE=MyISAM;

--
-- Table structure for table `friends`
--

CREATE TABLE IF NOT EXISTS `friends` (
  `char_id` INT(11) NOT NULL DEFAULT '0',
  `friend_account` INT(11) NOT NULL DEFAULT '0',
  `friend_id` INT(11) NOT NULL DEFAULT '0',
  KEY  `char_id` (`char_id`)
) ENGINE=MyISAM;

--
-- Table structure for table `hotkey`
--

CREATE TABLE IF NOT EXISTS `hotkey` (
  `char_id` INT(11) NOT NULL,
  `hotkey` TINYINT(2) UNSIGNED NOT NULL,
  `type` TINYINT(1) UNSIGNED NOT NULL DEFAULT '0',
  `itemskill_id` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `skill_lvl` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0',
  PRIMARY KEY (`char_id`,`hotkey`)
) ENGINE=MyISAM;

--
-- Table structure for table `global_acc_reg_num_db`
--

CREATE TABLE IF NOT EXISTS `global_acc_reg_num_db` (
  `account_id` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `key` VARCHAR(32) BINARY NOT NULL DEFAULT '',
  `index` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `value` INT(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`account_id`,`key`,`index`),
  KEY `account_id` (`account_id`)
) ENGINE=MyISAM;

--
-- Table structure for table `global_acc_reg_str_db`
--

CREATE TABLE IF NOT EXISTS `global_acc_reg_str_db` (
  `account_id` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `key` VARCHAR(32) BINARY NOT NULL DEFAULT '',
  `index` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `value` VARCHAR(254) NOT NULL DEFAULT '0',
  PRIMARY KEY (`account_id`,`key`,`index`),
  KEY `account_id` (`account_id`)
) ENGINE=MyISAM;

--
-- Table structure for table `guild`
--

CREATE TABLE IF NOT EXISTS `guild` (
  `guild_id` INT(11) UNSIGNED NOT NULL AUTO_INCREMENT,
  `name` VARCHAR(24) NOT NULL DEFAULT '',
  `char_id` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `master` VARCHAR(24) NOT NULL DEFAULT '',
  `guild_lv` TINYINT(6) UNSIGNED NOT NULL DEFAULT '0',
  `connect_member` TINYINT(6) UNSIGNED NOT NULL DEFAULT '0',
  `max_member` TINYINT(6) UNSIGNED NOT NULL DEFAULT '0',
  `max_storage` SMALLINT(6) UNSIGNED NOT NULL DEFAULT '0',
  `average_lv` SMALLINT(6) UNSIGNED NOT NULL DEFAULT '1',
  `exp` BIGINT(20) UNSIGNED NOT NULL DEFAULT '0',
  `next_exp` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `skill_point` TINYINT(11) UNSIGNED NOT NULL DEFAULT '0',
  `mes1` VARCHAR(60) NOT NULL DEFAULT '',
  `mes2` VARCHAR(120) NOT NULL DEFAULT '',
  `emblem_len` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `emblem_id` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `emblem_data` blob,
  PRIMARY KEY (`guild_id`,`char_id`),
  UNIQUE KEY `guild_id` (`guild_id`),
  KEY `char_id` (`char_id`)
) ENGINE=MyISAM;

--
-- Table structure for table `guild_alliance`
--

CREATE TABLE IF NOT EXISTS `guild_alliance` (
  `guild_id` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `opposition` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `alliance_id` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `name` VARCHAR(24) NOT NULL DEFAULT '',
  PRIMARY KEY (`guild_id`,`alliance_id`),
  KEY `alliance_id` (`alliance_id`)
) ENGINE=MyISAM;

--
-- Table structure for table `guild_castle`
--

CREATE TABLE IF NOT EXISTS `guild_castle` (
  `castle_id` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `guild_id` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `economy` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `defense` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `triggerE` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `triggerD` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `nextTime` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `payTime` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `createTime` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `visibleC` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `visibleG0` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `visibleG1` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `visibleG2` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `visibleG3` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `visibleG4` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `visibleG5` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `visibleG6` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `visibleG7` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  PRIMARY KEY (`castle_id`),
  KEY `guild_id` (`guild_id`)
) ENGINE=MyISAM;

--
-- Table structure for table `guild_expulsion`
--

CREATE TABLE IF NOT EXISTS `guild_expulsion` (
  `guild_id` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `account_id` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `name` VARCHAR(24) NOT NULL DEFAULT '',
  `mes` VARCHAR(40) NOT NULL DEFAULT '',
  PRIMARY KEY (`guild_id`,`name`)
) ENGINE=MyISAM;

--
-- Table structure for table `guild_member`
--

CREATE TABLE IF NOT EXISTS `guild_member` (
  `guild_id` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `account_id` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `char_id` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `hair` TINYINT(6) UNSIGNED NOT NULL DEFAULT '0',
  `hair_color` SMALLINT(6) UNSIGNED NOT NULL DEFAULT '0',
  `gender` TINYINT(6) UNSIGNED NOT NULL DEFAULT '0',
  `class` SMALLINT(6) UNSIGNED NOT NULL DEFAULT '0',
  `lv` SMALLINT(6) UNSIGNED NOT NULL DEFAULT '0',
  `exp` BIGINT(20) UNSIGNED NOT NULL DEFAULT '0',
  `exp_payper` TINYINT(11) UNSIGNED NOT NULL DEFAULT '0',
  `online` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0',
  `position` TINYINT(6) UNSIGNED NOT NULL DEFAULT '0',
  `name` VARCHAR(24) NOT NULL DEFAULT '',
  PRIMARY KEY (`guild_id`,`char_id`),
  KEY `char_id` (`char_id`)
) ENGINE=MyISAM;

--
-- Table structure for table `guild_position`
--

CREATE TABLE IF NOT EXISTS `guild_position` (
  `guild_id` INT(9) UNSIGNED NOT NULL DEFAULT '0',
  `position` TINYINT(6) UNSIGNED NOT NULL DEFAULT '0',
  `name` VARCHAR(24) NOT NULL DEFAULT '',
  `mode` SMALLINT(5) UNSIGNED NOT NULL DEFAULT '0',
  `exp_mode` TINYINT(11) UNSIGNED NOT NULL DEFAULT '0',
  PRIMARY KEY (`guild_id`,`position`)
) ENGINE=MyISAM;

--
-- Table structure for table `guild_skill`
--

CREATE TABLE IF NOT EXISTS `guild_skill` (
  `guild_id` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `id` SMALLINT(11) UNSIGNED NOT NULL DEFAULT '0',
  `lv` TINYINT(11) UNSIGNED NOT NULL DEFAULT '0',
  PRIMARY KEY (`guild_id`,`id`)
) ENGINE=MyISAM;

--
-- Table structure for table `guild_storage`
--

CREATE TABLE IF NOT EXISTS `guild_storage` (
  `id` INT(10) UNSIGNED NOT NULL AUTO_INCREMENT,
  `guild_id` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `nameid` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `amount` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `equip` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `identify` SMALLINT(6) UNSIGNED NOT NULL DEFAULT '0',
  `refine` TINYINT(3) UNSIGNED NOT NULL DEFAULT '0',
  `attribute` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0',
  `card0` SMALLINT(11) NOT NULL DEFAULT '0',
  `card1` SMALLINT(11) NOT NULL DEFAULT '0',
  `card2` SMALLINT(11) NOT NULL DEFAULT '0',
  `card3` SMALLINT(11) NOT NULL DEFAULT '0',
  `expire_time` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `bound` TINYINT(1) UNSIGNED NOT NULL DEFAULT '0',
  `unique_id` BIGINT(20) UNSIGNED NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  KEY `guild_id` (`guild_id`)
) ENGINE=MyISAM;

--
-- Table structure for table `homunculus`
--

CREATE TABLE IF NOT EXISTS `homunculus` (
  `homun_id` INT(11) NOT NULL AUTO_INCREMENT,
  `char_id` INT(11) NOT NULL,
  `class` MEDIUMINT(9) UNSIGNED NOT NULL DEFAULT '0',
  `prev_class` MEDIUMINT(9) NOT NULL DEFAULT '0',
  `name` VARCHAR(24) NOT NULL DEFAULT '',
  `level` SMALLINT(4) NOT NULL DEFAULT '0',
  `exp` INT(12) NOT NULL DEFAULT '0',
  `intimacy` INT(12) NOT NULL DEFAULT '0',
  `hunger` SMALLINT(4) NOT NULL DEFAULT '0',
  `str` SMALLINT(4) UNSIGNED NOT NULL DEFAULT '0',
  `agi` SMALLINT(4) UNSIGNED NOT NULL DEFAULT '0',
  `vit` SMALLINT(4) UNSIGNED NOT NULL DEFAULT '0',
  `INT` SMALLINT(4) UNSIGNED NOT NULL DEFAULT '0',
  `dex` SMALLINT(4) UNSIGNED NOT NULL DEFAULT '0',
  `luk` SMALLINT(4) UNSIGNED NOT NULL DEFAULT '0',
  `hp` INT(12) NOT NULL DEFAULT '1',
  `max_hp` INT(12) NOT NULL DEFAULT '1',
  `sp` INT(12) NOT NULL DEFAULT '1',
  `max_sp` INT(12) NOT NULL DEFAULT '1',
  `skill_point` SMALLINT(4) UNSIGNED NOT NULL DEFAULT '0',
  `alive` TINYINT(2) NOT NULL DEFAULT '1',
  `rename_flag` TINYINT(2) NOT NULL DEFAULT '0',
  `vaporize` TINYINT(2) NOT NULL DEFAULT '0',
  PRIMARY KEY (`homun_id`)
) ENGINE=MyISAM;

-- 
-- Table structure for table `interlog`
--

CREATE TABLE IF NOT EXISTS `interlog` (
  `time` DATETIME NOT NULL DEFAULT '0000-00-00 00:00:00',
  `log` VARCHAR(255) NOT NULL DEFAULT ''
) ENGINE=MyISAM; 

--
-- Table structure for table `inventory`
--

CREATE TABLE IF NOT EXISTS `inventory` (
  `id` INT(11) UNSIGNED NOT NULL AUTO_INCREMENT,
  `char_id` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `nameid` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `amount` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `equip` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `identify` SMALLINT(6) NOT NULL DEFAULT '0',
  `refine` TINYINT(3) UNSIGNED NOT NULL DEFAULT '0',
  `attribute` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0',
  `card0` SMALLINT(11) NOT NULL DEFAULT '0',
  `card1` SMALLINT(11) NOT NULL DEFAULT '0',
  `card2` SMALLINT(11) NOT NULL DEFAULT '0',
  `card3` SMALLINT(11) NOT NULL DEFAULT '0',
  `expire_time` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `favorite` TINYINT(3) UNSIGNED NOT NULL DEFAULT '0',
  `bound` TINYINT(1) UNSIGNED NOT NULL DEFAULT '0',
  `unique_id` BIGINT(20) UNSIGNED NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  KEY `char_id` (`char_id`)
) ENGINE=MyISAM;

--
-- Table structure for table `ipbanlist`
--

CREATE TABLE IF NOT EXISTS `ipbanlist` (
  `list` VARCHAR(255) NOT NULL DEFAULT '',
  `btime` DATETIME NOT NULL DEFAULT '0000-00-00 00:00:00',
  `rtime` DATETIME NOT NULL DEFAULT '0000-00-00 00:00:00',
  `reason` VARCHAR(255) NOT NULL DEFAULT '',
  KEY (`list`)
) ENGINE=MyISAM;

--
-- Table structure for table `login`
--

CREATE TABLE IF NOT EXISTS `login` (
  `account_id` INT(11) UNSIGNED NOT NULL AUTO_INCREMENT,
  `userid` VARCHAR(23) NOT NULL DEFAULT '',
  `user_pass` VARCHAR(32) NOT NULL DEFAULT '',
  `sex` ENUM('M','F','S') NOT NULL DEFAULT 'M',
  `email` VARCHAR(39) NOT NULL DEFAULT '',
  `group_id` TINYINT(3) NOT NULL DEFAULT '0',
  `state` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `unban_time` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `expiration_time` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `logincount` MEDIUMINT(9) UNSIGNED NOT NULL DEFAULT '0',
  `lastlogin` DATETIME NOT NULL DEFAULT '0000-00-00 00:00:00',
  `last_ip` VARCHAR(100) NOT NULL DEFAULT '',
  `birthdate` DATE NOT NULL DEFAULT '0000-00-00',
  `character_slots` TINYINT(3) UNSIGNED NOT NULL DEFAULT '0',
  `pincode` VARCHAR(4) NOT NULL DEFAULT '',
  `pincode_change` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  PRIMARY KEY (`account_id`),
  KEY `name` (`userid`)
) ENGINE=MyISAM AUTO_INCREMENT=2000000; 

-- added standard accounts for servers, VERY INSECURE!!!
-- inserted into the table called login which is above

INSERT IGNORE INTO `login` (`account_id`, `userid`, `user_pass`, `sex`, `email`) VALUES ('1', 's1', 'p1', 'S','athena@athena.com');

--
-- Table structure for table `mapreg`
--

CREATE TABLE IF NOT EXISTS `mapreg` (
  `varname` VARCHAR(32) BINARY NOT NULL,
  `index` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `value` VARCHAR(255) NOT NULL,
  PRIMARY KEY (`varname`,`index`)
) ENGINE=MyISAM;

--
-- Table structure for table `npc_market_data`
--

CREATE TABLE IF NOT EXISTS `npc_market_data` (
  `name` VARCHAR(24) NOT NULL DEFAULT '',
  `itemid` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `amount` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  PRIMARY KEY (`name`,`itemid`)
) ENGINE=MyISAM;

--
-- Table structure for table `sc_data`
--

CREATE TABLE IF NOT EXISTS `sc_data` (
  `account_id` INT(11) UNSIGNED NOT NULL,
  `char_id` INT(11) UNSIGNED NOT NULL,
  `type` SMALLINT(11) UNSIGNED NOT NULL,
  `tick` INT(11) NOT NULL,
  `val1` INT(11) NOT NULL DEFAULT '0',
  `val2` INT(11) NOT NULL DEFAULT '0',
  `val3` INT(11) NOT NULL DEFAULT '0',
  `val4` INT(11) NOT NULL DEFAULT '0',
  KEY (`account_id`),
  KEY (`char_id`),
  PRIMARY KEY (`account_id`,`char_id`,`type`)
) ENGINE=MyISAM;

--
-- Table structure for table `mail`
--

CREATE TABLE IF NOT EXISTS `mail` (
  `id` BIGINT(20) UNSIGNED NOT NULL AUTO_INCREMENT,
  `send_name` VARCHAR(30) NOT NULL DEFAULT '',
  `send_id` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `dest_name` VARCHAR(30) NOT NULL DEFAULT '',
  `dest_id` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `title` VARCHAR(45) NOT NULL DEFAULT '',
  `message` VARCHAR(255) NOT NULL DEFAULT '',
  `time` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `status` TINYINT(2) NOT NULL DEFAULT '0',
  `zeny` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `nameid` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `amount` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `refine` TINYINT(3) UNSIGNED NOT NULL DEFAULT '0',
  `attribute` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0',
  `identify` SMALLINT(6) NOT NULL DEFAULT '0',
  `card0` SMALLINT(11) NOT NULL DEFAULT '0',
  `card1` SMALLINT(11) NOT NULL DEFAULT '0',
  `card2` SMALLINT(11) NOT NULL DEFAULT '0',
  `card3` SMALLINT(11) NOT NULL DEFAULT '0',
  `unique_id` BIGINT(20) UNSIGNED NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=MyISAM;

--
-- Table structure for table `memo`
--

CREATE TABLE IF NOT EXISTS `memo` (
  `memo_id` INT(11) UNSIGNED NOT NULL AUTO_INCREMENT,
  `char_id` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `map` VARCHAR(11) NOT NULL DEFAULT '',
  `x` SMALLINT(4) UNSIGNED NOT NULL DEFAULT '0',
  `y` SMALLINT(4) UNSIGNED NOT NULL DEFAULT '0',
  PRIMARY KEY (`memo_id`),
  KEY `char_id` (`char_id`)
) ENGINE=MyISAM;

--
-- Table structure for table `mercenary`
--

CREATE TABLE IF NOT EXISTS `mercenary` (
  `mer_id` INT(11) UNSIGNED NOT NULL AUTO_INCREMENT,
  `char_id` INT(11) NOT NULL,
  `class` MEDIUMINT(9) UNSIGNED NOT NULL DEFAULT '0',
  `hp` INT(12) NOT NULL DEFAULT '1',
  `sp` INT(12) NOT NULL DEFAULT '1',
  `kill_counter` INT(11) NOT NULL,
  `life_time` INT(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`mer_id`)
) ENGINE=MyISAM;

--
-- Table structure for table `mercenary_owner`
--

CREATE TABLE IF NOT EXISTS `mercenary_owner` (
  `char_id` INT(11) NOT NULL,
  `merc_id` INT(11) NOT NULL DEFAULT '0',
  `arch_calls` INT(11) NOT NULL DEFAULT '0',
  `arch_faith` INT(11) NOT NULL DEFAULT '0',
  `spear_calls` INT(11) NOT NULL DEFAULT '0',
  `spear_faith` INT(11) NOT NULL DEFAULT '0',
  `sword_calls` INT(11) NOT NULL DEFAULT '0',
  `sword_faith` INT(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`char_id`)
) ENGINE=MyISAM;

--
-- Table structure for table `party`
--

CREATE TABLE IF NOT EXISTS `party` (
  `party_id` INT(11) UNSIGNED NOT NULL AUTO_INCREMENT,
  `name` VARCHAR(24) NOT NULL DEFAULT '',
  `exp` TINYINT(11) UNSIGNED NOT NULL DEFAULT '0',
  `item` TINYINT(11) UNSIGNED NOT NULL DEFAULT '0',
  `leader_id` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `leader_char` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  PRIMARY KEY (`party_id`)
) ENGINE=MyISAM;

--
-- Table structure for table `pet`
--

CREATE TABLE IF NOT EXISTS `pet` (
  `pet_id` INT(11) UNSIGNED NOT NULL AUTO_INCREMENT,
  `class` MEDIUMINT(9) UNSIGNED NOT NULL DEFAULT '0',
  `name` VARCHAR(24) NOT NULL DEFAULT '',
  `account_id` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `char_id` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `level` SMALLINT(4) UNSIGNED NOT NULL DEFAULT '0',
  `egg_id` SMALLINT(11) UNSIGNED NOT NULL DEFAULT '0',
  `equip` MEDIUMINT(8) UNSIGNED NOT NULL DEFAULT '0',
  `intimate` SMALLINT(9) UNSIGNED NOT NULL DEFAULT '0',
  `hungry` SMALLINT(9) UNSIGNED NOT NULL DEFAULT '0',
  `rename_flag` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0',
  `incubate` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  PRIMARY KEY (`pet_id`)
) ENGINE=MyISAM;

--
-- Table structure for table `quest`
--

CREATE TABLE IF NOT EXISTS `quest` (
  `char_id` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `quest_id` INT(10) UNSIGNED NOT NULL,
  `state` ENUM('0','1','2') NOT NULL DEFAULT '0',
  `time` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `count1` MEDIUMINT(8) UNSIGNED NOT NULL DEFAULT '0',
  `count2` MEDIUMINT(8) UNSIGNED NOT NULL DEFAULT '0',
  `count3` MEDIUMINT(8) UNSIGNED NOT NULL DEFAULT '0',
  PRIMARY KEY (`char_id`,`quest_id`)
) ENGINE=MyISAM;

--
-- Table structure for table `ragsrvinfo`
--

CREATE TABLE IF NOT EXISTS `ragsrvinfo` (
  `index` INT(11) NOT NULL DEFAULT '0',
  `name` VARCHAR(255) NOT NULL DEFAULT '',
  `exp` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `jexp` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `drop` INT(11) UNSIGNED NOT NULL DEFAULT '0'
) ENGINE=MyISAM;

--
-- Table structure for table `skill`
--

CREATE TABLE IF NOT EXISTS `skill` (
  `char_id` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `id` SMALLINT(11) UNSIGNED NOT NULL DEFAULT '0',
  `lv` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0',
  `flag` TINYINT(1) UNSIGNED NOT NULL DEFAULT 0,
  PRIMARY KEY (`char_id`,`id`)
) ENGINE=MyISAM;

--
-- Table structure for table `skill_homunculus`
--

CREATE TABLE IF NOT EXISTS `skill_homunculus` (
  `homun_id` INT(11) NOT NULL,
  `id` INT(11) NOT NULL,
  `lv` SMALLINT(6) NOT NULL,
  PRIMARY KEY (`homun_id`,`id`)
) ENGINE=MyISAM;

--
-- Table structure for table `sql_updates`
--

CREATE TABLE IF NOT EXISTS `sql_updates` (
  `timestamp` INT(11) UNSIGNED NOT NULL,
  `ignored` ENUM('Yes','No') NOT NULL DEFAULT 'No',
  PRIMARY KEY (`timestamp`)
) ENGINE=MyISAM;

-- Existent updates to enter
INSERT IGNORE INTO `sql_updates` (`timestamp`) VALUES (1360858500); -- 2013-02-14--16-15.sql
INSERT IGNORE INTO `sql_updates` (`timestamp`) VALUES (1360951560); -- 2013-02-15--18-06.sql
INSERT IGNORE INTO `sql_updates` (`timestamp`) VALUES (1362445531); -- 2013-03-05--01-05.sql
INSERT IGNORE INTO `sql_updates` (`timestamp`) VALUES (1362528000); -- 2013-03-06--00-00.sql
INSERT IGNORE INTO `sql_updates` (`timestamp`) VALUES (1362794218); -- 2013-03-09--01-56.sql
INSERT IGNORE INTO `sql_updates` (`timestamp`) VALUES (1364409316); -- 2013-03-27--18-35.sql
INSERT IGNORE INTO `sql_updates` (`timestamp`) VALUES (1366075474); -- 2013-04-16--01-24.sql
INSERT IGNORE INTO `sql_updates` (`timestamp`) VALUES (1366078541); -- 2013-04-16--02-15.sql
INSERT IGNORE INTO `sql_updates` (`timestamp`) VALUES (1381354728); -- 2013-10-09--21-38.sql
INSERT IGNORE INTO `sql_updates` (`timestamp`) VALUES (1381423003); -- 2013-10-10--16-36.sql
INSERT IGNORE INTO `sql_updates` (`timestamp`) VALUES (1382892428); -- 2013-10-27--16-47.sql
INSERT IGNORE INTO `sql_updates` (`timestamp`) VALUES (1383162785); -- 2013-10-30--19-53.sql
INSERT IGNORE INTO `sql_updates` (`timestamp`) VALUES (1383167577); -- 2013-10-30--21-12.sql
INSERT IGNORE INTO `sql_updates` (`timestamp`) VALUES (1383205740); -- 2013-10-31--07-49.sql
INSERT IGNORE INTO `sql_updates` (`timestamp`) VALUES (1383955424); -- 2013-11-09--00-03.sql
INSERT IGNORE INTO `sql_updates` (`timestamp`) VALUES (1384473995); -- 2013-11-15--00-06.sql
INSERT IGNORE INTO `sql_updates` (`timestamp`) VALUES (1384545461); -- 2013-11-15--19-57.sql
INSERT IGNORE INTO `sql_updates` (`timestamp`) VALUES (1384588175); -- 2013-11-16--07-49.sql
INSERT IGNORE INTO `sql_updates` (`timestamp`) VALUES (1384763034); -- 2013-11-18--08-23.sql
INSERT IGNORE INTO `sql_updates` (`timestamp`) VALUES (1387844126); -- 2013-12-24--00-15.sql
INSERT IGNORE INTO `sql_updates` (`timestamp`) VALUES (1388854043); -- 2014-01-04--16-47.sql
INSERT IGNORE INTO `sql_updates` (`timestamp`) VALUES (1389028967); -- 2014-01-06--17-22.sql
INSERT IGNORE INTO `sql_updates` (`timestamp`) VALUES (1392832626); -- 2014-02-19--17-57.sql
INSERT IGNORE INTO `sql_updates` (`timestamp`) VALUES (1395789302); -- 2014-03-25--23-57.sql
INSERT IGNORE INTO `sql_updates` (`timestamp`) VALUES (1396893866); -- 2014-04-07--22-04.sql
INSERT IGNORE INTO `sql_updates` (`timestamp`) VALUES (1398477600); -- 2014-04-26--10-00.sql
INSERT IGNORE INTO `sql_updates` (`timestamp`) VALUES (1400256139); -- 2014-05-17--00-06.sql
INSERT IGNORE INTO `sql_updates` (`timestamp`) VALUES (1409590380); -- 2014-09-01--16-53.sql
INSERT IGNORE INTO `sql_updates` (`timestamp`) VALUES (1414975503); -- 2014-11-03--00-45.sql
INSERT IGNORE INTO `sql_updates` (`timestamp`) VALUES (1435860840); -- 2015-07-02--18-14.sql
INSERT IGNORE INTO `sql_updates` (`timestamp`) VALUES (1436360978); -- 2015-07-08--13-08.sql
INSERT IGNORE INTO `sql_updates` (`timestamp`) VALUES (1440688342); -- 2015-08-27--20-42.sql
INSERT IGNORE INTO `sql_updates` (`timestamp`) VALUES (1450241859); -- 2015-12-16--12-57.sql
INSERT IGNORE INTO `sql_updates` (`timestamp`) VALUES (1450367880); -- 2015-12-17--15-58.sql
INSERT IGNORE INTO `sql_updates` (`timestamp`) VALUES (1457638175); -- 2016-03-10--22-18.sql
INSERT IGNORE INTO `sql_updates` (`timestamp`) VALUES (1458421443); -- 2016-03-19--21-04.sql

--
-- Table structure for table `storage`
--

CREATE TABLE IF NOT EXISTS `storage` (
  `id` INT(11) UNSIGNED NOT NULL AUTO_INCREMENT,
  `account_id` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `nameid` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `amount` SMALLINT(11) UNSIGNED NOT NULL DEFAULT '0',
  `equip` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `identify` SMALLINT(6) UNSIGNED NOT NULL DEFAULT '0',
  `refine` TINYINT(3) UNSIGNED NOT NULL DEFAULT '0',
  `attribute` TINYINT(4) UNSIGNED NOT NULL DEFAULT '0',
  `card0` SMALLINT(11) NOT NULL DEFAULT '0',
  `card1` SMALLINT(11) NOT NULL DEFAULT '0',
  `card2` SMALLINT(11) NOT NULL DEFAULT '0',
  `card3` SMALLINT(11) NOT NULL DEFAULT '0',
  `expire_time` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `bound` TINYINT(1) UNSIGNED NOT NULL DEFAULT '0',
  `unique_id` BIGINT(20) UNSIGNED NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  KEY `account_id` (`account_id`)
) ENGINE=MyISAM;
