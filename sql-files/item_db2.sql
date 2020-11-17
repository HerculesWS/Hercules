-- This file is part of Hercules.
-- http://herc.ws - http://github.com/HerculesWS/Hercules
--
-- Copyright (C) 2013-2020 Hercules Dev Team
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

-- NOTE: This file was auto-generated and should never be manually edited,
--       as it will get overwritten. If you need to modify this file,
--       please consider modifying the corresponding .conf file inside
--       the db folder, and then re-run the db2sql plugin.

-- GENERATED FILE DO NOT EDIT --

--
-- Table structure for table `item_db2`
--

DROP TABLE IF EXISTS `item_db2`;
CREATE TABLE `item_db2` (
  `id` int UNSIGNED NOT NULL DEFAULT '0',
  `name_english` varchar(50) NOT NULL DEFAULT '',
  `name_japanese` varchar(50) NOT NULL DEFAULT '',
  `type` tinyint UNSIGNED NOT NULL DEFAULT '0',
  `subtype` tinyint UNSIGNED DEFAULT NULL,
  `price_buy` mediumint DEFAULT NULL,
  `price_sell` mediumint DEFAULT NULL,
  `weight` smallint UNSIGNED DEFAULT NULL,
  `atk` smallint UNSIGNED DEFAULT NULL,
  `matk` smallint UNSIGNED DEFAULT NULL,
  `defence` smallint UNSIGNED DEFAULT NULL,
  `range` tinyint UNSIGNED DEFAULT NULL,
  `slots` tinyint UNSIGNED DEFAULT NULL,
  `equip_jobs` bigint UNSIGNED DEFAULT NULL,
  `equip_upper` tinyint UNSIGNED DEFAULT NULL,
  `equip_genders` tinyint UNSIGNED DEFAULT NULL,
  `equip_locations` mediumint UNSIGNED DEFAULT NULL,
  `weapon_level` tinyint UNSIGNED DEFAULT NULL,
  `equip_level_min` smallint UNSIGNED DEFAULT NULL,
  `equip_level_max` smallint UNSIGNED DEFAULT NULL,
  `refineable` tinyint UNSIGNED DEFAULT NULL,
  `disable_options` tinyint UNSIGNED DEFAULT NULL,
  `view_sprite` smallint UNSIGNED DEFAULT NULL,
  `bindonequip` tinyint UNSIGNED DEFAULT NULL,
  `forceserial` tinyint UNSIGNED DEFAULT NULL,
  `buyingstore` tinyint UNSIGNED DEFAULT NULL,
  `delay` mediumint UNSIGNED DEFAULT NULL,
  `trade_flag` smallint UNSIGNED DEFAULT NULL,
  `trade_group` smallint UNSIGNED DEFAULT NULL,
  `nouse_flag` smallint UNSIGNED DEFAULT NULL,
  `nouse_group` smallint UNSIGNED DEFAULT NULL,
  `stack_amount` mediumint UNSIGNED DEFAULT NULL,
  `stack_flag` tinyint UNSIGNED DEFAULT NULL,
  `sprite` mediumint UNSIGNED DEFAULT NULL,
  `script` text,
  `equip_script` text,
  `unequip_script` text,
 PRIMARY KEY (`id`)
) ENGINE=MyISAM;

