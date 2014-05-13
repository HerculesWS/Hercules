-- rAthena to Hercules main database upgrade query.
-- This upgrades a FULLY UPGRADED rAthena to a FULLY UPGRADED Hercules
-- Please don't use if either rAthena or Hercules launched a SQL update after last revision date of this file.
-- Remember to make a backup before applying.
-- We are not liable for any data loss this may cause.
-- Apply in the same database you applied your main.sql
-- Last revised: April 26, 2014 12:29 GMT

-- Drop table contents from `sc_data` since we use a different status order than rAthena
-- /!\ WARNING /!\ This will remove _ALL_ of the status effects active on the server
-- You can disable this, but this is a SECURITY MEASURE
-- This will remove even jailed status from users!
TRUNCATE TABLE `sc_data`;

-- Drop table `skillcooldown` since it's not used in Hercules
DROP TABLE IF EXISTS `skillcooldown`;


-- Upgrades for table `cart_inventory`
ALTER TABLE `cart_inventory` MODIFY `bound` tinyint(1) unsigned NOT NULL default '0';


-- Upgrades for table `char`
ALTER TABLE `char` CHANGE `moves` `slotchange` SMALLINT(3) UNSIGNED NOT NULL default '0',
	ADD `char_opt` INT( 11 ) UNSIGNED NOT NULL default '0' AFTER `slotchange`,
	ADD `font` TINYINT( 3 ) UNSIGNED NOT NULL DEFAULT '0' AFTER `char_opt`,
	MODIFY `uniqueitem_counter` BIGINT(20) UNSIGNED NOT NULL DEFAULT '0';


-- Upgrades for table `charlog`
ALTER TABLE `charlog` ADD COLUMN `char_id` int(11) unsigned NOT NULL default '0' AFTER `account_id`;


-- Upgrades for table `guild_storage`
ALTER TABLE `guild_storage` MODIFY `bound` tinyint(1) unsigned NOT NULL default '0';


-- Upgrades for table `inventory`
ALTER TABLE `inventory` MODIFY `bound` tinyint(1) unsigned NOT NULL default '0';


-- Login table will be upgraded at a later point on this file
-- so that we can save the bank vault.


-- Upgrades for table `mapreg`
ALTER TABLE `mapreg` MODIFY `varname` varchar(32) BINARY NOT NULL,
	DROP KEY `varname`,
	DROP KEY `index`,
	ADD PRIMARY KEY  (`varname`,`index`);


-- Upgrades for table `sc_data`
ALTER TABLE `sc_data` ADD PRIMARY KEY  (`account_id`,`char_id`,`type`);


--
-- Table structure for table `sql_updates`
--

CREATE TABLE IF NOT EXISTS `sql_updates` (
  `timestamp` int(11) unsigned NOT NULL,
  `ignored` enum('Yes','No') NOT NULL DEFAULT 'No',
  PRIMARY KEY (`timestamp`)
) ENGINE=MyISAM;

-- Existent updates to enter
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
INSERT INTO `sql_updates` (`timestamp`) VALUES (1395789302); -- 2014-03-25--23-57.sql
INSERT INTO `sql_updates` (`timestamp`) VALUES (1396893866); -- 2014-04-07--22-04.sql
INSERT INTO `sql_updates` (`timestamp`) VALUES (1398477600); -- 2014-04-26--10-00.sql


-- Updates to table `storage`
ALTER TABLE `storage` MODIFY `bound` tinyint(1) unsigned NOT NULL default '0';


--
-- Table structure for table `account_data`
--

CREATE TABLE IF NOT EXISTS `account_data` (
  `account_id` int(11) unsigned NOT NULL default '0',
  `bank_vault` int(11) unsigned NOT NULL default '0',
  `base_exp` TINYINT( 4 ) UNSIGNED NOT NULL default '100',
  `base_drop` TINYINT( 4 ) UNSIGNED NOT NULL default '100',
  `base_death` TINYINT( 4 ) UNSIGNED NOT NULL default '100',
  PRIMARY KEY (`account_id`)
) ENGINE=MyISAM;

-- Saving bank_vault data from rAthena's login table
-- to our account_data table. There may be some not working cases.
INSERT INTO `account_data` (`account_id`, `bank_vault`) SELECT `account_id`, `bank_vault` FROM `login` WHERE `bank_vault` > 0 ;


-- Upgrades for table `login`
ALTER TABLE `login` DROP COLUMN `vip_time`,
	DROP COLUMN `old_group`,
	DROP COLUMN `bank_vault`;


-- Drop table `bonus_script` since it's not used in Hercules
DROP TABLE IF EXISTS `bonus_script`;


--
-- Table structure for table `npc_market_data`
--

CREATE TABLE IF NOT EXISTS `npc_market_data` (
  `name` varchar(24) NOT NULL default '',
  `itemid` int(11) unsigned NOT NULL default '0',
  `amount` int(11) unsigned NOT NULL default '0',
  PRIMARY KEY  (`name`,`itemid`)
) ENGINE=MyISAM;


-- Autotrade saving. Very special thanks to Dastgir Pojee!
--
-- Vending Database Update
--

-- Vending_Items Update
ALTER TABLE `vending_items`
  ADD `char_id` INT(11) NOT NULL DEFAULT '0' AFTER `index`;

UPDATE `vending_items` v1, `vendings` v2
  SET v1.char_id = v2.char_id
  WHERE v1.vending_id = v2.id;

ALTER TABLE `vending_items`
  DROP `vending_id`,
  DROP `index`,
  CHANGE `cartinventory_id` `itemkey` INT(11) NOT NULL DEFAULT '0',
  MODIFY `amount` INT(11) NOT NULL DEFAULT '0',
  MODIFY `price` INT(11) NOT NULL DEFAULT '0';

ALTER TABLE `vending_items`
  ADD PRIMARY KEY( `char_id`, `itemkey`);

RENAME TABLE `vending_items` TO `autotrade_data`;

-- Vending Data Update
ALTER TABLE `vendings`
  DROP `id`,
  DROP `map`,
  DROP `x`,
  DROP `y`,
  DROP `autotrade`;
  
ALTER TABLE `vendings`
  CHANGE `sex` `sex_ref` ENUM('F','M') CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL DEFAULT 'M';

ALTER TABLE `vendings`
  MODIFY `account_id` INT(11) NOT NULL DEFAULT '0', 
  MODIFY `char_id` INT(11) NOT NULL DEFAULT '0',
  ADD `sex` TINYINT(2) NOT NULL DEFAULT '0' AFTER `char_id`,
  MODIFY `title` VARCHAR(80) NOT NULL DEFAULT 'Buy From Me!';

UPDATE `vendings`
  SET `sex` = 0
  WHERE `sex_ref` = 'F';
  
UPDATE `vendings`
  SET `sex` = 1
  WHERE `sex_ref` = 'M';

ALTER TABLE `vendings` DROP `sex_ref`;

ALTER TABLE `vendings` ADD PRIMARY KEY( `account_id`, `char_id`);

RENAME TABLE `vendings` TO `autotrade_merchants`;

-- Autotrade saving ended


-- We don't support saving buyingstores yet...
-- Comment next statement if you want to preserve them anyways
DROP TABLE IF EXISTS `buyingstores`, `buyingstore_items`;


-- Saving contents of `global_reg_value`

--
-- Table structure for table `acc_reg_num_db`
--

CREATE TABLE IF NOT EXISTS `acc_reg_num_db` (
  `account_id` int(11) unsigned NOT NULL default '0',
  `key` varchar(32) BINARY NOT NULL default '',
  `index` int(11) unsigned NOT NULL default '0',
  `value` int(11) NOT NULL default '0',
  PRIMARY KEY  (`account_id`,`key`,`index`),
  KEY `account_id` (`account_id`)
) ENGINE=MyISAM;


--
-- Table structure for table `acc_reg_str_db`
--

CREATE TABLE IF NOT EXISTS `acc_reg_str_db` (
  `account_id` int(11) unsigned NOT NULL default '0',
  `key` varchar(32) BINARY NOT NULL default '',
  `index` int(11) unsigned NOT NULL default '0',
  `value` varchar(254) NOT NULL default '0',
  PRIMARY KEY  (`account_id`,`key`,`index`),
  KEY `account_id` (`account_id`)
) ENGINE=MyISAM;


--
-- Table structure for table `char_reg_num_db`
--

CREATE TABLE IF NOT EXISTS `char_reg_num_db` (
  `char_id` int(11) unsigned NOT NULL default '0',
  `key` varchar(32) BINARY NOT NULL default '',
  `index` int(11) unsigned NOT NULL default '0',
  `value` int(11) NOT NULL default '0',
  PRIMARY KEY  (`char_id`,`key`,`index`),
  KEY `char_id` (`char_id`)
) ENGINE=MyISAM;


--
-- Table structure for table `char_reg_str_db`
--

CREATE TABLE IF NOT EXISTS `char_reg_str_db` (
  `char_id` int(11) unsigned NOT NULL default '0',
  `key` varchar(32) BINARY NOT NULL default '',
  `index` int(11) unsigned NOT NULL default '0',
  `value` varchar(254) NOT NULL default '0',
  PRIMARY KEY  (`char_id`,`key`,`index`),
  KEY `char_id` (`char_id`)
) ENGINE=MyISAM;


--
-- Table structure for table `global_acc_reg_num_db`
--

CREATE TABLE IF NOT EXISTS `global_acc_reg_num_db` (
  `account_id` int(11) unsigned NOT NULL default '0',
  `key` varchar(32) BINARY NOT NULL default '',
  `index` int(11) unsigned NOT NULL default '0',
  `value` int(11) NOT NULL default '0',
  PRIMARY KEY  (`account_id`,`key`,`index`),
  KEY `account_id` (`account_id`)
) ENGINE=MyISAM;


--
-- Table structure for table `global_acc_reg_str_db`
--

CREATE TABLE IF NOT EXISTS `global_acc_reg_str_db` (
  `account_id` int(11) unsigned NOT NULL default '0',
  `key` varchar(32) BINARY NOT NULL default '',
  `index` int(11) unsigned NOT NULL default '0',
  `value` varchar(254) NOT NULL default '0',
  PRIMARY KEY  (`account_id`,`key`,`index`),
  KEY `account_id` (`account_id`)
) ENGINE=MyISAM;


-- Saving the data
INSERT INTO `acc_reg_num_db` (`account_id`, `key`, `index`, `value`) SELECT `account_id`, `str`, 0, `value` FROM `global_reg_value` WHERE `type` = 2 AND `str` NOT LIKE '%$';
INSERT INTO `acc_reg_str_db` (`account_id`, `key`, `index`, `value`) SELECT `account_id`, `str`, 0, `value` FROM `global_reg_value` WHERE `type` = 2 AND `str` LIKE '%$';
INSERT INTO `char_reg_num_db` (`char_id`, `key`, `index`, `value`) SELECT `char_id`, `str`, 0, `value` FROM `global_reg_value` WHERE `type` = 3 AND `str` NOT LIKE '%$';
INSERT INTO `char_reg_str_db` (`char_id`, `key`, `index`, `value`) SELECT `char_id`, `str`, 0, `value` FROM `global_reg_value` WHERE `type` = 3 AND `str` LIKE '%$';
INSERT INTO `global_acc_reg_num_db` (`account_id`, `key`, `index`, `value`) SELECT `account_id`, `str`, 0, `value` FROM `global_reg_value` WHERE `type` = 1 AND `str` NOT LIKE '%$';
INSERT INTO `global_acc_reg_str_db` (`account_id`, `key`, `index`, `value`) SELECT `account_id`, `str`, 0, `value` FROM `global_reg_value` WHERE `type` = 1 AND `str` LIKE '%$';

-- Dropping now useless table
DROP TABLE `global_reg_value`;
