#!/usr/bin/env sh

# grep -v "kenpachi2k11" as workaround for avoid .mailmap issue
git log --format=format:"%aN <%aE>"|grep -v "54d463be-8e91-2dee-dedb-b68131a5f0ec"|grep -v "kenpachi2k11"|sort|uniq
