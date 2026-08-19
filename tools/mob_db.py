#!/usr/bin/env python3
# Converts a mob_db.txt file (read from stdin) into SQL REPLACE INTO
# statements for the `mob_db` table (written to stdout).
# usage example: python tools/mob_db.py < db/mob_db.txt > mob_db.sql

import re
import sys

DB = "mob_db"
NB_COLUMNS = 57
STR_COL = (1, 2, 3)

CREATE_TABLE = """#
# Table structure for table `mob_db`
#

DROP TABLE IF EXISTS `mob_db`;
CREATE TABLE `mob_db` (
  `ID` mediumint(9) unsigned NOT NULL default '0',
  `Sprite` text NOT NULL,
  `kName` text NOT NULL,
  `iName` text NOT NULL,
  `LV` tinyint(6) unsigned NOT NULL default '0',
  `HP` int(9) unsigned NOT NULL default '0',
  `SP` mediumint(9) unsigned NOT NULL default '0',
  `EXP` mediumint(9) unsigned NOT NULL default '0',
  `JEXP` mediumint(9) unsigned NOT NULL default '0',
  `Range1` tinyint(4) unsigned NOT NULL default '0',
  `ATK1` smallint(6) unsigned NOT NULL default '0',
  `ATK2` smallint(6) unsigned NOT NULL default '0',
  `DEF` smallint(6) unsigned NOT NULL default '0',
  `MDEF` smallint(6) unsigned NOT NULL default '0',
  `STR` smallint(6) unsigned NOT NULL default '0',
  `AGI` smallint(6) unsigned NOT NULL default '0',
  `VIT` smallint(6) unsigned NOT NULL default '0',
  `INT` smallint(6) unsigned NOT NULL default '0',
  `DEX` smallint(6) unsigned NOT NULL default '0',
  `LUK` smallint(6) unsigned NOT NULL default '0',
  `Range2` tinyint(4) unsigned NOT NULL default '0',
  `Range3` tinyint(4) unsigned NOT NULL default '0',
  `Scale` tinyint(4) unsigned NOT NULL default '0',
  `Race` tinyint(4) unsigned NOT NULL default '0',
  `Element` tinyint(4) unsigned NOT NULL default '0',
  `Mode` smallint(6) unsigned NOT NULL default '0',
  `Speed` smallint(6) unsigned NOT NULL default '0',
  `aDelay` smallint(6) unsigned NOT NULL default '0',
  `aMotion` smallint(6) unsigned NOT NULL default '0',
  `dMotion` smallint(6) unsigned NOT NULL default '0',
  `MEXP` mediumint(9) unsigned NOT NULL default '0',
  `MVP1id` smallint(9) unsigned NOT NULL default '0',
  `MVP1per` smallint(9) unsigned NOT NULL default '0',
  `MVP2id` smallint(9) unsigned NOT NULL default '0',
  `MVP2per` smallint(9) unsigned NOT NULL default '0',
  `MVP3id` smallint(9) unsigned NOT NULL default '0',
  `MVP3per` smallint(9) unsigned NOT NULL default '0',
  `Drop1id` smallint(9) unsigned NOT NULL default '0',
  `Drop1per` smallint(9) unsigned NOT NULL default '0',
  `Drop2id` smallint(9) unsigned NOT NULL default '0',
  `Drop2per` smallint(9) unsigned NOT NULL default '0',
  `Drop3id` smallint(9) unsigned NOT NULL default '0',
  `Drop3per` smallint(9) unsigned NOT NULL default '0',
  `Drop4id` smallint(9) unsigned NOT NULL default '0',
  `Drop4per` smallint(9) unsigned NOT NULL default '0',
  `Drop5id` smallint(9) unsigned NOT NULL default '0',
  `Drop5per` smallint(9) unsigned NOT NULL default '0',
  `Drop6id` smallint(9) unsigned NOT NULL default '0',
  `Drop6per` smallint(9) unsigned NOT NULL default '0',
  `Drop7id` smallint(9) unsigned NOT NULL default '0',
  `Drop7per` smallint(9) unsigned NOT NULL default '0',
  `Drop8id` smallint(9) unsigned NOT NULL default '0',
  `Drop8per` smallint(9) unsigned NOT NULL default '0',
  `Drop9id` smallint(9) unsigned NOT NULL default '0',
  `Drop9per` smallint(9) unsigned NOT NULL default '0',
  `DropCardid` smallint(9) unsigned NOT NULL default '0',
  `DropCardper` smallint(9) unsigned NOT NULL default '0',
  PRIMARY KEY  (`ID`)
) ENGINE=MyISAM;
"""


def perl_split_comma(s):
	parts = s.split(',')
	while parts and parts[-1] == '':
		parts.pop()
	return parts


def escape(s):
	return s.replace("'", "\\'")


def print_field(out, s, suffix, id_col):
	# Remove first { and last }
	m = re.search(r'\{.*\}', s)
	if m:
		s = m.group()[1:-1]
	# Remove comment at end of line
	m = re.search(r'[^/]*//', s)
	if m:
		s = m.group()[:-2]
	# If nothing, put NULL
	if s == "":
		out.write("NULL%s" % suffix)
	else:
		flag = id_col in STR_COL
		if flag:
			out.write("'%s'%s" % (escape(s), suffix))
		else:
			out.write("%s%s" % (s, suffix))


def main():
	out = sys.stdout
	out.write("%s\n" % CREATE_TABLE)

	for raw in sys.stdin:
		ligne = raw.rstrip('\r\n')
		if ligne == '':
			continue

		if ligne.startswith('//'):
			out.write("# ")
			ligne = ligne[2:]

		champ = perl_split_comma(ligne)
		if len(champ) != NB_COLUMNS:
			# Can't parse, it's a real comment
			out.write("%s\n" % ligne)
		else:
			out.write("REPLACE INTO `%s` VALUES (" % DB)
			for i in range(len(champ) - 1):
				print_field(out, champ[i], ",", i)
			print_field(out, champ[-1], ");\n", len(champ) - 1)

	out.write("\n")


if __name__ == '__main__':
	main()
