#1613840320

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

CREATE TABLE IF NOT EXISTS `hotkeys` (
  `account_id` INT UNSIGNED NOT NULL DEFAULT '0',
  `tab` INT UNSIGNED NOT NULL DEFAULT 0,
  `desc` VARCHAR(116) NOT NULL DEFAULT '',
  `index` INT UNSIGNED NOT NULL DEFAULT 0,
  `key1` INT UNSIGNED NOT NULL DEFAULT 0,
  `key2` INT UNSIGNED NOT NULL DEFAULT 0,
  PRIMARY KEY (`account_id`, `tab`, `index`, `key1`, `key2`),
  KEY `key` (`account_id`, `tab`)
) CHARACTER SET utf8mb4 ENGINE=MyISAM;

-- Add update timestamp.
INSERT INTO `sql_updates` (`timestamp`) VALUES (1613840320);
