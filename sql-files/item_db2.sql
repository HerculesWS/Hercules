#
# Table structure for table `item_db2`
#

DROP TABLE IF EXISTS `item_db2`;
CREATE TABLE `item_db2` (
   `id` smallint(5) unsigned NOT NULL DEFAULT '0',
   `name_english` varchar(50) NOT NULL DEFAULT '',
   `name_japanese` varchar(50) NOT NULL DEFAULT '',
   `type` tinyint(2) unsigned NOT NULL DEFAULT '0',
   `price_buy` mediumint(10) DEFAULT NULL,
   `price_sell` mediumint(10) DEFAULT NULL,
   `weight` smallint(5) unsigned DEFAULT NULL,
   `atk` smallint(5) unsigned DEFAULT NULL,
   `matk` smallint(5) unsigned DEFAULT NULL,
   `defence` smallint(5) unsigned DEFAULT NULL,
   `range` tinyint(2) unsigned DEFAULT NULL,
   `slots` tinyint(2) unsigned DEFAULT NULL,
   `equip_jobs` int(12) unsigned DEFAULT NULL,
   `equip_upper` tinyint(8) unsigned DEFAULT NULL,
   `equip_genders` tinyint(2) unsigned DEFAULT NULL,
   `equip_locations` smallint(4) unsigned DEFAULT NULL,
   `weapon_level` tinyint(2) unsigned DEFAULT NULL,
   `equip_level_min` smallint(5) unsigned DEFAULT NULL,
   `equip_level_max` smallint(5) unsigned DEFAULT NULL,
   `refineable` tinyint(1) unsigned DEFAULT NULL,
   `view` smallint(3) unsigned DEFAULT NULL,
   `bindonequip` tinyint(1) unsigned DEFAULT NULL,
   `script` text,
   `equip_script` text,
   `unequip_script` text,
 PRIMARY KEY (`id`)
) ENGINE=MyISAM;

