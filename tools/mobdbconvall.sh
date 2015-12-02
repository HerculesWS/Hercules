#!/bin/bash

./mobdbconverter.py re .. ../db/re/mob_db.txt > ../db/re/mob_db.conf
./mobdbconverter.py re .. ../db/mob_db2.txt > ../db/mob_db2.conf
./mobdbconverter.py pre-re .. ../db/pre-re/mob_db.txt > ../db/pre-re/mob_db.conf
#./mobdbconverter.py pre-re .. ../db/mob_db2.txt > ../db/mob_db2.conf
