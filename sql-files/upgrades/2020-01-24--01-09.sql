#1579817630

-- This file is part of Hercules.
-- http://herc.ws - http://github.com/HerculesWS/Hercules
--
-- Copyright (C) 2013-2023 Hercules Dev Team
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

CREATE TABLE IF NOT EXISTS `npc_expanded_barter_data` (
  `name` VARCHAR(24) NOT NULL DEFAULT '',
  `itemId` INT UNSIGNED NOT NULL DEFAULT '0',
  `amount` INT UNSIGNED NOT NULL DEFAULT '0',
  `zeny` INT UNSIGNED NOT NULL DEFAULT '0',
  `currencyId1` INT UNSIGNED NOT NULL DEFAULT '0',
  `currencyAmount1` INT UNSIGNED NOT NULL DEFAULT '0',
  `currencyRefine1` INT NOT NULL DEFAULT '0',
  `currencyId2` INT UNSIGNED NOT NULL DEFAULT '0',
  `currencyAmount2` INT UNSIGNED NOT NULL DEFAULT '0',
  `currencyRefine2` INT NOT NULL DEFAULT '0',
  `currencyId3` INT UNSIGNED NOT NULL DEFAULT '0',
  `currencyAmount3` INT UNSIGNED NOT NULL DEFAULT '0',
  `currencyRefine3` INT NOT NULL DEFAULT '0',
  `currencyId4` INT UNSIGNED NOT NULL DEFAULT '0',
  `currencyAmount4` INT UNSIGNED NOT NULL DEFAULT '0',
  `currencyRefine4` INT NOT NULL DEFAULT '0',
  `currencyId5` INT UNSIGNED NOT NULL DEFAULT '0',
  `currencyAmount5` INT UNSIGNED NOT NULL DEFAULT '0',
  `currencyRefine5` INT NOT NULL DEFAULT '0',
  `currencyId6` INT UNSIGNED NOT NULL DEFAULT '0',
  `currencyAmount6` INT UNSIGNED NOT NULL DEFAULT '0',
  `currencyRefine6` INT NOT NULL DEFAULT '0',
  `currencyId7` INT UNSIGNED NOT NULL DEFAULT '0',
  `currencyAmount7` INT UNSIGNED NOT NULL DEFAULT '0',
  `currencyRefine7` INT NOT NULL DEFAULT '0',
  `currencyId8` INT UNSIGNED NOT NULL DEFAULT '0',
  `currencyAmount8` INT UNSIGNED NOT NULL DEFAULT '0',
  `currencyRefine8` INT NOT NULL DEFAULT '0',
  `currencyId9` INT UNSIGNED NOT NULL DEFAULT '0',
  `currencyAmount9` INT UNSIGNED NOT NULL DEFAULT '0',
  `currencyRefine9` INT NOT NULL DEFAULT '0',
  `currencyId10` INT UNSIGNED NOT NULL DEFAULT '0',
  `currencyAmount10` INT UNSIGNED NOT NULL DEFAULT '0',
  `currencyRefine10` INT NOT NULL DEFAULT '0',
  PRIMARY KEY (`name`, `itemid`, `zeny`,
    `currencyId1`, `currencyAmount1`, `currencyRefine1`,
    `currencyId2`, `currencyAmount2`, `currencyRefine2`,
    `currencyId3`, `currencyAmount3`, `currencyRefine3`,
    `currencyId4`, `currencyAmount4`, `currencyRefine4`
)
) ENGINE=MyISAM;
INSERT INTO `sql_updates` (`timestamp`) VALUES (1579817630);
