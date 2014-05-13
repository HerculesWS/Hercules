#1392832626
DELETE FROM `sc_data` WHERE `tick` = '-1';
ALTER TABLE `sc_data` ADD PRIMARY KEY  (`account_id`,`char_id`,`type`);
INSERT INTO `sql_updates` (`timestamp`) VALUES (1392832626);
