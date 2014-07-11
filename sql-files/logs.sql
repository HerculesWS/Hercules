-- PickLog Types
--	(M)onsters Drop
--	(P)layers Drop/Take
--	Mobs Drop (L)oot Drop/Take
--	Players (T)rade Give/Take
--	Players (V)ending Sell/Take
--	(S)hop Sell/Take
--	(N)PC Give/Take
--	(C)onsumable Items
--	(A)dministrators Create/Delete
--	Sto(R)age
--	(G)uild Storage
--	(E)mail attachment
--	(B)uying Store
--	Pr(O)duced Items/Ingredients
--	Auct(I)oned Items
--	(X) Other
--	(D) Stolen from mobs
--	(U) MVP Prizes

--
-- Table structure for table `atcommandlog`
--

CREATE TABLE IF NOT EXISTS `atcommandlog` (
  `atcommand_id` MEDIUMINT(9) UNSIGNED NOT NULL AUTO_INCREMENT,
  `atcommand_date` DATETIME NOT NULL DEFAULT '0000-00-00 00:00:00',
  `account_id` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `char_id` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `char_name` VARCHAR(25) NOT NULL DEFAULT '',
  `map` VARCHAR(11) NOT NULL DEFAULT '',
  `command` VARCHAR(255) NOT NULL DEFAULT '',
  PRIMARY KEY (`atcommand_id`),
  INDEX (`account_id`),
  INDEX (`char_id`)
) ENGINE=MyISAM AUTO_INCREMENT=1 ;

--
-- Table structure for table `branchlog`
--

CREATE TABLE IF NOT EXISTS `branchlog` (
  `branch_id` MEDIUMINT(9) UNSIGNED NOT NULL AUTO_INCREMENT,
  `branch_date` DATETIME NOT NULL DEFAULT '0000-00-00 00:00:00',
  `account_id` INT(11) NOT NULL DEFAULT '0',
  `char_id` INT(11) NOT NULL DEFAULT '0',
  `char_name` VARCHAR(25) NOT NULL DEFAULT '',
  `map` VARCHAR(11) NOT NULL DEFAULT '',
  PRIMARY KEY(`branch_id`),
  INDEX (`account_id`),
  INDEX (`char_id`)
) ENGINE=MyISAM AUTO_INCREMENT=1;

--
-- Table structure for table `chatlog`
--

CREATE TABLE IF NOT EXISTS `chatlog` (
  `id` BIGINT(20) NOT NULL AUTO_INCREMENT,
  `time` DATETIME NOT NULL DEFAULT '0000-00-00 00:00:00',
  `type` ENUM('O','W','P','G','M') NOT NULL DEFAULT 'O',
  `type_id` INT(11) NOT NULL DEFAULT '0',
  `src_charid` INT(11) NOT NULL DEFAULT '0',
  `src_accountid` INT(11) NOT NULL DEFAULT '0',
  `src_map` VARCHAR(11) NOT NULL DEFAULT '',
  `src_map_x` SMALLINT(4) NOT NULL DEFAULT '0',
  `src_map_y` SMALLINT(4) NOT NULL DEFAULT '0',
  `dst_charname` VARCHAR(25) NOT NULL DEFAULT '',
  `message` VARCHAR(150) NOT NULL DEFAULT '',
  PRIMARY KEY (`id`),
  INDEX (`src_accountid`),
  INDEX (`src_charid`)
) ENGINE=MyISAM AUTO_INCREMENT=1;

--
-- Table structure for table `loginlog`
--

CREATE TABLE IF NOT EXISTS `loginlog` (
  `time` DATETIME NOT NULL DEFAULT '0000-00-00 00:00:00',
  `ip` VARCHAR(15) NOT NULL DEFAULT '',
  `user` VARCHAR(23) NOT NULL DEFAULT '',
  `rcode` TINYINT(4) NOT NULL DEFAULT '0',
  `log` VARCHAR(255) NOT NULL DEFAULT '',
  INDEX (`ip`)
) ENGINE=MyISAM;

--
-- Table structure for table `mvplog`
--

CREATE TABLE IF NOT EXISTS `mvplog` (
  `mvp_id` MEDIUMINT(9) UNSIGNED NOT NULL AUTO_INCREMENT,
  `mvp_date` DATETIME NOT NULL DEFAULT '0000-00-00 00:00:00',
  `kill_char_id` INT(11) NOT NULL DEFAULT '0',
  `monster_id` SMALLINT(6) NOT NULL DEFAULT '0',
  `prize` INT(11) NOT NULL DEFAULT '0',
  `mvpexp` MEDIUMINT(9) NOT NULL DEFAULT '0',
  `map` VARCHAR(11) NOT NULL DEFAULT '',
  PRIMARY KEY (`mvp_id`)
) ENGINE=MyISAM AUTO_INCREMENT=1;

--
-- Table structure for table `npclog`
--

CREATE TABLE IF NOT EXISTS `npclog` (
  `npc_id` MEDIUMINT(9) UNSIGNED NOT NULL AUTO_INCREMENT,
  `npc_date` DATETIME NOT NULL DEFAULT '0000-00-00 00:00:00',
  `account_id` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `char_id` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `char_name` VARCHAR(25) NOT NULL DEFAULT '',
  `map` VARCHAR(11) NOT NULL DEFAULT '',
  `mes` VARCHAR(255) NOT NULL DEFAULT '',
  PRIMARY KEY (`npc_id`),
  INDEX (`account_id`),
  INDEX (`char_id`)
) ENGINE=MyISAM AUTO_INCREMENT=1;

--
-- Table structure for table `picklog`
--

CREATE TABLE IF NOT EXISTS `picklog` (
  `id` INT(11) NOT NULL AUTO_INCREMENT,
  `time` DATETIME NOT NULL DEFAULT '0000-00-00 00:00:00',
  `char_id` INT(11) NOT NULL DEFAULT '0',
  `type` ENUM('M','P','L','T','V','S','N','C','A','R','G','E','B','O','I','X','D','U') NOT NULL DEFAULT 'P',
  `nameid` INT(11) NOT NULL DEFAULT '0',
  `amount` INT(11) NOT NULL DEFAULT '1',
  `refine` TINYINT(3) UNSIGNED NOT NULL DEFAULT '0',
  `card0` INT(11) NOT NULL DEFAULT '0',
  `card1` INT(11) NOT NULL DEFAULT '0',
  `card2` INT(11) NOT NULL DEFAULT '0',
  `card3` INT(11) NOT NULL DEFAULT '0',
  `unique_id` BIGINT(20) UNSIGNED NOT NULL DEFAULT '0',
  `map` VARCHAR(11) NOT NULL DEFAULT '',
  PRIMARY KEY (`id`),
  INDEX (`type`)
) ENGINE=MyISAM AUTO_INCREMENT=1;

--
-- Table structure for table `zenylog`
--

CREATE TABLE IF NOT EXISTS `zenylog` (
  `id` INT(11) NOT NULL AUTO_INCREMENT,
  `time` DATETIME NOT NULL DEFAULT '0000-00-00 00:00:00',
  `char_id` INT(11) NOT NULL DEFAULT '0',
  `src_id` INT(11) NOT NULL DEFAULT '0',
  `type` ENUM('T','V','P','M','S','N','D','C','A','E','I','B') NOT NULL DEFAULT 'S',
  `amount` INT(11) NOT NULL DEFAULT '0',
  `map` VARCHAR(11) NOT NULL DEFAULT '',
  PRIMARY KEY (`id`),
  INDEX (`type`)
) ENGINE=MyISAM AUTO_INCREMENT=1;

