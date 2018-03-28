/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2016  Hercules Dev Team
 * Copyright (C) 2016  Haru <haru@dotalux.com>
 *
 * Hercules is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/// db/constants.conf -> doc/constants.md generator plugin

#include "common/hercules.h"
//#include "common/memmgr.h"
#include "common/nullpo.h"
#include "common/strlib.h"
#include "map/itemdb.h"
#include "map/mob.h"
#include "map/script.h"
#include "map/skill.h"

#include "common/HPMDataCheck.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

#define OUTPUTFILEHEADER "doc" PATHSEP_STR "constants.md"
#define OUTPUTFILECONTENT "doc" PATHSEP_STR "constants_content.md"

HPExport struct hplugin_info pinfo = {
	"constdb2doc",   // Plugin name
	SERVER_TYPE_MAP, // Which server types this plugin works with?
	"0.1",           // Plugin version
	HPM_VERSION,     // HPM Version (don't change, macro is automatically updated)
};

FILE *out_header;
FILE *out_fp;
bool torun = false;
char *anchor;

char * get_anchorid(char anchorstring[])
{
	char *anchor = (char *) malloc(256);

	strcpy(anchor, anchorstring);

    int i, j;
    for (i = 0, j = 0; anchor[i] != 0; i++) {
        anchor[i] = tolower(anchor[i]);

        // Check space
        if (anchor[i] == 32) {
            anchor[i] = 45; // Change space to hyphen
        }

        // Check allow character. number, letter, underscore or hyphen(space)
        // All special character will be remove
        if ((anchor[i] >= 97 && anchor[i] <= 122) || (anchor[i] >= 48 && anchor[i] <= 57) || anchor[i] == 45 || anchor[i] == 95) {
            anchor[j] = anchor[i];
            j++;
        }
    }
    anchor[j] = '\0';

	return anchor;
}

/// To override script_constdb_comment
void constdb2doc_constdb_comment(const char *comment)
{
	nullpo_retv(out_header);
	nullpo_retv(out_fp);
	if (comment == NULL) {
		fprintf(out_fp, "\n");
		fprintf(out_header, "\n");
	}
	else {
		fprintf(out_fp, "\n### %s\n\n", comment);
		anchor = strdup(comment);
		fprintf(out_header, "- [%s](#%s)\n", comment, get_anchorid(anchor));
		free(anchor);
	}
}

/// To override script_set_constant, called by script_read_constdb
void constdb2doc_script_set_constant(const char *name, int value, bool is_parameter, bool is_deprecated)
{
	nullpo_retv(out_fp);

	if (is_parameter)
		fprintf(out_fp, "- `%s`: [param]%s\n", name, is_deprecated ? " **(DEPRECATED)**" : "");
	else
		fprintf(out_fp, "- `%s`: %d%s\n", name, value, is_deprecated ? " **(DEPRECATED)**" : "");
}

void constdb2doc_constdb(void)
{
	void (*script_set_constant) (const char* name, int value, bool is_parameter, bool is_deprecated) = NULL;
	void (*script_constdb_comment) (const char *comment) = NULL;

	nullpo_retv(out_fp);
	nullpo_retv(out_header);

	/* Link */
	script_set_constant = script->set_constant;
	script->set_constant = constdb2doc_script_set_constant;
	script_constdb_comment = script->constdb_comment;
	script->constdb_comment = constdb2doc_constdb_comment;

	/* Run */
	fprintf(out_fp, "## Constants (db/constants.conf)\n\n");
	fprintf(out_header, "#### 1. [Constants (db/constants.conf)](#%s)\n", get_anchorid("Constants (db/constants.conf)"));
	script->read_constdb();
	fprintf(out_fp, "\n");

	fprintf(out_fp, "## Hardcoded Constants (source)\n\n");
	fprintf(out_header, "#### 2. [Hardcoded Constants (source)](#%s)\n", get_anchorid("Hardcoded Constants (source)"));
	script->hardcoded_constants();
	fprintf(out_fp, "\n");

	fprintf(out_fp, "## Parameters (source)\n\n");
	fprintf(out_header, "#### 3. [Parameters (source)](#%s)\n", get_anchorid("Parameters (source)"));
	script->load_parameters();
	fprintf(out_fp, "\n");

	/* Unlink */
	script->set_constant = script_set_constant;
	script->constdb_comment = script_constdb_comment;
}

void constdb2doc_skilldb(void)
{
	int i;

	nullpo_retv(out_header);
	nullpo_retv(out_fp);

	fprintf(out_fp, "## Skills (db/"DBPATH"skill_db.txt)\n\n");
	fprintf(out_header, "#### 4. [Skills (db/"DBPATH"skill_db.txt)](#%s)\n\n", get_anchorid("Skills (db/"DBPATH"skill_db.txt)"));
	for (i = 1; i < MAX_SKILL_DB; i++) {
		if (skill->dbs->db[i].name[0] != '\0')
			fprintf(out_fp, "- `%s`: %d\n", skill->dbs->db[i].name, skill->dbs->db[i].nameid);
	}
	fprintf(out_fp, "\n");
}

void constdb2doc_mobdb(void)
{
	int i;

	nullpo_retv(out_header);
	nullpo_retv(out_fp);

	fprintf(out_fp, "## Mobs (db/"DBPATH"mob_db.txt)\n\n");
	fprintf(out_header, "#### 5. [Mobs (db/"DBPATH"mob_db.txt)](#%s)\n\n", get_anchorid("Mobs (db/"DBPATH"mob_db.txt)"));
	for (i = 0; i < MAX_MOB_DB; i++) {
		struct mob_db *md = mob->db(i);
		if (md == mob->dummy || md->sprite[0] == '\0')
			continue;
		fprintf(out_fp, "- `%s`: %d\n", md->sprite, i);
	}
	fprintf(out_fp, "\n");
}

/// Cloned from itemdb_search
struct item_data *constdb2doc_itemdb_search(int nameid)
{
	if (nameid >= 0 && nameid < ARRAYLENGTH(itemdb->array))
		return itemdb->array[nameid];

	return idb_get(itemdb->other, nameid);
}

void constdb2doc_itemdb(void)
{
	int i;

	nullpo_retv(out_header);
	nullpo_retv(out_fp);

	fprintf(out_fp, "## Items (db/"DBPATH"item_db.conf)\n");
	fprintf(out_header, "#### 6. [Items (db/"DBPATH"item_db.conf)](#%s)\n", get_anchorid("Items (db/"DBPATH"item_db.conf"));
	for (i = 0; i < ARRAYLENGTH(itemdb->array); i++) {
		struct item_data *id = constdb2doc_itemdb_search(i);
		if (id == NULL || id->name[0] == '\0')
			continue;
		fprintf(out_fp, "- `%s`: %d\n", id->name, id->nameid);
	}
	fprintf(out_fp, "\n");
}

void merge_files()
{
	// Open herder and content files
	out_header = fopen(OUTPUTFILEHEADER, "a");
	out_fp = fopen(OUTPUTFILECONTENT, "r");

	if (out_header == NULL || out_fp == NULL) {
		ShowError("do_constdb2doc: Unable to open output file.\n");
		return;
	}

	char c;
	// Append content to headder
	while ((c = fgetc(out_fp)) != EOF)
		fputc(c, out_header);

	fclose(out_header);
	fclose(out_fp);

	// Remove content file
	remove(OUTPUTFILECONTENT);
}

void do_constdb2doc(void)
{
	/* File Type Detector */
	if ((out_fp = fopen(OUTPUTFILECONTENT, "wt+")) == NULL) {
		ShowError("do_constdb2doc: Unable to open output file.\n");
		return;
	}
	if ((out_header = fopen(OUTPUTFILEHEADER, "wt+")) == NULL) {
		ShowError("do_constdb2doc: Unable to open output file.\n");
		return;
	}

	fprintf(out_header,
		"# Constants\n\n"
		"> This document contains all the constants available to the script engine.\n\n"
		"## Table of contents\n");

	constdb2doc_constdb();

	constdb2doc_skilldb();

	constdb2doc_mobdb();

	constdb2doc_itemdb();

	fprintf(out_fp, "> End of list\n\n");
	fprintf(out_fp, "<!--GENERATED FILE DO NOT EDIT-->\n");

	fclose(out_header);
	fclose(out_fp);

	merge_files();

	ShowInfo("Finish export constants docs\n");

}
CPCMD(constdb2doc) {
	do_constdb2doc();
}
CMDLINEARG(constdb2doc)
{
	map->minimal = torun = true;
	return true;
}
HPExport void server_preinit(void) {
	addArg("--constdb2doc", false, constdb2doc, NULL);
}
HPExport void plugin_init(void) {
	addCPCommand("server:tools:constdb2doc", constdb2doc);
}
HPExport void server_online(void) {
	if (torun)
		do_constdb2doc();
}
