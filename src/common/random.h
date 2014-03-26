// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#ifndef _COMMON_RANDOM_H_
#define _COMMON_RANDOM_H_

#include "../common/cbasetypes.h"

void rnd_init(void);
void rnd_seed(uint32);

int32 rnd(void);// [0, SINT32_MAX]
uint32 rnd_roll(uint32 dice_faces);// [0, dice_faces)
int32 rnd_value(int32 min, int32 max);// [min, max]
double rnd_uniform(void);// [0.0, 1.0)
double rnd_uniform53(void);// [0.0, 1.0)

#endif /* _COMMON_RANDOM_H_ */
