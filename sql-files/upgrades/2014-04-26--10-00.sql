#1398477600
ALTER TABLE `char` CHANGE COLUMN `uniqueitem_counter` `uniqueitem_counter` BIGINT(20) UNSIGNED NOT NULL DEFAULT '0';
INSERT INTO `sql_updates` (`timestamp`) VALUES (1398477600);
