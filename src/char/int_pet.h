// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef CHAR_INT_PET_H
#define CHAR_INT_PET_H

struct s_pet;

#ifdef HERCULES_CORE
void inter_pet_defaults(void);
#endif // HERCULES_CORE

/**
 * inter_pet interface
 **/
struct inter_pet_interface {
	struct s_pet *pt;
	int (*tosql) (int pet_id, struct s_pet* p);
	int (*fromsql) (int pet_id, struct s_pet* p);
	int (*sql_init) (void);
	void (*sql_final) (void);
	int (*delete_) (int pet_id);
	int (*parse_frommap) (int fd);
};

struct inter_pet_interface *inter_pet;

#endif /* CHAR_INT_PET_H */
