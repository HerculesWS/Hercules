#1382892428
ALTER TABLE `inventory` MODIFY `equip` int(11) unsigned NOT NULL default '0';
ALTER TABLE `storage` MODIFY `equip` int(11) unsigned NOT NULL default '0';
ALTER TABLE `cart_inventory` MODIFY `equip` int(11) unsigned NOT NULL default '0';
ALTER TABLE `guild_storage` MODIFY `equip` int(11) unsigned NOT NULL default '0';
INSERT INTO `sql_updates` (`timestamp`) VALUES (1382892428);
