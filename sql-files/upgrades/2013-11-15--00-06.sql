#1384473995


-- This file is part of Hercules.
-- http://herc.ws - http://github.com/HerculesWS/Hercules
--
-- Copyright (C) 2013-2015  Hercules Dev Team
-- Copyright (C) 2013  Haru <haru@dotalux.com>
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

-- Note: If you're running a MySQL version earlier than 5.0 (or if this scripts fails for you for any reason)
--       you'll need to run the following queries manually:
--
-- [ Pre-Renewal only ]
-- ALTER TABLE item_db2 ADD COLUMN `matk` SMALLINT(5) UNSIGNED DEFAULT NULL AFTER atk;
-- ALTER TABLE item_db2 CHANGE COLUMN `equip_level` `equip_level_min` SMALLINT(5) UNSIGNED DEFAULT NULL;
-- ALTER TABLE item_db2 ADD COLUMN `equip_level_max` SMALLINT(5) UNSIGNED DEFAULT NULL AFTER equip_level_min;
-- [ Both Pre-Renewal and Renewal ]
-- ALTER TABLE item_db2 MODIFY COLUMN `price_buy` MEDIUMINT(10) DEFAULT NULL;
-- ALTER TABLE item_db2 MODIFY COLUMN `price_sell` MEDIUMINT(10) DEFAULT NULL;
-- ALTER TABLE item_db2 MODIFY COLUMN `weight` SMALLINT(5) UNSIGNED DEFAULT NULL;
-- ALTER TABLE item_db2 MODIFY COLUMN `atk` SMALLINT(5) UNSIGNED DEFAULT NULL;
-- ALTER TABLE item_db2 MODIFY COLUMN `matk` SMALLINT(5) UNSIGNED DEFAULT NULL;
-- ALTER TABLE item_db2 MODIFY COLUMN `defence` SMALLINT(5) UNSIGNED DEFAULT NULL;
-- ALTER TABLE item_db2 MODIFY COLUMN `range` TINYINT(2) UNSIGNED DEFAULT NULL;
-- ALTER TABLE item_db2 MODIFY COLUMN `slots` TINYINT(2) UNSIGNED DEFAULT NULL;
-- ALTER TABLE item_db2 MODIFY COLUMN `equip_jobs` INT(12) UNSIGNED DEFAULT NULL;
-- ALTER TABLE item_db2 MODIFY COLUMN `equip_upper` TINYINT(8) UNSIGNED DEFAULT NULL;
-- ALTER TABLE item_db2 MODIFY COLUMN `equip_genders` TINYINT(2) UNSIGNED DEFAULT NULL;
-- ALTER TABLE item_db2 MODIFY COLUMN `equip_locations` SMALLINT(4) UNSIGNED DEFAULT NULL;
-- ALTER TABLE item_db2 MODIFY COLUMN `weapon_level` TINYINT(2) UNSIGNED DEFAULT NULL;
-- ALTER TABLE item_db2 MODIFY COLUMN `equip_level_min` SMALLINT(5) UNSIGNED DEFAULT NULL;
-- ALTER TABLE item_db2 MODIFY COLUMN `equip_level_max` SMALLINT(5) UNSIGNED DEFAULT NULL;
-- ALTER TABLE item_db2 MODIFY COLUMN `refineable` TINYINT(1) UNSIGNED DEFAULT NULL;
-- ALTER TABLE item_db2 MODIFY COLUMN `view` SMALLINT(3) UNSIGNED DEFAULT NULL;
-- INSERT INTO `sql_updates` (`timestamp`) VALUES (1384473995);
--
-- [ End ]
-- What follows is the automated script that does all of the above.

DELIMITER $$

DROP PROCEDURE IF EXISTS alter_if_not_exists $$
DROP PROCEDURE IF EXISTS alter_if_exists $$

CREATE PROCEDURE alter_if_not_exists(my_table TINYTEXT, my_column TINYTEXT, my_command TINYTEXT, my_predicate TEXT)
BEGIN
  set @dbname = DATABASE();
  IF EXISTS (
    SELECT * FROM information_schema.TABLES
      WHERE TABLE_SCHEMA = @dbname
        AND TABLE_NAME = my_table
  ) AND NOT EXISTS (
    SELECT * FROM information_schema.COLUMNS
      WHERE TABLE_SCHEMA = @dbname
        AND TABLE_NAME = my_table
        AND COLUMN_NAME = my_column
  )
  THEN
    SET @q = CONCAT('ALTER TABLE ', @dbname, '.', my_table, ' ',
      my_command, ' `', my_column, '` ', my_predicate);
    PREPARE STMT FROM @q;
    EXECUTE STMT;
  END IF;

END $$

CREATE PROCEDURE alter_if_exists(my_table TINYTEXT, my_column TINYTEXT, my_command TINYTEXT, my_predicate TEXT)
BEGIN
  set @dbname = DATABASE();
  IF EXISTS (
    SELECT * FROM information_schema.COLUMNS
      WHERE TABLE_SCHEMA = @dbname
        AND TABLE_NAME = my_table
        AND COLUMN_NAME = my_column
  )
  THEN
    SET @q = CONCAT('ALTER TABLE ', @dbname, '.', my_table, ' ',
      my_command, ' `', my_column, '` ', my_predicate);
    PREPARE STMT FROM @q;
    EXECUTE STMT;
  END IF;

END $$

CALL alter_if_not_exists('item_db2', 'matk', 'ADD COLUMN', 'SMALLINT(5) UNSIGNED DEFAULT NULL AFTER atk') $$
CALL alter_if_exists('item_db2', 'equip_level', 'CHANGE COLUMN', 'equip_level_min SMALLINT(5) UNSIGNED DEFAULT NULL') $$
CALL alter_if_not_exists('item_db2', 'equip_level_max', 'ADD COLUMN', 'SMALLINT(5) UNSIGNED DEFAULT NULL AFTER equip_level_min') $$

CALL alter_if_exists('item_db2', 'price_buy', 'MODIFY COLUMN', 'MEDIUMINT(10) DEFAULT NULL') $$
CALL alter_if_exists('item_db2', 'price_sell', 'MODIFY COLUMN', 'MEDIUMINT(10) DEFAULT NULL') $$
CALL alter_if_exists('item_db2', 'weight', 'MODIFY COLUMN', 'SMALLINT(5) UNSIGNED DEFAULT NULL') $$
CALL alter_if_exists('item_db2', 'atk', 'MODIFY COLUMN', 'SMALLINT(5) UNSIGNED DEFAULT NULL') $$
CALL alter_if_exists('item_db2', 'matk', 'MODIFY COLUMN', 'SMALLINT(5) UNSIGNED DEFAULT NULL') $$
CALL alter_if_exists('item_db2', 'defence', 'MODIFY COLUMN', 'SMALLINT(5) UNSIGNED DEFAULT NULL') $$
CALL alter_if_exists('item_db2', 'range', 'MODIFY COLUMN', 'TINYINT(2) UNSIGNED DEFAULT NULL') $$
CALL alter_if_exists('item_db2', 'slots', 'MODIFY COLUMN', 'TINYINT(2) UNSIGNED DEFAULT NULL') $$
CALL alter_if_exists('item_db2', 'equip_jobs', 'MODIFY COLUMN', 'INT(12) UNSIGNED DEFAULT NULL') $$
CALL alter_if_exists('item_db2', 'equip_upper', 'MODIFY COLUMN', 'TINYINT(8) UNSIGNED DEFAULT NULL') $$
CALL alter_if_exists('item_db2', 'equip_genders', 'MODIFY COLUMN', 'TINYINT(2) UNSIGNED DEFAULT NULL') $$
CALL alter_if_exists('item_db2', 'equip_locations', 'MODIFY COLUMN', 'SMALLINT(4) UNSIGNED DEFAULT NULL') $$
CALL alter_if_exists('item_db2', 'weapon_level', 'MODIFY COLUMN', 'TINYINT(2) UNSIGNED DEFAULT NULL') $$
CALL alter_if_exists('item_db2', 'equip_level_min', 'MODIFY COLUMN', 'SMALLINT(5) UNSIGNED DEFAULT NULL') $$
CALL alter_if_exists('item_db2', 'equip_level_max', 'MODIFY COLUMN', 'SMALLINT(5) UNSIGNED DEFAULT NULL') $$
CALL alter_if_exists('item_db2', 'refineable', 'MODIFY COLUMN', 'TINYINT(1) UNSIGNED DEFAULT NULL') $$
CALL alter_if_exists('item_db2', 'view', 'MODIFY COLUMN', 'SMALLINT(3) UNSIGNED DEFAULT NULL') $$

DROP PROCEDURE IF EXISTS alter_if_not_exists $$
DROP PROCEDURE IF EXISTS alter_if_exists $$

DELIMITER ';'

INSERT INTO `sql_updates` (`timestamp`) VALUES (1384473995);
