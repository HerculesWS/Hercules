// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Sample Hercules Plugin

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../common/HPMi.h"
#include "../common/mmo.h"
#include "../common/socket.h"
#include "../common/malloc.h"
#include "../map/script.h"
#include "../map/pc.h"
#include "../map/clif.h"

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
struct sample_data_struct {
	struct point lastMSGPosition;
	unsigned int someNumber;
};
/* sample packet implementation */
/* cmd 0xf3 - it is a client-server existent id, for clif_parse_GlobalMessage */
/* in this sample we do nothing and simply redirect */
void sample_packet0f3(int fd) {
	struct map_session_data *sd = session[fd]->session_data;
	struct sample_data_struct *data;
	
	if( !sd ) return;/* socket didn't fully log-in? this packet shouldn't do anything then! */
	
	ShowInfo("sample_packet0f3: Hello World! received 0xf3 for '%s', redirecting!\n",sd->status.name);
	
	/* sample usage of appending data to a socket_data (session[]) entry */
	if( !(data = HPMi->getFromSession(session[fd],HPMi->pid,0)) ) {
		CREATE(data,struct sample_data_struct,1);
		
		data->lastMSGPosition.map = sd->status.last_point.map;
		data->lastMSGPosition.x = sd->status.last_point.x;
		data->lastMSGPosition.y = sd->status.last_point.y;
		data->someNumber = rand()%777;
		
		ShowInfo("Created Appended session[] data, %d %d %d %d\n",data->lastMSGPosition.map,data->lastMSGPosition.x,data->lastMSGPosition.y,data->someNumber);
		HPMi->addToSession(session[fd],data,HPMi->pid,0,true);
	} else {
		ShowInfo("Existent Appended session[] data, %d %d %d %d\n",data->lastMSGPosition.map,data->lastMSGPosition.x,data->lastMSGPosition.y,data->someNumber);
		if( rand()%4 == 2 ) {
			ShowInfo("Removing Appended session[] data\n");
			HPMi->removeFromSession(session[fd],HPMi->pid,0);
		}
	}
	
	/* sample usage of appending data to a map_session_data (sd) entry */
	if( !(data = HPMi->getFromMSD(sd,HPMi->pid,0)) ) {
		CREATE(data,struct sample_data_struct,1);
		
		data->lastMSGPosition.map = sd->status.last_point.map;
		data->lastMSGPosition.x = sd->status.last_point.x;
		data->lastMSGPosition.y = sd->status.last_point.y;
		data->someNumber = rand()%777;
		
		ShowInfo("Created Appended map_session_data data, %d %d %d %d\n",data->lastMSGPosition.map,data->lastMSGPosition.x,data->lastMSGPosition.y,data->someNumber);
		HPMi->addToMSD(sd,data,HPMi->pid,0,true);
	} else {
		ShowInfo("Existent Appended map_session_data data, %d %d %d %d\n",data->lastMSGPosition.map,data->lastMSGPosition.x,data->lastMSGPosition.y,data->someNumber);
		if( rand()%4 == 2 ) {
			ShowInfo("Removing Appended map_session_data data\n");
			HPMi->removeFromMSD(sd,HPMi->pid,0);
		}
	}

	
	clif->pGlobalMessage(fd,sd);
}
/* run when server starts */
HPExport void plugin_init (void) {
	char *server_type;
	char *server_name;
		
	/* core vars */
	server_type = GET_SYMBOL("SERVER_TYPE");
	server_name = GET_SYMBOL("SERVER_NAME");

	/* core interfaces */
	iMalloc = GET_SYMBOL("iMalloc");

	/* map-server interfaces */
	script = GET_SYMBOL("script");
	clif = GET_SYMBOL("clif");
	
	/* session[] */
	session = GET_SYMBOL("session");

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
	
	if( HPMi->addPacket != NULL ) {//link our 'sample' packet to map-server
		HPMi->addPacket(0xf3,-1,sample_packet0f3,hpClif_Parse,HPMi->pid);
	}

}
/* run when server is ready (online) */
HPExport void server_online (void) {
	
}
/* run when server is shutting down */
HPExport void plugin_final (void) {
	ShowInfo ("%s says ~Bye world\n",pinfo.name);
}