#!/usr/bin/env sh

git log --format=format:"%aN <%aE>" | grep -v "54d463be-8e91-2dee-dedb-b68131a5f0ec" | sort | uniq
