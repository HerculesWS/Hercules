#1725211254

-- This file is part of Hercules.
-- http://herc.ws - http://github.com/HerculesWS/Hercules
--
-- Copyright (C) 2025 Hercules Dev Team
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

-- SQL date/time fields and general improvements

-- Convert DATETIME to TIMESTAMP for timezone support
ALTER TABLE `charlog` MODIFY `time` TIMESTAMP NULL;
ALTER TABLE `interlog` MODIFY `time` TIMESTAMP NULL;
ALTER TABLE `ipbanlist` MODIFY `btime` TIMESTAMP NULL;
ALTER TABLE `ipbanlist` MODIFY `rtime` TIMESTAMP NULL;
ALTER TABLE `login` MODIFY `lastlogin` TIMESTAMP NULL;

-- Convert BIGINT to TIMESTAMP for last_login
ALTER TABLE `char` MODIFY `last_login` TIMESTAMP NULL DEFAULT NULL;

-- Allow NULL for optional IDs
ALTER TABLE `char` MODIFY `party_id` INT UNSIGNED NULL DEFAULT NULL;
ALTER TABLE `char` MODIFY `guild_id` INT UNSIGNED NULL DEFAULT NULL;
ALTER TABLE `char` MODIFY `clan_id` INT UNSIGNED NULL DEFAULT NULL;
ALTER TABLE `char` MODIFY `pet_id` INT UNSIGNED NULL DEFAULT NULL;
ALTER TABLE `char` MODIFY `homun_id` INT UNSIGNED NULL DEFAULT NULL;
ALTER TABLE `char` MODIFY `elemental_id` INT UNSIGNED NULL DEFAULT NULL;
ALTER TABLE `char` MODIFY `partner_id` INT UNSIGNED NULL DEFAULT NULL;
ALTER TABLE `char` MODIFY `father` INT UNSIGNED NULL DEFAULT NULL;
ALTER TABLE `char` MODIFY `mother` INT UNSIGNED NULL DEFAULT NULL;
ALTER TABLE `char` MODIFY `child` INT UNSIGNED NULL DEFAULT NULL;

-- Log tables
ALTER TABLE `atcommandlog` MODIFY `atcommand_date` TIMESTAMP NULL;
ALTER TABLE `branchlog` MODIFY `branch_date` TIMESTAMP NULL;
ALTER TABLE `chatlog` MODIFY `time` TIMESTAMP NULL;
ALTER TABLE `loginlog` MODIFY `time` TIMESTAMP NULL;
ALTER TABLE `mvplog` MODIFY `mvp_date` TIMESTAMP NULL;
ALTER TABLE `npclog` MODIFY `npc_date` TIMESTAMP NULL;
ALTER TABLE `picklog` MODIFY `time` TIMESTAMP NULL;
ALTER TABLE `zenylog` MODIFY `time` TIMESTAMP NULL;

INSERT INTO `sql_updates` (`timestamp`) VALUES (1725211254);