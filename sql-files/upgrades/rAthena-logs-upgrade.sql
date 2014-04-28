-- rAthena to Hercules log database upgrade query.
-- This upgrades a FULLY UPGRADED rAthena to a FULLY UPGRADED Hercules
-- Please don't use if either rAthena or Hercules launched a SQL update after last revision date of this file.
-- Remember to make a backup before applying.
-- We are not liable for any data loss this may cause.
-- Apply in the same database you applied your logs.sql
-- Last revised: March 21, 2014 20:30 GMT

-- Upgrades to table `picklog`
ALTER TABLE `picklog` MODIFY `type` enum('M','P','L','T','V','S','N','C','A','R','G','E','B','O','I','X','D','U') NOT NULL default 'P';

-- Drop table `cashlog` since it's not used in Hercules
DROP TABLE IF EXISTS `cashlog`;
