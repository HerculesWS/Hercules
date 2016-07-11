/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2015  Hercules Dev Team
 * Copyright (C)  Athena Dev Teams
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
#define HERCULES_CORE

#include "geoip.h"

#include "common/cbasetypes.h"
#include "common/memmgr.h"
#include "common/showmsg.h"

#include <errno.h>
#include <stdio.h>
#include <sys/stat.h> // for stat/lstat/fstat - [Dekamaster/Ultimate GM Tool]

struct s_geoip geoip_data;

struct geoip_interface geoip_s;
struct geoip_interface *geoip;

/* [Dekamaster/Nightroad] */
#define GEOIP_MAX_COUNTRIES 255
#define GEOIP_STRUCTURE_INFO_MAX_SIZE 20
#define GEOIP_COUNTRY_BEGIN 16776960

const char * geoip_countryname[GEOIP_MAX_COUNTRIES] = {"Unknown","Asia/Pacific Region","Europe","Andorra","United Arab Emirates","Afghanistan","Antigua and Barbuda","Anguilla","Albania","Armenia","Netherlands Antilles",
		"Angola","Antarctica","Argentina","American Samoa","Austria","Australia","Aruba","Azerbaijan","Bosnia and Herzegovina","Barbados",
		"Bangladesh","Belgium","Burkina Faso","Bulgaria","Bahrain","Burundi","Benin","Bermuda","Brunei Darussalam","Bolivia",
		"Brazil","Bahamas","Bhutan","Bouvet Island","Botswana","Belarus","Belize","Canada","Cocos (Keeling) Islands","Congo, The Democratic Republic of the",
		"Central African Republic","Congo","Switzerland","Cote D'Ivoire","Cook Islands","Chile","Cameroon","China","Colombia","Costa Rica",
		"Cuba","Cape Verde","Christmas Island","Cyprus","Czech Republic","Germany","Djibouti","Denmark","Dominica","Dominican Republic",
		"Algeria","Ecuador","Estonia","Egypt","Western Sahara","Eritrea","Spain","Ethiopia","Finland","Fiji",
		"Falkland Islands (Malvinas)","Micronesia, Federated States of","Faroe Islands","France","France, Metropolitan","Gabon","United Kingdom","Grenada","Georgia","French Guiana",
		"Ghana","Gibraltar","Greenland","Gambia","Guinea","Guadeloupe","Equatorial Guinea","Greece","South Georgia and the South Sandwich Islands","Guatemala",
		"Guam","Guinea-Bissau","Guyana","Hong Kong","Heard Island and McDonald Islands","Honduras","Croatia","Haiti","Hungary","Indonesia",
		"Ireland","Israel","India","British Indian Ocean Territory","Iraq","Iran, Islamic Republic of","Iceland","Italy","Jamaica","Jordan",
		"Japan","Kenya","Kyrgyzstan","Cambodia","Kiribati","Comoros","Saint Kitts and Nevis","Korea, Democratic People's Republic of","Korea, Republic of","Kuwait",
		"Cayman Islands","Kazakhstan","Lao People's Democratic Republic","Lebanon","Saint Lucia","Liechtenstein","Sri Lanka","Liberia","Lesotho","Lithuania",
		"Luxembourg","Latvia","Libyan Arab Jamahiriya","Morocco","Monaco","Moldova, Republic of","Madagascar","Marshall Islands","Macedonia","Mali",
		"Myanmar","Mongolia","Macau","Northern Mariana Islands","Martinique","Mauritania","Montserrat","Malta","Mauritius","Maldives",
		"Malawi","Mexico","Malaysia","Mozambique","Namibia","New Caledonia","Niger","Norfolk Island","Nigeria","Nicaragua",
		"Netherlands","Norway","Nepal","Nauru","Niue","New Zealand","Oman","Panama","Peru","French Polynesia",
		"Papua New Guinea","Philippines","Pakistan","Poland","Saint Pierre and Miquelon","Pitcairn Islands","Puerto Rico","Palestinian Territory","Portugal","Palau",
		"Paraguay","Qatar","Reunion","Romania","Russian Federation","Rwanda","Saudi Arabia","Solomon Islands","Seychelles","Sudan",
		"Sweden","Singapore","Saint Helena","Slovenia","Svalbard and Jan Mayen","Slovakia","Sierra Leone","San Marino","Senegal","Somalia","Suriname",
		"Sao Tome and Principe","El Salvador","Syrian Arab Republic","Swaziland","Turks and Caicos Islands","Chad","French Southern Territories","Togo","Thailand",
		"Tajikistan","Tokelau","Turkmenistan","Tunisia","Tonga","Timor-Leste","Turkey","Trinidad and Tobago","Tuvalu","Taiwan",
		"Tanzania, United Republic of","Ukraine","Uganda","United States Minor Outlying Islands","United States","Uruguay","Uzbekistan","Holy See (Vatican City State)","Saint Vincent and the Grenadines","Venezuela",
		"Virgin Islands, British","Virgin Islands, U.S.","Vietnam","Vanuatu","Wallis and Futuna","Samoa","Yemen","Mayotte","Serbia","South Africa",
		"Zambia","Montenegro","Zimbabwe","Anonymous Proxy","Satellite Provider","Other","Aland Islands","Guernsey","Isle of Man","Jersey",
		"Saint Barthelemy", "Saint Martin", "Bonaire, Saint Eustatius and Saba", "South Sudan"};

/* [Dekamaster/Nightroad] */
/* WHY NOT A DBMAP: There are millions of entries in GeoIP and it has its own algorithm to go quickly through them, a DBMap wouldn't be efficient */
const char* geoip_getcountry(uint32 ipnum)
{
	int depth;
	unsigned int x;
	unsigned int offset = 0;

	if( geoip->data->active == false )
		return geoip_countryname[0];

	for (depth = 31; depth >= 0; depth--) {
		const unsigned char *buf = geoip->data->cache + (long)6 *offset;
		if (ipnum & (1 << depth)) {
			/* Take the right-hand branch */
			x =   (buf[3*1 + 0] << (0*8))
				+ (buf[3*1 + 1] << (1*8))
				+ (buf[3*1 + 2] << (2*8));
		} else {
			/* Take the left-hand branch */
			x =   (buf[3*0 + 0] << (0*8))
				+ (buf[3*0 + 1] << (1*8))
				+ (buf[3*0 + 2] << (2*8));
		}
		if (x >= GEOIP_COUNTRY_BEGIN) {
			x = x-GEOIP_COUNTRY_BEGIN;

			if( x >= GEOIP_MAX_COUNTRIES )
				return geoip_countryname[0];

			return geoip_countryname[x];
		}
		offset = x;
	}
	ShowError("geoip_getcountry(): Error traversing database for ipnum %u\n", ipnum);
	ShowWarning("geoip_getcountry(): Possible database corruption!\n");

	return geoip_countryname[0];
}

/**
 * Disables GeoIP
 * frees geoip.cache
 **/
void geoip_final(bool shutdown)
{
	if (geoip->data->cache) {
		aFree(geoip->data->cache);
		geoip->data->cache = NULL;
	}

	if (geoip->data->active) {
		if (!shutdown)
			ShowStatus("GeoIP "CL_RED"disabled"CL_RESET".\n");
		geoip->data->active = false;
	}
}

/**
 * Reads GeoIP database and stores it into memory
 * geoip.cache should be freed after use!
 * http://dev.maxmind.com/geoip/legacy/geolite/
 **/
void geoip_init(void)
{
	int fno;
	char db_type = 1;
	struct stat bufa;
	FILE *db;

	geoip->data->active = true;

	db = fopen("./db/GeoIP.dat","rb");
	if (db == NULL) {
		ShowError("geoip_readdb: Error reading GeoIP.dat!\n");
		geoip->final(false);
		return;
	}
	fno = fileno(db);
	if (fstat(fno, &bufa) < 0) {
		ShowError("geoip_readdb: Error stating GeoIP.dat! Error %d\n", errno);
		fclose(db);
		geoip->final(false);
		return;
	}
	geoip->data->cache = aMalloc(sizeof(unsigned char) * bufa.st_size);
	if (fread(geoip->data->cache, sizeof(unsigned char), bufa.st_size, db) != bufa.st_size) {
		ShowError("geoip_cache: Couldn't read all elements!\n");
		fclose(db);
		geoip->final(false);
		return;
	}

	// Search database type
	if (fseek(db, -3l, SEEK_END) != 0) {
		db_type = 0;
	} else {
		int i;
		unsigned char delim[3];
		for (i = 0; i < GEOIP_STRUCTURE_INFO_MAX_SIZE; i++) {
			if (fread(delim, sizeof(delim[0]), 3, db) != 3) {
				db_type = 0;
				break;
			}
			if (delim[0] == 255 && delim[1] == 255 && delim[2] == 255) {
				if (fread(&db_type, sizeof(db_type), 1, db) != 1) {
					db_type = 0;
				}
				break;
			}
			if (fseek(db, -4l, SEEK_CUR) != 0) {
				db_type = 0;
				break;
			}
		}
	}

	fclose(db);

	if (db_type != 1) {
		if (db_type)
			ShowError("geoip_init(): Database type is not supported %d!\n", db_type);
		else
			ShowError("geoip_init(): GeoIP is corrupted!\n");

		geoip->final(false);
		return;
	}
	ShowStatus("Finished Reading "CL_GREEN"GeoIP"CL_RESET" Database.\n");
}

void geoip_defaults(void)
{
	geoip = &geoip_s;

	geoip->data = &geoip_data;

	geoip->getcountry = geoip_getcountry;
	geoip->final = geoip_final;
	geoip->init = geoip_init;
}
