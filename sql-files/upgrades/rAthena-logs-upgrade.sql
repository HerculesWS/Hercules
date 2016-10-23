-- This file is part of Hercules.
-- http://herc.ws - http://github.com/HerculesWS/Hercules
--
-- Copyright (C) 2013-2014  Hercules Dev Team
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

--
-- rAthena to Hercules log database upgrade query.
-- This upgrades a FULLY UPGRADED rAthena to a FULLY UPGRADED Hercules
-- Please don't use if either rAthena or Hercules launched a SQL update after last revision date of this file.
-- Remember to make a backup before applying.
-- We are not liable for any data loss this may cause.
-- Apply in the same database you applied your logs.sql
-- Last revised: July 22, 2014 20:45 GMT
--

-- Drop table `cashlog` since it's not used in Hercules
-- Comment it if you wish to keep the table
DROP TABLE IF EXISTS `cashlog`;

-- Upgrades to table `mvplog`
ALTER TABLE `mvplog` MODIFY `prize` INT(11) NOT NULL DEFAULT '0';

-- Upgrades to table `picklog`
ALTER TABLE `picklog` MODIFY `type` enum('M','P','L','T','V','S','N','C','A','R','G','E','B','O','I','X','D','U','K','Y','Z','W','Q','J','H','@','0','1','2') NOT NULL default 'P';
ALTER TABLE `picklog` MODIFY `nameid` INT(11) NOT NULL DEFAULT '0';
ALTER TABLE `picklog` MODIFY `card0` INT(11) NOT NULL DEFAULT '0';
ALTER TABLE `picklog` MODIFY `card1` INT(11) NOT NULL DEFAULT '0';
ALTER TABLE `picklog` MODIFY `card2` INT(11) NOT NULL DEFAULT '0';
ALTER TABLE `picklog` MODIFY `card3` INT(11) NOT NULL DEFAULT '0';
ALTER TABLE `atcommandlog` MODIFY `atcommand_date` DATETIME NULL;
ALTER TABLE `branchlog` MODIFY `branch_date` DATETIME NULL;
ALTER TABLE `chatlog` MODIFY `time` DATETIME NULL;
ALTER TABLE `loginlog` MODIFY `time` DATETIME NULL;
ALTER TABLE `mvplog` MODIFY `mvp_date` DATETIME NULL;
ALTER TABLE `npclog` MODIFY `npc_date` DATETIME NULL;
ALTER TABLE `picklog` MODIFY `time` DATETIME NULL;
ALTER TABLE `zenylog` MODIFY `time` DATETIME NULL;
