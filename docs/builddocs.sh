#! /bin/bash
HOMEDIR=build/
HOMEDIRDOC=docs/
mkdir -p ../build/usr/share/man/
LIST_COMMAND='gunzip'
echo Generate hlp
for I in $LIST_COMMAND; do
echo Generate $I
# ../md2hlp/src/
cat $I.md | md2hlp.py3 -c md2hlp.cfg > ../build/usr/share/man/$I.hlp
done
