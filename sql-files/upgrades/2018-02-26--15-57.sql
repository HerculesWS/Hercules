#1519671456

ALTER TABLE `rodex_mail`
	ADD COLUMN `sender_read` TINYINT(2) NOT NULL DEFAULT '0' AFTER `is_read`;

INSERT INTO `sql_updates` (`timestamp`, `ignored`) VALUES (1519671456 , 'No');
