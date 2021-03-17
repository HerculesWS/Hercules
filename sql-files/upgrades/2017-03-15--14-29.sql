#1489588190

-- This file is part of Hercules.
-- http://herc.ws - http://github.com/HerculesWS/Hercules
--
-- Copyright (C) 2017-2021 Hercules Dev Team
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
	`id` INT NOT NULL AUTO_INCREMENT,
	`mail_id` BIGINT NOT NULL DEFAULT '0',
	`nameid` INT NOT NULL DEFAULT '0',
	`amount` INT NOT NULL DEFAULT '0',
	`equip` INT UNSIGNED NOT NULL DEFAULT '0',
	`identify` SMALLINT NOT NULL DEFAULT '0',
	`refine` TINYINT UNSIGNED NOT NULL DEFAULT '0',
	`attribute` TINYINT NOT NULL DEFAULT '0',
	`card0` SMALLINT NOT NULL DEFAULT '0',
	`card1` SMALLINT NOT NULL DEFAULT '0',
	`card2` SMALLINT NOT NULL DEFAULT '0',
	`card3` SMALLINT NOT NULL DEFAULT '0',
	`opt_idx0` SMALLINT UNSIGNED NOT NULL DEFAULT '0',
	`opt_val0` SMALLINT NOT NULL DEFAULT '0',
	`opt_idx1` SMALLINT UNSIGNED NOT NULL DEFAULT '0',
	`opt_val1` SMALLINT NOT NULL DEFAULT '0',
	`opt_idx2` SMALLINT UNSIGNED NOT NULL DEFAULT '0',
	`opt_val2` SMALLINT NOT NULL DEFAULT '0',
	`opt_idx3` SMALLINT UNSIGNED NOT NULL DEFAULT '0',
	`opt_val3` SMALLINT NOT NULL DEFAULT '0',
	`opt_idx4` SMALLINT UNSIGNED NOT NULL DEFAULT '0',
	`opt_val4` SMALLINT NOT NULL DEFAULT '0',
	`expire_time` INT UNSIGNED NOT NULL DEFAULT '0',
	`bound` TINYINT UNSIGNED NOT NULL DEFAULT '0',
	`unique_id` BIGINT UNSIGNED NOT NULL DEFAULT '0',
	PRIMARY KEY (`id`),
  KEY `mail_id` (`mail_id`)
) ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS `rodex_mail` (
	`mail_id` BIGINT NOT NULL AUTO_INCREMENT,
	`sender_name` VARCHAR(30) COLLATE 'utf8_unicode_ci' NOT NULL,
	`sender_id` INT NOT NULL,
	`receiver_name` VARCHAR(30) COLLATE 'utf8_unicode_ci' NOT NULL,
	`receiver_id` INT NOT NULL,
	`receiver_accountid` INT NOT NULL,
	`title` VARCHAR(50) COLLATE 'utf8_unicode_ci' NOT NULL,
	`body` VARCHAR(510) COLLATE 'utf8_unicode_ci' NOT NULL,
	`zeny` BIGINT NOT NULL,
	`type` TINYINT UNSIGNED NOT NULL,
	`is_read` TINYINT NOT NULL,
	`send_date` INT NOT NULL,
	`expire_date` INT NOT NULL,
	`weight` INT NOT NULL,
	PRIMARY KEY (`mail_id`),
  KEY `sender_id` (`sender_id`),
  KEY `receiver_id` (`receiver_id`),
  KEY `receiver_accountid` (`receiver_accountid`),
  KEY `send_date` (`send_date`),
  KEY `expire_date` (`expire_date`)
) ENGINE=MyISAM;

INSERT INTO `sql_updates` (`timestamp`, `ignored`) VALUES (1489588190 , 'No');
