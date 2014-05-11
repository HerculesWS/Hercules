#!/bin/bash

MODE="$1"
shift

function foo {
	for i in "$@"; do
		echo "> $i"
	done
}

function usage {
	echo "usage:"
	echo "    $0 createdb <dbname> [dbuser] [dbpassword]"
	echo "    $0 importdb <dbname> [dbuser] [dbpassword]"
	echo "    $0 build [configure args]"
	echo "    $0 test <dbname> [dbuser] [dbpassword]"
	echo "    $0 getplugins"
	exit 1
}

function aborterror {
	echo $@
	exit 1
}

case "$MODE" in
	createdb|importdb|test)
		DBNAME="$1"
		DBUSER="$2"
		DBPASS="$3"
		if [ -z "$DBNAME" ]; then
			usage
		fi
		if [ "$MODE" != "test" ]; then
			if [ -n "$DBUSER" ]; then
				DBUSER="-u $DBUSER"
			fi
			if [ -n "$DBPASS" ]; then
				DBPASS="-p$DBPASS"
			fi
		fi
		;;
esac

case "$MODE" in
	createdb)
		echo "Creating database $DBNAME..."
		mysql $DBUSER $DBPASS -e "create database $DBNAME;" || aborterror "Unable to create database."
		;;
	importdb)
		echo "Importing tables into $DBNAME..."
		mysql $DBUSER $DBPASS $DBNAME < sql-files/main.sql || aborterror "Unable to import main database."
		mysql $DBUSER $DBPASS $DBNAME < sql-files/logs.sql || aborterror "Unable to import logs database."
		;;
	build)
		./configure $@ || aborterror "Configure error, aborting build."
		make sql -j3 || aborterror "Build failed."
		if [ -f src/plugins/script_mapquit.c ]; then
			make plugin.script_mapquit -j3 || aborterror "Build failed."
		fi
		;;
	test)
		cat >> conf/import/login_conf.txt << EOF
ipban.sql.db_username: $DBUSER
ipban.sql.db_password: $DBPASS
ipban.sql.db_database: $DBNAME
account.sql.db_username: $DBUSER
account.sql.db_password: $DBPASS
account.sql.db_database: $DBNAME
account.sql.db_hostname: localhost
EOF
		[ $? -eq 0 ] || aborterror "Unable to import configuration, aborting tests."
		cat >> conf/import/inter_conf.txt << EOF
sql.db_username: $DBUSER
sql.db_password: $DBPASS
sql.db_database: $DBNAME
sql.db_hostname: localhost
char_server_id: $DBUSER
char_server_pw: $DBPASS
char_server_db: $DBNAME
char_server_ip: localhost
map_server_id: $DBUSER
map_server_pw: $DBPASS
map_server_db: $DBNAME
map_server_ip: localhost
log_db_id: $DBUSER
log_db_pw: $DBPASS
log_db_db: $DBNAME
log_db_ip: localhost
EOF
		[ $? -eq 0 ] || aborterror "Unable to import configuration, aborting tests."
		ARGS="--load-script npc/dev/test.txt "
		if [ -f src/plugins/script_mapquit.c ]; then
			ARGS="--load-plugin script_mapquit $ARGS --load-script npc/dev/ci_test.txt"
		fi
		echo "Running Hercules with command line: ./map-server --run-once $ARGS"
		./map-server --run-once $ARGS || aborterror "Test failed."
		;;
	getplugins)
		echo "Cloning plugins repository..."
		git clone http://github.com/HerculesWS/StaffPlugins.git || aborterror "Unable to fetch plugin repository"
		if [ -f StaffPlugins/Haru/script_mapquit/script_mapquit.c -a -f StaffPlugins/Haru/script_mapquit/examples/ci_test.txt ]; then
			pushd src/plugins || aborterror "Unable to enter plugins directory."
			ln -s ../../StaffPlugins/Haru/script_mapquit/script_mapquit.c ./
			popd
			pushd npc/dev || aborterror "Unable to enter scripts directory."
			ln -s ../../StaffPlugins/Haru/script_mapquit/examples/ci_test.txt ./
			popd
		else
			echo "Plugin not found, skipping advanced tests."
		fi
		;;
	*)
		usage
		;;
esac
