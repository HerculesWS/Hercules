/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2013-2022 Hercules Dev Team
 * Copyright (C) 2020-2022 Andrei Karas (4144)
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

/// Http sample Hercules Plugin

#include "common/hercules.h" /* Should always be the first Hercules file included! (if you don't make it first, you won't be able to use interfaces) */
#include "common/memmgr.h"
#include "common/mmo.h"
#include "common/random.h"
#include "common/socket.h"
#include "common/strlib.h"
#include "api/aclif.h"
#include "api/aloginif.h"
#include "api/apipackets.h"
#include "api/apisessiondata.h"
#include "api/httpsender.h"
#include "char/apipackets.h"
#include "login/login.h"
#include "login/lclif.p.h"
#include "map/clif.h"
#include "map/pc.h"
#include "map/script.h"

#include "plugins/HPMHooking.h"
#include "common/HPMDataCheck.h" /* should always be the last Hercules file included! (if you don't make it last, it'll intentionally break compile time) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

HPExport struct hplugin_info pinfo = {
	"Http sample",    // Plugin name
	SERVER_TYPE_CHAR|SERVER_TYPE_LOGIN|SERVER_TYPE_MAP|SERVER_TYPE_API, // Which server types this plugin works with?
	"0.1",       // Plugin version
	HPM_VERSION, // HPM Version (don't change, macro is automatically updated)
};

struct PACKET_API_sample_api_data_request {
	char text[100];
	int flag;
};

struct PACKET_API_REPLY_sample_api_data_response {
	int users_count;
};

// runs on api server
HTTP_URL(sample_test_simple)
{
	ShowInfo("/httpsample/simple url called %d: %d\n", fd, sd->parser.method);

	char buf[1000];
	// get client user agent
	const char *user_agent = (const char*)strdb_get(sd->headers_db, "User-Agent");
	const char *format = "<html>Hercules test from sample plugin.<br/>Your user agent is: %s<br/></html>\n";
	if (user_agent != NULL) {
		// copy user agent from http request to buffer
		safesnprintf(buf, sizeof(buf), format, user_agent);
	} else {
		// use unknown as user agent string
		safestrncpy(buf, "unknown", 8);
	}

	// send html text back to client
	httpsender->send_html(fd, buf);

	// terminating http connection
	aclif->terminate_connection(fd);

	return true;
}

// runs on api server
HTTP_URL(sample_test_char)
{
	ShowInfo("/httpsample/char url called %d: %d\n", fd, sd->parser.method);
	// this request unrelated to loggedin player, we should select char server for send packet to
	// sleecting default in hercules configs: server_name: "Hercules"
	sd->world_name = "Hercules";

	// create variable with custom data for send other char server
	CREATE_HTTP_DATA(sample_api_data_request);
	// get client user agent
	const char *user_agent = (const char*)strdb_get(sd->headers_db, "User-Agent");
	if (user_agent != NULL) {
		// copy user agent from http request to text field
		safestrncpy(data.text, user_agent, sizeof(data.text));
	} else {
		// use unknown as user agent string
		safestrncpy(data.text, "unknown", 8);
	}
	// set flag field to value 123
	data.flag = 123;
	// send packet from api server to char server
	SEND_ASYNC_DATA(API_MSG_CUSTOM, &data, sizeof(data));
	// not termination http connection here because waiting packet from char server with data...
	return true;
}

// runs on api server
// sample handler for receiving data from char server for http request /httpsample/char
HTTP_DATA(sample_test_char)
{
	ShowInfo("sample_test_char called\n");

	// unpacking own data struct
	GET_HTTP_DATA(p, sample_api_data_response);

	// generate html and send
	const char *format = "<html>Hercules test from sample plugin.<br/>Users count on char server: %d<br/></html>\n";
	char buf[1000];
	safesnprintf(buf, sizeof(buf), format, p->users_count);
	httpsender->send_html(fd, buf);

	// terminating http connection here after we got requested data from char server
	aclif->terminate_connection(fd);
}

// runs on char server
// sample handler for message from api server url /test/msg
void sample_char_api_packet(int fd)
{
	// define variable with received data from packet
	RFIFO_API_DATA(sdata, sample_api_data_request);
	ShowInfo("sample_char_api_packet called: %s, %d\n", sdata->text, sdata->flag);
	// deine variable with sending packet
	WFIFO_APICHAR_PACKET_REPLY(sample_api_data_response);
	// store user count into packet field
	data->users_count = chr->count_users();
	// send created packet
	WFIFOSET(chr->login_fd, packet->packet_len);
}

/* run when server starts */
HPExport void plugin_init (void)
{
	ShowInfo("Server type is ");

	switch (SERVER_TYPE) {
		case SERVER_TYPE_LOGIN: printf("Login Server\n"); break;
		case SERVER_TYPE_CHAR: printf("Char Server\n"); break;
		case SERVER_TYPE_MAP: printf ("Map Server\n"); break;
		case SERVER_TYPE_API: printf ("Api Server\n"); break;
		case SERVER_TYPE_UNKNOWN: printf ("Unknown Server\n"); break;
	}

	ShowInfo("I'm being run from the '%s' filename\n", SERVER_NAME);

	if (SERVER_TYPE == SERVER_TYPE_CHAR) {
		// Add handler for message from api server url /test/msg
		addPacket(API_MSG_CUSTOM, WFIFO_APICHAR_SIZE + sizeof(struct PACKET_API_sample_api_data_request), sample_char_api_packet, hpProxy_ApiChar);
	}
}
/* triggered when server starts loading, before any server-specific data is set */
HPExport void server_preinit(void)
{
}

/* run when server is ready (online) */
HPExport void server_online (void)
{
	// Register url for GET request
	if (SERVER_TYPE == SERVER_TYPE_API) {
		addHttpHandler(HTTP_GET, "/httpsample/simple", sample_test_simple, REQ_DEFAULT);
		addHttpHandler(HTTP_GET, "/httpsample/char", sample_test_char, REQ_DEFAULT);
		addProxyPacketHandler(sample_test_char, API_MSG_CUSTOM);
	}
}

/* run when server is shutting down */
HPExport void plugin_final (void) {
	ShowInfo ("%s says ~Bye world\n", pinfo.name);
}
