#1467935469

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

ALTER TABLE `atcommandlog` MODIFY `atcommand_date` DATETIME NULL;
ALTER TABLE `branchlog` MODIFY `branch_date` DATETIME NULL;
ALTER TABLE `chatlog` MODIFY `time` DATETIME NULL;
ALTER TABLE `loginlog` MODIFY `time` DATETIME NULL;
ALTER TABLE `mvplog` MODIFY `mvp_date` DATETIME NULL;
ALTER TABLE `npclog` MODIFY `npc_date` DATETIME NULL;
ALTER TABLE `picklog` MODIFY `time` DATETIME NULL;
ALTER TABLE `zenylog` MODIFY `time` DATETIME NULL;

INSERT INTO `sql_updates` (`timestamp`) VALUES (1467935469)
