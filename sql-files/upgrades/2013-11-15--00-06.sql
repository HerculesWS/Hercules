#1384473995

-- Note: If you're running a MySQL version earlier than 5.0 (or if this scripts fails for you for any reason)
--       you'll need to run the following queries manually:
--
-- [ Pre-Renewal only ]
-- ALTER TABLE item_db2 ADD COLUMN `matk` smallint(5) unsigned DEFAULT NULL AFTER atk;
-- ALTER TABLE item_db2 CHANGE COLUMN `equip_level` `equip_level_min` smallint(5) unsigned DEFAULT NULL;
-- ALTER TABLE item_db2 ADD COLUMN `equip_level_max` smallint(5) unsigned DEFAULT NULL AFTER equip_level_min;
-- [ Both Pre-Renewal and Renewal ]
-- ALTER TABLE item_db2 MODIFY COLUMN `price_buy` mediumint(10) DEFAULT NULL;
-- ALTER TABLE item_db2 MODIFY COLUMN `price_sell` mediumint(10) DEFAULT NULL;
-- ALTER TABLE item_db2 MODIFY COLUMN `weight` smallint(5) unsigned DEFAULT NULL;
-- ALTER TABLE item_db2 MODIFY COLUMN `atk` smallint(5) unsigned DEFAULT NULL;
-- ALTER TABLE item_db2 MODIFY COLUMN `matk` smallint(5) unsigned DEFAULT NULL;
-- ALTER TABLE item_db2 MODIFY COLUMN `defence` smallint(5) unsigned DEFAULT NULL;
-- ALTER TABLE item_db2 MODIFY COLUMN `range` tinyint(2) unsigned DEFAULT NULL;
-- ALTER TABLE item_db2 MODIFY COLUMN `slots` tinyint(2) unsigned DEFAULT NULL;
-- ALTER TABLE item_db2 MODIFY COLUMN `equip_jobs` int(12) unsigned DEFAULT NULL;
-- ALTER TABLE item_db2 MODIFY COLUMN `equip_upper` tinyint(8) unsigned DEFAULT NULL;
-- ALTER TABLE item_db2 MODIFY COLUMN `equip_genders` tinyint(2) unsigned DEFAULT NULL;
-- ALTER TABLE item_db2 MODIFY COLUMN `equip_locations` smallint(4) unsigned DEFAULT NULL;
-- ALTER TABLE item_db2 MODIFY COLUMN `weapon_level` tinyint(2) unsigned DEFAULT NULL;
-- ALTER TABLE item_db2 MODIFY COLUMN `equip_level_min` smallint(5) unsigned DEFAULT NULL;
-- ALTER TABLE item_db2 MODIFY COLUMN `equip_level_max` smallint(5) unsigned DEFAULT NULL;
-- ALTER TABLE item_db2 MODIFY COLUMN `refineable` tinyint(1) unsigned DEFAULT NULL;
-- ALTER TABLE item_db2 MODIFY COLUMN `view` smallint(3) unsigned DEFAULT NULL;
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

CALL alter_if_not_exists('item_db2', 'matk', 'ADD COLUMN', 'smallint(5) unsigned DEFAULT NULL AFTER atk') $$
CALL alter_if_exists('item_db2', 'equip_level', 'CHANGE COLUMN', 'equip_level_min smallint(5) unsigned DEFAULT NULL') $$
CALL alter_if_not_exists('item_db2', 'equip_level_max', 'ADD COLUMN', 'smallint(5) unsigned DEFAULT NULL AFTER equip_level_min') $$

CALL alter_if_exists('item_db2', 'price_buy', 'MODIFY COLUMN', 'mediumint(10) DEFAULT NULL') $$
CALL alter_if_exists('item_db2', 'price_sell', 'MODIFY COLUMN', 'mediumint(10) DEFAULT NULL') $$
CALL alter_if_exists('item_db2', 'weight', 'MODIFY COLUMN', 'smallint(5) unsigned DEFAULT NULL') $$
CALL alter_if_exists('item_db2', 'atk', 'MODIFY COLUMN', 'smallint(5) unsigned DEFAULT NULL') $$
CALL alter_if_exists('item_db2', 'matk', 'MODIFY COLUMN', 'smallint(5) unsigned DEFAULT NULL') $$
CALL alter_if_exists('item_db2', 'defence', 'MODIFY COLUMN', 'smallint(5) unsigned DEFAULT NULL') $$
CALL alter_if_exists('item_db2', 'range', 'MODIFY COLUMN', 'tinyint(2) unsigned DEFAULT NULL') $$
CALL alter_if_exists('item_db2', 'slots', 'MODIFY COLUMN', 'tinyint(2) unsigned DEFAULT NULL') $$
CALL alter_if_exists('item_db2', 'equip_jobs', 'MODIFY COLUMN', 'int(12) unsigned DEFAULT NULL') $$
CALL alter_if_exists('item_db2', 'equip_upper', 'MODIFY COLUMN', 'tinyint(8) unsigned DEFAULT NULL') $$
CALL alter_if_exists('item_db2', 'equip_genders', 'MODIFY COLUMN', 'tinyint(2) unsigned DEFAULT NULL') $$
CALL alter_if_exists('item_db2', 'equip_locations', 'MODIFY COLUMN', 'smallint(4) unsigned DEFAULT NULL') $$
CALL alter_if_exists('item_db2', 'weapon_level', 'MODIFY COLUMN', 'tinyint(2) unsigned DEFAULT NULL') $$
CALL alter_if_exists('item_db2', 'equip_level_min', 'MODIFY COLUMN', 'smallint(5) unsigned DEFAULT NULL') $$
CALL alter_if_exists('item_db2', 'equip_level_max', 'MODIFY COLUMN', 'smallint(5) unsigned DEFAULT NULL') $$
CALL alter_if_exists('item_db2', 'refineable', 'MODIFY COLUMN', 'tinyint(1) unsigned DEFAULT NULL') $$
CALL alter_if_exists('item_db2', 'view', 'MODIFY COLUMN', 'smallint(3) unsigned DEFAULT NULL') $$

DROP PROCEDURE IF EXISTS alter_if_not_exists $$
DROP PROCEDURE IF EXISTS alter_if_exists $$

DELIMITER ';'

INSERT INTO `sql_updates` (`timestamp`) VALUES (1384473995);
