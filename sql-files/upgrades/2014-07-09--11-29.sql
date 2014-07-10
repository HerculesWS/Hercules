#1404905340
ALTER TABLE  `guild_position` CHANGE  `mode`  `mode` SMALLINT(5) UNSIGNED NOT NULL DEFAULT  '0'
INSERT INTO `sql_updates` (`timestamp`) VALUES (1404905340);