#1435860840

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

CALL alter_if_not_exists('item_db', 'forceserial', 'ADD COLUMN', 'TINYINT(1) UNSIGNED DEFAULT NULL AFTER `bindonequip`') $$
CALL alter_if_not_exists('item_db2', 'forceserial', 'ADD COLUMN', 'TINYINT(1) UNSIGNED DEFAULT NULL AFTER `bindonequip`') $$

DROP PROCEDURE IF EXISTS alter_if_not_exists $$
DROP PROCEDURE IF EXISTS alter_if_exists $$

DELIMITER ';'

INSERT INTO `sql_updates` (`timestamp`) VALUES (1435860840);
