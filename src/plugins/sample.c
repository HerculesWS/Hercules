/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2013-2015  Hercules Dev Team
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

/// Sample Hercules Plugin

#include "common/hercules.h" /* Should always be the first Hercules file included! (if you don't make it first, you won't be able to use interfaces) */
#include "common/memmgr.h"
#include "common/mmo.h"
#include "common/socket.h"
#include "common/strlib.h"
#include "map/clif.h"
#include "map/pc.h"
#include "map/script.h"

#include "plugins/HPMHooking.h"
#include "common/HPMDataCheck.h" /* should always be the last Hercules file included! (if you don't make it last, it'll intentionally break compile time) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

HPExport struct hplugin_info pinfo = {
	"Sample",    // Plugin name
	SERVER_TYPE_CHAR|SERVER_TYPE_LOGIN|SERVER_TYPE_MAP,// Which server types this plugin works with?
	"0.1",       // Plugin version
	HPM_VERSION, // HPM Version (don't change, macro is automatically updated)
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

int my_setting;

/* sample packet implementation */
/* cmd 0xf3 - it is a client-server existent id, for clif_parse_GlobalMessage */
/* in this sample we do nothing and simply redirect */
void sample_packet0f3(int fd) {
	struct map_session_data *sd = sockt->session[fd]->session_data;
	struct sample_data_struct *data;

	if( !sd ) return;/* socket didn't fully log-in? this packet shouldn't do anything then! */

	ShowInfo("sample_packet0f3: Hello World! received 0xf3 for '%s', redirecting!\n",sd->status.name);

	/* sample usage of appending data to a socket_data (sockt->session[]) entry */
	if( !(data = getFromSession(sockt->session[fd],0)) ) {
		CREATE(data,struct sample_data_struct,1);

		data->lastMSGPosition.map = sd->status.last_point.map;
		data->lastMSGPosition.x = sd->status.last_point.x;
		data->lastMSGPosition.y = sd->status.last_point.y;
		data->someNumber = rand()%777;

		ShowInfo("Created Appended sockt->session[] data, %d %d %d %u\n",data->lastMSGPosition.map,data->lastMSGPosition.x,data->lastMSGPosition.y,data->someNumber);
		addToSession(sockt->session[fd],data,0,true);
	} else {
		ShowInfo("Existent Appended sockt->session[] data, %d %d %d %u\n",data->lastMSGPosition.map,data->lastMSGPosition.x,data->lastMSGPosition.y,data->someNumber);
		if( rand()%4 == 2 ) {
			ShowInfo("Removing Appended sockt->session[] data\n");
			removeFromSession(sockt->session[fd],0);
		}
	}

	/* sample usage of appending data to a map_session_data (sd) entry */
	if( !(data = getFromMSD(sd,0)) ) {
		CREATE(data,struct sample_data_struct,1);

		data->lastMSGPosition.map = sd->status.last_point.map;
		data->lastMSGPosition.x = sd->status.last_point.x;
		data->lastMSGPosition.y = sd->status.last_point.y;
		data->someNumber = rand()%777;

		ShowInfo("Created Appended map_session_data data, %d %d %d %u\n",data->lastMSGPosition.map,data->lastMSGPosition.x,data->lastMSGPosition.y,data->someNumber);
		addToMSD(sd,data,0,true);
	} else {
		ShowInfo("Existent Appended map_session_data data, %d %d %d %u\n",data->lastMSGPosition.map,data->lastMSGPosition.x,data->lastMSGPosition.y,data->someNumber);
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
		clif->messagecolor_self(sd->fd, COLOR_RED, output);
	}
	return 1;
}
/*
* Key is the setting name in our example it's 'my_setting' while val is the value of it.
* this way you can manage more than one setting in one function instead of define multiable ones
*/

void parse_my_setting(const char *key, const char *val) {
	ShowDebug("Received '%s:%s'\n",key,val);
	/* do anything with the var e.g. config_switch(val) */
	/* for our example we will save it in global variable */

	/* please note, battle settings can be only returned as int for scripts and other usage */
	if (strcmpi(key,"my_setting") == 0)
		my_setting = atoi(val);
}

/* Battle Config Settings need to have return function in-case scripts need to read it */
int return_my_setting(const char *key)
{
	/* first we check the key to know what setting we need to return with strcmpi then return it */
	if (strcmpi(key, "my_setting") == 0)
		return my_setting;

	return 0;
}

/* run when server starts */
HPExport void plugin_init (void) {
	ShowInfo("Server type is ");

	switch (SERVER_TYPE) {
		case SERVER_TYPE_LOGIN: printf("Login Server\n"); break;
		case SERVER_TYPE_CHAR: printf("Char Server\n"); break;
		case SERVER_TYPE_MAP: printf ("Map Server\n"); break;
	}

	ShowInfo("I'm being run from the '%s' filename\n", SERVER_NAME);

	// Atcommands only make sense on the map server
	if (SERVER_TYPE == SERVER_TYPE_MAP) {
		/* addAtcommand("command-key",command-function) tells map server to call ACMD(sample) when "sample" command is used */
		/* - it will print a warning when used on a non-map-server plugin */
		addAtcommand("sample",sample);//link our '@sample' command
	}

	// Script commands only make sense on the map server
	if (SERVER_TYPE == SERVER_TYPE_MAP) {
		/* addScriptCommand("script-command-name","script-command-params-info",script-function) tells map server to call BUILDIN(sample) for the "sample(i)" command */
		/* - it will print a warning when used on a non-map-server plugin */
		addScriptCommand("sample","i",sample);
	}

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

	// The following hooks would show an error message where pc->dropitem doesn't exist (login or char server)
	if (SERVER_TYPE == SERVER_TYPE_MAP) {
		/* in this sample we add a PreHook to pc->dropitem */
		/* to identify whether the item being dropped is on amount higher than 1 */
		/* if so, it stores the amount on a variable (my_pc_dropitem_storage) and changes the amount to 1 */
		addHookPre(pc, dropitem, my_pc_dropitem_pre);

		/* in this sample we add a PostHook to pc->dropitem */
		/* if the original pc->dropitem was successful and the amount stored on my_pc_dropitem_storage is higher than 1, */
		/* our posthook will display a message to the user about the cap */
		/* - by checking whether it was successful (retVal value) it allows for the originals conditions to take place */
		addHookPost(pc, dropitem, my_pc_dropitem_post);
	}
}
/* triggered when server starts loading, before any server-specific data is set */
HPExport void server_preinit (void) {
	/* makes map server listen to mysetting:value in any "battleconf" file (including imported or custom ones) */
	/* value is not limited to numbers, its passed to our plugins handler (parse_my_setting) as const char *,
	 * however for battle config to be returned to our script engine we need it to be number (int) so keep use it as int only */
	addBattleConf("my_setting",parse_my_setting,return_my_setting);
}
/* run when server is ready (online) */
HPExport void server_online (void) {
}
/* run when server is shutting down */
HPExport void plugin_final (void) {
	ShowInfo ("%s says ~Bye world\n",pinfo.name);
}
