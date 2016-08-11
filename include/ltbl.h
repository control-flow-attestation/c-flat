/*
 * Loop table entry structure
 */
#ifndef LTBL_H
#define LTBL_H

#include "lib/cfa_common.h"

typedef struct ltbl_entry {
	cfa_addr_t entry;
	cfa_addr_t exit;
} ltbl_entry_t;

#endif /* LTBL_H */

