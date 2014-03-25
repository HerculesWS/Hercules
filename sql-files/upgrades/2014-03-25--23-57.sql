#1395789302
ALTER TABLE `charlog` ADD COLUMN `char_id` int(11) unsigned NOT NULL default '0' AFTER `account_id`;
INSERT INTO `sql_updates` (`timestamp`) VALUES (1395789302);
