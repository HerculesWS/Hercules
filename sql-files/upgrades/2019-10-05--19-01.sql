#1570309293

-- This file is part of Hercules.
-- http://herc.ws - http://github.com/HerculesWS/Hercules
--
-- Copyright (C) 2019  Hercules Dev Team
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

-- Adds new total_tick column
ALTER TABLE `sc_data` ADD COLUMN `total_tick` INT(11) NOT NULL DEFAULT '0' AFTER `tick`;

-- Copy current tick to total_tick so players doesn't lose their current
-- status_changes, although those will still appear wrong until they end
UPDATE `sc_data` SET `total_tick` = `tick`; 

INSERT INTO `sql_updates` (`timestamp`) VALUES (1570309293);
