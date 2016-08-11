/*
 * C-FLAT Secure World API
 */
#ifndef CFA_H
#define CFA_H

#ifdef CFA_STUB_H
#error "CFA_STUB_H already defined, did you mean to call the normal world API?"
#endif

#include <stdint.h>
#include <stdbool.h>

#include "list.h"

#include "btbl.h"
#include "ltbl.h"

#include "cfa_common.h"
#include "cfa_private.h"

typedef struct cfa_event {
	union {
		uint32_t type;
		uint32_t err;
	};
	union {
		cfa_addr_t src_addr;
		cfa_init_params_t* iparams;
		cfa_quote_params_t* qparams;
	};
	cfa_addr_t dst_addr;
	cfa_addr_t lr_value;
} cfa_event_t;

/* Node in the control flow graph */
typedef struct cfa_node {
	cfa_addr_t start;
	cfa_addr_t end;
} cfa_node_t;

/* Path descriptor */
typedef struct cfa_path_desc {
	uint32_t ctr;
	uint8_t digest[DIGEST_SIZE_BYTES];
} cfa_path_desc_t;

/* Currently executed path context */
typedef struct cfa_path_ctx {
	cfa_hash_state_t state;
} cfa_path_ctx_t;

/* Loop descriptor */
typedef struct cfa_loop_desc {
	uint8_t digest[DIGEST_SIZE_BYTES];
	list_t path_list;
} cfa_loop_desc_t;

/* Currently executed loop context */
typedef struct cfa_loop_ctx {
	cfa_node_t entry;
	cfa_addr_t exit;
	cfa_addr_t ret;
	cfa_path_ctx_t *path;
	cfa_loop_desc_t *desc;
} cfa_loop_ctx_t;

/* Context for CFA operations */
typedef struct cfa_ctx {
	cfa_hash_state_t state;
	cfa_node_t cur_node;
	cfa_addr_t main_start;
	cfa_addr_t main_end;
	const btbl_entry_t *btbl_start;
	const btbl_entry_t *btbl_end;
	const ltbl_entry_t *ltbl_start;
	const ltbl_entry_t *ltbl_end;
	list_t loop_stack;
	list_t loop_list;
	bool initialized;
} cfa_ctx_t;

/* Secure world API */

/*!
 * \brief cfa_get_ctx
 * Returns the current CFA context.
 */
cfa_ctx_t* cfa_get_ctx();

/*!
 * \brief cfa_init
 * Initialize the CFA subsystem.
 * \param main_start Start address of main function
 * \param main_end End address of main function
 * \param btbl_start Start address of branch table
 * \param btbl_end End address of branch table
 */
uint32_t cfa_init(cfa_ctx_t *ctx,
		  const cfa_addr_t main_start, const cfa_addr_t main_end,
		  const btbl_entry_t *btbl_start, const btbl_entry_t *btbl_end,
		  const ltbl_entry_t *ltbl_start, const ltbl_entry_t *ltbl_end);

/*!
 * \brief cfa_event
 * Extend the digest.
 * \param type Type of control flow event (e.g. CFA_EVENT_XX)
 * \param src_addr Source address of the control flow event
 * \param dst_addr Destination address of the control flow event
 */
uint32_t cfa_event(cfa_ctx_t *ctx, const cfa_event_t *event);

/*!
 * \brief cfa_quote
 * Quote the current digest value.
 * \param user_data Unpredictable user-supplied data to be included in quote
 * \param user_data_len Length of the user-supplied data
 * \param out Output buffer for storing the digest quote
 * \param outlen Length of the output buffer for storing the digest quote
 */
uint32_t cfa_quote(cfa_ctx_t *ctx,
		   const uint8_t *user_data, const uint32_t user_data_len,
		   uint8_t *out, uint32_t *out_len);

#endif /* CFA_H */
