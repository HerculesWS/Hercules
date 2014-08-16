-- NOTE: This file was auto-generated and should never be manually edited,
--       as it will get overwritten. If you need to modify this file,
--       please consider modifying the corresponding .conf file inside
--       the db folder, and then re-run the db2sql plugin.

--
-- Table structure for table `item_db2`
--

DROP TABLE IF EXISTS `item_db2`;
CREATE TABLE `item_db2` (
  `id` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `name_english` varchar(50) NOT NULL DEFAULT '',
  `name_japanese` varchar(50) NOT NULL DEFAULT '',
  `type` tinyint(2) UNSIGNED NOT NULL DEFAULT '0',
  `price_buy` mediumint(10) DEFAULT NULL,
  `price_sell` mediumint(10) DEFAULT NULL,
  `weight` smallint(5) UNSIGNED DEFAULT NULL,
  `atk` smallint(5) UNSIGNED DEFAULT NULL,
  `matk` smallint(5) UNSIGNED DEFAULT NULL,
  `defence` smallint(5) UNSIGNED DEFAULT NULL,
  `range` tinyint(2) UNSIGNED DEFAULT NULL,
  `slots` tinyint(2) UNSIGNED DEFAULT NULL,
  `equip_jobs` int(12) UNSIGNED DEFAULT NULL,
  `equip_upper` tinyint(8) UNSIGNED DEFAULT NULL,
  `equip_genders` tinyint(2) UNSIGNED DEFAULT NULL,
  `equip_locations` smallint(4) UNSIGNED DEFAULT NULL,
  `weapon_level` tinyint(2) UNSIGNED DEFAULT NULL,
  `equip_level_min` smallint(5) UNSIGNED DEFAULT NULL,
  `equip_level_max` smallint(5) UNSIGNED DEFAULT NULL,
  `refineable` tinyint(1) UNSIGNED DEFAULT NULL,
  `view` smallint(3) UNSIGNED DEFAULT NULL,
  `bindonequip` tinyint(1) UNSIGNED DEFAULT NULL,
  `buyingstore` tinyint(1) UNSIGNED DEFAULT NULL,
  `delay` mediumint(9) UNSIGNED DEFAULT NULL,
  `trade_flag` smallint(4) UNSIGNED DEFAULT NULL,
  `trade_group` smallint(3) UNSIGNED DEFAULT NULL,
  `nouse_flag` smallint(4) UNSIGNED DEFAULT NULL,
  `nouse_group` smallint(4) UNSIGNED DEFAULT NULL,
  `stack_amount` mediumint(6) UNSIGNED DEFAULT NULL,
  `stack_flag` tinyint(2) UNSIGNED DEFAULT NULL,
  `sprite` mediumint(6) UNSIGNED DEFAULT NULL,
  `script` text,
  `equip_script` text,
  `unequip_script` text,
 PRIMARY KEY (`id`)
) ENGINE=MyISAM;

