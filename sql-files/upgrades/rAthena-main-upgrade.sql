-- rAthena to Hercules Main database upgrade query.
-- This upgrades a FULLY UPGRADED rAthena to a FULLY UPGRADED Hercules
-- Please don't use if either rAthena or Hercules launched a SQL update after last revision date of this file.
-- Remember to make a backup before applying.
-- We are not liable for any data loss this may cause.
-- Apply in the same database you applied your logs.sql
-- Last revision: 3/10/2014 7:03:15 PM


/* Header line. Object: bonus_script. Script date: 3/10/2014 7:03:15 PM. */
-- Attention: Table `bonus_script` will be dropped.
DROP TABLE `bonus_script`;

/* Header line. Object: buyingstore_items. Script date: 3/10/2014 7:03:15 PM. */
-- Attention: Table `buyingstore_items` will be dropped.
DROP TABLE `buyingstore_items`;

/* Header line. Object: buyingstores. Script date: 3/10/2014 7:03:15 PM. */
-- Attention: Table `buyingstores` will be dropped.
DROP TABLE `buyingstores`;

/* Header line. Object: global_reg_value. Script date: 3/10/2014 7:03:15 PM. */
-- Attention: Table `global_reg_value` will be dropped.
DROP TABLE `global_reg_value`;

/* Header line. Object: skillcooldown. Script date: 3/10/2014 7:03:15 PM. */
-- Attention: Table `skillcooldown` will be dropped.
DROP TABLE `skillcooldown`;

/* Header line. Object: vending_items. Script date: 3/10/2014 7:03:15 PM. */
-- Attention: Table `vending_items` will be dropped.
DROP TABLE `vending_items`;

/* Header line. Object: vendings. Script date: 3/10/2014 7:03:15 PM. */
-- Attention: Table `vendings` will be dropped.
DROP TABLE `vendings`;

/* Header line. Object: acc_reg_num_db. Script date: 3/10/2014 7:03:15 PM. */
CREATE TABLE `acc_reg_num_db` (
	`account_id` int(11) unsigned NOT NULL default '0',
	`key` varchar(32) NOT NULL,
	`index` int(11) unsigned NOT NULL default '0',
	`value` int(11) NOT NULL default '0',
	KEY `account_id` ( `account_id` ),
	PRIMARY KEY  ( `account_id`, `key`, `index` )
)
ENGINE = MyISAM
CHARACTER SET = latin1
ROW_FORMAT = Dynamic
;

/* Header line. Object: acc_reg_str_db. Script date: 3/10/2014 7:03:15 PM. */
CREATE TABLE `acc_reg_str_db` (
	`account_id` int(11) unsigned NOT NULL default '0',
	`key` varchar(32) NOT NULL,
	`index` int(11) unsigned NOT NULL default '0',
	`value` varchar(254) NOT NULL default '0',
	KEY `account_id` ( `account_id` ),
	PRIMARY KEY  ( `account_id`, `key`, `index` )
)
ENGINE = MyISAM
CHARACTER SET = latin1
ROW_FORMAT = Dynamic
;

/* Header line. Object: account_data. Script date: 3/10/2014 7:03:15 PM. */
CREATE TABLE `account_data` (
	`account_id` int(11) unsigned NOT NULL default '0',
	`bank_vault` int(11) unsigned NOT NULL default '0',
	`base_exp` tinyint(4) unsigned NOT NULL default '100',
	`base_drop` tinyint(4) unsigned NOT NULL default '100',
	`base_death` tinyint(4) unsigned NOT NULL default '100',
	PRIMARY KEY  ( `account_id` )
)
ENGINE = MyISAM
CHARACTER SET = latin1
ROW_FORMAT = Fixed
;

/* Header line. Object: autotrade_data. Script date: 3/10/2014 7:03:15 PM. */
CREATE TABLE `autotrade_data` (
	`char_id` int(11) NOT NULL default '0',
	`itemkey` int(11) NOT NULL default '0',
	`amount` int(11) NOT NULL default '0',
	`price` int(11) NOT NULL default '0',
	PRIMARY KEY  ( `char_id`, `itemkey` )
)
ENGINE = MyISAM
CHARACTER SET = latin1
ROW_FORMAT = Fixed
;

/* Header line. Object: autotrade_merchants. Script date: 3/10/2014 7:03:15 PM. */
CREATE TABLE `autotrade_merchants` (
	`account_id` int(11) NOT NULL default '0',
	`char_id` int(11) NOT NULL default '0',
	`sex` tinyint(2) NOT NULL default '0',
	`title` varchar(80) NOT NULL default 'Buy From Me!',
	PRIMARY KEY  ( `account_id`, `char_id` )
)
ENGINE = MyISAM
CHARACTER SET = latin1
ROW_FORMAT = Dynamic
;

/* Header line. Object: cart_inventory. Script date: 3/10/2014 7:03:15 PM. */
DROP TABLE IF EXISTS `_temp_cart_inventory`;

CREATE TABLE `_temp_cart_inventory` (
	`id` int(11) NOT NULL auto_increment,
	`char_id` int(11) NOT NULL default '0',
	`nameid` int(11) NOT NULL default '0',
	`amount` int(11) NOT NULL default '0',
	`equip` int(11) unsigned NOT NULL default '0',
	`identify` smallint(6) NOT NULL default '0',
	`refine` tinyint(3) unsigned NOT NULL default '0',
	`attribute` tinyint(4) NOT NULL default '0',
	`card0` smallint(11) NOT NULL default '0',
	`card1` smallint(11) NOT NULL default '0',
	`card2` smallint(11) NOT NULL default '0',
	`card3` smallint(11) NOT NULL default '0',
	`expire_time` int(11) unsigned NOT NULL default '0',
	`bound` tinyint(1) unsigned NOT NULL default '0',
	`unique_id` bigint(20) unsigned NOT NULL default '0',
	KEY `char_id` ( `char_id` ),
	PRIMARY KEY  ( `id` )
)
ENGINE = MyISAM
CHARACTER SET = latin1
AUTO_INCREMENT = 1
ROW_FORMAT = Fixed
;

INSERT INTO `_temp_cart_inventory`
( `amount`, `attribute`, `bound`, `card0`, `card1`, `card2`, `card3`, `char_id`, `equip`, `expire_time`, `id`, `identify`, `nameid`, `refine`, `unique_id` )
SELECT
`amount`, `attribute`, `bound`, `card0`, `card1`, `card2`, `card3`, `char_id`, `equip`, `expire_time`, `id`, `identify`, `nameid`, `refine`, `unique_id`
FROM `cart_inventory`;

DROP TABLE `cart_inventory`;

ALTER TABLE `_temp_cart_inventory` RENAME `cart_inventory`;

/* Header line. Object: char. Script date: 3/10/2014 7:03:15 PM. */
DROP TABLE IF EXISTS `_temp_char`;

CREATE TABLE `_temp_char` (
	`char_id` int(11) unsigned NOT NULL auto_increment,
	`account_id` int(11) unsigned NOT NULL default '0',
	`char_num` tinyint(1) NOT NULL default '0',
	`name` varchar(30) NOT NULL,
	`class` smallint(6) unsigned NOT NULL default '0',
	`base_level` smallint(6) unsigned NOT NULL default '1',
	`job_level` smallint(6) unsigned NOT NULL default '1',
	`base_exp` bigint(20) unsigned NOT NULL default '0',
	`job_exp` bigint(20) unsigned NOT NULL default '0',
	`zeny` int(11) unsigned NOT NULL default '0',
	`str` smallint(4) unsigned NOT NULL default '0',
	`agi` smallint(4) unsigned NOT NULL default '0',
	`vit` smallint(4) unsigned NOT NULL default '0',
	`int` smallint(4) unsigned NOT NULL default '0',
	`dex` smallint(4) unsigned NOT NULL default '0',
	`luk` smallint(4) unsigned NOT NULL default '0',
	`max_hp` mediumint(8) unsigned NOT NULL default '0',
	`hp` mediumint(8) unsigned NOT NULL default '0',
	`max_sp` mediumint(6) unsigned NOT NULL default '0',
	`sp` mediumint(6) unsigned NOT NULL default '0',
	`status_point` int(11) unsigned NOT NULL default '0',
	`skill_point` int(11) unsigned NOT NULL default '0',
	`option` int(11) NOT NULL default '0',
	`karma` tinyint(3) NOT NULL default '0',
	`manner` smallint(6) NOT NULL default '0',
	`party_id` int(11) unsigned NOT NULL default '0',
	`guild_id` int(11) unsigned NOT NULL default '0',
	`pet_id` int(11) unsigned NOT NULL default '0',
	`homun_id` int(11) unsigned NOT NULL default '0',
	`elemental_id` int(11) unsigned NOT NULL default '0',
	`hair` tinyint(4) unsigned NOT NULL default '0',
	`hair_color` smallint(5) unsigned NOT NULL default '0',
	`clothes_color` smallint(5) unsigned NOT NULL default '0',
	`weapon` smallint(6) unsigned NOT NULL default '0',
	`shield` smallint(6) unsigned NOT NULL default '0',
	`head_top` smallint(6) unsigned NOT NULL default '0',
	`head_mid` smallint(6) unsigned NOT NULL default '0',
	`head_bottom` smallint(6) unsigned NOT NULL default '0',
	`robe` smallint(6) unsigned NOT NULL default '0',
	`last_map` varchar(11) NOT NULL,
	`last_x` smallint(4) unsigned NOT NULL default '53',
	`last_y` smallint(4) unsigned NOT NULL default '111',
	`save_map` varchar(11) NOT NULL,
	`save_x` smallint(4) unsigned NOT NULL default '53',
	`save_y` smallint(4) unsigned NOT NULL default '111',
	`partner_id` int(11) unsigned NOT NULL default '0',
	`online` tinyint(2) NOT NULL default '0',
	`father` int(11) unsigned NOT NULL default '0',
	`mother` int(11) unsigned NOT NULL default '0',
	`child` int(11) unsigned NOT NULL default '0',
	`fame` int(11) unsigned NOT NULL default '0',
	`rename` smallint(3) unsigned NOT NULL default '0',
	`delete_date` int(11) unsigned NOT NULL default '0',
	`slotchange` smallint(3) unsigned NOT NULL default '0',
	`char_opt` int(11) unsigned NOT NULL default '0',
	`font` tinyint(3) unsigned NOT NULL default '0',
	`unban_time` int(11) unsigned NOT NULL default '0',
	KEY `account_id` ( `account_id` ),
	KEY `guild_id` ( `guild_id` ),
	UNIQUE INDEX `name_key` ( `name` ),
	KEY `online` ( `online` ),
	KEY `party_id` ( `party_id` ),
	PRIMARY KEY  ( `char_id` )
)
ENGINE = MyISAM
CHARACTER SET = latin1
AUTO_INCREMENT = 150000
ROW_FORMAT = Dynamic
;

INSERT INTO `_temp_char`
( `account_id`, `agi`, `base_exp`, `base_level`, `char_id`, `char_num`, `child`, `class`, `clothes_color`, `delete_date`, `dex`, `elemental_id`, `fame`, `father`, `guild_id`, `hair`, `hair_color`, `head_bottom`, `head_mid`, `head_top`, `homun_id`, `hp`, `int`, `job_exp`, `job_level`, `karma`, `last_map`, `last_x`, `last_y`, `luk`, `manner`, `max_hp`, `max_sp`, `mother`, `name`, `online`, `option`, `partner_id`, `party_id`, `pet_id`, `rename`, `robe`, `save_map`, `save_x`, `save_y`, `shield`, `skill_point`, `sp`, `status_point`, `str`, `unban_time`, `vit`, `weapon`, `zeny` )
SELECT
`account_id`, `agi`, `base_exp`, `base_level`, `char_id`, `char_num`, `child`, `class`, `clothes_color`, `delete_date`, `dex`, `elemental_id`, `fame`, `father`, `guild_id`, `hair`, `hair_color`, `head_bottom`, `head_mid`, `head_top`, `homun_id`, `hp`, `int`, `job_exp`, `job_level`, `karma`, `last_map`, `last_x`, `last_y`, `luk`, `manner`, `max_hp`, `max_sp`, `mother`, `name`, `online`, `option`, `partner_id`, `party_id`, `pet_id`, `rename`, `robe`, `save_map`, `save_x`, `save_y`, `shield`, `skill_point`, `sp`, `status_point`, `str`, `unban_time`, `vit`, `weapon`, `zeny`
FROM `char`;

DROP TABLE `char`;

ALTER TABLE `_temp_char` RENAME `char`;

/* Header line. Object: char_reg_num_db. Script date: 3/10/2014 7:03:15 PM. */
CREATE TABLE `char_reg_num_db` (
	`char_id` int(11) unsigned NOT NULL default '0',
	`key` varchar(32) NOT NULL,
	`index` int(11) unsigned NOT NULL default '0',
	`value` int(11) NOT NULL default '0',
	KEY `char_id` ( `char_id` ),
	PRIMARY KEY  ( `char_id`, `key`, `index` )
)
ENGINE = MyISAM
CHARACTER SET = latin1
ROW_FORMAT = Dynamic
;

/* Header line. Object: char_reg_str_db. Script date: 3/10/2014 7:03:15 PM. */
CREATE TABLE `char_reg_str_db` (
	`char_id` int(11) unsigned NOT NULL default '0',
	`key` varchar(32) NOT NULL,
	`index` int(11) unsigned NOT NULL default '0',
	`value` varchar(254) NOT NULL default '0',
	KEY `char_id` ( `char_id` ),
	PRIMARY KEY  ( `char_id`, `key`, `index` )
)
ENGINE = MyISAM
CHARACTER SET = latin1
ROW_FORMAT = Dynamic
;

/* Header line. Object: global_acc_reg_str_db. Script date: 3/10/2014 7:03:15 PM. */
CREATE TABLE `global_acc_reg_str_db` (
	`account_id` int(11) unsigned NOT NULL default '0',
	`key` varchar(32) NOT NULL,
	`index` int(11) unsigned NOT NULL default '0',
	`value` varchar(254) NOT NULL default '0',
	KEY `account_id` ( `account_id` ),
	PRIMARY KEY  ( `account_id`, `key`, `index` )
)
ENGINE = MyISAM
CHARACTER SET = latin1
ROW_FORMAT = Dynamic
;

/* Header line. Object: guild_storage. Script date: 3/10/2014 7:03:15 PM. */
DROP TABLE IF EXISTS `_temp_guild_storage`;

CREATE TABLE `_temp_guild_storage` (
	`id` int(10) unsigned NOT NULL auto_increment,
	`guild_id` int(11) unsigned NOT NULL default '0',
	`nameid` int(11) unsigned NOT NULL default '0',
	`amount` int(11) unsigned NOT NULL default '0',
	`equip` int(11) unsigned NOT NULL default '0',
	`identify` smallint(6) unsigned NOT NULL default '0',
	`refine` tinyint(3) unsigned NOT NULL default '0',
	`attribute` tinyint(4) unsigned NOT NULL default '0',
	`card0` smallint(11) NOT NULL default '0',
	`card1` smallint(11) NOT NULL default '0',
	`card2` smallint(11) NOT NULL default '0',
	`card3` smallint(11) NOT NULL default '0',
	`expire_time` int(11) unsigned NOT NULL default '0',
	`bound` tinyint(1) unsigned NOT NULL default '0',
	`unique_id` bigint(20) unsigned NOT NULL default '0',
	KEY `guild_id` ( `guild_id` ),
	PRIMARY KEY  ( `id` )
)
ENGINE = MyISAM
CHARACTER SET = latin1
AUTO_INCREMENT = 1
ROW_FORMAT = Fixed
;

INSERT INTO `_temp_guild_storage`
( `amount`, `attribute`, `bound`, `card0`, `card1`, `card2`, `card3`, `equip`, `expire_time`, `guild_id`, `id`, `identify`, `nameid`, `refine`, `unique_id` )
SELECT
`amount`, `attribute`, `bound`, `card0`, `card1`, `card2`, `card3`, `equip`, `expire_time`, `guild_id`, `id`, `identify`, `nameid`, `refine`, `unique_id`
FROM `guild_storage`;

DROP TABLE `guild_storage`;

ALTER TABLE `_temp_guild_storage` RENAME `guild_storage`;

/* Header line. Object: inventory. Script date: 3/10/2014 7:03:15 PM. */
DROP TABLE IF EXISTS `_temp_inventory`;

CREATE TABLE `_temp_inventory` (
	`id` int(11) unsigned NOT NULL auto_increment,
	`char_id` int(11) unsigned NOT NULL default '0',
	`nameid` int(11) unsigned NOT NULL default '0',
	`amount` int(11) unsigned NOT NULL default '0',
	`equip` int(11) unsigned NOT NULL default '0',
	`identify` smallint(6) NOT NULL default '0',
	`refine` tinyint(3) unsigned NOT NULL default '0',
	`attribute` tinyint(4) unsigned NOT NULL default '0',
	`card0` smallint(11) NOT NULL default '0',
	`card1` smallint(11) NOT NULL default '0',
	`card2` smallint(11) NOT NULL default '0',
	`card3` smallint(11) NOT NULL default '0',
	`expire_time` int(11) unsigned NOT NULL default '0',
	`favorite` tinyint(3) unsigned NOT NULL default '0',
	`bound` tinyint(1) unsigned NOT NULL default '0',
	`unique_id` bigint(20) unsigned NOT NULL default '0',
	KEY `char_id` ( `char_id` ),
	PRIMARY KEY  ( `id` )
)
ENGINE = MyISAM
CHARACTER SET = latin1
AUTO_INCREMENT = 1
ROW_FORMAT = Fixed
;

INSERT INTO `_temp_inventory`
( `amount`, `attribute`, `bound`, `card0`, `card1`, `card2`, `card3`, `char_id`, `equip`, `expire_time`, `favorite`, `id`, `identify`, `nameid`, `refine`, `unique_id` )
SELECT
`amount`, `attribute`, `bound`, `card0`, `card1`, `card2`, `card3`, `char_id`, `equip`, `expire_time`, `favorite`, `id`, `identify`, `nameid`, `refine`, `unique_id`
FROM `inventory`;

DROP TABLE `inventory`;

ALTER TABLE `_temp_inventory` RENAME `inventory`;

/* Header line. Object: login. Script date: 3/10/2014 7:03:15 PM. */
DROP TABLE IF EXISTS `_temp_login`;

CREATE TABLE `_temp_login` (
	`account_id` int(11) unsigned NOT NULL auto_increment,
	`userid` varchar(23) NOT NULL,
	`user_pass` varchar(32) NOT NULL,
	`sex` enum('M','F','S') NOT NULL default 'M',
	`email` varchar(39) NOT NULL,
	`group_id` tinyint(3) NOT NULL default '0',
	`state` int(11) unsigned NOT NULL default '0',
	`unban_time` int(11) unsigned NOT NULL default '0',
	`expiration_time` int(11) unsigned NOT NULL default '0',
	`logincount` mediumint(9) unsigned NOT NULL default '0',
	`lastlogin` datetime NOT NULL default '0000-00-00 00:00:00',
	`last_ip` varchar(100) NOT NULL,
	`birthdate` date NOT NULL default '0000-00-00',
	`character_slots` tinyint(3) unsigned NOT NULL default '0',
	`pincode` varchar(4) NOT NULL,
	`pincode_change` int(11) unsigned NOT NULL default '0',
	KEY `name` ( `userid` ),
	PRIMARY KEY  ( `account_id` )
)
ENGINE = MyISAM
CHARACTER SET = latin1
AUTO_INCREMENT = 2000000
ROW_FORMAT = Dynamic
;

INSERT INTO `_temp_login`
( `account_id`, `birthdate`, `character_slots`, `email`, `expiration_time`, `group_id`, `last_ip`, `lastlogin`, `logincount`, `pincode`, `pincode_change`, `sex`, `state`, `unban_time`, `user_pass`, `userid` )
SELECT
`account_id`, `birthdate`, `character_slots`, `email`, `expiration_time`, `group_id`, `last_ip`, `lastlogin`, `logincount`, `pincode`, `pincode_change`, `sex`, `state`, `unban_time`, `user_pass`, `userid`
FROM `login`;

DROP TABLE `login`;

ALTER TABLE `_temp_login` RENAME `login`;

/* Header line. Object: mapreg. Script date: 3/10/2014 7:03:15 PM. */
DROP TABLE IF EXISTS `_temp_mapreg`;

CREATE TABLE `_temp_mapreg` (
	`varname` varchar(32) NOT NULL,
	`index` int(11) unsigned NOT NULL default '0',
	`value` varchar(255) NOT NULL,
	PRIMARY KEY  ( `varname`, `index` )
)
ENGINE = MyISAM
CHARACTER SET = latin1
ROW_FORMAT = Dynamic
;

INSERT INTO `_temp_mapreg`
( `index`, `value`, `varname` )
SELECT
`index`, `value`, `varname`
FROM `mapreg`;

DROP TABLE `mapreg`;

ALTER TABLE `_temp_mapreg` RENAME `mapreg`;

/* Header line. Object: npc_market_data. Script date: 3/10/2014 7:03:15 PM. */
CREATE TABLE `npc_market_data` (
	`name` varchar(24) NOT NULL,
	`itemid` int(11) unsigned NOT NULL default '0',
	`amount` int(11) unsigned NOT NULL default '0',
	PRIMARY KEY  ( `name`, `itemid` )
)
ENGINE = MyISAM
CHARACTER SET = latin1
ROW_FORMAT = Dynamic
;

/* Header line. Object: sc_data. Script date: 3/10/2014 7:03:15 PM. */
DROP TABLE IF EXISTS `_temp_sc_data`;

CREATE TABLE `_temp_sc_data` (
	`account_id` int(11) unsigned NOT NULL,
	`char_id` int(11) unsigned NOT NULL,
	`type` smallint(11) unsigned NOT NULL,
	`tick` int(11) NOT NULL,
	`val1` int(11) NOT NULL default '0',
	`val2` int(11) NOT NULL default '0',
	`val3` int(11) NOT NULL default '0',
	`val4` int(11) NOT NULL default '0',
	KEY `account_id` ( `account_id` ),
	KEY `char_id` ( `char_id` ),
	PRIMARY KEY  ( `account_id`, `char_id`, `type` )
)
ENGINE = MyISAM
CHARACTER SET = latin1
ROW_FORMAT = Fixed
;

INSERT INTO `_temp_sc_data`
( `account_id`, `char_id`, `tick`, `type`, `val1`, `val2`, `val3`, `val4` )
SELECT
`account_id`, `char_id`, `tick`, `type`, `val1`, `val2`, `val3`, `val4`
FROM `sc_data`;

DROP TABLE `sc_data`;

ALTER TABLE `_temp_sc_data` RENAME `sc_data`;

/* Header line. Object: sql_updates. Script date: 3/10/2014 7:03:15 PM. */
CREATE TABLE `sql_updates` (
	`timestamp` int(11) unsigned NOT NULL,
	`ignored` enum('Yes','No') NOT NULL default 'No',
	PRIMARY KEY  ( `timestamp` )
)
ENGINE = MyISAM
CHARACTER SET = latin1
ROW_FORMAT = Fixed
;
INSERT INTO `sql_updates` (`timestamp`) VALUES (1360858500); -- 2013-02-14--16-15.sql
INSERT INTO `sql_updates` (`timestamp`) VALUES (1360951560); -- 2013-02-15--18-06.sql
INSERT INTO `sql_updates` (`timestamp`) VALUES (1362445531); -- 2013-03-05--01-05.sql
INSERT INTO `sql_updates` (`timestamp`) VALUES (1362528000); -- 2013-03-06--00-00.sql
INSERT INTO `sql_updates` (`timestamp`) VALUES (1362794218); -- 2013-03-09--01-56.sql
INSERT INTO `sql_updates` (`timestamp`) VALUES (1364409316); -- 2013-03-27--18-35.sql
INSERT INTO `sql_updates` (`timestamp`) VALUES (1366075474); -- 2013-04-16--01-24.sql
INSERT INTO `sql_updates` (`timestamp`) VALUES (1366078541); -- 2013-04-16--02-15.sql
INSERT INTO `sql_updates` (`timestamp`) VALUES (1381354728); -- 2013-10-09--21-38.sql
INSERT INTO `sql_updates` (`timestamp`) VALUES (1381423003); -- 2013-10-10--16-36.sql
INSERT INTO `sql_updates` (`timestamp`) VALUES (1382892428); -- 2013-10-27--16-47.sql
INSERT INTO `sql_updates` (`timestamp`) VALUES (1383162785); -- 2013-10-30--19-53.sql
INSERT INTO `sql_updates` (`timestamp`) VALUES (1383167577); -- 2013-10-30--21-12.sql
INSERT INTO `sql_updates` (`timestamp`) VALUES (1383205740); -- 2013-10-31--07-49.sql
INSERT INTO `sql_updates` (`timestamp`) VALUES (1383955424); -- 2013-11-09--00-03.sql
INSERT INTO `sql_updates` (`timestamp`) VALUES (1384473995); -- 2013-11-15--00-06.sql
INSERT INTO `sql_updates` (`timestamp`) VALUES (1384545461); -- 2013-11-15--19-57.sql
INSERT INTO `sql_updates` (`timestamp`) VALUES (1384588175); -- 2013-11-16--07-49.sql
INSERT INTO `sql_updates` (`timestamp`) VALUES (1384763034); -- 2013-11-18--08-23.sql
INSERT INTO `sql_updates` (`timestamp`) VALUES (1387844126); -- 2013-12-24--00-15.sql
INSERT INTO `sql_updates` (`timestamp`) VALUES (1388854043); -- 2014-01-04--16-47.sql
INSERT INTO `sql_updates` (`timestamp`) VALUES (1389028967); -- 2014-01-06--17-22.sql
INSERT INTO `sql_updates` (`timestamp`) VALUES (1392832626); -- 2014-02-19--17-57.sql

/* Header line. Object: storage. Script date: 3/10/2014 7:03:15 PM. */
DROP TABLE IF EXISTS `_temp_storage`;

CREATE TABLE `_temp_storage` (
	`id` int(11) unsigned NOT NULL auto_increment,
	`account_id` int(11) unsigned NOT NULL default '0',
	`nameid` int(11) unsigned NOT NULL default '0',
	`amount` smallint(11) unsigned NOT NULL default '0',
	`equip` int(11) unsigned NOT NULL default '0',
	`identify` smallint(6) unsigned NOT NULL default '0',
	`refine` tinyint(3) unsigned NOT NULL default '0',
	`attribute` tinyint(4) unsigned NOT NULL default '0',
	`card0` smallint(11) NOT NULL default '0',
	`card1` smallint(11) NOT NULL default '0',
	`card2` smallint(11) NOT NULL default '0',
	`card3` smallint(11) NOT NULL default '0',
	`expire_time` int(11) unsigned NOT NULL default '0',
	`bound` tinyint(1) unsigned NOT NULL default '0',
	`unique_id` bigint(20) unsigned NOT NULL default '0',
	KEY `account_id` ( `account_id` ),
	PRIMARY KEY  ( `id` )
)
ENGINE = MyISAM
CHARACTER SET = latin1
AUTO_INCREMENT = 1
ROW_FORMAT = Fixed
;

INSERT INTO `_temp_storage`
( `account_id`, `amount`, `attribute`, `bound`, `card0`, `card1`, `card2`, `card3`, `equip`, `expire_time`, `id`, `identify`, `nameid`, `refine`, `unique_id` )
SELECT
`account_id`, `amount`, `attribute`, `bound`, `card0`, `card1`, `card2`, `card3`, `equip`, `expire_time`, `id`, `identify`, `nameid`, `refine`, `unique_id`
FROM `storage`;

DROP TABLE `storage`;

ALTER TABLE `_temp_storage` RENAME `storage`;

