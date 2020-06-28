#!/usr/bin/env bash

# This file is part of Hercules.
# http://herc.ws - http://github.com/HerculesWS/Hercules
#
# Copyright (C) 2014-2020 Hercules Dev Team
#
# Hercules is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

# Base Author: Haru @ http://herc.ws

MODE="$1"
shift

function foo {
	for i in "$@"; do
		echo "> $i"
	done
}

function usage {
	echo "usage:"
	echo "    $0 createdb <dbname> [dbuser] [dbpassword] [dbhost]"
	echo "    $0 importdb <dbname> [dbuser] [dbpassword] [dbhost]"
	echo "    $0 adduser <dbname> <new_user> <new_user_password> [dbuser] [dbpassword] [dbhost]"
	echo "    $0 build [configure args]"
	echo "    $0 test <dbname> [dbuser] [dbpassword] [dbhost]"
	echo "    $0 getplugins"
	echo "    $0 startmysql"
	echo "    $0 extratest"
	exit 1
}

function aborterror {
	echo $@
	exit 1
}

function run_server {
	echo "Running: $1 --run-once $2"
	rm -rf core* || true
	CRASH_PLEASE=1 $1 --run-once $2 2>runlog.txt
	export errcode=$?
	export teststr=$(head -c 10000 runlog.txt)
	if [[ -n "${teststr}" ]]; then
		echo "Errors found in running server $1."
		head -c 10000 runlog.txt
		aborterror "Errors found in running server $1."
	else
		echo "No errors found for server $1."
	fi
	if [ ${errcode} -ne 0 ]; then
		echo "server $1 terminated with exit code ${errcode}"
		COREFILE=$(find . -maxdepth 1 -name "core*" | head -n 1)
		if [[ -f "$COREFILE" ]]; then
			gdb -c "$COREFILE" $1 -ex "thread apply all bt" -ex "set pagination 0" -batch
		fi
		aborterror "Test failed"
	fi
}

function run_test {
	echo "Running: test_$1"
	sysctl -w kernel.core_pattern=core || true
	rm -rf core* || true
	CRASH_PLEASE=1 ./test_$1 2>runlog.txt
	export errcode=$?
	export teststr=$(head -c 10000 runlog.txt)
	if [[ -n "${teststr}" ]]; then
		echo "Errors found in running test $1."
		head -c 10000 runlog.txt
		aborterror "Errors found in running test $1."
	else
		echo "No errors found for test $1."
	fi
	if [ ${errcode} -ne 0 ]; then
		echo "test $1 terminated with exit code ${errcode}"
		echo cat runlog.txt
		cat runlog.txt
		echo crash dump
		COREFILE=$(find . -maxdepth 1 -name "core*" | head -n 1)
		if [[ -f "$COREFILE" ]]; then
			gdb -c "$COREFILE" $1 -ex "thread apply all bt" -ex "set pagination 0" -batch
		fi
		aborterror "Test failed"
	fi
}

# Defaults
DBNAME=ragnarok
DBUSER=ragnarok
DBPASS=ragnarok
DBHOST=localhost

case "$MODE" in
	createdb|importdb|test)
		if [ -z "$1" ]; then
			usage
		fi
		DBNAME="$1"
		if [ -n "$2" ]; then
			DBUSER_ARG="--user=$2"
			DBUSER="$2"
		fi
		if [ -n "$3" ]; then
			DBPASS_ARG="--password=$3"
			DBPASS="$3"
		fi
		if [ -n "$4" ]; then
			DBHOST_ARG="--host=$4"
			DBHOST="$4"
		fi
		;;
	adduser)
		if [ -z "$3" ]; then
			usage
		fi
		DBNAME="$1"
		NEWUSER="$2"
		NEWPASS="$3"
		if [ -n "$4" ]; then
			DBUSER_ARG="--user=$4"
			DBUSER="$4"
		fi
		if [ -n "$5" ]; then
			DBPASS_ARG="--password=$5"
			DBPASS="$5"
		fi
		if [ -n "$6" ]; then
			DBHOST_ARG="--host=$6"
			DBHOST="$6"
		fi
		;;
esac

case "$MODE" in
	createdb)
		echo "Creating database $DBNAME as $DBUSER..."
		mysql $DBUSER_ARG $DBPASS_ARG $DBHOST_ARG --execute="CREATE DATABASE $DBNAME;" || aborterror "Unable to create database."
		;;
	importdb)
		echo "Importing tables into $DBNAME as $DBUSER..."
		mysql $DBUSER_ARG $DBPASS_ARG $DBHOST_ARG --database=$DBNAME < sql-files/main.sql || aborterror "Unable to import main database."
		mysql $DBUSER_ARG $DBPASS_ARG $DBHOST_ARG --database=$DBNAME < sql-files/logs.sql || aborterror "Unable to import logs database."
		;;
	adduser)
		echo "Adding user $NEWUSER as $DBUSER, with access to database $DBNAME..."
		mysql $DBUSER_ARG $DBPASS_ARG $DBHOST_ARG --execute="GRANT SELECT,INSERT,UPDATE,DELETE ON $DBNAME.* TO '$NEWUSER'@'$DBHOST' IDENTIFIED BY '$NEWPASS';" || true
		mysql $DBUSER_ARG $DBPASS_ARG $DBHOST_ARG --execute="CREATE USER '$NEWUSER'@'$DBHOST' IDENTIFIED BY '$NEWPASS';" || true
		mysql $DBUSER_ARG $DBPASS_ARG $DBHOST_ARG --execute="GRANT SELECT,INSERT,UPDATE,DELETE ON $DBNAME.* TO '$NEWUSER'@'$DBHOST';" || true
		mysql $DBUSER_ARG $DBPASS_ARG $DBHOST_ARG --execute="ALTER USER '$NEWUSER'@'$DBHOST' IDENTIFIED BY '$NEWPASS';" || true
		mysql --defaults-file=/etc/mysql/debian.cnf $DBPASS_ARG $DBHOST_ARG --execute="CREATE USER '$NEWUSER'@'$DBHOST' IDENTIFIED BY '$NEWPASS';" || true
		mysql --defaults-file=/etc/mysql/debian.cnf $DBPASS_ARG $DBHOST_ARG --execute="GRANT SELECT,INSERT,UPDATE,DELETE ON $DBNAME.* TO '$NEWUSER'@'$DBHOST';" || true
		mysql --defaults-file=/etc/mysql/debian.cnf $DBPASS_ARG $DBHOST_ARG --execute="ALTER USER '$NEWUSER'@'$DBHOST' IDENTIFIED BY '$NEWPASS';" || true

		;;
	build)
		(cd tools && ./validateinterfaces.py silent) || aborterror "Interface validation error."
		./configure $@ || (cat config.log && aborterror "Configure error, aborting build.")
		make -j3 || aborterror "Build failed."
		make plugins -j3 || aborterror "Build failed."
		make plugin.script_mapquit -j3 || aborterror "Build failed."
		make test || aborterror "Build failed."
		;;
	buildhpm)
		./configure $@ || (cat config.log && aborterror "Configure error, aborting build.")
		cd tools/HPMHookGen
		make
		;;
	test)
		cat > conf/travis_sql_connection.conf << EOF
sql_connection: {
	//default_codepage: ""
	//case_sensitive: false
	db_hostname: "$DBHOST"
	db_username: "$DBUSER"
	db_password: "$DBPASS"
	db_database: "$DBNAME"
	//codepage:""
}
EOF
		[ $? -eq 0 ] || aborterror "Unable to write database configuration, aborting tests."
		cat > conf/import/login-server.conf << EOF
login_configuration: {
	account: {
		@include "conf/travis_sql_connection.conf"
		ipban: {
			@include "conf/travis_sql_connection.conf"
		}
	}
}
EOF
		[ $? -eq 0 ] || aborterror "Unable to override login-server configuration, aborting tests."
		cat > conf/import/char-server.conf << EOF
char_configuration: {
	@include "conf/travis_sql_connection.conf"
}
EOF
		[ $? -eq 0 ] || aborterror "Unable to override char-server configuration, aborting tests."
		cat > conf/import/map-server.conf << EOF
map_configuration: {
	@include "conf/travis_sql_connection.conf"
}
EOF
		[ $? -eq 0 ] || aborterror "Unable to override map-server configuration, aborting tests."
		cat > conf/import/inter-server.conf << EOF
inter_configuration: {
	log: {
		@include "conf/travis_sql_connection.conf"
	}
}
EOF
		[ $? -eq 0 ] || aborterror "Unable to override inter-server configuration, aborting tests."
		ARGS="--load-script npc/dev/test.txt "
		ARGS="--load-plugin script_mapquit $ARGS --load-script npc/dev/ci_test.txt"
		PLUGINS="--load-plugin HPMHooking"
		echo "run tests"
		if [[ $DBUSER == "travis" ]]; then
			echo "Disable leak dection on travis"
			export ASAN_OPTIONS=detect_leaks=0:detect_stack_use_after_return=true:strict_init_order=true:detect_odr_violation=0
		else
			export ASAN_OPTIONS=detect_stack_use_after_return=true:strict_init_order=true:detect_odr_violation=0
		fi
		# run_test spinlock # Not running the spinlock test for the time being (too time consuming)
		run_test libconfig
		echo "run all servers without HPM"
		run_server ./login-server
		run_server ./char-server
		run_server ./map-server "$ARGS"
		echo "run all servers with HPM"
		run_server ./login-server "$PLUGINS"
		run_server ./char-server "$PLUGINS"
		run_server ./map-server "$ARGS $PLUGINS"
		echo "run all servers with sample plugin"
		run_server ./login-server "$PLUGINS --load-plugin sample"
		run_server ./char-server "$PLUGINS --load-plugin sample"
		run_server ./map-server "$PLUGINS --load-plugin sample"
		echo "run all servers with constdb2doc"
		run_server ./map-server "$PLUGINS --load-plugin constdb2doc --constdb2doc"
		echo "run all servers with db2sql"
		run_server ./map-server "$PLUGINS --load-plugin db2sql --db2sql"
		run_server ./map-server "$PLUGINS --load-plugin db2sql --itemdb2sql"
		run_server ./map-server "$PLUGINS --load-plugin db2sql --mobdb2sql"
# look like works on windows only
#		echo "run all servers with dbghelpplug"
#		run_server ./login-server "$PLUGINS --load-plugin dbghelpplug"
#		run_server ./char-server "$PLUGINS --load-plugin dbghelpplug"
#		run_server ./map-server "$PLUGINS --load-plugin dbghelpplug"
		echo "run all servers with generate-translations"
		run_server ./map-server "$PLUGINS --load-plugin generate-translations --generate-translations"
		echo "run all servers with mapcache"
# for other flags need grf or other files
		run_server ./map-server "$PLUGINS --load-plugin mapcache --fix-md5"
		echo "run all servers with script_mapquit"
		run_server ./map-server "$PLUGINS --load-plugin script_mapquit"
		;;
	extratest)
		export ASAN_OPTIONS=detect_stack_use_after_return=true:strict_init_order=true:detect_odr_violation=0
		PLUGINS="--load-plugin HPMHooking"
		echo "run map server with uncommented old and custom scripts"
		find ./npc -type f -name "*.conf" -exec ./tools/ci/uncomment.sh {} \;
		run_server ./login-server "$PLUGINS"
		run_server ./char-server "$PLUGINS"
		run_server ./map-server "$ARGS $PLUGINS"
		;;
	getplugins)
		echo "Cloning plugins repository..."
		# Nothing to clone right now, all relevant plugins are part of the repository.
		#git clone http://github.com/HerculesWS/StaffPlugins.git || aborterror "Unable to fetch plugin repository"
		#if [ -f StaffPlugins/Haru/script_mapquit/script_mapquit.c -a -f StaffPlugins/Haru/script_mapquit/examples/ci_test.txt ]; then
		#	pushd src/plugins || aborterror "Unable to enter plugins directory."
		#	ln -s ../../StaffPlugins/Haru/script_mapquit/script_mapquit.c ./
		#	popd
		#else
		#	echo "Plugin not found, skipping advanced tests."
		#fi
		;;
	startmysql)
		echo "Starting mysql..."
		service mysql status || true
		service mysql stop || true
		service mysql start || true
		service mysql status || true
		;;
	*)
		usage
		;;
esac
