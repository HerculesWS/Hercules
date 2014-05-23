#1400256139
ALTER TABLE `pet` CHANGE `incuvate` `incubate` int(11) unsigned NOT NULL default '0';
INSERT INTO `sql_updates` (`timestamp`) VALUES (1400256139);
