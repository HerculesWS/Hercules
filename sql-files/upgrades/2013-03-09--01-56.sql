#1362794218
ALTER TABLE `login` ADD COLUMN `pincode` VARCHAR(4) NOT NULL DEFAULT '';
ALTER TABLE `login` ADD COLUMN `pincode_change` INT(11) unsigned NOT NULL DEFAULT '0';
INSERT INTO `sql_updates` (`timestamp`) VALUES (1362794218);
