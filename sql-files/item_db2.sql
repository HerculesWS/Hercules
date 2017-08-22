-- This file is part of Hercules.
-- http://herc.ws - http://github.com/HerculesWS/Hercules
--
-- Copyright (C) 2013-2017  Hercules Dev Team
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
  `id` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `name_english` varchar(50) NOT NULL DEFAULT '',
  `name_japanese` varchar(50) NOT NULL DEFAULT '',
  `type` tinyint(2) UNSIGNED NOT NULL DEFAULT '0',
  `subtype` tinyint(2) UNSIGNED DEFAULT NULL,
  `price_buy` mediumint(10) DEFAULT NULL,
  `price_sell` mediumint(10) DEFAULT NULL,
  `weight` smallint(5) UNSIGNED DEFAULT NULL,
  `atk` smallint(5) UNSIGNED DEFAULT NULL,
  `matk` smallint(5) UNSIGNED DEFAULT NULL,
  `defence` smallint(5) UNSIGNED DEFAULT NULL,
  `range` tinyint(2) UNSIGNED DEFAULT NULL,
  `slots` tinyint(2) UNSIGNED DEFAULT NULL,
  `equip_jobs` bigint(20) UNSIGNED DEFAULT NULL,
  `equip_upper` tinyint(8) UNSIGNED DEFAULT NULL,
  `equip_genders` tinyint(2) UNSIGNED DEFAULT NULL,
  `equip_locations` mediumint(8) UNSIGNED DEFAULT NULL,
  `weapon_level` tinyint(2) UNSIGNED DEFAULT NULL,
  `equip_level_min` smallint(5) UNSIGNED DEFAULT NULL,
  `equip_level_max` smallint(5) UNSIGNED DEFAULT NULL,
  `refineable` tinyint(1) UNSIGNED DEFAULT NULL,
  `disable_options` tinyint(1) UNSIGNED DEFAULT NULL,
  `view_sprite` smallint(3) UNSIGNED DEFAULT NULL,
  `bindonequip` tinyint(1) UNSIGNED DEFAULT NULL,
  `forceserial` tinyint(1) UNSIGNED DEFAULT NULL,
  `buyingstore` tinyint(1) UNSIGNED DEFAULT NULL,
  `delay` mediumint(9) UNSIGNED DEFAULT NULL,
  `trade_flag` smallint(4) UNSIGNED DEFAULT NULL,
  `trade_group` smallint(3) UNSIGNED DEFAULT NULL,
  `nouse_flag` smallint(4) UNSIGNED DEFAULT NULL,
  `nouse_group` smallint(4) UNSIGNED DEFAULT NULL,
  `stack_amount` mediumint(6) UNSIGNED DEFAULT NULL,
  `stack_flag` tinyint(2) UNSIGNED DEFAULT NULL,
  `sprite` mediumint(6) UNSIGNED DEFAULT NULL,
  `script` text,
  `equip_script` text,
  `unequip_script` text,
 PRIMARY KEY (`id`)
) ENGINE=MyISAM;

