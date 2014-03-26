// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _CHAR_INT_PET_H_
#define _CHAR_INT_PET_H_

struct s_pet;

int inter_pet_init(void);
void inter_pet_sql_final(void);
int inter_pet_save(void);
int inter_pet_delete(int pet_id);

int inter_pet_parse_frommap(int fd);
int inter_pet_sql_init(void);
//extern char pet_txt[256];

//Exported for use in the TXT-SQL converter.
int inter_pet_tosql(int pet_id, struct s_pet *p);

#endif /* _CHAR_INT_PET_H_ */
