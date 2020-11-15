#!/bin/bash

function checkdir {
    for sql in $1/*.sql
    do
        echo "checking ${sql}"
        php -d memory_limit=4G ./tools/php-sqllint/bin/php-sqllint "${sql}" || exit 1
    done
}

checkdir "sql-files"
checkdir "sql-files/upgrades"
checkdir "sql-files/tools"
