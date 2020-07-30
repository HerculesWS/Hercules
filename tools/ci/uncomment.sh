#!/usr/bin/env bash

sed -i "s|//\"npc/|\"npc/|g" $1
sed -i "s|\"npc/location/to/script.txt\"|//\"npc/location/to/script.txt\"|g" $1
