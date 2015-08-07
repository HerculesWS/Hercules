#1409590380
ALTER TABLE  `account_data` CHANGE  `base_exp`  `base_exp` SMALLINT(6) UNSIGNED NOT NULL DEFAULT  '100',
CHANGE  `base_drop`  `base_drop` SMALLINT(6) UNSIGNED NOT NULL DEFAULT  '100',
CHANGE  `base_death`  `base_death` SMALLINT(6) UNSIGNED NOT NULL DEFAULT  '100';
INSERT INTO `sql_updates` (`timestamp`) VALUES (1409590380);
