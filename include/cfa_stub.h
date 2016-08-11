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
#ifndef CFA_STUB_H
#define CFA_STUB_H

#ifdef CFA_H
#error "CFA_H already defined, did you mean to call the secure world API?"
#endif

#include <stddef.h>
#include <stdint.h>

#include "btbl.h"
#include "ltbl.h"

#include "cfa_common.h"

/* Normal world API */

/*!
 * \brief cfa_init
 * Initialize the CFA subsystem.
 * \param main_start Start address of main function
 * \param main_end End address of main function
 * \param u_start Start address of user code to be attested
 * \param u_end End address of user code to be attested
 */
uint32_t cfa_init(const cfa_addr_t main_start, const cfa_addr_t main_end,
		  const btbl_entry_t *btbl_start, const btbl_entry_t *btbl_end,
		  const ltbl_entry_t *ltbl_start, const ltbl_entry_t *ltbl_end);

/*!
 * \brief cfa_quote
 * Quote the current digest value.
 * \param out Output buffer for storing the digest quote
 * \param outlen Length of the output buffer for storing the digest quote
 */
uint32_t cfa_quote(const uint8_t *user_data, const uint32_t user_data_len,
		   uint8_t *out, uint32_t *out_len);

#endif /* CFA_STUB_H */

