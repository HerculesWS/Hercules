#!/bin/bash

# This file is part of Hercules.
# http://herc.ws - http://github.com/HerculesWS/Hercules
#
# Copyright (C) 2014-2015  Hercules Dev Team
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

function run_server {
	echo "Running: $1 --run-once $2"
	$1 --run-once $2 2>runlog.txt
	export errcode=$?
	export teststr=$(cat runlog.txt)
	if [[ -n "${teststr}" ]]; then
		echo "Errors found in running server $1."
		cat runlog.txt
		aborterror "Errors found in running server $1."
	else
		echo "No errors found for server $1."
	fi
	if [ ${errcode} -ne 0 ]; then
		echo "server $1 terminated with exit code ${errcode}"
		aborterror "Test failed"
	fi
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
		(cd tools && ./validateinterfaces.py silent) || aborterror "Interface validation error."
		./configure $@ || (cat config.log && aborterror "Configure error, aborting build.")
		make -j3 || aborterror "Build failed."
		make plugins -j3 || aborterror "Build failed."
		make plugin.script_mapquit -j3 || aborterror "Build failed."
		;;
	test)
		cat > conf/travis_sql_connection.conf << EOF
sql_connection: {
	//default_codepage: ""
	//case_sensitive: false
	db_hostname: "localhost"
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
		PLUGINS="--load-plugin HPMHooking --load-plugin sample"
		echo "run all servers without HPM"
		run_server ./login-server
		run_server ./char-server
		run_server ./map-server "$ARGS"
		echo "run all servers wit HPM"
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
	*)
		usage
		;;
esac
