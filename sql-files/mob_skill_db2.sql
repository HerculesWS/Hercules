-- This file is part of Hercules.
-- http://herc.ws - http://github.com/HerculesWS/Hercules
--
-- Copyright (C) 2013-2019  Hercules Dev Team
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
-- Table structure for table `mob_skill_db2`
--

DROP TABLE IF EXISTS `mob_skill_db2`;
CREATE TABLE `mob_skill_db2` (
  `MOB_ID` SMALLINT(6) NOT NULL,
  `INFO` TEXT NOT NULL,
  `STATE` TEXT NOT NULL,
  `SKILL_ID` SMALLINT(6) NOT NULL,
  `SKILL_LV` TINYINT(4) NOT NULL,
  `RATE` SMALLINT(4) NOT NULL,
  `CASTTIME` MEDIUMINT(9) NOT NULL,
  `DELAY` INT(9) NOT NULL,
  `CANCELABLE` TEXT NOT NULL,
  `TARGET` TEXT NOT NULL,
  `CONDITION` TEXT NOT NULL,
  `CONDITION_VALUE` TEXT,
  `VAL1` INT(11) DEFAULT NULL,
  `VAL2` INT(11) DEFAULT NULL,
  `VAL3` INT(11) DEFAULT NULL,
  `VAL4` INT(11) DEFAULT NULL,
  `VAL5` INT(11) DEFAULT NULL,
  `EMOTION` TEXT,
  `CHAT` TEXT
) ENGINE=MyISAM;

