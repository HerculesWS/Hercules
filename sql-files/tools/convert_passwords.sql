-- Convert passwords to MD5 Hash

UPDATE `login` SET `user_pass`=MD5(`user_pass`);
