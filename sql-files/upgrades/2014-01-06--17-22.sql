#1389028967
CREATE TABLE IF NOT EXISTS `autotrade_merchants` (
  `account_id` INT(11) NOT NULL DEFAULT '0',
  `char_id` INT(11) NOT NULL DEFAULT '0',
  `sex` TINYINT(2) NOT NULL DEFAULT '0',
  `title` varchar(80) NOT NULL DEFAULT 'Buy From Me!',
  PRIMARY KEY (`account_id`,`char_id`)
) ENGINE=MyISAM; 
CREATE TABLE IF NOT EXISTS `autotrade_data` (
  `char_id` INT(11) NOT NULL DEFAULT '0',
  `itemkey` INT(11) NOT NULL DEFAULT '0',
  `amount` INT(11) NOT NULL DEFAULT '0',
  `price` INT(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`char_id`,`itemkey`)
) ENGINE=MyISAM; 
INSERT INTO `sql_updates` (`timestamp`) VALUES (1389028967);
