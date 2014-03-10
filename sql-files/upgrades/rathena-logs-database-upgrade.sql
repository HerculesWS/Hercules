-- rAthena to Hercules log database upgrade query.
-- This upgrades a FULLY UPGRADED rAthena to a FULLY UPGRADED Hercules
-- Please don't use if either rAthena or Hercules launched a SQL update after last revision date of this file.
-- Remember to make a backup before applying.
-- We are not liable for any data loss this may cause.
-- Apply in the same database you applied your logs.sql
-- Last revision: 3/10/2014 7:03:15 PM

/* Header line. Object: picklog. Script date: 3/10/2014 7:03:15 PM. */
DROP TABLE IF EXISTS `_temp_picklog`;

CREATE TABLE `_temp_picklog` (
	`id` int(11) NOT NULL auto_increment,
	`time` datetime NOT NULL default '0000-00-00 00:00:00',
	`char_id` int(11) NOT NULL default '0',
	`type` enum('M','P','L','T','V','S','N','C','A','R','G','E','B','O','I','X','D','U') NOT NULL default 'P',
	`nameid` int(11) NOT NULL default '0',
	`amount` int(11) NOT NULL default '1',
	`refine` tinyint(3) unsigned NOT NULL default '0',
	`card0` int(11) NOT NULL default '0',
	`card1` int(11) NOT NULL default '0',
	`card2` int(11) NOT NULL default '0',
	`card3` int(11) NOT NULL default '0',
	`unique_id` bigint(20) unsigned NOT NULL default '0',
	`map` varchar(11) NOT NULL,
	PRIMARY KEY  ( `id` ),
	KEY `type` ( `type` )
)
ENGINE = MyISAM
CHARACTER SET = latin1
AUTO_INCREMENT = 1
ROW_FORMAT = Dynamic
;

INSERT INTO `_temp_picklog`
( `amount`, `card0`, `card1`, `card2`, `card3`, `char_id`, `id`, `map`, `nameid`, `refine`, `time`, `type`, `unique_id` )
SELECT
`amount`, `card0`, `card1`, `card2`, `card3`, `char_id`, `id`, `map`, `nameid`, `refine`, `time`, `type`, `unique_id`
FROM `picklog`;

DROP TABLE `picklog`;

ALTER TABLE `_temp_picklog` RENAME `picklog`;

/* Header line. Object: cashlog. Script date: 3/10/2014 7:03:15 PM. */
-- Attention: Table `cashlog` will be dropped.
DROP TABLE `cashlog`;
