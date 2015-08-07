#1384545461
UPDATE `account_data` SET `base_exp` = '100' WHERE `base_exp` = '0';
UPDATE `account_data` SET `base_drop` = '100' WHERE `base_drop` = '0';
UPDATE `account_data` SET `base_death` = '100' WHERE `base_death` = '0';
INSERT INTO `sql_updates` (`timestamp`) VALUES (1384545461);
