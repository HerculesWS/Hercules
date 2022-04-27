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
#include "login/apipackets.h"
#include "login/login.h"
#include "login/lclif.p.h"
#include "map/apipackets.h"
#include "map/clif.h"
#include "map/pc.h"
#include "map/script.h"

#include "plugins/HPMHooking.h"
#include "common/HPMDataCheck.h" /* should always be the last Hercules file included! (if you don't make it last, it'll intentionally break compile time) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// this message ids must not conflict with other plugins or hercules itself
enum apimessages {
	API_MSG_SAMPLE_LOGIN = API_MSG_CUSTOM + 0,
	API_MSG_SAMPLE_CHAR  = API_MSG_CUSTOM + 1,
	API_MSG_SAMPLE_MAP   = API_MSG_CUSTOM + 2
};

HPExport struct hplugin_info pinfo = {
	"Http sample",    // Plugin name
	SERVER_TYPE_CHAR|SERVER_TYPE_LOGIN|SERVER_TYPE_MAP|SERVER_TYPE_API, // Which server types this plugin works with?
	"0.1",       // Plugin version
	HPM_VERSION, // HPM Version (don't change, macro is automatically updated)
};

struct PACKET_API_sample_login_request_data {
	char text[100];
	int flag;
};

struct PACKET_API_sample_char_request_data {
	char text[100];
	int flag;
};

struct PACKET_API_sample_map_request_data {
	char text[100];
	int flag;
};

struct PACKET_API_REPLY_sample_login_response {
	int api_servers_count;
	int char_servers_count;
};

struct PACKET_API_REPLY_sample_char_response {
	int users_count;
};

struct PACKET_API_REPLY_sample_map_response {
	int users_count;
};

// runs on api server
HTTP_URL(my_sample_test_simple)
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
HTTP_URL(my_sample_test_login)
{
	ShowInfo("/httpsample/login url called %d: %d\n", fd, sd->parser.method);
	// create variable with custom data for send to login server
	CREATE_HTTP_DATA(data, sample_login_request);
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
	// send packet from api server to login server
	SEND_LOGIN_ASYNC_DATA(API_MSG_SAMPLE_LOGIN, &data, sizeof(data));
	// not terminating http connection here because waiting packet from login server with data...
	return true;
}

// runs on api server
// sample handler for receiving data from login server for http request /httpsample/login
HTTP_DATA(my_sample_test_login)
{
	ShowInfo("sample_test_login called\n");

	// unpacking own data struct
	GET_HTTP_DATA(p, sample_login_response);

	// generate html and send
	const char *format = "<html>Hercules test from sample plugin.<br/>\n"
		"Connected api servers count: %d<br/>\n"
		"Connected char servers count: %d<br/>\n"
		"</html>\n";
	char buf[1000];
	safesnprintf(buf, sizeof(buf), format, p->api_servers_count, p->char_servers_count);
	httpsender->send_html(fd, buf);

	// terminating http connection here after we got requested data from login server
	aclif->terminate_connection(fd);
}

// runs on api server
HTTP_URL(my_sample_test_char)
{
	ShowInfo("/httpsample/char url called %d: %d\n", fd, sd->parser.method);
	// this request unrelated to loggedin player, we should select char server for send packet to
	sd->world_name = aclif->get_first_world_name();

	// create variable with custom data for send other char server
	CREATE_HTTP_DATA(data, sample_char_request);
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
	SEND_CHAR_ASYNC_DATA(API_MSG_SAMPLE_CHAR, &data, sizeof(data));
	// not terminating http connection here because waiting packet from char server with data...
	return true;
}

// runs on api server
// sample handler for receiving data from char server for http request /httpsample/char
HTTP_DATA(my_sample_test_char)
{
	ShowInfo("sample_test_char called\n");

	// unpacking own data struct
	GET_HTTP_DATA(p, sample_char_response);

	// generate html and send
	const char *format = "<html>Hercules test from sample plugin.<br/>Users count on char server: %d<br/></html>\n";
	char buf[1000];
	safesnprintf(buf, sizeof(buf), format, p->users_count);
	httpsender->send_html(fd, buf);

	// terminating http connection here after we got requested data from char server
	aclif->terminate_connection(fd);
}

// runs on api server
HTTP_URL(my_sample_test_map)
{
	ShowInfo("/httpsample/map url called %d: %d\n", fd, sd->parser.method);
	// this request unrelated to loggedin player, we should select char server for send packet to
	sd->world_name = aclif->get_first_world_name();

	// create variable with custom data for send other char server
	CREATE_HTTP_DATA(data, sample_map_request);
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
	data.flag = 234;
	// send packet from api server to map server
	SEND_MAP_ASYNC_DATA(API_MSG_SAMPLE_MAP, &data, sizeof(data));
	// not terminating http connection here because waiting packet from map server with data...
	return true;
}

// runs on api server
// sample handler for receiving data from char server for http request /httpsample/char
HTTP_DATA(my_sample_test_map)
{
	ShowInfo("sample_test_map called\n");

	// unpacking own data struct
	GET_HTTP_DATA(p, sample_map_response);

	// generate html and send
	const char *format = "<html>Hercules test from sample plugin.<br/>Users count on map server: %d<br/></html>\n";
	char buf[1000];
	safesnprintf(buf, sizeof(buf), format, p->users_count);
	httpsender->send_html(fd, buf);

	// terminating http connection here after we got requested data from char server
	aclif->terminate_connection(fd);
}

// runs on login server
// sample handler for message from api server url /httpsample/login
void sample_login_api_packet(int fd)
{
	// define variable with received data from packet
	RFIFO_API_DATA(sdata, sample_login_request);
	ShowInfo("sample_login_api_packet called: %s, %d\n", sdata->text, sdata->flag);
	// deine variable with sending packet
	WFIFO_APILOGIN_PACKET_REPLY(fd, sample_login_response);
	// store servers count into packet fields

	int server_num = 0;
	for (int i = 0; i < ARRAYLENGTH(login->dbs->server); ++i) {
		if (sockt->session_is_active(login->dbs->server[i].fd))
			server_num ++;
	}
	data->char_servers_count = server_num;
	server_num = 0;
	for (int i = 0; i < ARRAYLENGTH(login->dbs->api_server); ++i) {
		if (sockt->session_is_active(login->dbs->api_server[i].fd))
			server_num ++;
	}
	data->api_servers_count = server_num;

	// send created packet
	WFIFOSET(fd, packet->packet_len);
}

// runs on char server
// sample handler for message from api server url /httpsample/char
void sample_char_api_packet(int fd)
{
	// define variable with received data from packet
	RFIFO_API_DATA(sdata, sample_char_request);
	ShowInfo("sample_char_api_packet called: %s, %d\n", sdata->text, sdata->flag);
	// deine variable with sending packet
	WFIFO_APICHAR_PACKET_REPLY(sample_char_response);
	// store user count into packet field
	data->users_count = chr->count_users();
	// send created packet
	WFIFOSET(chr->login_fd, packet->packet_len);
}

// runs on map server
// sample handler for message from api server url /httpsample/map
void sample_map_api_packet(int fd)
{
	// define variable with received data from packet
	RFIFO_API_DATA(sdata, sample_map_request);
	ShowInfo("sample_map_api_packet called: %s, %d\n", sdata->text, sdata->flag);
	// deine variable with sending packet
	WFIFO_APIMAP_PACKET_REPLY(sample_map_response);
	// store user count into packet field
	data->users_count = map->getusers();
	// send created packet
	WFIFOSET(chrif->fd, packet->packet_len);
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

	if (SERVER_TYPE == SERVER_TYPE_LOGIN) {
		// Add handler for message from api server url /httpsample/login
		addProxyPacket(API_MSG_SAMPLE_LOGIN, sample_login_request, sample_login_api_packet, hpProxy_ApiLogin);
	}
	if (SERVER_TYPE == SERVER_TYPE_CHAR) {
		// Add handler for message from api server url /httpsample/char
		addProxyPacket(API_MSG_SAMPLE_CHAR, sample_char_request, sample_char_api_packet, hpProxy_ApiChar);
	}
	if (SERVER_TYPE == SERVER_TYPE_MAP) {
		// Add handler for message from api server url /httpsample/map
		addProxyPacket(API_MSG_SAMPLE_MAP, sample_map_request, sample_map_api_packet, hpProxy_ApiMap);
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
		addHttpHandler(HTTP_GET, "/httpsample/simple", my_sample_test_simple, REQ_DEFAULT);
		addHttpHandler(HTTP_GET, "/httpsample/login", my_sample_test_login, REQ_DEFAULT);
		addHttpHandler(HTTP_GET, "/httpsample/char", my_sample_test_char, REQ_DEFAULT);
		addHttpHandler(HTTP_GET, "/httpsample/map", my_sample_test_map, REQ_DEFAULT);
		addProxyPacketHandler(my_sample_test_login, API_MSG_SAMPLE_LOGIN);
		addProxyPacketHandler(my_sample_test_char, API_MSG_SAMPLE_CHAR);
		addProxyPacketHandler(my_sample_test_map, API_MSG_SAMPLE_MAP);
	}
}

/* run when server is shutting down */
HPExport void plugin_final (void) {
	ShowInfo ("%s says ~Bye world\n", pinfo.name);
}
