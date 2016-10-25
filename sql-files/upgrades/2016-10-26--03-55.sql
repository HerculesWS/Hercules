#1477434595

-- This file is part of Hercules.
-- http://herc.ws - http://github.com/HerculesWS/Hercules
--
-- Copyright (C) 2015-2016  Hercules Dev Team
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

ALTER TABLE `zenylog` CHANGE `type` `type` ENUM('T','V','P','M','S','N','D','C','A','E','I','B','K')  CHARACTER SET utf8  COLLATE utf8_general_ci  NOT NULL  DEFAULT 'S';

INSERT INTO `sql_updates` (`timestamp`, `ignored`) VALUES (1477434595 , 'No');
