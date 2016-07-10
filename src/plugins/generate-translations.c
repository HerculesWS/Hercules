/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2016  Hercules Dev Team
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
#include "config/core.h"

#include "common/hercules.h"
#include "common/cbasetypes.h"
#include "common/memmgr.h"
#include "common/showmsg.h"
#include "common/strlib.h"
#include "common/sysinfo.h"
#include "map/atcommand.h"
#include "map/map.h"
#include "map/script.h"

#include "plugins/HPMHooking.h"
#include "common/HPMDataCheck.h"

#include <stdio.h>
#include <stdlib.h>

HPExport struct hplugin_info pinfo = {
	"generate-translations", // Plugin name
	SERVER_TYPE_MAP, // Which server types this plugin works with?
	"0.1",           // Plugin version
	HPM_VERSION,     // HPM Version (don't change, macro is automatically updated)
};

struct DBMap *translatable_strings; // string map parsed (used when exporting strings only)
/* Set during startup when attempting to export the lang, unset after server initialization is over */
FILE *lang_export_fp;
char *lang_export_file;/* for lang_export_fp */
struct script_string_buf lang_export_line_buf;
struct script_string_buf lang_export_escaped_buf;
int lang_export_stringcount;

/// Whether the translations template generator will automatically run.
bool generating_translations = false;

/**
 * --generate-translations
 *
 * Creates "./generated_translations.pot"
 * @see cmdline->exec
 */
CMDLINEARG(generatetranslations)
{
	lang_export_file = aStrdup("./generated_translations.pot");

	if ((lang_export_fp = fopen(lang_export_file, "wb")) == NULL) {
		ShowError("export-dialog: failed to open '%s' for writing\n", lang_export_file);
	} else {
		time_t t = time(NULL);
		struct tm *lt = localtime(&t);
		int year = lt->tm_year+1900;
		char timestring[128] = "";
		strftime(timestring, sizeof(timestring), "%Y-%m-%d %H:%M:%S%z", lt);
		fprintf(lang_export_fp,
				"# This file is part of Hercules.\n"
				"# http://herc.ws - http://github.com/HerculesWS/Hercules\n"
				"#\n"
				"# Copyright (C) 2013-%d  Hercules Dev Team\n"
				"#\n"
				"# Hercules is free software: you can redistribute it and/or modify\n"
				"# it under the terms of the GNU General Public License as published by\n"
				"# the Free Software Foundation, either version 3 of the License, or\n"
				"# (at your option) any later version.\n"
				"#\n"
				"# This program is distributed in the hope that it will be useful,\n"
				"# but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
				"# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
				"# GNU General Public License for more details.\n"
				"#\n"
				"# You should have received a copy of the GNU General Public License\n"
				"# along with this program.  If not, see <http://www.gnu.org/licenses/>.\n\n"

				"#,fuzzy\n"
				"msgid \"\"\n"
				"msgstr \"\"\n"
				"\"Project-Id-Version: %s\\n\"\n"
				"\"Report-Msgid-Bugs-To: dev@herc.ws\\n\"\n"
				"\"POT-Creation-Date: %s\\n\"\n"
				"\"PO-Revision-Date: YEAR-MO-DA HO:MI+ZONE\\n\"\n"
				"\"Last-Translator: FULL NAME <EMAIL@ADDRESS>\\n\"\n"
				"\"Language-Team: LANGUAGE <LL@li.org>\\n\"\n"
				"\"Language: \\n\"\n"
				"\"MIME-Version: 1.0\\n\"\n"
				"\"Content-Type: text/plain; charset=ISO-8859-1\\n\"\n"
				"\"Content-Transfer-Encoding: 8bit\\n\"\n\n",
				year, sysinfo->vcsrevision_scripts(), timestring);
	}
	generating_translations = true;
	return true;
}

void script_add_translatable_string_posthook(const struct script_string_buf *string, const char *start_point)
{
	bool duplicate = true;
	bool is_translatable_string = false;
	bool is_translatable_fmtstring = false;

	if (!generating_translations || lang_export_fp == NULL)
		return;

	/* When exporting we don't know what is a translation and what isn't */
	if (VECTOR_LENGTH(*string) > 1) {
		// The length of *string will always be at least 1 because of the '\0'
		if (translatable_strings == NULL) {
			translatable_strings = strdb_alloc(DB_OPT_DUP_KEY|DB_OPT_ALLOW_NULL_DATA, 0);
		}

		if (!strdb_exists(translatable_strings, VECTOR_DATA(*string))) {
			strdb_put(translatable_strings, VECTOR_DATA(*string), NULL);
			duplicate = false;
		}
	}

	if (!duplicate) {
		if (script->syntax.last_func == script->buildin_mes_offset
		 || script->syntax.last_func == script->buildin_select_offset
		 || script->syntax.lang_macro_active
		 ) {
			is_translatable_string = true;
		} else if (script->syntax.last_func == script->buildin_mesf_offset
				|| script->syntax.lang_macro_fmtstring_active
				) {
			is_translatable_fmtstring = true;
		}
	}

	if (is_translatable_string || is_translatable_fmtstring) {
		const char *line_start = start_point;
		const char *line_end = start_point;
		int line_length;
		bool has_percent_sign = false;

		if (!is_translatable_fmtstring && strchr(VECTOR_DATA(*string), '%') != NULL) {
			has_percent_sign = true;
		}

		while (line_start > script->parser_current_src && *line_start != '\n')
			line_start--;

		while (*line_end != '\n' && *line_end != '\0')
			line_end++;

		line_length = (int)(line_end - line_start);
		if (line_length > 0) {
			VECTOR_ENSURE(lang_export_line_buf, line_length + 1, 512);
			VECTOR_PUSHARRAY(lang_export_line_buf, line_start, line_length);
			VECTOR_PUSH(lang_export_line_buf, '\0');

			normalize_name(VECTOR_DATA(lang_export_line_buf), "\r\n\t "); // [!] Note: VECTOR_LENGTH() will lie.
		}

		VECTOR_ENSURE(lang_export_escaped_buf, 4*VECTOR_LENGTH(*string)+1, 1);
		VECTOR_LENGTH(lang_export_escaped_buf) = (int)sv->escape_c(VECTOR_DATA(lang_export_escaped_buf),
				VECTOR_DATA(*string),
				VECTOR_LENGTH(*string)-1, /* exclude null terminator */
				"\"");
		VECTOR_PUSH(lang_export_escaped_buf, '\0');

		fprintf(lang_export_fp, "\n#: %s\n"
				"# %s\n"
				"%s"
				"msgctxt \"%s\"\n"
				"msgid \"%s\"\n"
				"msgstr \"\"\n",
				script->parser_current_file ? script->parser_current_file : "Unknown File",
				VECTOR_DATA(lang_export_line_buf),
				is_translatable_fmtstring ? "#, c-format\n" : (has_percent_sign ? "#, no-c-format\n" : ""),
				script->parser_current_npc_name ? script->parser_current_npc_name : "Unknown NPC",
				VECTOR_DATA(lang_export_escaped_buf)
		);
		lang_export_stringcount++;
		VECTOR_TRUNCATE(lang_export_line_buf);
		VECTOR_TRUNCATE(lang_export_escaped_buf);
	}
}

struct script_code *parse_script_prehook(const char **src, const char **file, int *line, int *options, int **retval)
{
	if (translatable_strings != NULL) {
		db_destroy(translatable_strings);
		translatable_strings = NULL;
	}
	return NULL;
}

void script_parser_clean_leftovers_posthook(void)
{
	if (translatable_strings != NULL) {
		db_destroy(translatable_strings);
		translatable_strings = NULL;
	}

	VECTOR_CLEAR(lang_export_line_buf);
	VECTOR_CLEAR(lang_export_escaped_buf);
}

bool msg_config_read_posthook(bool retVal, const char *cfg_name, bool allow_override)
{
	static int called = 1;

	if (!generating_translations || lang_export_fp == NULL)
		return retVal;

	if (!retVal)
		return retVal;

	if (++called == 1) { // Original
		int i;
		for (i = 0; i < MAX_MSG; i++) {
			if (atcommand->msg_table[0][i] == NULL)
				continue;
			fprintf(lang_export_fp, "msgctxt \"messages.conf\"\n"
					"msgid \"%s\"\n"
					"msgstr \"\"\n",
					atcommand->msg_table[0][i]
			       );
			lang_export_stringcount++;
		}
	}

	return retVal;
}

HPExport void server_preinit(void)
{
	addArg("--generate-translations", false, generatetranslations,
			"Creates './generated_translations.pot' file with all translateable strings from scripts, server terminates afterwards.");
	VECTOR_INIT(lang_export_line_buf);
	VECTOR_INIT(lang_export_escaped_buf);
	addHookPost(script, add_translatable_string, script_add_translatable_string_posthook);
	addHookPre(script, parse, parse_script_prehook);
	addHookPost(script, parser_clean_leftovers, script_parser_clean_leftovers_posthook);
	addHookPost(atcommand, msg_read, msg_config_read_posthook);
	lang_export_stringcount = 0;
}

HPExport void plugin_init(void)
{
}

HPExport void server_online(void)
{
	if (generating_translations && lang_export_fp != NULL) {
		ShowInfo("Translations template exported to '%s' with %d strings.\n", lang_export_file, lang_export_stringcount);
		fclose(lang_export_fp);
		lang_export_fp = NULL;
	}
	core->runflag = CORE_ST_STOP;
}

HPExport void plugin_final(void)
{
	if (lang_export_file != NULL) {
		aFree(lang_export_file);
		lang_export_file = NULL;
	}
}
