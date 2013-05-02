// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Sample Hercules Plugin

#include <stdio.h>
#include <string.h>
#include "../common/HPMi.h"
#include "../map/script.h"
#include "../map/pc.h"

HPExport struct hplugin_info pinfo = {
	"Sample",		// Plugin name
	SERVER_TYPE_MAP,// Which server types this plugin works with?
	"0.1",			// Plugin version
	HPM_VERSION,	// HPM Version (don't change, macro is automatically updated)
};
ACMD(sample) {//@sample command - 5 params: const int fd, struct map_session_data* sd, const char* command, const char* message, struct AtCommandInfo *info
	printf("I'm being run! message -> '%s' by %s\n",message,sd->status.name);
	return true;
}
BUILDIN(sample) {//script command 'sample(num);' - 1 param: struct script_state* st
	int arg = script_getnum(st,2);
	ShowInfo("I'm being run! arg -> '%d'\n",arg);
	return true;
}
CPCMD(sample) {//console command 'sample' - 1 param: char *line
	ShowInfo("I'm being run! arg -> '%s'\n",line?line:"NONE");
}
struct script_interface *script;/* used by script commands */
/* run when server starts */
HPExport void plugin_init (void) {
	char *server_type;
	char *server_name;
	
	//get the symbols from the server
	server_type = GET_SYMBOL("SERVER_TYPE");
	server_name = GET_SYMBOL("SERVER_NAME");

	script = GET_SYMBOL("script");
		
	ShowInfo ("Server type is ");
	
	switch (*server_type) {
		case SERVER_TYPE_LOGIN: printf ("Login Server\n"); break;
		case SERVER_TYPE_CHAR: printf ("Char Server\n"); break;
		case SERVER_TYPE_MAP: printf ("Map Server\n"); break;
	}
	
	ShowInfo ("I'm being run from the '%s' filename\n", server_name);
	
	if( HPMi->addCommand != NULL ) {//link our '@sample' command
		HPMi->addCommand("sample",ACMD_A(sample));
	}
	
	if( HPMi->addScript != NULL ) {//link our 'sample' script command
		HPMi->addScript("sample","i",BUILDIN_A(sample));
	}
	
	if( HPMi->addCPCommand != NULL ) {//link our 'sample' console command
		HPMi->addCPCommand("this:is:a:sample",CPCMD_A(sample));
	}
}
/* run when server is ready (online) */
HPExport void server_online (void) {
	
}
/* run when server is shutting down */
HPExport void plugin_final (void) {
	ShowInfo ("%s says ~Bye world\n",pinfo.name);
}