#1546059075

-- This file is part of Hercules.
-- http://herc.ws - http://github.com/HerculesWS/Hercules
--
-- Copyright (C) 2013-2021 Hercules Dev Team
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

CREATE TABLE IF NOT EXISTS `npc_barter_data` (
  `name` VARCHAR(24) NOT NULL DEFAULT '',
  `itemId` INT UNSIGNED NOT NULL DEFAULT '0',
  `amount` INT UNSIGNED NOT NULL DEFAULT '0',
  `priceId` INT UNSIGNED NOT NULL DEFAULT '0',
  `priceAmount` INT UNSIGNED NOT NULL DEFAULT '0',
  PRIMARY KEY (`name`, `itemid`, `priceId`, `priceAmount`)
) ENGINE=MyISAM;
INSERT INTO `sql_updates` (`timestamp`) VALUES (1546059075);
