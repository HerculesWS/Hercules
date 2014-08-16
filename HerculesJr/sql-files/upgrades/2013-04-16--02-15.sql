#1366078541
ALTER TABLE `char` ADD `char_opt` INT(11) UNSIGNED NOT NULL DEFAULT '0';
INSERT INTO `sql_updates` (`timestamp`) VALUES (1366075474); -- for the previous that missed it..
INSERT INTO `sql_updates` (`timestamp`) VALUES (1366078541);
