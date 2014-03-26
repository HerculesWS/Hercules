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
#include "../common/strlib.h"

#include "../common/HPMDataCheck.h" /* should always be the last file included! (if you don't make it last, it'll intentionally break compile time) */

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
	if( !(data = getFromSession(session[fd],0)) ) {
		CREATE(data,struct sample_data_struct,1);
		
		data->lastMSGPosition.map = sd->status.last_point.map;
		data->lastMSGPosition.x = sd->status.last_point.x;
		data->lastMSGPosition.y = sd->status.last_point.y;
		data->someNumber = rand()%777;
		
		ShowInfo("Created Appended session[] data, %d %d %d %d\n",data->lastMSGPosition.map,data->lastMSGPosition.x,data->lastMSGPosition.y,data->someNumber);
		addToSession(session[fd],data,0,true);
	} else {
		ShowInfo("Existent Appended session[] data, %d %d %d %d\n",data->lastMSGPosition.map,data->lastMSGPosition.x,data->lastMSGPosition.y,data->someNumber);
		if( rand()%4 == 2 ) {
			ShowInfo("Removing Appended session[] data\n");
			removeFromSession(session[fd],0);
		}
	}
	
	/* sample usage of appending data to a map_session_data (sd) entry */
	if( !(data = getFromMSD(sd,0)) ) {
		CREATE(data,struct sample_data_struct,1);
		
		data->lastMSGPosition.map = sd->status.last_point.map;
		data->lastMSGPosition.x = sd->status.last_point.x;
		data->lastMSGPosition.y = sd->status.last_point.y;
		data->someNumber = rand()%777;
		
		ShowInfo("Created Appended map_session_data data, %d %d %d %d\n",data->lastMSGPosition.map,data->lastMSGPosition.x,data->lastMSGPosition.y,data->someNumber);
		addToMSD(sd,data,0,true);
	} else {
		ShowInfo("Existent Appended map_session_data data, %d %d %d %d\n",data->lastMSGPosition.map,data->lastMSGPosition.x,data->lastMSGPosition.y,data->someNumber);
		if( rand()%4 == 2 ) {
			ShowInfo("Removing Appended map_session_data data\n");
			removeFromMSD(sd,0);
		}
	}

	
	clif->pGlobalMessage(fd,sd);
}
int my_pc_dropitem_storage;/* storage var */
/* my custom prehook for pc_dropitem, checks if amount of item being dropped is higher than 1 and if so cap it to 1 and store the value of how much it was */
int my_pc_dropitem_pre(struct map_session_data *sd,int *n,int *amount) {
	my_pc_dropitem_storage = 0;
	if( *amount > 1 ) {
		my_pc_dropitem_storage = *amount;
		*amount = 1;
	}
	return 0;
}
/* postHook receive retVal as the first param, allows posthook to act accordingly to whatever the original was going to return */
int my_pc_dropitem_post(int retVal, struct map_session_data *sd,int *n,int *amount) {
	if( retVal != 1 ) return retVal;/* we don't do anything if pc_dropitem didn't return 1 (success) */
	if( my_pc_dropitem_storage ) {/* signs whether pre-hook did this */
		char output[99];
		safesnprintf(output,99,"[ Warning ] you can only drop 1 item at a time, capped from %d to 1",my_pc_dropitem_storage);
		clif->colormes(sd->fd,COLOR_RED,output);
	}
	return 1;
}
void parse_my_setting(const char *val) {
	ShowDebug("Received 'my_setting:%s'\n",val);
	/* do anything with the var e.g. config_switch(val) */
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
	pc = GET_SYMBOL("pc");
	strlib = GET_SYMBOL("strlib");

	/* session[] */
	session = GET_SYMBOL("session");

	ShowInfo ("Server type is ");
	
	switch (*server_type) {
		case SERVER_TYPE_LOGIN: printf ("Login Server\n"); break;
		case SERVER_TYPE_CHAR: printf ("Char Server\n"); break;
		case SERVER_TYPE_MAP: printf ("Map Server\n"); break;
	}
	
	ShowInfo ("I'm being run from the '%s' filename\n", server_name);
	
	/* addAtcommand("command-key",command-function) tells map server to call ACMD(sample) when "sample" command is used */
	/* - it will print a warning when used on a non-map-server plugin */
	addAtcommand("sample",sample);//link our '@sample' command
	
	/* addScriptCommand("script-command-name","script-command-params-info",script-function) tells map server to call BUILDIN(sample) for the "sample(i)" command */
	/* - it will print a warning when used on a non-map-server plugin */
	addScriptCommand("sample","i",sample);
	
	/* addCPCommand("console-command-name",command-function) tells server to call CPCMD(sample) for the 'this is a sample <optional-args>' console call */
	/* in "console-command-name" usage of ':' indicates a category, for example 'this:is:a:sample' translates to 'this is a sample',
	 * therefore 'this -> is -> a -> sample', it can be used to aggregate multiple commands under the same category or to append commands to existing categories
	 * categories inherit the special keyword 'help' which prints the subsequent commands, e.g. 'server help' prints all categories and commands under 'server'
	 * therefore 'this help' would inform about 'is (category) -> a (category) -> sample (command)'*/
	addCPCommand("this:is:a:sample",sample);
	
	/* addPacket(packetID,packetLength,packetFunction,packetIncomingPoint) */
	/* adds packetID of packetLength (-1 for dynamic length where length is defined in the packet { packetID (2 Byte) , packetLength (2 Byte) , ... })
	 * to trigger packetFunction in the packetIncomingPoint section ( available points listed in enum HPluginPacketHookingPoints within src/common/HPMi.h ) */
	addPacket(0xf3,-1,sample_packet0f3,hpClif_Parse);
		
	/* in this sample we add a PreHook to pc->dropitem */
	/* to identify whether the item being dropped is on amount higher than 1 */
	/* if so, it stores the amount on a variable (my_pc_dropitem_storage) and changes the amount to 1 */
	addHookPre("pc->dropitem",my_pc_dropitem_pre);
	
	/* in this sample we add a PostHook to pc->dropitem */
	/* if the original pc->dropitem was successful and the amount stored on my_pc_dropitem_storage is higher than 1, */
	/* our posthook will display a message to the user about the cap */
	/* - by checking whether it was successful (retVal value) it allows for the originals conditions to take place */
	addHookPost("pc->dropitem",my_pc_dropitem_post);
}
/* triggered when server starts loading, before any server-specific data is set */
HPExport void server_preinit (void) {
	/* makes map server listen to mysetting:value in any "battleconf" file (including imported or custom ones) */
	/* value is not limited to numbers, its passed to our plugins handler (parse_my_setting) as const char *,
	 * and thus can be manipulated at will */
	addBattleConf("my_setting",parse_my_setting);
}
/* run when server is ready (online) */
HPExport void server_online (void) {
	
}
/* run when server is shutting down */
HPExport void plugin_final (void) {
	ShowInfo ("%s says ~Bye world\n",pinfo.name);
}
