-- rAthena to Hercules log database upgrade query.
-- This upgrades a FULLY UPGRADED rAthena to a FULLY UPGRADED Hercules
-- Please don't use if either rAthena or Hercules launched a SQL update after last revision date of this file.
-- Remember to make a backup before applying.
-- We are not liable for any data loss this may cause.
-- Apply in the same database you applied your logs.sql
-- Last revision: 3/12/2014 7:24:15 PM

--
-- cashlog
-- 

DROP TABLE cashlog;

--
-- picklog
-- 

ALTER TABLE picklog CHANGE COLUMN type type ENUM('M','P','L','T','V','S','N','C','A','R','G','E','B','O','I','X','D','U') DEFAULT 'P' NOT NULL COMMENT '';

