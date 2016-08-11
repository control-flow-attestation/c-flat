/*
 * Copyright (c) 2016 Aalto University
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef CFA_COMMON_H
#define CFA_COMMON_H

/* Length of the computed digests */
#define DIGEST_SIZE_BYTES       16

/* Length of instruction in bytes */
#define INSTRUCTION_LEN		4

/* CFA event types */
#define CFA_EVENT_INIT		 0x00000000
#define CFA_EVENT_B		 0x00000001
#define CFA_EVENT_BL		 0x00000002
#define CFA_EVENT_BX_LR		 0x00000004
#define CFA_EVENT_POP_FP_PC	 0x00000008
#define CFA_EVENT_POP_FP_LR	 0x00000010
#define CFA_EVENT_BLX_R3	 0x00000020
#define CFA_EVENT_QUOTE		 0x80000000
#define CFA_EVENT_ERROR		 0x000000FF

/* Internal CFA return values */
#define CFA_SUCCESS              0x00000000
#define CFA_ERROR_GENERIC        0xFFFF0000
#define CFA_ERROR_BAD_PARAMETERS 0xFFFF0006
#define CFA_ERROR_OUT_OF_MEMORY  0xFFFF000C

#ifndef ASSEMBLY

#include <stdint.h>

/* Type alias for memory address */
typedef uint32_t cfa_addr_t;

/* Structure for transferring init parameters */
typedef struct cfa_init_params {
	cfa_addr_t main_start;
	cfa_addr_t main_end;
	const struct btbl_entry *btbl_start;
	const struct btbl_entry *btbl_end;
	const struct ltbl_entry *ltbl_start;
	const struct ltbl_entry *ltbl_end;
} cfa_init_params_t;

/* Structure for transferring quote parameters */
typedef struct cfa_quote_params {
	const uint8_t *user_data;
	uint32_t user_data_len;
	uint8_t *out;
	uint32_t *out_len;
} cfa_quote_params_t;

#endif

#endif /* CFA_COMMON_H */

