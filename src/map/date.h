// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _MAP_DATE_H_
#define _MAP_DATE_H_

#include "../common/cbasetypes.h"

int date_get_year(void);
int date_get_month(void);
int date_get_day(void);
int date_get_hour(void);
int date_get_min(void);
int date_get_sec(void);

bool is_day_of_sun(void);
bool is_day_of_moon(void);
bool is_day_of_star(void);

#endif /* _MAP_DATE_H_ */
