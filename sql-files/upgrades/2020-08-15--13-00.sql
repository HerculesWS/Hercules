#1597467600

-- This file is part of Hercules.
-- http://herc.ws - http://github.com/HerculesWS/Hercules
--
-- Copyright (C) 2019-2021 Hercules Dev Team
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

-- Add attendance data to accounta_data table
ALTER TABLE `account_data` ADD COLUMN `attendance_count` TINYINT UNSIGNED NOT NULL DEFAULT '0' AFTER `base_death`;
ALTER TABLE `account_data` ADD COLUMN `attendance_timer` BIGINT NULL DEFAULT '0' AFTER `attendance_count`;

-- Migrate data from the char table to the account_data table
REPLACE INTO `account_data` (`account_id`, `attendance_count`, `attendance_timer`)
  SELECT `account_id`, MAX(`attendance_count`), MAX(`attendance_timer`)
    FROM `char`
    WHERE `attendance_count` <> 0 AND `attendance_timer` <> 0
    GROUP BY `account_id`;

-- Delete column for attendance at char table
ALTER TABLE `char` DROP COLUMN `attendance_count`;
ALTER TABLE `char` DROP COLUMN `attendance_timer`;

-- Add update timestamp.
INSERT INTO `sql_updates` (`timestamp`) VALUES (1597467600);
