#1389028967
CREATE TABLE IF NOT EXISTS `autotrade_merchants` (
  `account_id` int(11) NOT NULL default '0',
  `char_id` int(11) NOT NULL default '0',
  `sex` tinyint(2) NOT NULL default '0',
  `title` varchar(80) NOT NULL default 'Buy From Me!',
  PRIMARY KEY  (`account_id`,`char_id`)
) ENGINE=MyISAM; 
CREATE TABLE IF NOT EXISTS `autotrade_data` (
  `char_id` int(11) NOT NULL default '0',
  `itemkey` int(11) NOT NULL default '0',
  `amount` int(11) NOT NULL default '0',
  `price` int(11) NOT NULL default '0',
  PRIMARY KEY  (`char_id`,`itemkey`)
) ENGINE=MyISAM; 
INSERT INTO `sql_updates` (`timestamp`) VALUES (1389028967);
