#!/bin/bash

function checkdir {
    for sql in $1/*.sql
    do
        echo "checking ${sql}"
        # add workaround for non really error
        if [ "$sql" = "sql-files/upgrades/2020-03-22--03-09.sql" ]; then
            continue
        fi
        php -d memory_limit=4G ./tools/php-sqllint/bin/php-sqllint "${sql}" || exit 1
    done
}

checkdir "sql-files"
checkdir "sql-files/upgrades"
checkdir "sql-files/tools"
