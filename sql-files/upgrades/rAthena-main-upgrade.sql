-- rAthena to Hercules Main database upgrade query.
-- This upgrades a FULLY UPGRADED rAthena to a FULLY UPGRADED Hercules
-- Please don't use if either rAthena or Hercules launched a SQL update after last revision date of this file.
-- Remember to make a backup before applying.
-- We are not liable for any data loss this may cause.
-- Apply in the same database you applied your main.sql
-- Last revision: 3/12/2014 7:24:15 PM


/* Header line. Object: bonus_script. Script date:  3/12/2014 7:24:15 PM. */
-- Attention: Table `bonus_script` will be dropped.
DROP TABLE `bonus_script`;

/* Header line. Object: buyingstore_items. Script date:  3/12/2014 7:24:15 PM. */
-- Attention: Table `buyingstore_items` will be dropped.
DROP TABLE `buyingstore_items`;

/* Header line. Object: buyingstores. Script date:  3/12/2014 7:24:15 PM. */
-- Attention: Table `buyingstores` will be dropped.
DROP TABLE `buyingstores`;

/* Header line. Object: skillcooldown. Script date:  3/12/2014 7:24:15 PM. */
-- Attention: Table `skillcooldown` will be dropped.
DROP TABLE `skillcooldown`;

--
-- Vending Database Update
--

-- Vending_Items Update
ALTER TABLE `vending_items` ADD `char_id` INT(11) NOT NULL DEFAULT '0' AFTER `index`;

UPDATE `vending_items` v1, `vendings` v2
SET v1.char_id = v2.char_id 
WHERE v1.vending_id = v2.id;

ALTER TABLE `vending_items`
  DROP `vending_id`,
  DROP `index`,
  CHANGE `cartinventory_id` `itemkey` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  CHANGE `amount` `amount` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  CHANGE `price` `price` INT(11) UNSIGNED NOT NULL DEFAULT '0';
  
ALTER TABLE `vending_items` ADD PRIMARY KEY( `char_id`, `itemkey`);

RENAME TABLE `vending_items` TO `autotrade_data`;

-- Vending Data Update
ALTER TABLE `vendings`
  DROP `id`,
  DROP `map`,
  DROP `x`,
  DROP `y`,
  DROP `autotrade`;
  
ALTER TABLE `vendings` CHANGE `sex` `sex_ref` ENUM('F','M') CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL DEFAULT 'M';

ALTER TABLE `vendings` CHANGE `account_id` `account_id` INT(11) NOT NULL DEFAULT '0', CHANGE `char_id` `char_id` INT(11) NOT NULL DEFAULT '0', ADD `sex` TINYINT(2) NOT NULL DEFAULT '0' AFTER `char_id`, CHANGE `title` `title` VARCHAR(80) NOT NULL DEFAULT 'Buy From Me!';

UPDATE  `vendings` SET `sex` = 0 WHERE `sex_ref` = 'F'; UPDATE  `vendings` SET `sex` = 1 WHERE `sex_ref` = 'M';

ALTER TABLE `vendings` DROP `sex_ref`;

ALTER TABLE `vendings` ADD PRIMARY KEY( `account_id`, `char_id`);

RENAME TABLE `vendings` TO `autotrade_merchants`;

-- Before Dropping global_reg_value

CREATE TABLE IF NOT EXISTS `acc_reg_num_db` (
  `account_id` int(11) unsigned NOT NULL default '0',
  `key` varchar(32) BINARY NOT NULL default '',
  `index` int(11) unsigned NOT NULL default '0',
  `value` int(11) NOT NULL default '0',
  PRIMARY KEY  (`account_id`,`key`,`index`),
  KEY `account_id` (`account_id`)
) ENGINE=MyISAM;

CREATE TABLE IF NOT EXISTS `acc_reg_str_db` (
  `account_id` int(11) unsigned NOT NULL default '0',
  `key` varchar(32) BINARY NOT NULL default '',
  `index` int(11) unsigned NOT NULL default '0',
  `value` varchar(254) NOT NULL default '0',
  PRIMARY KEY  (`account_id`,`key`,`index`),
  KEY `account_id` (`account_id`)
) ENGINE=MyISAM;

CREATE TABLE IF NOT EXISTS `char_reg_num_db` (
  `char_id` int(11) unsigned NOT NULL default '0',
  `key` varchar(32) BINARY NOT NULL default '',
  `index` int(11) unsigned NOT NULL default '0',
  `value` int(11) NOT NULL default '0',
  PRIMARY KEY  (`char_id`,`key`,`index`),
  KEY `char_id` (`char_id`)
) ENGINE=MyISAM;

CREATE TABLE IF NOT EXISTS `char_reg_str_db` (
  `char_id` int(11) unsigned NOT NULL default '0',
  `key` varchar(32) BINARY NOT NULL default '',
  `index` int(11) unsigned NOT NULL default '0',
  `value` varchar(254) NOT NULL default '0',
  PRIMARY KEY  (`char_id`,`key`,`index`),
  KEY `char_id` (`char_id`)
) ENGINE=MyISAM;

CREATE TABLE IF NOT EXISTS `global_acc_reg_num_db` (
  `account_id` int(11) unsigned NOT NULL default '0',
  `key` varchar(32) BINARY NOT NULL default '',
  `index` int(11) unsigned NOT NULL default '0',
  `value` int(11) NOT NULL default '0',
  PRIMARY KEY  (`account_id`,`key`,`index`),
  KEY `account_id` (`account_id`)
) ENGINE=MyISAM;

CREATE TABLE IF NOT EXISTS `global_acc_reg_str_db` (
  `account_id` int(11) unsigned NOT NULL default '0',
  `key` varchar(32) BINARY NOT NULL default '',
  `index` int(11) unsigned NOT NULL default '0',
  `value` varchar(254) NOT NULL default '0',
  PRIMARY KEY  (`account_id`,`key`,`index`),
  KEY `account_id` (`account_id`)
) ENGINE=MyISAM;

INSERT INTO `acc_reg_num_db` (`account_id`, `key`, `index`, `value`) SELECT `account_id`, `str`, 0, `value` FROM `global_reg_value` WHERE `type` = 2 AND `str` NOT LIKE '%$';
INSERT INTO `acc_reg_str_db` (`account_id`, `key`, `index`, `value`) SELECT `account_id`, `str`, 0, `value` FROM `global_reg_value` WHERE `type` = 2 AND `str` LIKE '%$';
INSERT INTO `char_reg_num_db` (`char_id`, `key`, `index`, `value`) SELECT `char_id`, `str`, 0, `value` FROM `global_reg_value` WHERE `type` = 3 AND `str` NOT LIKE '%$';
INSERT INTO `char_reg_str_db` (`char_id`, `key`, `index`, `value`) SELECT `char_id`, `str`, 0, `value` FROM `global_reg_value` WHERE `type` = 3 AND `str` LIKE '%$';
INSERT INTO `global_acc_reg_num_db` (`account_id`, `key`, `index`, `value`) SELECT `account_id`, `str`, 0, `value` FROM `global_reg_value` WHERE `type` = 1 AND `str` NOT LIKE '%$';
INSERT INTO `global_acc_reg_str_db` (`account_id`, `key`, `index`, `value`) SELECT `account_id`, `str`, 0, `value` FROM `global_reg_value` WHERE `type` = 1 AND `str` LIKE '%$';
DROP TABLE `global_reg_value`;

/* Header line. Object: account_data. Script date:  3/12/2014 7:24:15 PM. */
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

/* Header line. Object: cart_inventory. Script date:  3/12/2014 7:24:15 PM. */
ALTER TABLE cart_inventory CHANGE COLUMN bound bound TINYINT(1) UNSIGNED DEFAULT 0 NOT NULL COMMENT '';

/* Header line. Object: char. Script date:  3/12/2014 7:24:15 PM. */
ALTER TABLE char
ADD slotchange SMALLINT(3) UNSIGNED DEFAULT 0 NOT NULL;
ALTER TABLE char
ADD char_opt INT(11) UNSIGNED DEFAULT 0 NOT NULL;
ALTER TABLE char
ADD font TINYINT(3) UNSIGNED DEFAULT 0 NOT NULL;
ALTER TABLE char DROP COLUMN moves;

/* Header line. Object: guild_storage. Script date:  3/12/2014 7:24:15 PM. */
ALTER TABLE guild_storage CHANGE COLUMN bound bound TINYINT(1) UNSIGNED DEFAULT 0 NOT NULL COMMENT '';

/* Header line. Object: inventory. Script date:  3/12/2014 7:24:15 PM. */
ALTER TABLE inventory CHANGE COLUMN bound bound TINYINT(1) UNSIGNED DEFAULT 0 NOT NULL COMMENT '';

/* Header line. Object: login. Script date:  3/12/2014 7:24:15 PM. */
INSERT INTO `account_data` (`account_id`, `bank_vault`) SELECT `account_id`, `bank_vault` FROM `login`;
ALTER TABLE login DROP COLUMN bank_vault;
ALTER TABLE login DROP COLUMN vip_time;
ALTER TABLE login DROP COLUMN old_group;

/* Header line. Object: mapreg. Script date:  3/12/2014 7:24:15 PM. */
ALTER TABLE mapreg ADD PRIMARY KEY (varname,index);
DROP INDEX index ON mapreg;
DROP INDEX varname ON mapreg;

/* Header line. Object: npc_market_data. Script date:  3/12/2014 7:24:15 PM. */
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

/* Header line. Object: sc_data. Script date:  3/12/2014 7:24:15 PM. */
ALTER TABLE sc_data ADD PRIMARY KEY (account_id,char_id,type);

/* Header line. Object: sql_updates. Script date:  3/12/2014 7:24:15 PM. */
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

/* Header line. Object: storage. Script date:  3/12/2014 7:24:15 PM. */
ALTER TABLE storage CHANGE COLUMN bound bound TINYINT(1) UNSIGNED DEFAULT 0 NOT NULL COMMENT '';

