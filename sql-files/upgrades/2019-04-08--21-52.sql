#1554760320

-- This file is part of Hercules.
-- http://herc.ws - http://github.com/HerculesWS/Hercules
--
-- Copyright (C) 2013-2022 Hercules Dev Team
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

UPDATE `auction` SET `card3` = `card2` >> 16, `card2` = `card2` % 65536 WHERE `card2` > 65536 AND (`card0` = 255 OR `card0` = 254);
UPDATE `cart_inventory` SET `card3` = `card2` >> 16, `card2` = `card2` % 65536 WHERE `card2` > 65536 AND (`card0` = 255 OR `card0` = 254);
UPDATE `inventory` SET `card3` = `card2` >> 16, `card2` = `card2` % 65536 WHERE `card2` > 65536 AND (`card0` = 255 OR `card0` = 254);
UPDATE `guild_storage` SET `card3` = `card2` >> 16, `card2` = `card2` % 65536 WHERE `card2` > 65536 AND (`card0` = 255 OR `card0` = 254);
UPDATE `mail` SET `card3` = `card2` >> 16, `card2` = `card2` % 65536 WHERE `card2` > 65536 AND (`card0` = 255 OR `card0` = 254);
UPDATE `rodex_items` SET `card3` = `card2` >> 16, `card2` = `card2` % 65536 WHERE `card2` > 65536 AND (`card0` = 255 OR `card0` = 254);
UPDATE `storage` SET `card3` = `card2` >> 16, `card2` = `card2` % 65536 WHERE `card2` > 65536 AND (`card0` = 255 OR `card0` = 254);

INSERT INTO `sql_updates` (`timestamp`, `ignored`) VALUES (1554760320, 'No');
