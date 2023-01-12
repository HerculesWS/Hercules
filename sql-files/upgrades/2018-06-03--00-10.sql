#1527964800

-- This file is part of Hercules.
-- http://herc.ws - http://github.com/HerculesWS/Hercules
--
-- Copyright (C) 2018-2023 Hercules Dev Team
-- Copyright (C) Smokexyz
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

CREATE TABLE `char_achievements` (
	`char_id` INT UNSIGNED NOT NULL,
	`ach_id` INT UNSIGNED NOT NULL,
	`completed_at` INT UNSIGNED NOT NULL DEFAULT '0',
	`rewarded_at` INT UNSIGNED NOT NULL DEFAULT '0',
	`obj_0` INT UNSIGNED NOT NULL DEFAULT '0',
	`obj_1` INT UNSIGNED NOT NULL DEFAULT '0',
	`obj_2` INT UNSIGNED NOT NULL DEFAULT '0',
	`obj_3` INT UNSIGNED NOT NULL DEFAULT '0',
	`obj_4` INT UNSIGNED NOT NULL DEFAULT '0',
	`obj_5` INT UNSIGNED NOT NULL DEFAULT '0',
	`obj_6` INT UNSIGNED NOT NULL DEFAULT '0',
	`obj_7` INT UNSIGNED NOT NULL DEFAULT '0',
	`obj_8` INT UNSIGNED NOT NULL DEFAULT '0',
	`obj_9` INT UNSIGNED NOT NULL DEFAULT '0',
	PRIMARY KEY (`char_id`, `ach_id`)
) ENGINE=MyISAM;

INSERT INTO `sql_updates` (`timestamp`, `ignored`) VALUES (1527964800, 'No');
