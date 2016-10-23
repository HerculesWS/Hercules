-- This file is part of Hercules.
-- http://herc.ws - http://github.com/HerculesWS/Hercules
--
-- Copyright (C) 2012-2015  Hercules Dev Team
-- Copyright (C)  Athena Dev Teams
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

--
-- Hercules Database Converter 
-- MyISAM Engine -> InnoDB Engine
--

ALTER TABLE `account_data` ENGINE = InnoDB;
ALTER TABLE `acc_reg_num_db` ENGINE = InnoDB;
ALTER TABLE `acc_reg_str_db` ENGINE = InnoDB;
ALTER TABLE `auction` ENGINE = InnoDB;
ALTER TABLE `autotrade_data` ENGINE = InnoDB;
ALTER TABLE `autotrade_merchants` ENGINE = InnoDB;
ALTER TABLE `cart_inventory` ENGINE = InnoDB;
ALTER TABLE `char` ENGINE = InnoDB;
ALTER TABLE `char_reg_num_db` ENGINE = InnoDB;
ALTER TABLE `char_reg_str_db` ENGINE = InnoDB;
ALTER TABLE `charlog` ENGINE = InnoDB;
ALTER TABLE `elemental` ENGINE = InnoDB;
ALTER TABLE `friends` ENGINE = InnoDB;
ALTER TABLE `hotkey` ENGINE = InnoDB;
ALTER TABLE `global_acc_reg_num_db` ENGINE = InnoDB;
ALTER TABLE `global_acc_reg_str_db` ENGINE = InnoDB;
ALTER TABLE `guild` ENGINE = InnoDB;
ALTER TABLE `guild_alliance` ENGINE = InnoDB;
ALTER TABLE `guild_castle` ENGINE = InnoDB;
ALTER TABLE `guild_expulsion` ENGINE = InnoDB;
ALTER TABLE `guild_member` ENGINE = InnoDB;
ALTER TABLE `guild_position` ENGINE = InnoDB;
ALTER TABLE `guild_skill` ENGINE = InnoDB;
ALTER TABLE `guild_storage` ENGINE = InnoDB;
ALTER TABLE `homunculus` ENGINE = InnoDB;
ALTER TABLE `interlog` ENGINE = InnoDB;
ALTER TABLE `inventory` ENGINE = InnoDB;
ALTER TABLE `ipbanlist` ENGINE = InnoDB;
-- ALTER TABLE `item_db` ENGINE = InnoDB;
-- ALTER TABLE `item_db2` ENGINE = InnoDB;
ALTER TABLE `login` ENGINE = InnoDB;
ALTER TABLE `mapreg` ENGINE = InnoDB;
ALTER TABLE `sc_data` ENGINE = InnoDB;
ALTER TABLE `mail` ENGINE = InnoDB;
ALTER TABLE `memo` ENGINE = InnoDB;
ALTER TABLE `mercenary` ENGINE = InnoDB;
ALTER TABLE `mercenary_owner` ENGINE = InnoDB;
-- ALTER TABLE `mob_db` ENGINE = InnoDB;
-- ALTER TABLE `mob_db2` ENGINE = InnoDB;
ALTER TABLE `npc_market_data` ENGINE = InnoDB;
ALTER TABLE `party` ENGINE = InnoDB;
ALTER TABLE `pet` ENGINE = InnoDB;
ALTER TABLE `quest` ENGINE = InnoDB;
ALTER TABLE `ragsrvinfo` ENGINE = InnoDB;
ALTER TABLE `skill` ENGINE = InnoDB;
ALTER TABLE `skill_homunculus` ENGINE = InnoDB;
ALTER TABLE `sql_updates` ENGINE = InnoDB;
ALTER TABLE `sstatus` ENGINE = InnoDB;
ALTER TABLE `storage` ENGINE = InnoDB;
ALTER TABLE `interreg` ENGINE = InnoDB;

