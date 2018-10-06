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


## How do I stop using global configurations?

To stop using global configuration, all you have to do is copy the contents of
the file being imported and paste it _exactly_ where the include directive was.

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
