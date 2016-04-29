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

#include "inter.h"

#include "char/char.h"
#include "char/geoip.h"
#include "char/int_auction.h"
#include "char/int_elemental.h"
#include "char/int_guild.h"
#include "char/int_homun.h"
#include "char/int_mail.h"
#include "char/int_mercenary.h"
#include "char/int_party.h"
#include "char/int_pet.h"
#include "char/int_quest.h"
#include "char/int_storage.h"
#include "char/mapif.h"
#include "common/cbasetypes.h"
#include "common/db.h"
#include "common/memmgr.h"
#include "common/mmo.h"
#include "common/nullpo.h"
#include "common/showmsg.h"
#include "common/socket.h"
#include "common/sql.h"
#include "common/strlib.h"
#include "common/timer.h"

#include <stdio.h>
#include <stdlib.h>

#define WISDATA_TTL (60*1000) // Wis data Time To Live (60 seconds)
#define WISDELLIST_MAX 256    // Number of elements in the list Delete data Wis

struct inter_interface inter_s;
struct inter_interface *inter;

int char_server_port = 3306;
char char_server_ip[32] = "127.0.0.1";
char char_server_id[32] = "ragnarok";
char char_server_pw[100] = "ragnarok";
char char_server_db[32] = "ragnarok";
char default_codepage[32] = ""; //Feature by irmin.

unsigned int party_share_level = 10;

// recv. packet list
int inter_recv_packet_length[] = {
	-1,-1, 7,-1, -1,13,36, (2 + 4 + 4 + 4 + NAME_LENGTH),  0, 0, 0, 0,  0, 0,  0, 0, // 3000-
	 6,-1, 0, 0,  0, 0, 0, 0, 10,-1, 0, 0,  0, 0,  0, 0,    // 3010-
	-1,10,-1,14, 14,19, 6,-1, 14,14, 0, 0,  0, 0,  0, 0,    // 3020- Party
	-1, 6,-1,-1, 55,19, 6,-1, 14,-1,-1,-1, 18,19,186,-1,    // 3030-
	-1, 9, 0, 0,  0, 0, 0, 0,  7, 6,10,10, 10,-1,  0, 0,    // 3040-
	-1,-1,10,10,  0,-1,12, 0,  0, 0, 0, 0,  0, 0,  0, 0,    // 3050-  Auction System [Zephyrus], Item Bound [Mhalicot]
	 6,-1, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,    // 3060-  Quest system [Kevin] [Inkfish]
	-1,10, 6,-1,  0, 0, 0, 0,  0, 0, 0, 0, -1,10,  6,-1,    // 3070-  Mercenary packets [Zephyrus], Elemental packets [pakpil]
	48,14,-1, 6,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,    // 3080-
	-1,10,-1, 6,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0,  0, 0,    // 3090-  Homunculus packets [albator]
};

struct WisData {
	int id, fd, count, len;
	int64 tick;
	unsigned char src[24], dst[24], msg[512];
};
static struct DBMap *wis_db = NULL; // int wis_id -> struct WisData*
static int wis_dellist[WISDELLIST_MAX], wis_delnum;

#define MAX_JOB_NAMES 150
static char* msg_table[MAX_JOB_NAMES]; //  messages 550 ~ 699 are job names

const char* inter_msg_txt(int msg_number) {
	msg_number -= 550;
	if (msg_number >= 0 && msg_number < MAX_JOB_NAMES &&
	    msg_table[msg_number] != NULL && msg_table[msg_number][0] != '\0')
		return msg_table[msg_number];

	return "Unknown";
}

/**
 * Reads Message Data.
 *
 * This is a modified version of the mapserver's inter->msg_config_read to
 * only read messages with IDs between 550 and 550+MAX_JOB_NAMES.
 *
 * @param[in] cfg_name       configuration filename to read.
 * @param[in] allow_override whether to allow duplicate message IDs to override the original value.
 * @return success state.
 */
bool inter_msg_config_read(const char *cfg_name, bool allow_override)
{
	int msg_number;
	char line[1024], w1[1024], w2[1024];
	FILE *fp;
	static int called = 1;

	nullpo_ret(cfg_name);
	if ((fp = fopen(cfg_name, "r")) == NULL) {
		ShowError("Messages file not found: %s\n", cfg_name);
		return 1;
	}

	if ((--called) == 0)
		memset(msg_table, 0, sizeof(msg_table[0]) * MAX_JOB_NAMES);

	while(fgets(line, sizeof(line), fp) ) {
		if (line[0] == '/' && line[1] == '/')
			continue;
		if (sscanf(line, "%1023[^:]: %1023[^\r\n]", w1, w2) != 2)
			continue;

		if (strcmpi(w1, "import") == 0)
			inter->msg_config_read(w2, true);
		else {
			msg_number = atoi(w1);
			if( msg_number < 550 || msg_number > (550+MAX_JOB_NAMES) )
				continue;
			msg_number -= 550;
			if (msg_number >= 0 && msg_number < MAX_JOB_NAMES) {
				if (msg_table[msg_number] != NULL) {
					if (!allow_override) {
						ShowError("Duplicate message: ID '%d' was already used for '%s'. Message '%s' will be ignored.\n",
						          msg_number, w2, msg_table[msg_number]);
						continue;
					}
					aFree(msg_table[msg_number]);
				}
				msg_table[msg_number] = (char *)aMalloc((strlen(w2) + 1)*sizeof (char));
				strcpy(msg_table[msg_number],w2);
			}
		}
	}

	fclose(fp);

	return 0;
}

/*==========================================
 * Cleanup Message Data
 *------------------------------------------*/
void inter_do_final_msg(void)
{
	int i;
	for (i = 0; i < MAX_JOB_NAMES; i++)
		aFree(msg_table[i]);
}
/* from pc.c due to @accinfo. any ideas to replace this crap are more than welcome. */
const char* inter_job_name(int class_)
{
	switch (class_) {
		case JOB_NOVICE:   // 550
		case JOB_SWORDMAN: // 551
		case JOB_MAGE:     // 552
		case JOB_ARCHER:   // 553
		case JOB_ACOLYTE:  // 554
		case JOB_MERCHANT: // 555
		case JOB_THIEF:    // 556
			return inter->msg_txt(550 - JOB_NOVICE+class_);

		case JOB_KNIGHT:     // 557
		case JOB_PRIEST:     // 558
		case JOB_WIZARD:     // 559
		case JOB_BLACKSMITH: // 560
		case JOB_HUNTER:     // 561
		case JOB_ASSASSIN:   // 562
			return inter->msg_txt(557 - JOB_KNIGHT+class_);

		case JOB_KNIGHT2:
			return inter->msg_txt(557);

		case JOB_CRUSADER:  // 563
		case JOB_MONK:      // 564
		case JOB_SAGE:      // 565
		case JOB_ROGUE:     // 566
		case JOB_ALCHEMIST: // 567
		case JOB_BARD:      // 568
		case JOB_DANCER:    // 569
			return inter->msg_txt(563 - JOB_CRUSADER+class_);

		case JOB_CRUSADER2:
			return inter->msg_txt(563);

		case JOB_WEDDING:      // 570
		case JOB_SUPER_NOVICE: // 571
		case JOB_GUNSLINGER:   // 572
		case JOB_NINJA:        // 573
		case JOB_XMAS:         // 574
			return inter->msg_txt(570 - JOB_WEDDING+class_);

		case JOB_SUMMER:
			return inter->msg_txt(621);

		case JOB_NOVICE_HIGH:   // 575
		case JOB_SWORDMAN_HIGH: // 576
		case JOB_MAGE_HIGH:     // 577
		case JOB_ARCHER_HIGH:   // 578
		case JOB_ACOLYTE_HIGH:  // 579
		case JOB_MERCHANT_HIGH: // 580
		case JOB_THIEF_HIGH:    // 581
			return inter->msg_txt(575 - JOB_NOVICE_HIGH+class_);

		case JOB_LORD_KNIGHT:    // 582
		case JOB_HIGH_PRIEST:    // 583
		case JOB_HIGH_WIZARD:    // 584
		case JOB_WHITESMITH:     // 585
		case JOB_SNIPER:         // 586
		case JOB_ASSASSIN_CROSS: // 587
			return inter->msg_txt(582 - JOB_LORD_KNIGHT+class_);

		case JOB_LORD_KNIGHT2:
			return inter->msg_txt(582);

		case JOB_PALADIN:   // 588
		case JOB_CHAMPION:  // 589
		case JOB_PROFESSOR: // 590
		case JOB_STALKER:   // 591
		case JOB_CREATOR:   // 592
		case JOB_CLOWN:     // 593
		case JOB_GYPSY:     // 594
			return inter->msg_txt(588 - JOB_PALADIN + class_);

		case JOB_PALADIN2:
			return inter->msg_txt(588);

		case JOB_BABY:          // 595
		case JOB_BABY_SWORDMAN: // 596
		case JOB_BABY_MAGE:     // 597
		case JOB_BABY_ARCHER:   // 598
		case JOB_BABY_ACOLYTE:  // 599
		case JOB_BABY_MERCHANT: // 600
		case JOB_BABY_THIEF:    // 601
			return inter->msg_txt(595 - JOB_BABY + class_);

		case JOB_BABY_KNIGHT:     // 602
		case JOB_BABY_PRIEST:     // 603
		case JOB_BABY_WIZARD:     // 604
		case JOB_BABY_BLACKSMITH: // 605
		case JOB_BABY_HUNTER:     // 606
		case JOB_BABY_ASSASSIN:   // 607
			return inter->msg_txt(602 - JOB_BABY_KNIGHT + class_);

		case JOB_BABY_KNIGHT2:
			return inter->msg_txt(602);

		case JOB_BABY_CRUSADER:  // 608
		case JOB_BABY_MONK:      // 609
		case JOB_BABY_SAGE:      // 610
		case JOB_BABY_ROGUE:     // 611
		case JOB_BABY_ALCHEMIST: // 612
		case JOB_BABY_BARD:      // 613
		case JOB_BABY_DANCER:    // 614
			return inter->msg_txt(608 - JOB_BABY_CRUSADER + class_);

		case JOB_BABY_CRUSADER2:
			return inter->msg_txt(608);

		case JOB_SUPER_BABY:
			return inter->msg_txt(615);

		case JOB_TAEKWON:
			return inter->msg_txt(616);
		case JOB_STAR_GLADIATOR:
		case JOB_STAR_GLADIATOR2:
			return inter->msg_txt(617);
		case JOB_SOUL_LINKER:
			return inter->msg_txt(618);

		case JOB_GANGSI:         // 622
		case JOB_DEATH_KNIGHT:   // 623
		case JOB_DARK_COLLECTOR: // 624
			return inter->msg_txt(622 - JOB_GANGSI+class_);

		case JOB_RUNE_KNIGHT:      // 625
		case JOB_WARLOCK:          // 626
		case JOB_RANGER:           // 627
		case JOB_ARCH_BISHOP:      // 628
		case JOB_MECHANIC:         // 629
		case JOB_GUILLOTINE_CROSS: // 630
			return inter->msg_txt(625 - JOB_RUNE_KNIGHT+class_);

		case JOB_RUNE_KNIGHT_T:      // 656
		case JOB_WARLOCK_T:          // 657
		case JOB_RANGER_T:           // 658
		case JOB_ARCH_BISHOP_T:      // 659
		case JOB_MECHANIC_T:         // 660
		case JOB_GUILLOTINE_CROSS_T: // 661
			return inter->msg_txt(656 - JOB_RUNE_KNIGHT_T+class_);

		case JOB_ROYAL_GUARD:   // 631
		case JOB_SORCERER:      // 632
		case JOB_MINSTREL:      // 633
		case JOB_WANDERER:      // 634
		case JOB_SURA:          // 635
		case JOB_GENETIC:       // 636
		case JOB_SHADOW_CHASER: // 637
			return inter->msg_txt(631 - JOB_ROYAL_GUARD+class_);

		case JOB_ROYAL_GUARD_T:   // 662
		case JOB_SORCERER_T:      // 663
		case JOB_MINSTREL_T:      // 664
		case JOB_WANDERER_T:      // 665
		case JOB_SURA_T:          // 666
		case JOB_GENETIC_T:       // 667
		case JOB_SHADOW_CHASER_T: // 668
			return inter->msg_txt(662 - JOB_ROYAL_GUARD_T+class_);

		case JOB_RUNE_KNIGHT2:
			return inter->msg_txt(625);

		case JOB_RUNE_KNIGHT_T2:
			return inter->msg_txt(656);

		case JOB_ROYAL_GUARD2:
			return inter->msg_txt(631);

		case JOB_ROYAL_GUARD_T2:
			return inter->msg_txt(662);

		case JOB_RANGER2:
			return inter->msg_txt(627);

		case JOB_RANGER_T2:
			return inter->msg_txt(658);

		case JOB_MECHANIC2:
			return inter->msg_txt(629);

		case JOB_MECHANIC_T2:
			return inter->msg_txt(660);

		case JOB_BABY_RUNE:     // 638
		case JOB_BABY_WARLOCK:  // 639
		case JOB_BABY_RANGER:   // 640
		case JOB_BABY_BISHOP:   // 641
		case JOB_BABY_MECHANIC: // 642
		case JOB_BABY_CROSS:    // 643
		case JOB_BABY_GUARD:    // 644
		case JOB_BABY_SORCERER: // 645
		case JOB_BABY_MINSTREL: // 646
		case JOB_BABY_WANDERER: // 647
		case JOB_BABY_SURA:     // 648
		case JOB_BABY_GENETIC:  // 649
		case JOB_BABY_CHASER:   // 650
			return inter->msg_txt(638 - JOB_BABY_RUNE+class_);

		case JOB_BABY_RUNE2:
			return inter->msg_txt(638);

		case JOB_BABY_GUARD2:
			return inter->msg_txt(644);

		case JOB_BABY_RANGER2:
			return inter->msg_txt(640);

		case JOB_BABY_MECHANIC2:
			return inter->msg_txt(642);

		case JOB_SUPER_NOVICE_E: // 651
		case JOB_SUPER_BABY_E:   // 652
			return inter->msg_txt(651 - JOB_SUPER_NOVICE_E+class_);

		case JOB_KAGEROU: // 653
		case JOB_OBORO:   // 654
			return inter->msg_txt(653 - JOB_KAGEROU+class_);

		case JOB_REBELLION:
			return inter->msg_txt(655);

		default:
			return inter->msg_txt(620); // "Unknown Job"
	}
}

/**
 * Argument-list version of inter_msg_to_fd
 * @see inter_msg_to_fd
 */
void inter_vmsg_to_fd(int fd, int u_fd, int aid, char* msg, va_list ap)
{
	char msg_out[512];
	va_list apcopy;
	int len = 1;/* yes we start at 1 */

	nullpo_retv(msg);
	va_copy(apcopy, ap);
	len += vsnprintf(msg_out, 512, msg, apcopy);
	va_end(apcopy);

	WFIFOHEAD(fd,12 + len);

	WFIFOW(fd,0) = 0x3807;
	WFIFOW(fd,2) = 12 + (unsigned short)len;
	WFIFOL(fd,4) = u_fd;
	WFIFOL(fd,8) = aid;
	safestrncpy(WFIFOP(fd,12), msg_out, len);

	WFIFOSET(fd,12 + len);

	return;
}

/**
 * Sends a message to map server (fd) to a user (u_fd) although we use fd we
 * keep aid for safe-check.
 * @param fd   Mapserver's fd
 * @param u_fd Recipient's fd
 * @param aid  Recipient's expected for sanity checks on the mapserver
 * @param msg  Message format string
 * @param ...  Additional parameters for (v)sprinf
 */
void inter_msg_to_fd(int fd, int u_fd, int aid, char *msg, ...) __attribute__((format(printf, 4, 5)));
void inter_msg_to_fd(int fd, int u_fd, int aid, char *msg, ...)
{
	va_list ap;
	va_start(ap,msg);
	inter->vmsg_to_fd(fd, u_fd, aid, msg, ap);
	va_end(ap);
}

/* [Dekamaster/Nightroad] */
void mapif_parse_accinfo(int fd)
{
	int u_fd = RFIFOL(fd,2), aid = RFIFOL(fd,6), castergroup = RFIFOL(fd,10);
	char query[NAME_LENGTH], query_esq[NAME_LENGTH*2+1];
	int account_id;
	char *data;

	safestrncpy(query, RFIFOP(fd,14), NAME_LENGTH);

	SQL->EscapeString(inter->sql_handle, query_esq, query);

	account_id = atoi(query);

	if (account_id < START_ACCOUNT_NUM) {
		// is string
		if ( SQL_ERROR == SQL->Query(inter->sql_handle, "SELECT `account_id`,`name`,`class`,`base_level`,`job_level`,`online` FROM `%s` WHERE `name` LIKE '%s' LIMIT 10", char_db, query_esq)
				|| SQL->NumRows(inter->sql_handle) == 0 ) {
			if( SQL->NumRows(inter->sql_handle) == 0 ) {
				inter->msg_to_fd(fd, u_fd, aid, "No matches were found for your criteria, '%s'",query);
			} else {
				Sql_ShowDebug(inter->sql_handle);
				inter->msg_to_fd(fd, u_fd, aid, "An error occurred, bother your admin about it.");
			}
			SQL->FreeResult(inter->sql_handle);
			return;
		} else {
			if( SQL->NumRows(inter->sql_handle) == 1 ) {//we found a perfect match
				SQL->NextRow(inter->sql_handle);
				SQL->GetData(inter->sql_handle, 0, &data, NULL); account_id = atoi(data);
				SQL->FreeResult(inter->sql_handle);
			} else {// more than one, listing... [Dekamaster/Nightroad]
				inter->msg_to_fd(fd, u_fd, aid, "Your query returned the following %d results, please be more specific...",(int)SQL->NumRows(inter->sql_handle));
				while ( SQL_SUCCESS == SQL->NextRow(inter->sql_handle) ) {
					int class_;
					short base_level, job_level, online;
					char name[NAME_LENGTH];

					SQL->GetData(inter->sql_handle, 0, &data, NULL); account_id = atoi(data);
					SQL->GetData(inter->sql_handle, 1, &data, NULL); safestrncpy(name, data, sizeof(name));
					SQL->GetData(inter->sql_handle, 2, &data, NULL); class_ = atoi(data);
					SQL->GetData(inter->sql_handle, 3, &data, NULL); base_level = atoi(data);
					SQL->GetData(inter->sql_handle, 4, &data, NULL); job_level = atoi(data);
					SQL->GetData(inter->sql_handle, 5, &data, NULL); online = atoi(data);

					inter->msg_to_fd(fd, u_fd, aid, "[AID: %d] %s | %s | Level: %d/%d | %s", account_id, name, inter->job_name(class_), base_level, job_level, online?"Online":"Offline");
				}
				SQL->FreeResult(inter->sql_handle);
				return;
			}
		}
	}

	/* it will only get here if we have a single match */
	/* and we will send packet with account id to login server asking for account info */
	if( account_id ) {
		mapif->on_parse_accinfo(account_id, u_fd, aid, castergroup, fd);
	}

	return;
}
void mapif_parse_accinfo2(bool success, int map_fd, int u_fd, int u_aid, int account_id, const char *userid, const char *user_pass,
		const char *email, const char *last_ip, const char *lastlogin, const char *pin_code, const char *birthdate,
		int group_id, int logincount, int state)
{
	nullpo_retv(userid);
	nullpo_retv(user_pass);
	nullpo_retv(email);
	nullpo_retv(last_ip);
	nullpo_retv(lastlogin);
	nullpo_retv(birthdate);
	if (map_fd <= 0 || !sockt->session_is_active(map_fd))
		return; // check if we have a valid fd

	if (!success) {
		inter->msg_to_fd(map_fd, u_fd, u_aid, "No account with ID '%d' was found.", account_id);
		return;
	}

	inter->msg_to_fd(map_fd, u_fd, u_aid, "-- Account %d --", account_id);
	inter->msg_to_fd(map_fd, u_fd, u_aid, "User: %s | GM Group: %d | State: %d", userid, group_id, state);

	if (*user_pass != '\0') { /* password is only received if your gm level is greater than the one you're searching for */
		if (pin_code && *pin_code != '\0')
			inter->msg_to_fd(map_fd, u_fd, u_aid, "Password: %s (PIN:%s)", user_pass, pin_code);
		else
			inter->msg_to_fd(map_fd, u_fd, u_aid, "Password: %s", user_pass );
	}

	inter->msg_to_fd(map_fd, u_fd, u_aid, "Account e-mail: %s | Birthdate: %s", email, birthdate);
	inter->msg_to_fd(map_fd, u_fd, u_aid, "Last IP: %s (%s)", last_ip, geoip->getcountry(sockt->str2ip(last_ip)));
	inter->msg_to_fd(map_fd, u_fd, u_aid, "This user has logged %d times, the last time were at %s", logincount, lastlogin);
	inter->msg_to_fd(map_fd, u_fd, u_aid, "-- Character Details --");

	if ( SQL_ERROR == SQL->Query(inter->sql_handle, "SELECT `char_id`, `name`, `char_num`, `class`, `base_level`, `job_level`, `online` "
	                                         "FROM `%s` WHERE `account_id` = '%d' ORDER BY `char_num` LIMIT %d", char_db, account_id, MAX_CHARS)
	  || SQL->NumRows(inter->sql_handle) == 0 ) {
		if (SQL->NumRows(inter->sql_handle) == 0) {
			inter->msg_to_fd(map_fd, u_fd, u_aid, "This account doesn't have characters.");
		} else {
			inter->msg_to_fd(map_fd, u_fd, u_aid, "An error occurred, bother your admin about it.");
			Sql_ShowDebug(inter->sql_handle);
		}
	} else {
		while ( SQL_SUCCESS == SQL->NextRow(inter->sql_handle) ) {
			char *data;
			int char_id, class_;
			short char_num, base_level, job_level, online;
			char name[NAME_LENGTH];

			SQL->GetData(inter->sql_handle, 0, &data, NULL); char_id = atoi(data);
			SQL->GetData(inter->sql_handle, 1, &data, NULL); safestrncpy(name, data, sizeof(name));
			SQL->GetData(inter->sql_handle, 2, &data, NULL); char_num = atoi(data);
			SQL->GetData(inter->sql_handle, 3, &data, NULL); class_ = atoi(data);
			SQL->GetData(inter->sql_handle, 4, &data, NULL); base_level = atoi(data);
			SQL->GetData(inter->sql_handle, 5, &data, NULL); job_level = atoi(data);
			SQL->GetData(inter->sql_handle, 6, &data, NULL); online = atoi(data);

			inter->msg_to_fd(map_fd, u_fd, u_aid, "[Slot/CID: %d/%d] %s | %s | Level: %d/%d | %s", char_num, char_id, name, inter->job_name(class_), base_level, job_level, online?"On":"Off");
		}
	}
	SQL->FreeResult(inter->sql_handle);

	return;
}
/**
 * Handles save reg data from map server and distributes accordingly.
 *
 * @param val either str or int, depending on type
 * @param type false when int, true otherwise
 **/
void inter_savereg(int account_id, int char_id, const char *key, unsigned int index, intptr_t val, bool is_string)
{
	nullpo_retv(key);
	/* to login server we go! */
	if( key[0] == '#' && key[1] == '#' ) {/* global account reg */
		if (sockt->session_is_valid(chr->login_fd))
			chr->global_accreg_to_login_add(key,index,val,is_string);
		else {
			ShowError("Login server unavailable, cant perform update on '%s' variable for AID:%d CID:%d\n",key,account_id,char_id);
		}
	} else if ( key[0] == '#' ) {/* local account reg */
		if( is_string ) {
			if( val ) {
				if( SQL_ERROR == SQL->Query(inter->sql_handle, "REPLACE INTO `%s` (`account_id`,`key`,`index`,`value`) VALUES ('%d','%s','%u','%s')", acc_reg_str_db, account_id, key, index, (char*)val) )
					Sql_ShowDebug(inter->sql_handle);
			} else {
				if( SQL_ERROR == SQL->Query(inter->sql_handle, "DELETE FROM `%s` WHERE `account_id` = '%d' AND `key` = '%s' AND `index` = '%u' LIMIT 1", acc_reg_str_db, account_id, key, index) )
					Sql_ShowDebug(inter->sql_handle);
			}
		} else {
			if( val ) {
				if( SQL_ERROR == SQL->Query(inter->sql_handle, "REPLACE INTO `%s` (`account_id`,`key`,`index`,`value`) VALUES ('%d','%s','%u','%d')", acc_reg_num_db, account_id, key, index, (int)val) )
					Sql_ShowDebug(inter->sql_handle);
			} else {
				if( SQL_ERROR == SQL->Query(inter->sql_handle, "DELETE FROM `%s` WHERE `account_id` = '%d' AND `key` = '%s' AND `index` = '%u' LIMIT 1", acc_reg_num_db, account_id, key, index) )
					Sql_ShowDebug(inter->sql_handle);
			}
		}
	} else { /* char reg */
		if( is_string ) {
			if( val ) {
				if( SQL_ERROR == SQL->Query(inter->sql_handle, "REPLACE INTO `%s` (`char_id`,`key`,`index`,`value`) VALUES ('%d','%s','%u','%s')", char_reg_str_db, char_id, key, index, (char*)val) )
					Sql_ShowDebug(inter->sql_handle);
			} else {
				if( SQL_ERROR == SQL->Query(inter->sql_handle, "DELETE FROM `%s` WHERE `char_id` = '%d' AND `key` = '%s' AND `index` = '%u' LIMIT 1", char_reg_str_db, char_id, key, index) )
					Sql_ShowDebug(inter->sql_handle);
			}
		} else {
			if( val ) {
				if( SQL_ERROR == SQL->Query(inter->sql_handle, "REPLACE INTO `%s` (`char_id`,`key`,`index`,`value`) VALUES ('%d','%s','%u','%d')", char_reg_num_db, char_id, key, index, (int)val) )
					Sql_ShowDebug(inter->sql_handle);
			} else {
				if( SQL_ERROR == SQL->Query(inter->sql_handle, "DELETE FROM `%s` WHERE `char_id` = '%d' AND `key` = '%s' AND `index` = '%u' LIMIT 1", char_reg_num_db, char_id, key, index) )
					Sql_ShowDebug(inter->sql_handle);
			}
		}
	}
}

// Load account_reg from sql (type=2)
int inter_accreg_fromsql(int account_id,int char_id, int fd, int type)
{
	char* data;
	size_t len;
	unsigned int plen = 0;

	switch( type ) {
		case 3: //char reg
			if( SQL_ERROR == SQL->Query(inter->sql_handle, "SELECT `key`, `index`, `value` FROM `%s` WHERE `char_id`='%d'", char_reg_str_db, char_id) )
				Sql_ShowDebug(inter->sql_handle);
			break;
		case 2: //account reg
			if( SQL_ERROR == SQL->Query(inter->sql_handle, "SELECT `key`, `index`, `value` FROM `%s` WHERE `account_id`='%d'", acc_reg_str_db, account_id) )
				Sql_ShowDebug(inter->sql_handle);
			break;
		case 1: //account2 reg
			ShowError("inter->accreg_fromsql: Char server shouldn't handle type 1 registry values (##). That is the login server's work!\n");
			return 0;
		default:
			ShowError("inter->accreg_fromsql: Invalid type %d\n", type);
			return 0;
	}

	WFIFOHEAD(fd, 60000 + 300);
	WFIFOW(fd, 0) = 0x3804;
	/* 0x2 = length, set prior to being sent */
	WFIFOL(fd, 4) = account_id;
	WFIFOL(fd, 8) = char_id;
	WFIFOB(fd, 12) = 0;/* var type (only set when all vars have been sent, regardless of type) */
	WFIFOB(fd, 13) = 1;/* is string type */
	WFIFOW(fd, 14) = 0;/* count */
	plen = 16;

	/**
	 * Vessel!
	 *
	 * str type
	 * { keyLength(B), key(<keyLength>), index(L), valLength(B), val(<valLength>) }
	 **/
	while ( SQL_SUCCESS == SQL->NextRow(inter->sql_handle) ) {
		SQL->GetData(inter->sql_handle, 0, &data, NULL);
		len = strlen(data)+1;

		WFIFOB(fd, plen) = (unsigned char)len;/* won't be higher; the column size is 32 */
		plen += 1;

		safestrncpy(WFIFOP(fd,plen), data, len);
		plen += len;

		SQL->GetData(inter->sql_handle, 1, &data, NULL);

		WFIFOL(fd, plen) = (unsigned int)atol(data);
		plen += 4;

		SQL->GetData(inter->sql_handle, 2, &data, NULL);
		len = strlen(data)+1;

		WFIFOB(fd, plen) = (unsigned char)len;/* won't be higher; the column size is 254 */
		plen += 1;

		safestrncpy(WFIFOP(fd,plen), data, len);
		plen += len;

		WFIFOW(fd, 14) += 1;

		if( plen > 60000 ) {
			WFIFOW(fd, 2) = plen;
			WFIFOSET(fd, plen);

			/* prepare follow up */
			WFIFOHEAD(fd, 60000 + 300);
			WFIFOW(fd, 0) = 0x3804;
			/* 0x2 = length, set prior to being sent */
			WFIFOL(fd, 4) = account_id;
			WFIFOL(fd, 8) = char_id;
			WFIFOB(fd, 12) = 0;/* var type (only set when all vars have been sent, regardless of type) */
			WFIFOB(fd, 13) = 1;/* is string type */
			WFIFOW(fd, 14) = 0;/* count */
			plen = 16;
		}
	}

	/* mark & go. */
	WFIFOW(fd, 2) = plen;
	WFIFOSET(fd, plen);

	SQL->FreeResult(inter->sql_handle);

	switch( type ) {
		case 3: //char reg
			if( SQL_ERROR == SQL->Query(inter->sql_handle, "SELECT `key`, `index`, `value` FROM `%s` WHERE `char_id`='%d'", char_reg_num_db, char_id) )
				Sql_ShowDebug(inter->sql_handle);
			break;
		case 2: //account reg
			if( SQL_ERROR == SQL->Query(inter->sql_handle, "SELECT `key`, `index`, `value` FROM `%s` WHERE `account_id`='%d'", acc_reg_num_db, account_id) )
				Sql_ShowDebug(inter->sql_handle);
			break;
#if 0 // This is already checked above.
		case 1: //account2 reg
			ShowError("inter->accreg_fromsql: Char server shouldn't handle type 1 registry values (##). That is the login server's work!\n");
			return 0;
#endif // 0
	}

	WFIFOHEAD(fd, 60000 + 300);
	WFIFOW(fd, 0) = 0x3804;
	/* 0x2 = length, set prior to being sent */
	WFIFOL(fd, 4) = account_id;
	WFIFOL(fd, 8) = char_id;
	WFIFOB(fd, 12) = 0;/* var type (only set when all vars have been sent, regardless of type) */
	WFIFOB(fd, 13) = 0;/* is int type */
	WFIFOW(fd, 14) = 0;/* count */
	plen = 16;

	/**
	 * Vessel!
	 *
	 * int type
	 * { keyLength(B), key(<keyLength>), index(L), value(L) }
	 **/
	while ( SQL_SUCCESS == SQL->NextRow(inter->sql_handle) ) {
		SQL->GetData(inter->sql_handle, 0, &data, NULL);
		len = strlen(data)+1;

		WFIFOB(fd, plen) = (unsigned char)len;/* won't be higher; the column size is 32 */
		plen += 1;

		safestrncpy(WFIFOP(fd,plen), data, len);
		plen += len;

		SQL->GetData(inter->sql_handle, 1, &data, NULL);

		WFIFOL(fd, plen) = (unsigned int)atol(data);
		plen += 4;

		SQL->GetData(inter->sql_handle, 2, &data, NULL);

		WFIFOL(fd, plen) = atoi(data);
		plen += 4;

		WFIFOW(fd, 14) += 1;

		if( plen > 60000 ) {
			WFIFOW(fd, 2) = plen;
			WFIFOSET(fd, plen);

			/* prepare follow up */
			WFIFOHEAD(fd, 60000 + 300);
			WFIFOW(fd, 0) = 0x3804;
			/* 0x2 = length, set prior to being sent */
			WFIFOL(fd, 4) = account_id;
			WFIFOL(fd, 8) = char_id;
			WFIFOB(fd, 12) = 0;/* var type (only set when all vars have been sent, regardless of type) */
			WFIFOB(fd, 13) = 0;/* is int type */
			WFIFOW(fd, 14) = 0;/* count */
			plen = 16;
		}
	}

	/* mark as complete & go. */
	WFIFOB(fd, 12) = type;
	WFIFOW(fd, 2) = plen;
	WFIFOSET(fd, plen);

	SQL->FreeResult(inter->sql_handle);
	return 1;
}

/*==========================================
 * read config file
 *------------------------------------------*/
static int inter_config_read(const char* cfgName)
{
	char line[1024], w1[1024], w2[1024];
	FILE* fp;

	nullpo_retr(1, cfgName);
	fp = fopen(cfgName, "r");
	if(fp == NULL) {
		ShowError("File not found: %s\n", cfgName);
		return 1;
	}

	while (fgets(line, sizeof(line), fp)) {
		int i = sscanf(line, "%1023[^:]: %1023[^\r\n]", w1, w2);
		if(i != 2)
			continue;

		if(!strcmpi(w1,"char_server_ip")) {
			safestrncpy(char_server_ip, w2, sizeof(char_server_ip));
		} else if(!strcmpi(w1,"char_server_port")) {
			char_server_port = atoi(w2);
		} else if(!strcmpi(w1,"char_server_id")) {
			safestrncpy(char_server_id, w2, sizeof(char_server_id));
		} else if(!strcmpi(w1,"char_server_pw")) {
			safestrncpy(char_server_pw, w2, sizeof(char_server_pw));
		} else if(!strcmpi(w1,"char_server_db")) {
			safestrncpy(char_server_db, w2, sizeof(char_server_db));
		} else if(!strcmpi(w1,"default_codepage")) {
			safestrncpy(default_codepage, w2, sizeof(default_codepage));
		} else if(!strcmpi(w1,"party_share_level"))
			party_share_level = atoi(w2);
		else if(!strcmpi(w1,"log_inter"))
			log_inter = atoi(w2);
		else if(!strcmpi(w1,"import"))
			inter->config_read(w2);
	}
	fclose(fp);

	ShowInfo ("Done reading %s.\n", cfgName);

	return 0;
}

/**
 * Save interlog into sql (arglist version)
 * @see inter_log
 */
int inter_vlog(char* fmt, va_list ap)
{
	char str[255];
	char esc_str[sizeof(str)*2+1];// escaped str
	va_list apcopy;

	va_copy(apcopy, ap);
	vsnprintf(str, sizeof(str), fmt, apcopy);
	va_end(apcopy);

	SQL->EscapeStringLen(inter->sql_handle, esc_str, str, strnlen(str, sizeof(str)));
	if( SQL_ERROR == SQL->Query(inter->sql_handle, "INSERT INTO `%s` (`time`, `log`) VALUES (NOW(),  '%s')", interlog_db, esc_str) )
		Sql_ShowDebug(inter->sql_handle);

	return 0;
}

/**
 * Save interlog into sql
 * @param fmt Message's format string
 * @param ... Additional (printf-like) arguments
 * @return Always 0 // FIXME
 */
int inter_log(char* fmt, ...)
{
	va_list ap;
	int ret;

	va_start(ap,fmt);
	ret = inter->vlog(fmt, ap);
	va_end(ap);

	return ret;
}

// initialize
int inter_init_sql(const char *file)
{
	//int i;

	inter->config_read(file);

	//DB connection initialized
	inter->sql_handle = SQL->Malloc();
	ShowInfo("Connect Character DB server.... (Character Server)\n");
	if( SQL_ERROR == SQL->Connect(inter->sql_handle, char_server_id, char_server_pw, char_server_ip, (uint16)char_server_port, char_server_db) )
	{
		Sql_ShowDebug(inter->sql_handle);
		SQL->Free(inter->sql_handle);
		exit(EXIT_FAILURE);
	}

	if( *default_codepage ) {
		if( SQL_ERROR == SQL->SetEncoding(inter->sql_handle, default_codepage) )
			Sql_ShowDebug(inter->sql_handle);
	}

	wis_db = idb_alloc(DB_OPT_RELEASE_DATA);
	inter_guild->sql_init();
	inter_storage->sql_init();
	inter_party->sql_init();
	inter_pet->sql_init();
	inter_homunculus->sql_init();
	inter_mercenary->sql_init();
	inter_elemental->sql_init();
	inter_mail->sql_init();
	inter_auction->sql_init();

	geoip->init();
	inter->msg_config_read("conf/messages.conf", false);
	return 0;
}

// finalize
void inter_final(void)
{
	wis_db->destroy(wis_db, NULL);

	inter_guild->sql_final();
	inter_storage->sql_final();
	inter_party->sql_final();
	inter_pet->sql_final();
	inter_homunculus->sql_final();
	inter_mercenary->sql_final();
	inter_elemental->sql_final();
	inter_mail->sql_final();
	inter_auction->sql_final();

	geoip->final(true);
	inter->do_final_msg();
	return;
}

int inter_mapif_init(int fd)
{
	return 0;
}


//--------------------------------------------------------

// broadcast sending
int mapif_broadcast(const unsigned char *mes, int len, unsigned int fontColor, short fontType, short fontSize, short fontAlign, short fontY, int sfd)
{
	unsigned char *buf = (unsigned char*)aMalloc((len)*sizeof(unsigned char));

	nullpo_ret(mes);
	Assert_ret(len >= 16);
	WBUFW(buf,0) = 0x3800;
	WBUFW(buf,2) = len;
	WBUFL(buf,4) = fontColor;
	WBUFW(buf,8) = fontType;
	WBUFW(buf,10) = fontSize;
	WBUFW(buf,12) = fontAlign;
	WBUFW(buf,14) = fontY;
	memcpy(WBUFP(buf,16), mes, len - 16);
	mapif->sendallwos(sfd, buf, len);

	aFree(buf);
	return 0;
}

// Wis sending
int mapif_wis_message(struct WisData *wd)
{
	unsigned char buf[2048];
	nullpo_ret(wd);
	//if (wd->len > 2047-56) wd->len = 2047-56; //Force it to fit to avoid crashes. [Skotlex]
	if (wd->len < 0)
		wd->len = 0;
	if (wd->len >= (int)sizeof(wd->msg) - 1)
		wd->len = (int)sizeof(wd->msg) - 1;

	WBUFW(buf, 0) = 0x3801;
	WBUFW(buf, 2) = 56 +wd->len;
	WBUFL(buf, 4) = wd->id;
	memcpy(WBUFP(buf, 8), wd->src, NAME_LENGTH);
	memcpy(WBUFP(buf,32), wd->dst, NAME_LENGTH);
	memcpy(WBUFP(buf,56), wd->msg, wd->len);
	wd->count = mapif->sendall(buf,WBUFW(buf,2));

	return 0;
}

void mapif_wis_response(int fd, const unsigned char *src, int flag)
{
	unsigned char buf[27];
	nullpo_retv(src);
	WBUFW(buf, 0)=0x3802;
	memcpy(WBUFP(buf, 2),src,24);
	WBUFB(buf,26)=flag;
	mapif->send(fd,buf,27);
}

// Wis sending result
int mapif_wis_end(struct WisData *wd, int flag)
{
	nullpo_ret(wd);
	mapif->wis_response(wd->fd, wd->src, flag);
	return 0;
}

#if 0
// Account registry transfer to map-server
static void mapif_account_reg(int fd, unsigned char *src)
{
	nullpo_retv(src);
	WBUFW(src,0)=0x3804; //NOTE: writing to RFIFO
	mapif->sendallwos(fd, src, WBUFW(src,2));
}
#endif // 0

// Send the requested account_reg
int mapif_account_reg_reply(int fd,int account_id,int char_id, int type)
{
	inter->accreg_fromsql(account_id,char_id,fd,type);
	return 0;
}

//Request to kick char from a certain map server. [Skotlex]
int mapif_disconnectplayer(int fd, int account_id, int char_id, int reason)
{
	if (fd >= 0)
	{
		WFIFOHEAD(fd,7);
		WFIFOW(fd,0) = 0x2b1f;
		WFIFOL(fd,2) = account_id;
		WFIFOB(fd,6) = reason;
		WFIFOSET(fd,7);
		return 0;
	}
	return -1;
}

//--------------------------------------------------------

/**
 * Existence check of WISP data
 * @see DBApply
 */
int inter_check_ttl_wisdata_sub(union DBKey key, struct DBData *data, va_list ap)
{
	int64 tick;
	struct WisData *wd = DB->data2ptr(data);
	nullpo_ret(wd);
	tick = va_arg(ap, int64);

	if (DIFF_TICK(tick, wd->tick) > WISDATA_TTL && wis_delnum < WISDELLIST_MAX)
		wis_dellist[wis_delnum++] = wd->id;

	return 0;
}

int inter_check_ttl_wisdata(void)
{
	int64 tick = timer->gettick();
	int i;

	do {
		wis_delnum = 0;
		wis_db->foreach(wis_db, inter->check_ttl_wisdata_sub, tick);
		for(i = 0; i < wis_delnum; i++) {
			struct WisData *wd = (struct WisData*)idb_get(wis_db, wis_dellist[i]);
			ShowWarning("inter: wis data id=%d time out : from %s to %s\n", wd->id, wd->src, wd->dst);
			// removed. not send information after a timeout. Just no answer for the player
			//mapif->wis_end(wd, 1); // flag: 0: success to send whisper, 1: target character is not logged in?, 2: ignored by target
			idb_remove(wis_db, wd->id);
		}
	} while(wis_delnum >= WISDELLIST_MAX);

	return 0;
}

//--------------------------------------------------------

// broadcast sending
int mapif_parse_broadcast(int fd)
{
	mapif->broadcast(RFIFOP(fd,16), RFIFOW(fd,2), RFIFOL(fd,4), RFIFOW(fd,8), RFIFOW(fd,10), RFIFOW(fd,12), RFIFOW(fd,14), fd);
	return 0;
}


// Wisp/page request to send
int mapif_parse_WisRequest(int fd)
{
	struct WisData* wd;
	static int wisid = 0;
	char name[NAME_LENGTH];
	char esc_name[NAME_LENGTH*2+1];// escaped name
	char* data;
	size_t len;


	if ( fd <= 0 ) {return 0;} // check if we have a valid fd

	if (RFIFOW(fd,2)-52 >= sizeof(wd->msg)) {
		ShowWarning("inter: Wis message size too long.\n");
		return 0;
	} else if (RFIFOW(fd,2)-52 <= 0) { // normally, impossible, but who knows...
		ShowError("inter: Wis message doesn't exist.\n");
		return 0;
	}

	safestrncpy(name, RFIFOP(fd,28), NAME_LENGTH); //Received name may be too large and not contain \0! [Skotlex]

	SQL->EscapeStringLen(inter->sql_handle, esc_name, name, strnlen(name, NAME_LENGTH));
	if( SQL_ERROR == SQL->Query(inter->sql_handle, "SELECT `name` FROM `%s` WHERE `name`='%s'", char_db, esc_name) )
		Sql_ShowDebug(inter->sql_handle);

	// search if character exists before to ask all map-servers
	if( SQL_SUCCESS != SQL->NextRow(inter->sql_handle) )
	{
		mapif->wis_response(fd, RFIFOP(fd, 4), 1);
	}
	else
	{// Character exists. So, ask all map-servers
		// to be sure of the correct name, rewrite it
		SQL->GetData(inter->sql_handle, 0, &data, &len);
		memset(name, 0, NAME_LENGTH);
		memcpy(name, data, min(len, NAME_LENGTH));
		// if source is destination, don't ask other servers.
		if (strncmp(RFIFOP(fd,4), name, NAME_LENGTH) == 0) {
			mapif->wis_response(fd, RFIFOP(fd, 4), 1);
		}
		else
		{
			CREATE(wd, struct WisData, 1);

			// Whether the failure of previous wisp/page transmission (timeout)
			inter->check_ttl_wisdata();

			wd->id = ++wisid;
			wd->fd = fd;
			wd->len= RFIFOW(fd,2)-52;
			memcpy(wd->src, RFIFOP(fd, 4), NAME_LENGTH);
			memcpy(wd->dst, RFIFOP(fd,28), NAME_LENGTH);
			memcpy(wd->msg, RFIFOP(fd,52), wd->len);
			wd->tick = timer->gettick();
			idb_put(wis_db, wd->id, wd);
			mapif->wis_message(wd);
		}
	}

	SQL->FreeResult(inter->sql_handle);
	return 0;
}


// Wisp/page transmission result
int mapif_parse_WisReply(int fd)
{
	int id, flag;
	struct WisData *wd;

	id = RFIFOL(fd,2);
	flag = RFIFOB(fd,6);
	wd = (struct WisData*)idb_get(wis_db, id);
	if (wd == NULL)
		return 0; // This wisp was probably suppress before, because it was timeout of because of target was found on another map-server

	if ((--wd->count) <= 0 || flag != 1) {
		mapif->wis_end(wd, flag); // flag: 0: success to send whisper, 1: target character is not logged in?, 2: ignored by target
		idb_remove(wis_db, id);
	}

	return 0;
}

// Received wisp message from map-server for ALL gm (just copy the message and resends it to ALL map-servers)
int mapif_parse_WisToGM(int fd)
{
	unsigned char buf[2048]; // 0x3003/0x3803 <packet_len>.w <wispname>.24B <min_gm_level>.w <message>.?B

	memcpy(WBUFP(buf,0), RFIFOP(fd,0), RFIFOW(fd,2)); // Message contains the NUL terminator (see intif_wis_message_to_gm())
	WBUFW(buf, 0) = 0x3803;
	mapif->sendall(buf, RFIFOW(fd,2));

	return 0;
}

// Save account_reg into sql (type=2)
int mapif_parse_Registry(int fd)
{
	int account_id = RFIFOL(fd, 4), char_id = RFIFOL(fd, 8), count = RFIFOW(fd, 12);

	if( count ) {
		int cursor = 14, i;
		char key[SCRIPT_VARNAME_LENGTH+1], sval[254];
		bool isLoginActive = sockt->session_is_active(chr->login_fd);

		if( isLoginActive )
			chr->global_accreg_to_login_start(account_id,char_id);

		for(i = 0; i < count; i++) {
			unsigned int index;
			int len = RFIFOB(fd, cursor);
			safestrncpy(key, RFIFOP(fd, cursor + 1), min((int)sizeof(key), len));
			cursor += len + 1;

			index = RFIFOL(fd, cursor);
			cursor += 4;

			switch (RFIFOB(fd, cursor++)) {
				/* int */
				case 0:
					inter->savereg(account_id,char_id,key,index,RFIFOL(fd, cursor),false);
					cursor += 4;
					break;
				case 1:
					inter->savereg(account_id,char_id,key,index,0,false);
					break;
				/* str */
				case 2:
					len = RFIFOB(fd, cursor);
					safestrncpy(sval, RFIFOP(fd, cursor + 1), min((int)sizeof(sval), len));
					cursor += len + 1;
					inter->savereg(account_id,char_id,key,index,(intptr_t)sval,true);
					break;
				case 3:
					inter->savereg(account_id,char_id,key,index,0,true);
					break;
				default:
					ShowError("mapif->parse_Registry: unknown type %d\n",RFIFOB(fd, cursor - 1));
					return 1;
			}

		}

		if (isLoginActive)
			chr->global_accreg_to_login_send();
	}
	return 0;
}

// Request the value of all registries.
int mapif_parse_RegistryRequest(int fd)
{
	//Load Char Registry
	if (RFIFOB(fd,12)) mapif->account_reg_reply(fd,RFIFOL(fd,2),RFIFOL(fd,6),3); // 3: char reg
	//Load Account Registry
	if (RFIFOB(fd,11)) mapif->account_reg_reply(fd,RFIFOL(fd,2),RFIFOL(fd,6),2); // 2: account reg
	//Ask Login Server for Account2 values.
	if (RFIFOB(fd,10)) chr->request_accreg2(RFIFOL(fd,2),RFIFOL(fd,6));
	return 1;
}

void mapif_namechange_ack(int fd, int account_id, int char_id, int type, int flag, const char *const name)
{
	nullpo_retv(name);
	WFIFOHEAD(fd, NAME_LENGTH+13);
	WFIFOW(fd, 0) = 0x3806;
	WFIFOL(fd, 2) = account_id;
	WFIFOL(fd, 6) = char_id;
	WFIFOB(fd,10) = type;
	WFIFOB(fd,11) = flag;
	memcpy(WFIFOP(fd, 12), name, NAME_LENGTH);
	WFIFOSET(fd, NAME_LENGTH+13);
}

int mapif_parse_NameChangeRequest(int fd)
{
	int account_id, char_id, type;
	const char *name;
	int i;

	account_id = RFIFOL(fd,2);
	char_id = RFIFOL(fd,6);
	type = RFIFOB(fd,10);
	name = RFIFOP(fd,11);

	// Check Authorized letters/symbols in the name
	if (char_name_option == 1) { // only letters/symbols in char_name_letters are authorized
		for (i = 0; i < NAME_LENGTH && name[i]; i++)
		if (strchr(char_name_letters, name[i]) == NULL) {
			mapif->namechange_ack(fd, account_id, char_id, type, 0, name);
			return 0;
		}
	} else if (char_name_option == 2) { // letters/symbols in char_name_letters are forbidden
		for (i = 0; i < NAME_LENGTH && name[i]; i++)
		if (strchr(char_name_letters, name[i]) != NULL) {
			mapif->namechange_ack(fd, account_id, char_id, type, 0, name);
			return 0;
		}
	}
	//TODO: type holds the type of object to rename.
	//If it were a player, it needs to have the guild information and db information
	//updated here, because changing it on the map won't make it be saved [Skotlex]

	//name allowed.
	mapif->namechange_ack(fd, account_id, char_id, type, 1, name);
	return 0;
}

//--------------------------------------------------------

/// Returns the length of the next complete packet to process,
/// or 0 if no complete packet exists in the queue.
///
/// @param length The minimum allowed length, or -1 for dynamic lookup
int inter_check_length(int fd, int length)
{
	if( length == -1 )
	{// variable-length packet
		if( RFIFOREST(fd) < 4 )
			return 0;
		length = RFIFOW(fd,2);
	}

	if( (int)RFIFOREST(fd) < length )
		return 0;

	return length;
}

int inter_parse_frommap(int fd)
{
	int cmd;
	int len = 0;
	cmd = RFIFOW(fd,0);
	// Check is valid packet entry
	if(cmd < 0x3000 || cmd >= 0x3000 + ARRAYLENGTH(inter_recv_packet_length) || inter_recv_packet_length[cmd - 0x3000] == 0)
		return 0;

	// Check packet length
	if((len = inter->check_length(fd, inter_recv_packet_length[cmd - 0x3000])) == 0)
		return 2;

	switch(cmd) {
	case 0x3000: mapif->parse_broadcast(fd); break;
	case 0x3001: mapif->parse_WisRequest(fd); break;
	case 0x3002: mapif->parse_WisReply(fd); break;
	case 0x3003: mapif->parse_WisToGM(fd); break;
	case 0x3004: mapif->parse_Registry(fd); break;
	case 0x3005: mapif->parse_RegistryRequest(fd); break;
	case 0x3006: mapif->parse_NameChangeRequest(fd); break;
	case 0x3007: mapif->parse_accinfo(fd); break;
	/* 0x3008 is used by the report stuff */
	default:
		if(  inter_party->parse_frommap(fd)
		  || inter_guild->parse_frommap(fd)
		  || inter_storage->parse_frommap(fd)
		  || inter_pet->parse_frommap(fd)
		  || inter_homunculus->parse_frommap(fd)
		  || inter_mercenary->parse_frommap(fd)
		  || inter_elemental->parse_frommap(fd)
		  || inter_mail->parse_frommap(fd)
		  || inter_auction->parse_frommap(fd)
		  || inter_quest->parse_frommap(fd)
		   )
			break;
		else
			return 0;
	}

	RFIFOSKIP(fd, len);
	return 1;
}

void inter_defaults(void)
{
	inter = &inter_s;

	inter->sql_handle = NULL;

	inter->msg_txt = inter_msg_txt;
	inter->msg_config_read = inter_msg_config_read;
	inter->do_final_msg = inter_do_final_msg;
	inter->job_name = inter_job_name;
	inter->vmsg_to_fd = inter_vmsg_to_fd;
	inter->msg_to_fd = inter_msg_to_fd;
	inter->savereg = inter_savereg;
	inter->accreg_fromsql = inter_accreg_fromsql;
	inter->config_read = inter_config_read;
	inter->vlog = inter_vlog;
	inter->log = inter_log;
	inter->init_sql = inter_init_sql;
	inter->mapif_init = inter_mapif_init;
	inter->check_ttl_wisdata_sub = inter_check_ttl_wisdata_sub;
	inter->check_ttl_wisdata = inter_check_ttl_wisdata;
	inter->check_length = inter_check_length;
	inter->parse_frommap = inter_parse_frommap;
	inter->final = inter_final;
}
