-- This file is part of Hercules.
-- http://herc.ws - http://github.com/HerculesWS/Hercules
--
-- Copyright (C) 2013-2015  Hercules Dev Team
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
-- Upgrade file to be used when going from eAthena to Hercules
-- Note: If you're not up to date with eAthena, go through their upgrade files first and run them before this file.
-- Note: After runing this file run Hercules upgrade files.
--

ALTER TABLE `global_reg_value` MODIFY `type` TINYINT(1) UNSIGNED NOT NULL DEFAULT '3';

-- `ExpPer` column removed from `mob_db` and `mob_db2` tables
ALTER TABLE `mob_db` DROP COLUMN `ExpPer`;
ALTER TABLE `mob_db2` DROP COLUMN `ExpPer`;

-- Rename `level` column to `group_id` in `login` table
ALTER TABLE `login` CHANGE COLUMN `level` `group_id` TINYINT(3) NOT NULL DEFAULT '0';

ALTER TABLE `char` ADD COLUMN `elemental_id` INT(11) UNSIGNED NOT NULL DEFAULT '0';

CREATE TABLE IF NOT EXISTS `elemental` (
  `ele_id` INT(11) UNSIGNED NOT NULL auto_increment,
  `char_id` INT(11) NOT NULL,
  `class` MEDIUMINT(9) UNSIGNED NOT NULL DEFAULT '0',
  `mode` INT(11) UNSIGNED NOT NULL DEFAULT '1',
  `hp` INT(12) NOT NULL DEFAULT '1',
  `sp` INT(12) NOT NULL DEFAULT '1',
  `max_hp` MEDIUMINT(8) UNSIGNED NOT NULL DEFAULT '0',
  `max_sp` MEDIUMINT(6) UNSIGNED NOT NULL DEFAULT '0',
  `str` SMALLINT(4) UNSIGNED NOT NULL DEFAULT '0',
  `agi` SMALLINT(4) UNSIGNED NOT NULL DEFAULT '0',
  `vit` SMALLINT(4) UNSIGNED NOT NULL DEFAULT '0',
  `INT` SMALLINT(4) UNSIGNED NOT NULL DEFAULT '0',
  `dex` SMALLINT(4) UNSIGNED NOT NULL DEFAULT '0',
  `luk` SMALLINT(4) UNSIGNED NOT NULL DEFAULT '0',
  `life_time` INT(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`ele_id`)
) ENGINE=MyISAM;

ALTER TABLE `char` ADD CONSTRAINT `name_key` UNIQUE (`name`);

ALTER TABLE `inventory`  ADD COLUMN `favorite` TINYINT(3) UNSIGNED NOT NULL DEFAULT '0' AFTER `expire_time`;

ALTER TABLE `item_db_re` CHANGE `equip_level` `equip_level` VARCHAR(10) DEFAULT '';

ALTER TABLE `item_db_re` MODIFY COLUMN `atk:matk` VARCHAR(11) DEFAULT '';

ALTER TABLE `item_db_re` MODIFY COLUMN `defence` SMALLINT(5) UNSIGNED DEFAULT NULL;

ALTER TABLE  `homunculus` ADD  `prev_class` MEDIUMINT( 9 ) NOT NULL AFTER  `class`;

ALTER TABLE `item_db_re` MODIFY `defence` SMALLINT(5) DEFAULT NULL;

ALTER TABLE `item_db` MODIFY `defence` SMALLINT(5) DEFAULT NULL;

ALTER TABLE `elemental` CHANGE COLUMN `str` `atk1` MEDIUMINT(6) UNSIGNED NOT NULL DEFAULT 0,
  CHANGE COLUMN `agi` `atk2` MEDIUMINT(6) UNSIGNED NOT NULL DEFAULT 0,
  CHANGE COLUMN `vit` `matk` MEDIUMINT(6) UNSIGNED NOT NULL DEFAULT 0,
  CHANGE COLUMN `INT` `aspd` SMALLINT(4) UNSIGNED NOT NULL DEFAULT 0,
  CHANGE COLUMN `dex` `def` SMALLINT(4) UNSIGNED NOT NULL DEFAULT 0,
  CHANGE COLUMN `luk` `mdef` SMALLINT(4) UNSIGNED NOT NULL DEFAULT 0,
  CHANGE COLUMN `life_time` `flee` SMALLINT(4) UNSIGNED NOT NULL DEFAULT 0,
  ADD COLUMN `hit` SMALLINT(4) UNSIGNED NOT NULL DEFAULT 0 AFTER `flee`,
  ADD COLUMN `life_time` INT(11) NOT NULL DEFAULT 0 AFTER `hit`;

CREATE TABLE IF NOT EXISTS `interreg` (
  `varname` VARCHAR(11) NOT NULL,
  `value` VARCHAR(20) NOT NULL,
  PRIMARY KEY (`varname`)
) ENGINE=InnoDB;

INSERT INTO `interreg` (`varname`, `value`) VALUES ('nsiuid', '0');

ALTER TABLE `auction` ADD `nsiuid` BIGINT NOT NULL DEFAULT '0';

ALTER TABLE `cart_inventory` ADD `nsiuid` BIGINT NOT NULL DEFAULT '0';

ALTER TABLE `guild_storage` ADD `nsiuid` BIGINT NOT NULL DEFAULT '0';

ALTER TABLE `inventory` ADD `nsiuid` BIGINT NOT NULL DEFAULT '0';

ALTER TABLE `mail` ADD `nsiuid` BIGINT NOT NULL DEFAULT '0';

ALTER TABLE `storage` ADD `nsiuid` BIGINT NOT NULL DEFAULT '0';

UPDATE `interreg` SET `varname` = 'unique_id' WHERE `interreg`.`varname` = 'nsiuid';

ALTER TABLE `auction` CHANGE `nsiuid` `unique_id` BIGINT( 20 ) NOT NULL DEFAULT '0';

ALTER TABLE `cart_inventory` CHANGE `nsiuid` `unique_id` BIGINT( 20 ) NOT NULL DEFAULT '0';

ALTER TABLE `guild_storage` CHANGE `nsiuid` `unique_id` BIGINT( 20 ) NOT NULL DEFAULT '0';

ALTER TABLE `inventory` CHANGE `nsiuid` `unique_id` BIGINT( 20 ) NOT NULL DEFAULT '0';

ALTER TABLE `mail` CHANGE `nsiuid` `unique_id` BIGINT( 20 ) NOT NULL DEFAULT '0';

ALTER TABLE `storage` CHANGE `nsiuid` `unique_id` BIGINT( 20 ) NOT NULL DEFAULT '0';

ALTER TABLE `charlog` MODIFY `time` DATETIME NULL;
ALTER TABLE `interlog` MODIFY `time` DATETIME NULL;
ALTER TABLE `ipbanlist` MODIFY `btime` DATETIME NULL;
ALTER TABLE `ipbanlist` MODIFY `rtime` DATETIME NULL;
ALTER TABLE `login` MODIFY `lastlogin` DATETIME NULL;
ALTER TABLE `login` MODIFY `birthdate` DATE NULL;

UPDATE `charlog` SET `time` = NULL WHERE `time` = '0000-00-00 00:00:00';
UPDATE `interlog` SET `time` = NULL WHERE `time` = '0000-00-00 00:00:00';
UPDATE `ipbanlist` SET `btime` = NULL WHERE `btime` = '0000-00-00 00:00:00';
UPDATE `ipbanlist` SET `rtime` = NULL WHERE `rtime` = '0000-00-00 00:00:00';
UPDATE `login` SET `lastlogin` = NULL WHERE `lastlogin` = '0000-00-00 00:00:00';
UPDATE `login` SET `birthdate` = NULL WHERE `birthdate` = '0000-00-00';
