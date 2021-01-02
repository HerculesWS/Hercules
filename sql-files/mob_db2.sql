-- This file is part of Hercules.
-- http://herc.ws - http://github.com/HerculesWS/Hercules
--
-- Copyright (C) 2013-2021 Hercules Dev Team
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
-- Table structure for table `mob_db2`
--

DROP TABLE IF EXISTS `mob_db2`;
CREATE TABLE `mob_db2` (
  `ID` mediumint UNSIGNED NOT NULL DEFAULT '0',
  `Sprite` text NOT NULL,
  `kName` text NOT NULL,
  `iName` text NOT NULL,
  `LV` tinyint UNSIGNED NOT NULL DEFAULT '0',
  `HP` int UNSIGNED NOT NULL DEFAULT '0',
  `SP` mediumint UNSIGNED NOT NULL DEFAULT '0',
  `EXP` mediumint UNSIGNED NOT NULL DEFAULT '0',
  `JEXP` mediumint UNSIGNED NOT NULL DEFAULT '0',
  `Range1` tinyint UNSIGNED NOT NULL DEFAULT '0',
  `ATK1` smallint UNSIGNED NOT NULL DEFAULT '0',
  `ATK2` smallint UNSIGNED NOT NULL DEFAULT '0',
  `DEF` smallint UNSIGNED NOT NULL DEFAULT '0',
  `MDEF` smallint UNSIGNED NOT NULL DEFAULT '0',
  `STR` smallint UNSIGNED NOT NULL DEFAULT '0',
  `AGI` smallint UNSIGNED NOT NULL DEFAULT '0',
  `VIT` smallint UNSIGNED NOT NULL DEFAULT '0',
  `INT` smallint UNSIGNED NOT NULL DEFAULT '0',
  `DEX` smallint UNSIGNED NOT NULL DEFAULT '0',
  `LUK` smallint UNSIGNED NOT NULL DEFAULT '0',
  `Range2` tinyint UNSIGNED NOT NULL DEFAULT '0',
  `Range3` tinyint UNSIGNED NOT NULL DEFAULT '0',
  `Scale` tinyint UNSIGNED NOT NULL DEFAULT '0',
  `Race` tinyint UNSIGNED NOT NULL DEFAULT '0',
  `Element` tinyint UNSIGNED NOT NULL DEFAULT '0',
  `Mode` int UNSIGNED NOT NULL DEFAULT '0',
  `Speed` smallint UNSIGNED NOT NULL DEFAULT '0',
  `aDelay` smallint UNSIGNED NOT NULL DEFAULT '0',
  `aMotion` smallint UNSIGNED NOT NULL DEFAULT '0',
  `dMotion` smallint UNSIGNED NOT NULL DEFAULT '0',
  `MEXP` mediumint UNSIGNED NOT NULL DEFAULT '0',
  `MVP1id` smallint UNSIGNED NOT NULL DEFAULT '0',
  `MVP1per` smallint UNSIGNED NOT NULL DEFAULT '0',
  `MVP2id` smallint UNSIGNED NOT NULL DEFAULT '0',
  `MVP2per` smallint UNSIGNED NOT NULL DEFAULT '0',
  `MVP3id` smallint UNSIGNED NOT NULL DEFAULT '0',
  `MVP3per` smallint UNSIGNED NOT NULL DEFAULT '0',
  `Drop1id` smallint UNSIGNED NOT NULL DEFAULT '0',
  `Drop1per` smallint UNSIGNED NOT NULL DEFAULT '0',
  `Drop2id` smallint UNSIGNED NOT NULL DEFAULT '0',
  `Drop2per` smallint UNSIGNED NOT NULL DEFAULT '0',
  `Drop3id` smallint UNSIGNED NOT NULL DEFAULT '0',
  `Drop3per` smallint UNSIGNED NOT NULL DEFAULT '0',
  `Drop4id` smallint UNSIGNED NOT NULL DEFAULT '0',
  `Drop4per` smallint UNSIGNED NOT NULL DEFAULT '0',
  `Drop5id` smallint UNSIGNED NOT NULL DEFAULT '0',
  `Drop5per` smallint UNSIGNED NOT NULL DEFAULT '0',
  `Drop6id` smallint UNSIGNED NOT NULL DEFAULT '0',
  `Drop6per` smallint UNSIGNED NOT NULL DEFAULT '0',
  `Drop7id` smallint UNSIGNED NOT NULL DEFAULT '0',
  `Drop7per` smallint UNSIGNED NOT NULL DEFAULT '0',
  `Drop8id` smallint UNSIGNED NOT NULL DEFAULT '0',
  `Drop8per` smallint UNSIGNED NOT NULL DEFAULT '0',
  `Drop9id` smallint UNSIGNED NOT NULL DEFAULT '0',
  `Drop9per` smallint UNSIGNED NOT NULL DEFAULT '0',
  `DropCardid` smallint UNSIGNED NOT NULL DEFAULT '0',
  `DropCardper` smallint UNSIGNED NOT NULL DEFAULT '0',
  PRIMARY KEY (`ID`)
) ENGINE=MyISAM;

