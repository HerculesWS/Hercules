#1396893866
ALTER TABLE `char` ADD COLUMN `uniqueitem_counter` BIGINT(20) NOT NULL AFTER `unban_time`;
INSERT INTO `sql_updates` (`timestamp`) VALUES (1396893866);
