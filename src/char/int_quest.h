// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef CHAR_INT_QUEST_H
#define CHAR_INT_QUEST_H

#include "common/hercules.h"

/**
 * inter_quest interface
 **/
struct inter_quest_interface {
	int (*parse_frommap) (int fd);
};

#ifdef HERCULES_CORE
void inter_quest_defaults(void);
#endif // HERCULES_CORE

HPShared struct inter_quest_interface *inter_quest;

#endif /* CHAR_INT_QUEST_H */

