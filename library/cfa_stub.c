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
#include "sm/smcall.h"

#include "lib/cfa_stub.h"

/* Normal world API */

uint32_t cfa_init(const cfa_addr_t main_start, const cfa_addr_t main_end,
		  const btbl_entry_t *btbl_start, const btbl_entry_t *btbl_end,
		  const ltbl_entry_t *ltbl_start, const ltbl_entry_t *ltbl_end)
{
	cfa_init_params_t init_params;
	init_params.main_start = main_start;
	init_params.main_end = main_end;
	init_params.btbl_start = btbl_start;
	init_params.btbl_end = btbl_end;
	init_params.ltbl_start = ltbl_start;
	init_params.ltbl_end = ltbl_end;

	return smcall(CFA_EVENT_INIT, (uint32_t)&init_params, 0, 0);
}

uint32_t cfa_quote(const uint8_t *user_data, const uint32_t user_data_len,
		   uint8_t *out, uint32_t *out_len)
{
	cfa_quote_params_t quote_params;
	quote_params.user_data = user_data;
	quote_params.user_data_len = user_data_len;
	quote_params.out = out;
	quote_params.out_len = out_len;

	return smcall(CFA_EVENT_QUOTE, (uint32_t)&quote_params, 0, 0);
}

