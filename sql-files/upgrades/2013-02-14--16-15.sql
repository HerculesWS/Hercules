#1360858500

-- This file is part of Hercules.
-- http://herc.ws - http://github.com/HerculesWS/Hercules
--
-- Copyright (C) 2013-2015  Hercules Dev Team
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

CREATE TABLE IF NOT EXISTS `sql_updates` (
  `timestamp` INT(11) UNSIGNED NOT NULL,
  `ignored` ENUM('Yes','No') NOT NULL DEFAULT 'No'
) ENGINE=MyISAM;
ALTER TABLE `skill` ADD COLUMN `flag` TINYINT(1) UNSIGNED NOT NULL DEFAULT 0;
INSERT INTO `sql_updates` (`timestamp`) VALUES (1360858500);
