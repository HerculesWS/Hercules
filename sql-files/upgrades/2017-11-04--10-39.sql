#1509835214

ALTER TABLE `homunculus`
	ADD COLUMN `autofeed` TINYINT NOT NULL DEFAULT '0' AFTER `vaporize`;

INSERT INTO `sql_updates` (`timestamp`, `ignored`) VALUES (1509835214 , 'No');
