#1381423003
CREATE TABLE IF NOT EXISTS `account_data` (
  `account_id` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `bank_vault` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  PRIMARY KEY (`account_id`)
) ENGINE=MyISAM; 
INSERT INTO `sql_updates` (`timestamp`) VALUES (1381423003);
