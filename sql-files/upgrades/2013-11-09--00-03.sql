#1383955424
ALTER TABLE `account_data` MODIFY `base_exp` TINYINT( 4 ) UNSIGNED NOT NULL default '100';
ALTER TABLE `account_data` MODIFY `base_drop` TINYINT( 4 ) UNSIGNED NOT NULL default '100';
ALTER TABLE `account_data` MODIFY `base_death` TINYINT( 4 ) UNSIGNED NOT NULL default '100';
INSERT INTO `sql_updates` (`timestamp`) VALUES (1383955424);
