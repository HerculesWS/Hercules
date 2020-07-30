/* Copyright Abandoned 1996, 1999, 2001 MySQL AB
   This file is public domain and comes with NO WARRANTY of any kind */

/* Version numbers for protocol & mysqld */

#ifndef _mariadb_version_h_
#define _mariadb_version_h_

#ifdef _CUSTOMCONFIG_
#include <custom_conf.h>
#else
#define PROTOCOL_VERSION		10
#define MARIADB_CLIENT_VERSION_STR	"10.4.3"
#define MARIADB_BASE_VERSION		"mariadb-10.4"
#define MARIADB_VERSION_ID		100403
#define MARIADB_PORT	        	3306
#define MARIADB_UNIX_ADDR               "/tmp/mysql.sock"

#define MYSQL_CONFIG_NAME               "my"
#define MYSQL_VERSION_ID                100403
#define MYSQL_SERVER_VERSION            "10.4.3-MariaDB"

#define MARIADB_PACKAGE_VERSION "3.1.5"
#define MARIADB_PACKAGE_VERSION_ID 30105
#define MARIADB_SYSTEM_TYPE "Windows"
#define MARIADB_MACHINE_TYPE "AMD64"
#define MARIADB_PLUGINDIR "./3rdparty/mariadb/lib/plugin"

/* mysqld compile time options */
#ifndef MYSQL_CHARSET
#define MYSQL_CHARSET			""
#endif
#endif

/* Source information */
#define CC_SOURCE_REVISION "980f2dbea6586091333057bb2994b18747466942"

#endif /* _mariadb_version_h_ */
