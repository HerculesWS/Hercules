#1364409316
ALTER TABLE `char` ADD COLUMN `slotchange` SMALLINT(3) unsigned NOT NULL default '0';
INSERT INTO `sql_updates` (`timestamp`) VALUES (1364409316);
