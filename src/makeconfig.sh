#!/bin/bash

# Copyright (C) 2015 Lukas Bartl
# Diese Datei ist Teil des Klassenchats.
#
# This program is free software: you can redistribute it and/or modify
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

# Diese Datei wird von QMake aufgerufen um den Oberadmin und die std_admins aus den Dateien zu lesen

case "$1" in
"verwalter")
    echo "\\\"$(cat ../config/verwalter)\\\"";;
"oberadmin")
    echo "\\\"$(cat ../config/oberadmin)\\\"";;
"std_admins")
    all="{ oberadmin"
    for i in $(cat ../config/std_admins); do 
        all+=", \\\"$i\\\""
    done
    all+=" }"
    echo "$all";;
esac
