#1381423003
CREATE TABLE IF NOT EXISTS `account_data` (
  `account_id` int(11) unsigned NOT NULL default '0',
  `bank_vault` int(11) unsigned NOT NULL default '0',
  PRIMARY KEY  (`account_id`)
) ENGINE=MyISAM; 
INSERT INTO `sql_updates` (`timestamp`) VALUES (1381423003);
