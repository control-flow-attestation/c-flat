/*
 * C-FLAT Private Declarations
 */
#ifndef CFA_PRIVATE_H
#define CFA_PRIVATE_H

#include "lib/blake2.h"

typedef blake2s_state cfa_hash_state_t;

#ifdef CFA_LTBL_BSEARCH
extern const ltbl_entry_t *ltbl_bsearch(const ltbl_entry_t *start, const ltbl_entry_t *end, cfa_addr_t addr);
#endif

#endif /* CFA_PRIVATE_H */
