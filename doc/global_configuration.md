# Global configuration reference

## What is global configuration?

Global configuration is an import system that allows configuration files to be
shared between servers (login, char, map), but can also be used independently
in each server.


## How does it work?

It works by using the `@include` directive from libconfig:

> "A configuration file may "include" the contents of another file using an
>  include directive. This directive has the effect of inlining the contents of
>  the named file at the point of inclusion.

An include directive must appear on its own line and takes this form:

```
	@include "filename"
```

Any backslashes or double quotes in the filename must be escaped as `\\` and
`\"`, respectively.

## Changing a global configuration based on the server

Include directives will first look into
`conf/import/include/[SERVERNAME]/[INCLUDE_PATH]` first, and after that,
search for the file in `[INCLUDE_PATH]`.

Where:
- `[SERVERNAME]` stands for the server executable that is reading the config, it may be:
	- `login-server` (`login-server.exe` in Windows systems)
	- `char-sever` (`char-server.exe` in Windows systems)
	- `map-server` (`map-server.exe` in Windows systems)
- `[INCLUDE_PATH]` the value given to the `@include` directive.

It is important to note that, when a file is created in include folder,
the original file will **NOT** be loaded.
In other words, if you are creating an include file to change only 1 setting,
make sure to copy the other ones too, or they will not be defined.

### Example

For example, SQL connection settings is defined in
`conf/global/sql_connection.conf`,
and servers include it by using the following `@include` directive:

```
	@include "conf/global/sql_connection.conf"
```

If you want to change char server's SQL connection settings, in an Unix system,
you may create the file
`conf/import/include/char-server/conf/global/sql_connection.conf`
with your settings in it, for example:

```
	sql_connection: {
		// Those settings are copied as is
		// because we are no longer loading the original file
		db_hostname: "127.0.0.1"
		db_port: 3306
		db_username: "ragnarok"
		db_password: "ragnarok"
		
		// This is the actual change
		db_database: "hercules"
	}
```

This would make char server use `hercules` as its database name.

> For Windows systems, the folder `char-server` would be called `char-server.exe` instead.

## How do I stop using global configurations?

To stop using global configuration, all you have to do is copy the contents of
the file being imported and paste it _exactly_ where the include directive was.

Depending on what you want to achieve,
[Changing a global configuration based on the server](#changing-a-global-configuration-based-on-the-server)
may be a better alternative.

### Example

If you want map server and char server to have their own separate SQL connection
settings, you would search in `conf/map/map-server.conf` and
`conf/char/char-server.conf` for this line:

```
	@include "conf/global/sql_connection.conf"
```

And replace it with:

```
	sql_connection: {
		// [INTER] You can specify the codepage to use in your mySQL tables here.
		// (Note that this feature requires MySQL 4.1+)
		//default_codepage: ""

		// [LOGIN] Is `userid` in account_db case sensitive?
		//case_sensitive: false

		// For IPs, ideally under linux, you want to use localhost instead of 127.0.0.1.
		// Under windows, you want to use 127.0.0.1.  If you see a message like
		// "Can't connect to local MySQL server through socket '/tmp/mysql.sock' (2)"
		// and you have localhost, switch it to 127.0.0.1
		db_hostname: "127.0.0.1"
		db_port: 3306
		db_username: "ragnarok"
		db_password: "ragnarok"
		db_database: "ragnarok"
		//codepage:""
	}
```
