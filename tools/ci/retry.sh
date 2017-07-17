#!/usr/bin/env bash

# This file is part of Hercules.
# http://herc.ws - http://github.com/HerculesWS/Hercules
#
# Copyright (C) 2016  Hercules Dev Team
# Copyright (C) 2016  Haru <haru@herc.ws>
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

n=0

while true; do
	$@ && break
	if [[ $n -ge 5 ]]; then
		exit -1
	fi
	WAITTIME=$((2**n))
	echo "Execution of $@ failed. Retrying in $WAITTIME seconds..."
	sleep $WAITTIME
	n=$((n+1))
done
