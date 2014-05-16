#1381354728
ALTER TABLE `zenylog` MODIFY `type` enum('T','V','P','M','S','N','D','C','A','E','I','B','K') NOT NULL DEFAULT 'S';
INSERT INTO `sql_updates` (`timestamp`) VALUES (1381354728);
