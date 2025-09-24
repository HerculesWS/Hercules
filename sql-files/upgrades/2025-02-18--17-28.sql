#1739910620

-- This file is part of Hercules.
-- http://herc.ws - http://github.com/HerculesWS/Hercules
--
-- Copyright (C) 2022-2025 Hercules Dev Team
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

UPDATE `char_reg_num_db` SET `key` = 'ep14_3_newerabs' WHERE `key` = 'ep14_3_dimensional_travel' AND `index` = 0 AND `value` < 2;
UPDATE `char_reg_num_db` SET `key` = 'ep14_3_newerabs', `value` = 3 WHERE `key` = 'ep14_3_dimensional_travel' AND `index` = 0 AND `value` = 2;
UPDATE `char_reg_num_db` SET `key` = 'ep14_3_newerabs', `value` = `value` + 2 WHERE `key` = 'ep14_3_dimensional_travel' AND `index` = 0 AND `value` < 8;
UPDATE `char_reg_num_db` SET `key` = 'ep14_3_newerabs', `value` = `value` + 7 WHERE `key` = 'ep14_3_dimensional_travel' AND `index` = 0 AND `value` > 7;

INSERT INTO `sql_updates` (`timestamp`) VALUES (1739910620);
