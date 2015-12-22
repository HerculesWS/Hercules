#1447620161
DELETE FROM `acc_reg_num_db` WHERE LENGTH(`key`) > 31;
DELETE FROM `acc_reg_str_db` WHERE LENGTH(`key`) > 31;
DELETE FROM `char_reg_num_db` WHERE LENGTH(`key`) > 31;
DELETE FROM `char_reg_str_db` WHERE LENGTH(`key`) > 31;
DELETE FROM `global_acc_reg_num_db` WHERE LENGTH(`key`) > 31;
DELETE FROM `global_acc_reg_str_db` WHERE LENGTH(`key`) > 31;
DELETE FROM `mapreg` WHERE LENGTH(`varname`) > 31;

ALTER TABLE `acc_reg_num_db` MODIFY COLUMN `key` VARCHAR(31) NOT NULL DEFAULT '';
ALTER TABLE `acc_reg_str_db` MODIFY COLUMN `key` VARCHAR(31) NOT NULL DEFAULT '';
ALTER TABLE `char_reg_num_db` MODIFY COLUMN `key` VARCHAR(31) NOT NULL DEFAULT '';
ALTER TABLE `char_reg_str_db` MODIFY COLUMN `key` VARCHAR(31) NOT NULL DEFAULT '';
ALTER TABLE `global_acc_reg_num_db` MODIFY COLUMN `key` VARCHAR(31) NOT NULL DEFAULT '';
ALTER TABLE `global_acc_reg_str_db` MODIFY COLUMN `key` VARCHAR(31) NOT NULL DEFAULT '';
ALTER TABLE `mapreg` MODIFY COLUMN `varname` VARCHAR(31) NOT NULL DEFAULT '';

INSERT INTO `sql_updates` (`timestamp`) VALUES (1447620161);
