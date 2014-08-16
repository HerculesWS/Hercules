// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef CHAR_INT_ELEMENTAL_H
#define CHAR_INT_ELEMENTAL_H

#include "../common/cbasetypes.h"

void inter_elemental_sql_init(void);
void inter_elemental_sql_final(void);
int inter_elemental_parse_frommap(int fd);

bool mapif_elemental_delete(int ele_id);

#endif /* CHAR_INT_ELEMENTAL_H */
