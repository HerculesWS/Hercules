/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2012-2018  Hercules Dev Team
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

#include "pincode.h"

#include "char/char.h"
#include "common/cbasetypes.h"
#include "common/conf.h"
#include "common/db.h"
#include "common/memmgr.h"
#include "common/mmo.h"
#include "common/nullpo.h"
#include "common/random.h"
#include "common/showmsg.h"
#include "common/socket.h"
#include "common/strlib.h"

#include <stdio.h>
#include <stdlib.h>

static struct pincode_interface pincode_s;
struct pincode_interface *pincode;

static void pincode_handle(int fd, struct char_session_data *sd)
{
	struct online_char_data* character;

	nullpo_retv(sd);

	character = (struct online_char_data*)idb_get(chr->online_char_db, sd->account_id);

	if (character && character->pincode_enable > pincode->charselect) {
		character->pincode_enable = pincode->charselect * 2;
	} else {
		pincode->loginstate(fd, sd, PINCODE_LOGIN_OK);
		return;
	}

	if (strlen(sd->pincode) == 4) {
		if (pincode->check_blacklist && pincode->isBlacklisted(sd->pincode)) {
			// Ask player to change pincode to be able to connect
			pincode->loginstate(fd, sd, PINCODE_LOGIN_EXPIRED);
		} else if (pincode->changetime && time(NULL) > (sd->pincode_change + pincode->changetime)) {
			// User hasn't changed his PIN code for a long time
			pincode->loginstate(fd, sd, PINCODE_LOGIN_EXPIRED);
		} else { // Ask user for his PIN code
			pincode->loginstate(fd, sd, PINCODE_LOGIN_ASK);
		}
	} else // No PIN code has been set yet
		pincode->loginstate(fd, sd, PINCODE_LOGIN_NOTSET);

	if (character)
		character->pincode_enable = -1;
}

static void pincode_check(int fd, struct char_session_data *sd)
{
	char pin[5] = "\0\0\0\0";

	nullpo_retv(sd);

	if (strlen(sd->pincode) != 4)
		return;

	safestrncpy(pin, RFIFOP(fd, 6), sizeof(pin));
	pincode->decrypt(sd->pincode_seed, pin);

	if (pincode->check_blacklist && pincode->isBlacklisted(pin)) {
		pincode->loginstate(fd, sd, PINCODE_LOGIN_RESTRICT_PW);
		return;
	}

	if (pincode->compare(fd, sd, pin)) {
		struct online_char_data* character;
		if ((character = (struct online_char_data*)idb_get(chr->online_char_db, sd->account_id)))
			character->pincode_enable = pincode->charselect * 2;
		pincode->loginstate(fd, sd, PINCODE_LOGIN_OK);
	} else {
#if PACKETVER_MAIN_NUM >= 20180124 || PACKETVER_RE_NUM >= 20180124 || PACKETVER_ZERO_NUM >= 20180131
		pincode->loginstate2(fd, sd, PINCODE_LOGIN_WRONG, PINCODE_LOGIN_FLAG_WRONG);
#else
		pincode->loginstate(fd, sd, PINCODE_LOGIN_WRONG);
#endif
	}
}

/**
 * Check if this pincode is blacklisted or not
 *
 * @param (const char *) pin The pin to be verified
 * @return bool
 */
static bool pincode_isBlacklisted(const char *pin)
{
	int i;

	nullpo_retr(false, pin);

	ARR_FIND(0, VECTOR_LENGTH(pincode->blacklist), i, strcmp(VECTOR_INDEX(pincode->blacklist, i), pin) == 0);

	if (i < VECTOR_LENGTH(pincode->blacklist)) {
		return true;
	}

	return false;
}

static int pincode_compare(int fd, struct char_session_data *sd, char *pin)
{
	nullpo_ret(sd);
	nullpo_ret(pin);

	if (strcmp(sd->pincode, pin) == 0) {
		sd->pincode_try = 0;
		return 1;
	} else {
		if (pincode->maxtry && ++sd->pincode_try >= pincode->maxtry) {
			pincode->error(sd->account_id);
			chr->authfail_fd(fd, 0);
			chr->disconnect_player(sd->account_id);
		}
		return 0;
	}
}

static void pincode_change(int fd, struct char_session_data *sd)
{
	char oldpin[5] = "\0\0\0\0", newpin[5] = "\0\0\0\0";

	nullpo_retv(sd);

	if (strlen(sd->pincode) != 4)
		return;

	safestrncpy(oldpin, RFIFOP(fd, 6), sizeof(oldpin));
	pincode->decrypt(sd->pincode_seed, oldpin);

	if (!pincode->compare(fd, sd, oldpin)) {
		pincode->editstate(fd, sd, PINCODE_EDIT_FAILED);
		pincode->loginstate(fd, sd, PINCODE_LOGIN_ASK);
		return;
	}

	safestrncpy(newpin, RFIFOP(fd, 10), sizeof(newpin));
	pincode->decrypt(sd->pincode_seed, newpin);

	if (pincode->check_blacklist && pincode->isBlacklisted(newpin)) {
		pincode->editstate(fd, sd, PINCODE_EDIT_RESTRICT_PW);
		return;
	}

	pincode->update(sd->account_id, newpin);
	safestrncpy(sd->pincode, newpin, sizeof(sd->pincode));
	pincode->editstate(fd, sd, PINCODE_EDIT_SUCCESS);
	pincode->loginstate(fd, sd, PINCODE_LOGIN_ASK);
}

static void pincode_setnew(int fd, struct char_session_data *sd)
{
	char newpin[5] = "\0\0\0\0";

	nullpo_retv(sd);

	if (strlen(sd->pincode) == 4)
		return;

	safestrncpy(newpin, RFIFOP(fd, 6), sizeof(newpin));
	pincode->decrypt(sd->pincode_seed, newpin);

	if (pincode->check_blacklist && pincode->isBlacklisted(newpin)) {
		pincode->makestate(fd, sd, PINCODE_MAKE_RESTRICT_PW);
		return;
	}

	pincode->update(sd->account_id, newpin);
	safestrncpy(sd->pincode, newpin, sizeof(sd->pincode));
	pincode->makestate(fd, sd, PINCODE_MAKE_SUCCESS);
	pincode->loginstate(fd, sd, PINCODE_LOGIN_ASK);
}

/**
 * Send state of making new pincode
 *
 * @param[in] fd
 * @param[in, out] sd Session Data
 * @param[in] state Pincode Edit State
 */
static void pincode_makestate(int fd, struct char_session_data *sd, enum pincode_make_response state)
{
	nullpo_retv(sd);

	WFIFOHEAD(fd, 8);
	WFIFOW(fd, 0) = 0x8bb;
	WFIFOW(fd, 2) = state;
	WFIFOL(fd, 4) = sd->pincode_seed;
	WFIFOSET(fd, 8);
}

/**
 * Send state of editing pincode
 *
 * @param[in] fd
 * @param[in, out] sd Session Data
 * @param[in] state Pincode Edit State
 */
static void pincode_editstate(int fd, struct char_session_data *sd, enum pincode_edit_response state)
{
	nullpo_retv(sd);

	WFIFOHEAD(fd, 8);
	WFIFOW(fd, 0) = 0x8bf;
	WFIFOW(fd, 2) = state;
	WFIFOL(fd, 4) = sd->pincode_seed = rnd() % 0xFFFF;
	WFIFOSET(fd, 8);
}

// 0 = pin is correct
// 1 = ask for pin - client sends 0x8b8
// 2 = create new pin - client sends 0x8ba
// 3 = pin must be changed - client 0x8be
// 4 = create new pin ?? - client sends 0x8ba
// 5 = client shows msgstr(1896)
// 6 = client shows msgstr(1897) Unable to use your KSSN number
// 7 = char select window shows a button - client sends 0x8c5
// 8 = pincode was incorrect
static void pincode_loginstate(int fd, struct char_session_data *sd, enum pincode_login_response state)
{
	nullpo_retv(sd);

	WFIFOHEAD(fd, 12);
	WFIFOW(fd, 0) = 0x8b9;
	WFIFOL(fd, 2) = sd->pincode_seed = rnd() % 0xFFFF;
	WFIFOL(fd, 6) = sd->account_id;
	WFIFOW(fd, 10) = state;
	WFIFOSET(fd, 12);
}

// 0 = pin is correct
// 1 = ask for pin - client sends 0x8b8
// 2 = create new pin - client sends 0x8ba
// 3 = pin must be changed - client 0x8be
// 4 = create new pin ?? - client sends 0x8ba
// 5 = client shows msgstr(1896)
// 6 = client shows msgstr(1897) Unable to use your KSSN number
// 7 = char select window shows a button - client sends 0x8c5
// 8 = pincode was incorrect
// [4144] pincode_loginstate2 can replace pincode_loginstate,
// but kro using pincode_loginstate2 only for send wrong pin error or locked after 3 pins wrong
static void pincode_loginstate2(int fd, struct char_session_data *sd, enum pincode_login_response state, enum pincode_login_response2 flag)
{
#if PACKETVER_MAIN_NUM >= 20180124 || PACKETVER_RE_NUM >= 20180124 || PACKETVER_ZERO_NUM >= 20180131
	nullpo_retv(sd);

	WFIFOHEAD(fd, 13);
	WFIFOW(fd, 0) = 0xae9;
	WFIFOL(fd, 2) = sd->pincode_seed = rnd() % 0xFFFF;
	WFIFOL(fd, 6) = sd->account_id;
	WFIFOW(fd, 10) = state;
	WFIFOW(fd, 12) = flag;
	WFIFOSET(fd, 13);
#endif
}

static void pincode_notifyLoginPinUpdate(int account_id, char *pin)
{
	nullpo_retv(pin);

	Assert_retv(chr->login_fd != -1);
	WFIFOHEAD(chr->login_fd, 11);
	WFIFOW(chr->login_fd, 0) = 0x2738;
	WFIFOL(chr->login_fd, 2) = account_id;
	safestrncpy(WFIFOP(chr->login_fd, 6), pin, 5);
	WFIFOSET(chr->login_fd, 11);
}

static void pincode_notifyLoginPinError(int account_id)
{
	WFIFOHEAD(chr->login_fd, 6);
	WFIFOW(chr->login_fd, 0) = 0x2739;
	WFIFOL(chr->login_fd, 2) = account_id;
	WFIFOSET(chr->login_fd, 6);
}

static void pincode_decrypt(unsigned int userSeed, char *pin)
{
	int i;
	char tab[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

	nullpo_retv(pin);

	for (i = 1; i < 10; i++) {
		int pos;
		userSeed = pincode->baseSeed + userSeed * pincode->multiplier;
		pos = userSeed % (i + 1);
		if (i != pos) {
			tab[i] ^= tab[pos];
			tab[pos] ^= tab[i];
			tab[i] ^= tab[pos];
		}
	}

	for (i = 0; i < 4; i++) {
		if (pin[i] < '0' || pin[i] > '9')
			pin[i] = '0';
		else
			pin[i] = tab[pin[i] - '0'];
	}

	sprintf(pin, "%d%d%d%d", pin[0], pin[1], pin[2], pin[3]);
}

/**
 * Reads the 'char_configuration/pincode' config entry and initializes required variables.
 *
 * @param filename Path to configuration file (used in error and warning messages).
 * @param config   The current config being parsed.
 * @param imported Whether the current config is imported from another file.
 *
 * @retval false in case of error.
 */
static bool pincode_config_read(const char *filename, const struct config_t *config, bool imported)
{
	const struct config_setting_t *setting = NULL;
	const struct config_setting_t *temp = NULL;

	nullpo_retr(false, filename);
	nullpo_retr(false, config);

	if ((setting = libconfig->lookup(config, "char_configuration/pincode")) == NULL) {
		if (imported)
			return true;
		ShowError("char_config_read: char_configuration/pincode was not found in %s!\n", filename);
		return false;
	}

	if (libconfig->setting_lookup_bool(setting, "enabled", &pincode->enabled) == CONFIG_TRUE) {
#if PACKETVER < 20110309
		if (pincode->enabled) {
			ShowWarning("pincode_enabled requires PACKETVER 20110309 or higher. disabling...\n");
			pincode->enabled = 0;
		}
#endif
	}

	if (libconfig->setting_lookup_int(setting, "change_time", &pincode->changetime) == CONFIG_TRUE)
		pincode->changetime *= 60;

	if (libconfig->setting_lookup_int(setting, "max_tries", &pincode->maxtry) == CONFIG_TRUE) {
		if (pincode->maxtry > 3) {
			ShowWarning("pincode_maxtry is too high (%d); Maximum allowed: 3! Capping to 3...\n",pincode->maxtry);
			pincode->maxtry = 3;
		}
	}

	if (libconfig->setting_lookup_int(setting, "request", &pincode->charselect) == CONFIG_TRUE) {
		if (pincode->charselect != 1 && pincode->charselect != 0) {
			ShowWarning("Invalid pincode/request! Defaulting to 0\n");
			pincode->charselect = 0;
		}
	}

	if (libconfig->setting_lookup_bool_real(setting, "check_blacklisted", &pincode->check_blacklist) == CONFIG_FALSE) {
		if (!imported) {
			ShowWarning("pincode 'check_blaclisted' not found, defaulting to false...\n");
			pincode->check_blacklist = false;
		}
	}

	if (pincode->check_blacklist) {
		if ((temp = libconfig->setting_get_member(setting, "blacklist")) != NULL) {
			VECTOR_DECL(char *) duplicate;
			int i, j, size = libconfig->setting_length(temp);
			VECTOR_INIT(duplicate);
			VECTOR_ENSURE(duplicate, size, 1);
			for (i = 0; i < size; i++) {
				const char *pin = libconfig->setting_get_string_elem(temp, i);

				if (pin == NULL)
					continue;

				if (strlen(pin) != 4) {
					ShowError("Wrong size on element %d of blacklist. Desired size = 4, received = %d\n", i, (int)strlen(pin));
					continue;
				}

				ARR_FIND(0, VECTOR_LENGTH(duplicate), j, strcmp(VECTOR_INDEX(duplicate, j), pin) == 0);

				if (j < VECTOR_LENGTH(duplicate)) {
					ShowWarning("Duplicate pin on pincode blacklist. Item #%d\n", i);
					continue;
				}

				VECTOR_ENSURE(pincode->blacklist, 1, 1);
				VECTOR_PUSH(pincode->blacklist, aStrdup(pin));
				VECTOR_PUSH(duplicate, aStrdup(pin));
			}
			while (VECTOR_LENGTH(duplicate) > 0) {
				aFree(VECTOR_POP(duplicate));
			}
			VECTOR_CLEAR(duplicate);
		} else if (!imported) {
			ShowError("Pincode Blacklist Check is enabled but there's no blacklist setting! Disabling check.\n");
			pincode->check_blacklist = false;
		}
	}

	return true;
}

static void do_pincode_init(void)
{
	VECTOR_INIT(pincode->blacklist);
}

static void do_pincode_final(void)
{
	while (VECTOR_LENGTH(pincode->blacklist) > 0) {
		aFree(VECTOR_POP(pincode->blacklist));
	}
	VECTOR_CLEAR(pincode->blacklist);
}

void pincode_defaults(void)
{
	pincode = &pincode_s;

	pincode->enabled = 0;
	pincode->changetime = 0;
	pincode->maxtry = 3;
	pincode->charselect = 0;
	pincode->check_blacklist = false;
	pincode->multiplier = 0x3498;
	pincode->baseSeed = 0x881234;

	pincode->init = do_pincode_init;
	pincode->final = do_pincode_final;

	pincode->handle = pincode_handle;
	pincode->decrypt = pincode_decrypt;
	pincode->error = pincode_notifyLoginPinError;
	pincode->update = pincode_notifyLoginPinUpdate;
	pincode->makestate = pincode_makestate;
	pincode->editstate = pincode_editstate;
	pincode->loginstate = pincode_loginstate;
	pincode->loginstate2 = pincode_loginstate2;
	pincode->setnew = pincode_setnew;
	pincode->change = pincode_change;
	pincode->isBlacklisted = pincode_isBlacklisted;
	pincode->compare = pincode_compare;
	pincode->check = pincode_check;
	pincode->config_read = pincode_config_read;
}
