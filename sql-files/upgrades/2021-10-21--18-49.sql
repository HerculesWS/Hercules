#1634838524
-- This file is part of Hercules.
-- http://herc.ws - http://github.com/HerculesWS/Hercules
--
-- Copyright (C) 2021-2022 Hercules Dev Team
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

ALTER TABLE `auction`
	ADD COLUMN `grade` TINYINT UNSIGNED NOT NULL DEFAULT '0' AFTER `refine`;

ALTER TABLE `cart_inventory`
	ADD COLUMN `grade` TINYINT UNSIGNED NOT NULL DEFAULT '0' AFTER `refine`;

ALTER TABLE `guild_storage`
	ADD COLUMN `grade` TINYINT UNSIGNED NOT NULL DEFAULT '0' AFTER `refine`;
	
ALTER TABLE `inventory`
	ADD COLUMN `grade` TINYINT UNSIGNED NOT NULL DEFAULT '0' AFTER `refine`;

ALTER TABLE `mail`
	ADD COLUMN `grade` TINYINT UNSIGNED NOT NULL DEFAULT '0' AFTER `refine`;
	
ALTER TABLE `storage`
	ADD COLUMN `grade` TINYINT UNSIGNED NOT NULL DEFAULT '0' AFTER `refine`;

ALTER TABLE `rodex_items`
	ADD COLUMN `grade` TINYINT UNSIGNED NOT NULL DEFAULT '0' AFTER `refine`;

ALTER TABLE `picklog`
	ADD COLUMN `grade` TINYINT UNSIGNED NOT NULL DEFAULT '0' AFTER `refine`;

INSERT INTO `sql_updates` (`timestamp`, `ignored`) VALUES (1634838524 , 'No');
