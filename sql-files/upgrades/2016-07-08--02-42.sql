#1467934919

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

ALTER TABLE `charlog` MODIFY `time` DATETIME NULL;
ALTER TABLE `interlog` MODIFY `time` DATETIME NULL;
ALTER TABLE `ipbanlist` MODIFY `btime` DATETIME NULL;
ALTER TABLE `ipbanlist` MODIFY `rtime` DATETIME NULL;
ALTER TABLE `login` MODIFY `lastlogin` DATETIME NULL;
ALTER TABLE `login` MODIFY `birthdate` DATE NULL;

UPDATE `charlog` SET `time` = NULL WHERE `time` = '0000-00-00 00:00:00';
UPDATE `interlog` SET `time` = NULL WHERE `time` = '0000-00-00 00:00:00';
UPDATE `ipbanlist` SET `btime` = NULL WHERE `btime` = '0000-00-00 00:00:00';
UPDATE `ipbanlist` SET `rtime` = NULL WHERE `rtime` = '0000-00-00 00:00:00';
UPDATE `login` SET `lastlogin` = NULL WHERE `lastlogin` = '0000-00-00 00:00:00';
UPDATE `login` SET `birthdate` = NULL WHERE `birthdate` = '0000-00-00';

INSERT INTO `sql_updates` (`timestamp`) VALUES (1467934919)
