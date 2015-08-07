// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef CHAR_QUEST_H
#define CHAR_QUEST_H

#ifdef HERCULES_CORE
void inter_quest_defaults(void);
#endif // HERCULES_CORE

/**
 * inter_quest interface
 **/
struct inter_quest_interface {
	int (*parse_frommap) (int fd);
};

struct inter_quest_interface *inter_quest;

#endif /* CHAR_QUEST_H */

