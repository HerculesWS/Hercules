// Copyright (c) Athena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder
#ifndef COMMON_DES_H
#define COMMON_DES_H

#include "../common/cbasetypes.h"

/// One 64-bit block.
typedef struct BIT64 { uint8_t b[8]; } BIT64;

#ifdef HERCULES_CORE
void des_decrypt_block(BIT64* block);
void des_decrypt(unsigned char* data, size_t size);
#endif // HERCULES_CORE

#endif // COMMON_DES_H
