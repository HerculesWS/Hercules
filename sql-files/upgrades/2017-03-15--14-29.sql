#1489588190

-- This file is part of Hercules.
-- http://herc.ws - http://github.com/HerculesWS/Hercules
--
-- Copyright (C) 2017  Hercules Dev Team
--
-- Hercules is free software: you can redistribute it and/or modify
-- it under the terms of the GNU General Public License as published by
-- the Free Software Foundation, either version 3 of the License, or
-- (at your option) any later version.
--
-- This program is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU General Public License for more details.
--
-- You should have received a copy of the GNU General Public License
-- along with this program.  If not, see <http://www.gnu.org/licenses/>.

CREATE TABLE IF NOT EXISTS `rodex_items` (
	`id` INT(11) NOT NULL AUTO_INCREMENT,
	`mail_id` BIGINT(20) NOT NULL DEFAULT '0',
	`nameid` INT(11) NOT NULL DEFAULT '0',
	`amount` INT(11) NOT NULL DEFAULT '0',
	`equip` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`identify` SMALLINT(6) NOT NULL DEFAULT '0',
	`refine` TINYINT(3) UNSIGNED NOT NULL DEFAULT '0',
	`attribute` TINYINT(4) NOT NULL DEFAULT '0',
	`card0` SMALLINT(11) NOT NULL DEFAULT '0',
	`card1` SMALLINT(11) NOT NULL DEFAULT '0',
	`card2` SMALLINT(11) NOT NULL DEFAULT '0',
	`card3` SMALLINT(11) NOT NULL DEFAULT '0',
	`opt_idx0` SMALLINT(5) UNSIGNED NOT NULL DEFAULT '0',
	`opt_val0` SMALLINT(5) NOT NULL DEFAULT '0',
	`opt_idx1` SMALLINT(5) UNSIGNED NOT NULL DEFAULT '0',
	`opt_val1` SMALLINT(5) NOT NULL DEFAULT '0',
	`opt_idx2` SMALLINT(5) UNSIGNED NOT NULL DEFAULT '0',
	`opt_val2` SMALLINT(5) NOT NULL DEFAULT '0',
	`opt_idx3` SMALLINT(5) UNSIGNED NOT NULL DEFAULT '0',
	`opt_val3` SMALLINT(5) NOT NULL DEFAULT '0',
	`opt_idx4` SMALLINT(5) UNSIGNED NOT NULL DEFAULT '0',
	`opt_val4` SMALLINT(5) NOT NULL DEFAULT '0',
	`expire_time` INT(11) UNSIGNED NOT NULL DEFAULT '0',
	`bound` TINYINT(1) UNSIGNED NOT NULL DEFAULT '0',
	`unique_id` BIGINT(20) UNSIGNED NOT NULL DEFAULT '0',
	PRIMARY KEY (`id`),
  KEY `mail_id` (`mail_id`)
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS `rodex_mail` (
	`mail_id` BIGINT(20) NOT NULL AUTO_INCREMENT,
	`sender_name` VARCHAR(30) NOT NULL COLLATE 'utf8_unicode_ci',
	`sender_id` INT(11) NOT NULL,
	`receiver_name` VARCHAR(30) NOT NULL COLLATE 'utf8_unicode_ci',
	`receiver_id` INT(11) NOT NULL,
	`receiver_accountid` INT(11) NOT NULL,
	`title` VARCHAR(50) NOT NULL COLLATE 'utf8_unicode_ci',
	`body` VARCHAR(510) NOT NULL COLLATE 'utf8_unicode_ci',
	`zeny` BIGINT(20) NOT NULL,
	`type` TINYINT(8) UNSIGNED NOT NULL,
	`is_read` TINYINT(8) NOT NULL,
	`send_date` INT(11) NOT NULL,
	`expire_date` INT(11) NOT NULL,
	`weight` INT(11) NOT NULL,
	PRIMARY KEY (`mail_id`),
  KEY `sender_id` (`sender_id`),
  KEY `receiver_id` (`receiver_id`),
  KEY `receiver_accountid` (`receiver_accountid`),
  KEY `send_date` (`send_date`),
  KEY `expire_date` (`expire_date`)
) ENGINE=MyISAM;

INSERT INTO `sql_updates` (`timestamp`, `ignored`) VALUES (1489588190 , 'No');
