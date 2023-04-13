#1599908598

-- This file is part of Hercules.
-- http://herc.ws - http://github.com/HerculesWS/Hercules
--
-- Copyright (C) 2019-2020 Hercules Dev Team
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

CREATE TABLE IF NOT EXISTS `emotes` (
  `account_id` INT UNSIGNED NOT NULL DEFAULT '0',
  `emote0` VARCHAR(50) NOT NULL DEFAULT '',
  `emote1` VARCHAR(50) NOT NULL DEFAULT '',
  `emote2` VARCHAR(50) NOT NULL DEFAULT '',
  `emote3` VARCHAR(50) NOT NULL DEFAULT '',
  `emote4` VARCHAR(50) NOT NULL DEFAULT '',
  `emote5` VARCHAR(50) NOT NULL DEFAULT '',
  `emote6` VARCHAR(50) NOT NULL DEFAULT '',
  `emote7` VARCHAR(50) NOT NULL DEFAULT '',
  `emote8` VARCHAR(50) NOT NULL DEFAULT '',
  `emote9` VARCHAR(50) NOT NULL DEFAULT '',
  PRIMARY KEY (`account_id`),
  KEY `account_id` (`account_id`)
) CHARACTER SET utf8mb4 ENGINE=MyISAM;

-- Add update timestamp.
INSERT INTO `sql_updates` (`timestamp`) VALUES (1599908598);
