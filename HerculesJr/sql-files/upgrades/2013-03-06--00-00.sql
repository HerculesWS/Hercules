#1362528000
-- This script resets all dewata quests that were done by your users before this revision
-- Author: Euphy
DELETE FROM `quest` WHERE `quest_id` > 5034 AND `quest_id` < 5055;
DELETE FROM `quest` WHERE `quest_id` > 9154 AND `quest_id` < 9166;
DELETE FROM `global_reg_value` WHERE `str` = 'dewata_gatti';
DELETE FROM `global_reg_value` WHERE `str` = 'dewata_legend';
DELETE FROM `global_reg_value` WHERE `str` = 'dewata_oldman';
INSERT INTO `sql_updates` (`timestamp`) VALUES (1362528000);
