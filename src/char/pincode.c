// Copyright (c) Hercules Dev Team, licensed under GNU GPL.
// See the LICENSE file

#include "../common/cbasetypes.h"
#include "../common/mmo.h"
#include "../common/random.h"
#include "../common/showmsg.h"
#include "../common/socket.h"
#include "../common/strlib.h"
#include "char.h"
#include "pincode.h"

#include <stdlib.h>

int enabled = PINCODE_OK;
int changetime = 0;
int maxtry = 3;
unsigned long multiplier = 0x3498, baseSeed = 0x881234;

void pincode_handle ( int fd, struct char_session_data* sd ) {
	if( pincode->enabled ){
		// PIN code system enabled
		if( sd->pincode[0] == '\0' ){
			// No PIN code has been set yet
			pincode->state( fd, sd, PINCODE_NOTSET );
		} else {
			if( !pincode->changetime || !( time(NULL) > (sd->pincode_change+*pincode->changetime) ) ){
				// Ask user for his PIN code
				pincode->state( fd, sd, PINCODE_ASK );
			} else {
				// User hasnt changed his PIN code too long
				pincode->state( fd, sd, PINCODE_EXPIRED );
			}
		}
	} else {
		// PIN code system, disabled
		pincode->state( fd, sd, PINCODE_OK );
	}
}

void pincode_check(int fd, struct char_session_data* sd) {
	char pin[5];
	
	safestrncpy((char*)pin, (char*)RFIFOP(fd, 6), 4+1);
	
	pincode->decrypt(sd->pincode_seed, pin);
	
	if( pincode->compare( fd, sd, pin ) ){
		pincode->state( fd, sd, PINCODE_OK );
	}
}

int pincode_compare(int fd, struct char_session_data* sd, char* pin) {
	if( strcmp( sd->pincode, pin ) == 0 ){
		sd->pincode_try = 0;
		return 1;
	} else {
		pincode->state( fd, sd, PINCODE_WRONG );
		
		if( pincode->maxtry && ++sd->pincode_try >= *pincode->maxtry ){
			pincode->error( sd->account_id );
		}
		
		return 0;
	}
}

void pincode_change(int fd, struct char_session_data* sd) {
	char oldpin[5], newpin[5];
	
	safestrncpy(oldpin, (char*)RFIFOP(fd,6), 4+1);
	pincode->decrypt(sd->pincode_seed,oldpin);
	if( !pincode->compare( fd, sd, oldpin ) )
		return;
	
	safestrncpy(newpin, (char*)RFIFOP(fd,10), 4+1);
	pincode->decrypt(sd->pincode_seed,newpin);
	pincode->update( sd->account_id, newpin );
	
	pincode->state( fd, sd, PINCODE_OK );
}

void pincode_setnew(int fd, struct char_session_data* sd) {
	char newpin[5];
	
	safestrncpy(newpin, (char*)RFIFOP(fd,6), 4+1);
	pincode->decrypt(sd->pincode_seed,newpin);
	
	pincode->update( sd->account_id, newpin );
	
	pincode->state( fd, sd, PINCODE_OK );
}

// 0 = disabled / pin is correct
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
	WFIFOL(fd, 2) = sd->pincode_seed = rnd() % 0xFFFF;
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

void pincode_decrypt(unsigned long userSeed, char* pin) {
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
		pin[i] = tab[pin[i]- '0'];
	}
	
	sprintf(pin, "%d%d%d%d", pin[0], pin[1], pin[2], pin[3]);
}

bool pincode_config_read(char *w1, char *w2) {
	
	while ( true ) {
		
		if ( strcmpi(w1, "pincode_enabled") == 0 ) {
			enabled = atoi(w2);
		} else if ( strcmpi(w1, "pincode_changetime") == 0 ) {
			changetime = atoi(w2)*60;
		} else if ( strcmpi(w1, "pincode_maxtry") == 0 ) {
			maxtry = atoi(w2);
			if( maxtry > 3 ) {
				ShowWarning("pincode_maxtry is too high (%d); maximum allowed: 3! capping to 3...\n",maxtry);
				maxtry = 3;
			}
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
	pincode->multiplier = &multiplier;
	pincode->baseSeed = &baseSeed;
	
	pincode->handle = pincode_handle;
	pincode->decrypt = pincode_decrypt;
	pincode->error = pincode_notifyLoginPinError;
	pincode->update = pincode_notifyLoginPinUpdate;
	pincode->state = pincode_sendstate;
	pincode->new = pincode_setnew;
	pincode->change = pincode_change;
	pincode->compare = pincode_compare;
	pincode->check = pincode_check;
	pincode->config_read = pincode_config_read;

}