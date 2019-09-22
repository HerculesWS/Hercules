#1565293394

-- This file is part of Hercules.
-- http://herc.ws - http://github.com/HerculesWS/Hercules
--
-- Copyright (C) 2019  Hercules Dev Team
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

ALTER TABLE `guild_castle` DROP PRIMARY KEY;
ALTER TABLE `guild_castle` ADD COLUMN `castle_name` VARCHAR(24) AFTER `castle_id`;
UPDATE `guild_castle` SET `castle_name` = 'aldeg_cas01' WHERE castle_id = 0;
UPDATE `guild_castle` SET `castle_name` = 'aldeg_cas02' WHERE castle_id = 1;
UPDATE `guild_castle` SET `castle_name` = 'aldeg_cas03' WHERE castle_id = 2;
UPDATE `guild_castle` SET `castle_name` = 'aldeg_cas04' WHERE castle_id = 3;
UPDATE `guild_castle` SET `castle_name` = 'aldeg_cas05' WHERE castle_id = 4;
UPDATE `guild_castle` SET `castle_name` = 'gefg_cas01' WHERE castle_id = 5;
UPDATE `guild_castle` SET `castle_name` = 'gefg_cas02' WHERE castle_id = 6;
UPDATE `guild_castle` SET `castle_name` = 'gefg_cas03' WHERE castle_id = 7;
UPDATE `guild_castle` SET `castle_name` = 'gefg_cas04' WHERE castle_id = 8;
UPDATE `guild_castle` SET `castle_name` = 'gefg_cas05' WHERE castle_id = 9;
UPDATE `guild_castle` SET `castle_name` = 'payg_cas01' WHERE castle_id = 10;
UPDATE `guild_castle` SET `castle_name` = 'payg_cas02' WHERE castle_id = 11;
UPDATE `guild_castle` SET `castle_name` = 'payg_cas03' WHERE castle_id = 12;
UPDATE `guild_castle` SET `castle_name` = 'payg_cas04' WHERE castle_id = 13;
UPDATE `guild_castle` SET `castle_name` = 'payg_cas05' WHERE castle_id = 14;
UPDATE `guild_castle` SET `castle_name` = 'prtg_cas01' WHERE castle_id = 15;
UPDATE `guild_castle` SET `castle_name` = 'prtg_cas02' WHERE castle_id = 16;
UPDATE `guild_castle` SET `castle_name` = 'prtg_cas03' WHERE castle_id = 17;
UPDATE `guild_castle` SET `castle_name` = 'prtg_cas04' WHERE castle_id = 18;
UPDATE `guild_castle` SET `castle_name` = 'prtg_cas05' WHERE castle_id = 19;
UPDATE `guild_castle` SET `castle_name` = 'nguild_alde' WHERE castle_id = 20;
UPDATE `guild_castle` SET `castle_name` = 'nguild_gef' WHERE castle_id = 21;
UPDATE `guild_castle` SET `castle_name` = 'nguild_pay' WHERE castle_id = 22;
UPDATE `guild_castle` SET `castle_name` = 'nguild_prt' WHERE castle_id = 23;
UPDATE `guild_castle` SET `castle_name` = 'schg_cas01' WHERE castle_id = 24;
UPDATE `guild_castle` SET `castle_name` = 'schg_cas02' WHERE castle_id = 25;
UPDATE `guild_castle` SET `castle_name` = 'schg_cas03' WHERE castle_id = 26;
UPDATE `guild_castle` SET `castle_name` = 'schg_cas04' WHERE castle_id = 27;
UPDATE `guild_castle` SET `castle_name` = 'schg_cas05' WHERE castle_id = 28;
UPDATE `guild_castle` SET `castle_name` = 'arug_cas01' WHERE castle_id = 29;
UPDATE `guild_castle` SET `castle_name` = 'arug_cas02' WHERE castle_id = 30;
UPDATE `guild_castle` SET `castle_name` = 'arug_cas03' WHERE castle_id = 31;
UPDATE `guild_castle` SET `castle_name` = 'arug_cas04' WHERE castle_id = 32;
UPDATE `guild_castle` SET `castle_name` = 'arug_cas05' WHERE castle_id = 33;

-- Change the castle ids
UPDATE `guild_castle` SET `castle_id` = 1 WHERE castle_name = 'prtg_cas01';
UPDATE `guild_castle` SET `castle_id` = 2 WHERE castle_name = 'prtg_cas02';
UPDATE `guild_castle` SET `castle_id` = 3 WHERE castle_name = 'prtg_cas03';
UPDATE `guild_castle` SET `castle_id` = 4 WHERE castle_name = 'prtg_cas04';
UPDATE `guild_castle` SET `castle_id` = 5 WHERE castle_name = 'prtg_cas05';
UPDATE `guild_castle` SET `castle_id` = 6 WHERE castle_name = 'aldeg_cas01';
UPDATE `guild_castle` SET `castle_id` = 7 WHERE castle_name = 'aldeg_cas02';
UPDATE `guild_castle` SET `castle_id` = 8 WHERE castle_name = 'aldeg_cas03';
UPDATE `guild_castle` SET `castle_id` = 9 WHERE castle_name = 'aldeg_cas04';
UPDATE `guild_castle` SET `castle_id` = 10 WHERE castle_name = 'aldeg_cas05';
UPDATE `guild_castle` SET `castle_id` = 11 WHERE castle_name = 'gefg_cas01';
UPDATE `guild_castle` SET `castle_id` = 12 WHERE castle_name = 'gefg_cas02';
UPDATE `guild_castle` SET `castle_id` = 13 WHERE castle_name = 'gefg_cas03';
UPDATE `guild_castle` SET `castle_id` = 14 WHERE castle_name = 'gefg_cas04';
UPDATE `guild_castle` SET `castle_id` = 15 WHERE castle_name = 'gefg_cas05';
UPDATE `guild_castle` SET `castle_id` = 16 WHERE castle_name = 'payg_cas01';
UPDATE `guild_castle` SET `castle_id` = 17 WHERE castle_name = 'payg_cas02';
UPDATE `guild_castle` SET `castle_id` = 18 WHERE castle_name = 'payg_cas03';
UPDATE `guild_castle` SET `castle_id` = 19 WHERE castle_name = 'payg_cas04';
UPDATE `guild_castle` SET `castle_id` = 20 WHERE castle_name = 'payg_cas05';
UPDATE `guild_castle` SET `castle_id` = 21 WHERE castle_name = 'arug_cas01';
UPDATE `guild_castle` SET `castle_id` = 22 WHERE castle_name = 'arug_cas02';
UPDATE `guild_castle` SET `castle_id` = 23 WHERE castle_name = 'arug_cas03';
UPDATE `guild_castle` SET `castle_id` = 24 WHERE castle_name = 'arug_cas04';
UPDATE `guild_castle` SET `castle_id` = 25 WHERE castle_name = 'arug_cas05';
UPDATE `guild_castle` SET `castle_id` = 26 WHERE castle_name = 'schg_cas01';
UPDATE `guild_castle` SET `castle_id` = 27 WHERE castle_name = 'schg_cas02';
UPDATE `guild_castle` SET `castle_id` = 29 WHERE castle_name = 'schg_cas04';
UPDATE `guild_castle` SET `castle_id` = 28 WHERE castle_name = 'schg_cas03';
UPDATE `guild_castle` SET `castle_id` = 30 WHERE castle_name = 'schg_cas05';
UPDATE `guild_castle` SET `castle_id` = 31 WHERE castle_name = 'nguild_prt';
UPDATE `guild_castle` SET `castle_id` = 32 WHERE castle_name = 'nguild_alde';
UPDATE `guild_castle` SET `castle_id` = 33 WHERE castle_name = 'nguild_gef';
UPDATE `guild_castle` SET `castle_id` = 34 WHERE castle_name = 'nguild_pay';
ALTER TABLE `guild_castle` ADD PRIMARY KEY (`castle_id`);
ALTER TABLE `guild_castle` DROP COLUMN `castle_name`;
INSERT INTO `sql_updates` (`timestamp`) VALUES (1565293394);
