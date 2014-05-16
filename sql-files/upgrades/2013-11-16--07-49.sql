#1384588175
ALTER TABLE `char` ADD COLUMN `unban_time` int(11) unsigned NOT NULL default '0';
INSERT INTO `sql_updates` (`timestamp`) VALUES (1384588175);
