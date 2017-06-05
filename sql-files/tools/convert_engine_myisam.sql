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
-- InnoDB Engine -> MyISAM Engine
--

ALTER TABLE `account_data` ENGINE = MyISAM;
ALTER TABLE `acc_reg_num_db` ENGINE = MyISAM;
ALTER TABLE `acc_reg_str_db` ENGINE = MyISAM;
ALTER TABLE `auction` ENGINE = MyISAM;
ALTER TABLE `autotrade_data` ENGINE = MyISAM;
ALTER TABLE `autotrade_merchants` ENGINE = MyISAM;
ALTER TABLE `cart_inventory` ENGINE = MyISAM;
ALTER TABLE `char` ENGINE = MyISAM;
ALTER TABLE `char_reg_num_db` ENGINE = MyISAM;
ALTER TABLE `char_reg_str_db` ENGINE = MyISAM;
ALTER TABLE `charlog` ENGINE = MyISAM;
ALTER TABLE `elemental` ENGINE = MyISAM;
ALTER TABLE `friends` ENGINE = MyISAM;
ALTER TABLE `hotkey` ENGINE = MyISAM;
ALTER TABLE `global_acc_reg_num_db` ENGINE = MyISAM;
ALTER TABLE `global_acc_reg_str_db` ENGINE = MyISAM;
ALTER TABLE `guild` ENGINE = MyISAM;
ALTER TABLE `guild_alliance` ENGINE = MyISAM;
ALTER TABLE `guild_castle` ENGINE = MyISAM;
ALTER TABLE `guild_expulsion` ENGINE = MyISAM;
ALTER TABLE `guild_member` ENGINE = MyISAM;
ALTER TABLE `guild_position` ENGINE = MyISAM;
ALTER TABLE `guild_skill` ENGINE = MyISAM;
ALTER TABLE `guild_storage` ENGINE = MyISAM;
ALTER TABLE `homunculus` ENGINE = MyISAM;
ALTER TABLE `interlog` ENGINE = MyISAM;
ALTER TABLE `inventory` ENGINE = MyISAM;
ALTER TABLE `ipbanlist` ENGINE = MyISAM;
-- ALTER TABLE `item_db` ENGINE = MyISAM;
-- ALTER TABLE `item_db2` ENGINE = MyISAM;
ALTER TABLE `login` ENGINE = MyISAM;
ALTER TABLE `mapreg` ENGINE = MyISAM;
ALTER TABLE `sc_data` ENGINE = MyISAM;
ALTER TABLE `mail` ENGINE = MyISAM;
ALTER TABLE `memo` ENGINE = MyISAM;
ALTER TABLE `mercenary` ENGINE = MyISAM;
ALTER TABLE `mercenary_owner` ENGINE = MyISAM;
-- ALTER TABLE `mob_db` ENGINE = MyISAM;
-- ALTER TABLE `mob_db2` ENGINE = MyISAM;
ALTER TABLE `npc_market_data` ENGINE = MyISAM;
ALTER TABLE `party` ENGINE = MyISAM;
ALTER TABLE `pet` ENGINE = MyISAM;
ALTER TABLE `quest` ENGINE = MyISAM;
ALTER TABLE `ragsrvinfo` ENGINE = MyISAM;
ALTER TABLE `skill` ENGINE = MyISAM;
ALTER TABLE `skill_homunculus` ENGINE = MyISAM;
ALTER TABLE `sql_updates` ENGINE = MyISAM;
ALTER TABLE `sstatus` ENGINE = MyISAM;
ALTER TABLE `storage` ENGINE = MyISAM;
ALTER TABLE `interreg` ENGINE = MyISAM;

