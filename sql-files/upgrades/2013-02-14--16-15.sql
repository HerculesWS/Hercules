#1360858500
CREATE TABLE IF NOT EXISTS `sql_updates` (
  `timestamp` int(11) unsigned NOT NULL,
  `ignored` enum('Yes','No') NOT NULL DEFAULT 'No'
) ENGINE=MyISAM;
ALTER TABLE `skill` ADD COLUMN `flag` TINYINT(1) UNSIGNED NOT NULL DEFAULT 0;
INSERT INTO `sql_updates` (`timestamp`) VALUES (1360858500);
