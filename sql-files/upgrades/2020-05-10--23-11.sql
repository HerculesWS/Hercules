#1589145060

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

-- Add separate tables for global integer and string variables.
CREATE TABLE IF NOT EXISTS `map_reg_num_db` (
  `key` VARCHAR(32) BINARY NOT NULL DEFAULT '',
  `index` INT UNSIGNED NOT NULL DEFAULT '0',
  `value` INT NOT NULL DEFAULT '0',
  PRIMARY KEY (`key`, `index`)
) ENGINE=MyISAM;
CREATE TABLE IF NOT EXISTS `map_reg_str_db` (
  `key` VARCHAR(32) BINARY NOT NULL DEFAULT '',
  `index` INT UNSIGNED NOT NULL DEFAULT '0',
  `value` VARCHAR(255) NOT NULL DEFAULT '0',
  PRIMARY KEY (`key`, `index`)
) ENGINE=MyISAM;

-- Copy data from mapreg table to new map_reg_*_db tables.
INSERT INTO `map_reg_num_db` (`key`, `index`, `value`) SELECT `varname`, `index`, CAST(`value` AS SIGNED) FROM `mapreg` WHERE NOT RIGHT(`varname`, 1)='$';
INSERT INTO `map_reg_str_db` (`key`, `index`, `value`) SELECT `varname`, `index`, `value` FROM `mapreg` WHERE RIGHT(`varname`, 1)='$';

-- Remove mapreg table.
DROP TABLE IF EXISTS `mapreg`;

-- Add update timestamp.
INSERT INTO `sql_updates` (`timestamp`) VALUES (1589145060);
