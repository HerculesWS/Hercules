-- rAthena to Hercules main database upgrade query.
-- This upgrades a FULLY UPGRADED rAthena to a FULLY UPGRADED Hercules
-- Please don't use if either rAthena or Hercules launched a SQL update after last revision date of this file.
-- Remember to make a backup before applying.
-- We are not liable for any data loss this may cause.
-- Apply in the same database you applied your main.sql

-- Drop table contents from ´sc_data´ since we use a different status order than rAthena
-- /!\ WARNING /!\ This will remove _ALL_ of the status effects active on the server
-- This will remove even jailed status from users!
TRUNCATE TABLE `sc_data`;


-- Drop table `skillcooldown` since it's not used in Hercules
DROP TABLE IF EXISTS `skillcooldown`;

-- Upgrades for table `cart_inventory`
ALTER TABLE `cart_inventory` MODIFY `bound` tinyint(1) unsigned NOT NULL default '0';

-- Upgrades for table `char`
ALTER TABLE `char` CHANGE `moves` `slotchange` SMALLINT(3) UNSIGNED NOT NULL default '0',
	ADD `char_opt` INT( 11 ) UNSIGNED NOT NULL default '0',
	ADD `font` TINYINT( 3 ) UNSIGNED NOT NULL DEFAULT  '0';
	ADD `unban_time` int(11) unsigned NOT NULL default '0';

-- Upgrades for table `guild_storage`
ALTER TABLE `guild_storage` MODIFY `bound` tinyint(1) unsigned NOT NULL default '0';

-- Upgrades for table `inventory`
ALTER TABLE `inventory` MODIFY `bound` tinyint(1) unsigned NOT NULL default '0';

-- Bank vault is saved later since we need to make a table rAthena doesn't have first

--
-- Table structure for table `sql_updates`
--
CREATE TABLE IF NOT EXISTS `sql_updates` (
  `timestamp` int(11) unsigned NOT NULL,
  `ignored` enum('Yes','No') NOT NULL DEFAULT 'No',
  PRIMARY KEY (`timestamp`)
) ENGINE=MyISAM;

-- Existent updates to enter
INSERT INTO `sql_updates` (`timestamp`) VALUES (1360858500);
INSERT INTO `sql_updates` (`timestamp`) VALUES (1360951560);
INSERT INTO `sql_updates` (`timestamp`) VALUES (1362445531);
INSERT INTO `sql_updates` (`timestamp`) VALUES (1362528000);
INSERT INTO `sql_updates` (`timestamp`) VALUES (1362794218);
INSERT INTO `sql_updates` (`timestamp`) VALUES (1364409316);
INSERT INTO `sql_updates` (`timestamp`) VALUES (1366075474);
INSERT INTO `sql_updates` (`timestamp`) VALUES (1366078541);
INSERT INTO `sql_updates` (`timestamp`) VALUES (1381354728);
INSERT INTO `sql_updates` (`timestamp`) VALUES (1381423003);
INSERT INTO `sql_updates` (`timestamp`) VALUES (1382892428);
INSERT INTO `sql_updates` (`timestamp`) VALUES (1383162785);
INSERT INTO `sql_updates` (`timestamp`) VALUES (1383167577);
INSERT INTO `sql_updates` (`timestamp`) VALUES (1383205740);
INSERT INTO `sql_updates` (`timestamp`) VALUES (1383955424);
INSERT INTO `sql_updates` (`timestamp`) VALUES (1384545461);
INSERT INTO `sql_updates` (`timestamp`) VALUES (1384588175);

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
  PRIMARY KEY  (`account_id`)
) ENGINE=MyISAM; 

-- Saving bank_vault data from rAthena's login table
-- to our account_data table. There may be some not working cases.

INSERT INTO `account_data` (`account_id`, `bank_vault`) SELECT `account_id`, `bank_vault` FROM `login` WHERE `bank_vault` > 0 ;

-- Dropping bank_vault column from login table
ALTER TABLE `login` DROP COLUMN `bank_vault`;

-- Drop table `bonus_script` since it's not used in Hercules
DROP TABLE IF EXISTS `bonus_script`;
