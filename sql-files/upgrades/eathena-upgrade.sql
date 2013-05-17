#####
#Upgrade file to be used when going from eAthena to Hercules
#Note: If you're not up to date with eAthena, go through their upgrade files first and run them before this file.
#Note: After runing this file run Hercules upgrade files.
#####
ALTER TABLE `global_reg_value` MODIFY `type` TINYINT(1) UNSIGNED NOT NULL DEFAULT '3';
-- Adds 'I' and 'X' to `type` in `picklog` table
ALTER TABLE `picklog` MODIFY `type` ENUM('M','P','L','T','V','S','N','C','A','R','G','E','B','O','I','X') NOT NULL DEFAULT 'P';
-- Adds 'D' and 'U' to `type` in `picklog` table
ALTER TABLE `picklog` MODIFY `type` ENUM('M','P','L','T','V','S','N','C','A','R','G','E','B','O','I','X','D','U') NOT NULL DEFAULT 'P';
-- `ExpPer` column removed from `mob_db` and `mob_db2` tables
ALTER TABLE `mob_db` DROP COLUMN `ExpPer`;
ALTER TABLE `mob_db2` DROP COLUMN `ExpPer`;
-- Rename `level` column to `group_id` in `login` table
ALTER TABLE `login` CHANGE COLUMN `level` `group_id` TINYINT(3) NOT NULL DEFAULT '0';
-- Adds 'I' to `type` in `zenylog`
ALTER TABLE `zenylog` MODIFY `type` ENUM('M','T','V','S','N','A','E','B','I') NOT NULL DEFAULT 'S';
ALTER TABLE `char` ADD COLUMN `elemental_id` int(11) unsigned NOT NULL default '0';
CREATE TABLE IF NOT EXISTS `elemental` (
  `ele_id` int(11) unsigned NOT NULL auto_increment,
  `char_id` int(11) NOT NULL,
  `class` mediumint(9) unsigned NOT NULL default '0',
  `mode` int(11) unsigned NOT NULL default '1',
  `hp` int(12) NOT NULL default '1',
  `sp` int(12) NOT NULL default '1',
  `max_hp` mediumint(8) unsigned NOT NULL default '0',
  `max_sp` mediumint(6) unsigned NOT NULL default '0',
  `str` smallint(4) unsigned NOT NULL default '0',
  `agi` smallint(4) unsigned NOT NULL default '0',
  `vit` smallint(4) unsigned NOT NULL default '0',
  `int` smallint(4) unsigned NOT NULL default '0',
  `dex` smallint(4) unsigned NOT NULL default '0',
  `luk` smallint(4) unsigned NOT NULL default '0',
  `life_time` int(11) NOT NULL default '0',
  PRIMARY KEY  (`ele_id`)
) ENGINE=MyISAM;
-- Adds 'D' to `type` in `zenylog`
ALTER TABLE `zenylog` MODIFY `type` ENUM('M','T','V','S','N','A','E','B','I','D') NOT NULL DEFAULT 'S';
ALTER TABLE `char` ADD CONSTRAINT `name_key` UNIQUE (`name`);
ALTER TABLE `inventory`  ADD COLUMN `favorite` TINYINT(3) UNSIGNED NOT NULL DEFAULT '0' AFTER `expire_time`;
ALTER TABLE `item_db_re` CHANGE `equip_level` `equip_level` VARCHAR(10) DEFAULT '';
ALTER TABLE `item_db_re` MODIFY COLUMN `atk:matk` VARCHAR(11) DEFAULT '';
ALTER TABLE `item_db_re` MODIFY COLUMN `defence` SMALLINT(5) UNSIGNED DEFAULT NULL;
ALTER TABLE  `homunculus` ADD  `prev_class` MEDIUMINT( 9 ) NOT NULL AFTER  `class`
ALTER TABLE `item_db_re` MODIFY `defence` SMALLINT(5) DEFAULT NULL;
ALTER TABLE `item_db` MODIFY `defence` SMALLINT(5) DEFAULT NULL;
ALTER TABLE `zenylog` MODIFY `type` ENUM('T','V','P','M','S','N','D','C','A','E','I','B') NOT NULL DEFAULT 'S';
ALTER TABLE `elemental` CHANGE COLUMN `str` `atk1` MEDIUMINT(6) UNSIGNED NOT NULL DEFAULT 0,
 CHANGE COLUMN `agi` `atk2` MEDIUMINT(6) UNSIGNED NOT NULL DEFAULT 0,
 CHANGE COLUMN `vit` `matk` MEDIUMINT(6) UNSIGNED NOT NULL DEFAULT 0,
 CHANGE COLUMN `int` `aspd` SMALLINT(4) UNSIGNED NOT NULL DEFAULT 0,
 CHANGE COLUMN `dex` `def` SMALLINT(4) UNSIGNED NOT NULL DEFAULT 0,
 CHANGE COLUMN `luk` `mdef` SMALLINT(4) UNSIGNED NOT NULL DEFAULT 0,
 CHANGE COLUMN `life_time` `flee` SMALLINT(4) UNSIGNED NOT NULL DEFAULT 0,
 ADD COLUMN `hit` SMALLINT(4) UNSIGNED NOT NULL DEFAULT 0 AFTER `flee`,
 ADD COLUMN `life_time` INT(11) NOT NULL DEFAULT 0 AFTER `hit`;
ALTER TABLE `picklog` ADD `nsiuid` BIGINT NOT NULL DEFAULT '0' AFTER `card3`;
CREATE TABLE IF NOT EXISTS `interreg` (
  `varname` varchar(11) NOT NULL,
  `value` varchar(20) NOT NULL,
  PRIMARY KEY (`varname`)
) ENGINE=InnoDB;
INSERT INTO `interreg` (`varname`, `value`) VALUES
('nsiuid', '0');
ALTER TABLE `auction` ADD `nsiuid` BIGINT NOT NULL DEFAULT '0';
ALTER TABLE `cart_inventory` ADD `nsiuid` BIGINT NOT NULL DEFAULT '0';
ALTER TABLE `guild_storage` ADD `nsiuid` BIGINT NOT NULL DEFAULT '0';
ALTER TABLE `inventory` ADD `nsiuid` BIGINT NOT NULL DEFAULT '0';
ALTER TABLE `mail` ADD `nsiuid` BIGINT NOT NULL DEFAULT '0';
ALTER TABLE `storage` ADD `nsiuid` BIGINT NOT NULL DEFAULT '0';
ALTER TABLE `picklog` CHANGE `nsiuid` `unique_id` BIGINT( 20 ) NOT NULL DEFAULT '0';
UPDATE `interreg` SET `varname` = 'unique_id' WHERE `interreg`.`varname` = 'nsiuid';
ALTER TABLE `auction` CHANGE `nsiuid` `unique_id` BIGINT( 20 ) NOT NULL DEFAULT '0';
ALTER TABLE `cart_inventory` CHANGE `nsiuid` `unique_id` BIGINT( 20 ) NOT NULL DEFAULT '0';
ALTER TABLE `guild_storage` CHANGE `nsiuid` `unique_id` BIGINT( 20 ) NOT NULL DEFAULT '0';
ALTER TABLE `inventory` CHANGE `nsiuid` `unique_id` BIGINT( 20 ) NOT NULL DEFAULT '0';
ALTER TABLE `mail` CHANGE `nsiuid` `unique_id` BIGINT( 20 ) NOT NULL DEFAULT '0';
ALTER TABLE `storage` CHANGE `nsiuid` `unique_id` BIGINT( 20 ) NOT NULL DEFAULT '0';
