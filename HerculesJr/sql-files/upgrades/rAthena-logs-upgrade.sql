-- rAthena to Hercules log database upgrade query.
-- This upgrades a FULLY UPGRADED rAthena to a FULLY UPGRADED Hercules
-- Please don't use if either rAthena or Hercules launched a SQL update after last revision date of this file.
-- Remember to make a backup before applying.
-- We are not liable for any data loss this may cause.
-- Apply in the same database you applied your logs.sql
-- Last revised: July 22, 2014 20:45 GMT

-- Drop table `cashlog` since it's not used in Hercules
-- Comment it if you wish to keep the table
DROP TABLE IF EXISTS `cashlog`;

-- Upgrades to table `mvplog`
ALTER TABLE `mvplog` MODIFY `prize` INT(11) NOT NULL DEFAULT '0';

-- Upgrades to table `picklog`
ALTER TABLE `picklog` MODIFY `type` enum('M','P','L','T','V','S','N','C','A','R','G','E','B','O','I','X','D','U') NOT NULL default 'P';
ALTER TABLE `picklog` MODIFY `nameid` INT(11) NOT NULL DEFAULT '0';
ALTER TABLE `picklog` MODIFY `card0` INT(11) NOT NULL DEFAULT '0';
ALTER TABLE `picklog` MODIFY `card1` INT(11) NOT NULL DEFAULT '0';
ALTER TABLE `picklog` MODIFY `card2` INT(11) NOT NULL DEFAULT '0';
ALTER TABLE `picklog` MODIFY `card3` INT(11) NOT NULL DEFAULT '0';
