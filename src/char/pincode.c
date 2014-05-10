// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file
// Portions Copyright (c) Athena Dev Teams

#define HERCULES_CORE

#include "pincode.h"

#include <stdlib.h>

#include "char.h"
#include "../common/cbasetypes.h"
#include "../common/mmo.h"
#include "../common/random.h"
#include "../common/showmsg.h"
#include "../common/socket.h"
#include "../common/strlib.h"

int enabled = PINCODE_OK;
int changetime = 0;
int maxtry = 3;
int charselect = 0;
unsigned int multiplier = 0x3498, baseSeed = 0x881234;

void pincode_handle ( int fd, struct char_session_data* sd ) {
	struct online_char_data* character = (struct online_char_data*)idb_get(online_char_db, sd->account_id);
	
	if( character && character->pincode_enable > *pincode->charselect ){
		character->pincode_enable = *pincode->charselect * 2;
	}else{
		pincode->sendstate( fd, sd, PINCODE_OK );
		return;
	}
	
	if( strlen(sd->pincode) == 4 ){
		if( *pincode->changetime && time(NULL) > (sd->pincode_change+*pincode->changetime) ){ // User hasnt changed his PIN code for a long time
			pincode->sendstate( fd, sd, PINCODE_EXPIRED );
		} else { // Ask user for his PIN code
			pincode->sendstate( fd, sd, PINCODE_ASK );
		}
	} else // No PIN code has been set yet
		pincode->sendstate( fd, sd, PINCODE_NOTSET );

	if( character )
		character->pincode_enable = -1;
}

void pincode_check(int fd, struct char_session_data* sd) {
	char pin[5] = "\0\0\0\0";
	
	strncpy(pin, (char*)RFIFOP(fd, 6), 4+1);
	pincode->decrypt(sd->pincode_seed, pin);
	if( pincode->compare( fd, sd, pin ) ){
		struct online_char_data* character;
		if( (character = (struct online_char_data*)idb_get(online_char_db, sd->account_id)) )
			character->pincode_enable = *pincode->charselect * 2;
		pincode->sendstate( fd, sd, PINCODE_OK );
	}
}

int pincode_compare(int fd, struct char_session_data* sd, char* pin) {
	if( strcmp( sd->pincode, pin ) == 0 ){
		sd->pincode_try = 0;
		return 1;
	} else {
		pincode->sendstate( fd, sd, PINCODE_WRONG );
		if( *pincode->maxtry && ++sd->pincode_try >= *pincode->maxtry ){
			pincode->error( sd->account_id );
		}
		return 0;
	}
}

void pincode_change(int fd, struct char_session_data* sd) {
	char oldpin[5] = "\0\0\0\0", newpin[5] = "\0\0\0\0";

	strncpy(oldpin, (char*)RFIFOP(fd,6), sizeof(oldpin));
	pincode->decrypt(sd->pincode_seed,oldpin);
	if( !pincode->compare( fd, sd, oldpin ) )
		return;
	
	strncpy(newpin, (char*)RFIFOP(fd,10), sizeof(newpin));
	pincode->decrypt(sd->pincode_seed,newpin);
	pincode->update( sd->account_id, newpin );
	strncpy(sd->pincode, newpin, sizeof(sd->pincode));
	pincode->sendstate( fd, sd, PINCODE_ASK );
}

void pincode_setnew(int fd, struct char_session_data* sd) {
	char newpin[5] = "\0\0\0\0";

	strncpy(newpin, (char*)RFIFOP(fd,6), sizeof(newpin));
	pincode->decrypt(sd->pincode_seed,newpin);
	pincode->update( sd->account_id, newpin );
	strncpy(sd->pincode, newpin, sizeof(sd->pincode));
	pincode->sendstate( fd, sd, PINCODE_ASK );
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
void pincode_sendstate(int fd, struct char_session_data* sd, uint16 state) {
	WFIFOHEAD(fd, 12);
	WFIFOW(fd, 0) = 0x8b9;
	WFIFOL(fd, 2) = sd->pincode_seed = rand() % 0xFFFF;
	WFIFOL(fd, 6) = sd->account_id;
	WFIFOW(fd,10) = state;
	WFIFOSET(fd,12);
}

void pincode_notifyLoginPinUpdate(int account_id, char* pin) {
	WFIFOHEAD(login_fd,11);
	WFIFOW(login_fd,0) = 0x2738;
	WFIFOL(login_fd,2) = account_id;
	strncpy( (char*)WFIFOP(login_fd,6), pin, 5 );
	WFIFOSET(login_fd,11);
}

void pincode_notifyLoginPinError(int account_id) {
	WFIFOHEAD(login_fd,6);
	WFIFOW(login_fd,0) = 0x2739;
	WFIFOL(login_fd,2) = account_id;
	WFIFOSET(login_fd,6);
}

void pincode_decrypt(unsigned int userSeed, char* pin) {
	int i, pos;
	char tab[10] = {0,1,2,3,4,5,6,7,8,9};
	
	for( i = 1; i < 10; i++ ){
		userSeed = *pincode->baseSeed + userSeed * *pincode->multiplier;
		pos = userSeed % (i + 1);
		if( i != pos ){
			tab[i] ^= tab[pos];
			tab[pos] ^= tab[i];
			tab[i] ^= tab[pos];
		}
	}
	
	for( i = 0; i < 4; i++ ){
		pin[i] = tab[pin[i] - '0'];
	}
	
	sprintf(pin, "%d%d%d%d", pin[0], pin[1], pin[2], pin[3]);
}

bool pincode_config_read(char *w1, char *w2) {
	
	while ( true ) {
		
		if ( strcmpi(w1, "pincode_enabled") == 0 ) {
			enabled = atoi(w2);
#if PACKETVER < 20110309
			if( enabled ) {
				ShowWarning("pincode_enabled requires PACKETVER 20110309 or higher. disabling...\n");
				enabled = 0;
			}
#endif
		} else if ( strcmpi(w1, "pincode_changetime") == 0 ) {
			changetime = atoi(w2)*60;
		} else if ( strcmpi(w1, "pincode_maxtry") == 0 ) {
			maxtry = atoi(w2);
			if( maxtry > 3 ) {
				ShowWarning("pincode_maxtry is too high (%d); maximum allowed: 3! capping to 3...\n",maxtry);
				maxtry = 3;
			}
		} else if ( strcmpi(w1, "pincode_charselect") == 0 ) {
			charselect = atoi(w2);
		} else
			return false;
		
		break;
	}
	
	return true;
}

void pincode_defaults(void) {
	pincode = &pincode_s;
	
	pincode->enabled = &enabled;
	pincode->changetime = &changetime;
	pincode->maxtry = &maxtry;
	pincode->charselect = &charselect;
	pincode->multiplier = &multiplier;
	pincode->baseSeed = &baseSeed;
	
	pincode->handle = pincode_handle;
	pincode->decrypt = pincode_decrypt;
	pincode->error = pincode_notifyLoginPinError;
	pincode->update = pincode_notifyLoginPinUpdate;
	pincode->sendstate = pincode_sendstate;
	pincode->setnew = pincode_setnew;
	pincode->change = pincode_change;
	pincode->compare = pincode_compare;
	pincode->check = pincode_check;
	pincode->config_read = pincode_config_read;

}
