#1425345071
ALTER TABLE  `login` ADD COLUMN `auth_hash` BINARY(64) NOT NULL DEFAULT '' AFTER `user_pass`;
ALTER TABLE  `login` ADD COLUMN `auth_salt` BINARY(64) NOT NULL DEFAULT '' AFTER `auth_hash`;
ALTER TABLE  `login` ADD COLUMN `auth_iter_count` INT(11) UNSIGNED NOT NULL DEFAULT '0' AFTER `auth_iter_count`;
INSERT INTO `sql_updates` (`timestamp`) VALUES (1425345071);
