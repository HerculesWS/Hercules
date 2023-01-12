#1641405640

-- This file is part of Hercules.
-- http://herc.ws - http://github.com/HerculesWS/Hercules
--
-- Copyright (C) 2022-2023 Hercules Dev Team
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

ALTER TABLE `picklog` MODIFY `type` ENUM('M', 'P', 'L', 'T', 'V', 'S', 'N', 'C', 'A', 'R', 'G', 'E', 'B', 'O', 'I', 'X', 'D', 'U', 'K', 'Y', 'Z', 'W', 'Q', 'J', 'H', '@', '0', '1', '2', '3', '4') NOT NULL DEFAULT 'P';
ALTER TABLE `zenylog` MODIFY `type` ENUM('T','V','P','M','S','N','D','C','A','E','I','B','K','4') NOT NULL DEFAULT 'S';

INSERT INTO `sql_updates` (`timestamp`) VALUES (1641405640);

