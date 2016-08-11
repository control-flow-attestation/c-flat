/*
 * Branch table entry structure
 */
#ifndef BTBL_H
#define BTBL_H

#include <stdint.h>

typedef struct btbl_entry {
	uint32_t src;
	uint32_t dst;
} btbl_entry_t;

#endif /* BTBL_H */

