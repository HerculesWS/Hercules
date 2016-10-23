#1389028967

-- This file is part of Hercules.
-- http://herc.ws - http://github.com/HerculesWS/Hercules
--
-- Copyright (C) 2014-2015  Hercules Dev Team
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

CREATE TABLE IF NOT EXISTS `autotrade_merchants` (
  `account_id` INT(11) NOT NULL DEFAULT '0',
  `char_id` INT(11) NOT NULL DEFAULT '0',
  `sex` TINYINT(2) NOT NULL DEFAULT '0',
  `title` varchar(80) NOT NULL DEFAULT 'Buy From Me!',
  PRIMARY KEY (`account_id`,`char_id`)
) ENGINE=MyISAM; 
CREATE TABLE IF NOT EXISTS `autotrade_data` (
  `char_id` INT(11) NOT NULL DEFAULT '0',
  `itemkey` INT(11) NOT NULL DEFAULT '0',
  `amount` INT(11) NOT NULL DEFAULT '0',
  `price` INT(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`char_id`,`itemkey`)
) ENGINE=MyISAM; 
INSERT INTO `sql_updates` (`timestamp`) VALUES (1389028967);
