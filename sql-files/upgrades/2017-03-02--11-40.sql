#1488454834

-- This file is part of Hercules.
-- http://herc.ws - http://github.com/HerculesWS/Hercules
--
-- Copyright (C) 2015-2016  Hercules Dev Team
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
	ADD COLUMN `opt_idx0` SMALLINT(5) UNSIGNED NOT NULL DEFAULT '0' AFTER `card3`,
	ADD COLUMN `opt_val0` SMALLINT(5) NOT NULL DEFAULT '0' AFTER `opt_idx0`,
	ADD COLUMN `opt_idx1` SMALLINT(5) UNSIGNED NOT NULL DEFAULT '0' AFTER `opt_val0`,
	ADD COLUMN `opt_val1` SMALLINT(5) NOT NULL DEFAULT '0' AFTER `opt_idx1`,
	ADD COLUMN `opt_idx2` SMALLINT(5) UNSIGNED NOT NULL DEFAULT '0' AFTER `opt_val1`,
	ADD COLUMN `opt_val2` SMALLINT(5) NOT NULL DEFAULT '0' AFTER `opt_idx2`,
	ADD COLUMN `opt_idx3` SMALLINT(5) UNSIGNED NOT NULL DEFAULT '0' AFTER `opt_val2`,
	ADD COLUMN `opt_val3` SMALLINT(5) NOT NULL DEFAULT '0' AFTER `opt_idx3`,
	ADD COLUMN `opt_idx4` SMALLINT(5) UNSIGNED NOT NULL DEFAULT '0' AFTER `opt_val3`,
	ADD COLUMN `opt_val4` SMALLINT(5) NOT NULL DEFAULT '0' AFTER `opt_idx4`;

ALTER TABLE `cart_inventory`
	ADD COLUMN `opt_idx0` SMALLINT(5) UNSIGNED NOT NULL DEFAULT '0' AFTER `card3`,
	ADD COLUMN `opt_val0` SMALLINT(5) NOT NULL DEFAULT '0' AFTER `opt_idx0`,
	ADD COLUMN `opt_idx1` SMALLINT(5) UNSIGNED NOT NULL DEFAULT '0' AFTER `opt_val0`,
	ADD COLUMN `opt_val1` SMALLINT(5) NOT NULL DEFAULT '0' AFTER `opt_idx1`,
	ADD COLUMN `opt_idx2` SMALLINT(5) UNSIGNED NOT NULL DEFAULT '0' AFTER `opt_val1`,
	ADD COLUMN `opt_val2` SMALLINT(5) NOT NULL DEFAULT '0' AFTER `opt_idx2`,
	ADD COLUMN `opt_idx3` SMALLINT(5) UNSIGNED NOT NULL DEFAULT '0' AFTER `opt_val2`,
	ADD COLUMN `opt_val3` SMALLINT(5) NOT NULL DEFAULT '0' AFTER `opt_idx3`,
	ADD COLUMN `opt_idx4` SMALLINT(5) UNSIGNED NOT NULL DEFAULT '0' AFTER `opt_val3`,
	ADD COLUMN `opt_val4` SMALLINT(5) NOT NULL DEFAULT '0' AFTER `opt_idx4`;

ALTER TABLE `guild_storage`
	ADD COLUMN `opt_idx0` SMALLINT(5) UNSIGNED NOT NULL DEFAULT '0' AFTER `card3`,
	ADD COLUMN `opt_val0` SMALLINT(5) NOT NULL DEFAULT '0' AFTER `opt_idx0`,
	ADD COLUMN `opt_idx1` SMALLINT(5) UNSIGNED NOT NULL DEFAULT '0' AFTER `opt_val0`,
	ADD COLUMN `opt_val1` SMALLINT(5) NOT NULL DEFAULT '0' AFTER `opt_idx1`,
	ADD COLUMN `opt_idx2` SMALLINT(5) UNSIGNED NOT NULL DEFAULT '0' AFTER `opt_val1`,
	ADD COLUMN `opt_val2` SMALLINT(5) NOT NULL DEFAULT '0' AFTER `opt_idx2`,
	ADD COLUMN `opt_idx3` SMALLINT(5) UNSIGNED NOT NULL DEFAULT '0' AFTER `opt_val2`,
	ADD COLUMN `opt_val3` SMALLINT(5) NOT NULL DEFAULT '0' AFTER `opt_idx3`,
	ADD COLUMN `opt_idx4` SMALLINT(5) UNSIGNED NOT NULL DEFAULT '0' AFTER `opt_val3`,
	ADD COLUMN `opt_val4` SMALLINT(5) NOT NULL DEFAULT '0' AFTER `opt_idx4`;
	
ALTER TABLE `inventory`
	ADD COLUMN `opt_idx0` SMALLINT(5) UNSIGNED NOT NULL DEFAULT '0' AFTER `card3`,
	ADD COLUMN `opt_val0` SMALLINT(5) NOT NULL DEFAULT '0' AFTER `opt_idx0`,
	ADD COLUMN `opt_idx1` SMALLINT(5) UNSIGNED NOT NULL DEFAULT '0' AFTER `opt_val0`,
	ADD COLUMN `opt_val1` SMALLINT(5) NOT NULL DEFAULT '0' AFTER `opt_idx1`,
	ADD COLUMN `opt_idx2` SMALLINT(5) UNSIGNED NOT NULL DEFAULT '0' AFTER `opt_val1`,
	ADD COLUMN `opt_val2` SMALLINT(5) NOT NULL DEFAULT '0' AFTER `opt_idx2`,
	ADD COLUMN `opt_idx3` SMALLINT(5) UNSIGNED NOT NULL DEFAULT '0' AFTER `opt_val2`,
	ADD COLUMN `opt_val3` SMALLINT(5) NOT NULL DEFAULT '0' AFTER `opt_idx3`,
	ADD COLUMN `opt_idx4` SMALLINT(5) UNSIGNED NOT NULL DEFAULT '0' AFTER `opt_val3`,
	ADD COLUMN `opt_val4` SMALLINT(5) NOT NULL DEFAULT '0' AFTER `opt_idx4`;
	
ALTER TABLE `mail`
	ADD COLUMN `opt_idx0` SMALLINT(5) UNSIGNED NOT NULL DEFAULT '0' AFTER `card3`,
	ADD COLUMN `opt_val0` SMALLINT(5) NOT NULL DEFAULT '0' AFTER `opt_idx0`,
	ADD COLUMN `opt_idx1` SMALLINT(5) UNSIGNED NOT NULL DEFAULT '0' AFTER `opt_val0`,
	ADD COLUMN `opt_val1` SMALLINT(5) NOT NULL DEFAULT '0' AFTER `opt_idx1`,
	ADD COLUMN `opt_idx2` SMALLINT(5) UNSIGNED NOT NULL DEFAULT '0' AFTER `opt_val1`,
	ADD COLUMN `opt_val2` SMALLINT(5) NOT NULL DEFAULT '0' AFTER `opt_idx2`,
	ADD COLUMN `opt_idx3` SMALLINT(5) UNSIGNED NOT NULL DEFAULT '0' AFTER `opt_val2`,
	ADD COLUMN `opt_val3` SMALLINT(5) NOT NULL DEFAULT '0' AFTER `opt_idx3`,
	ADD COLUMN `opt_idx4` SMALLINT(5) UNSIGNED NOT NULL DEFAULT '0' AFTER `opt_val3`,
	ADD COLUMN `opt_val4` SMALLINT(5) NOT NULL DEFAULT '0' AFTER `opt_idx4`;
	
ALTER TABLE `storage`
	ADD COLUMN `opt_idx0` SMALLINT(5) UNSIGNED NOT NULL DEFAULT '0' AFTER `card3`,
	ADD COLUMN `opt_val0` SMALLINT(5) NOT NULL DEFAULT '0' AFTER `opt_idx0`,
	ADD COLUMN `opt_idx1` SMALLINT(5) UNSIGNED NOT NULL DEFAULT '0' AFTER `opt_val0`,
	ADD COLUMN `opt_val1` SMALLINT(5) NOT NULL DEFAULT '0' AFTER `opt_idx1`,
	ADD COLUMN `opt_idx2` SMALLINT(5) UNSIGNED NOT NULL DEFAULT '0' AFTER `opt_val1`,
	ADD COLUMN `opt_val2` SMALLINT(5) NOT NULL DEFAULT '0' AFTER `opt_idx2`,
	ADD COLUMN `opt_idx3` SMALLINT(5) UNSIGNED NOT NULL DEFAULT '0' AFTER `opt_val2`,
	ADD COLUMN `opt_val3` SMALLINT(5) NOT NULL DEFAULT '0' AFTER `opt_idx3`,
	ADD COLUMN `opt_idx4` SMALLINT(5) UNSIGNED NOT NULL DEFAULT '0' AFTER `opt_val3`,
	ADD COLUMN `opt_val4` SMALLINT(5) NOT NULL DEFAULT '0' AFTER `opt_idx4`;
 
INSERT INTO `sql_updates` (`timestamp`, `ignored`) VALUES (1488454834 , 'No');
