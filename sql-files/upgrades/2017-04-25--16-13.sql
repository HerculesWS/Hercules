#1493136780

-- This file is part of Hercules.
-- http://herc.ws - http://github.com/HerculesWS/Hercules
--
-- Copyright (C) 2017  Hercules Dev Team
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

ALTER TABLE `char` ADD COLUMN `clan_id` INT(11) UNSIGNED NOT NULL DEFAULT '0' AFTER `hotkey_rowshift`;
ALTER TABLE `char` ADD COLUMN `last_login` BIGINT(20) NULL DEFAULT '0' AFTER `robe`;
ALTER TABLE `chatlog` MODIFY COLUMN `type`  enum('O','W','P','G','M','C') CHARACTER SET latin1 COLLATE latin1_swedish_ci NOT NULL DEFAULT 'O' AFTER `time`;

INSERT INTO `sql_updates` (`timestamp`, `ignored`) VALUES (1493136780 , 'No');