// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _CHAR_INT_MERCENARY_H_
#define _CHAR_INT_MERCENARY_H_

struct s_mercenary;

int inter_mercenary_sql_init(void);
void inter_mercenary_sql_final(void);
int inter_mercenary_parse_frommap(int fd);

// Mercenary Owner Database
bool mercenary_owner_fromsql(int char_id, struct mmo_charstatus *status);
bool mercenary_owner_tosql(int char_id, struct mmo_charstatus *status);
bool mercenary_owner_delete(int char_id);

bool mapif_mercenary_delete(int merc_id);

#endif /* _CHAR_INT_MERCENARY_H_ */
