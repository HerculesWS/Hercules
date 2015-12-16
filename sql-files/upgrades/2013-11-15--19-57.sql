#1384545461

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

UPDATE `account_data` SET `base_exp` = '100' WHERE `base_exp` = '0';
UPDATE `account_data` SET `base_drop` = '100' WHERE `base_drop` = '0';
UPDATE `account_data` SET `base_death` = '100' WHERE `base_death` = '0';
INSERT INTO `sql_updates` (`timestamp`) VALUES (1384545461);
