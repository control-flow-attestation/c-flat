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

/* ltbl_entry_t *ltbl_bsearch(start, end, addr) */
.global ltbl_bsearch
.type ltbl_bsearch, %function
ltbl_bsearch:
	push    {r1-r4}
_ltbl_bsearch_start:
	cmp	r0, r1        /* has end of auxiliary data been reached? */
	bgt	_ltbl_bsearch_error

	add	r3, r0, r1
	mov	r3, r3, lsr #1
	bic 	r3, r3, #7    /* entry dest addr at 8-byte               */

	ldr	r4, [r3]
	cmp	r4, r2        /* has matching source address been found? */

	bgt	_ltbl_bsearch_right  /* no match, search lower addrs     */
	blt	_ltbl_bsearch_left   /* no match, search higher addrs    */
_ltbl_bsearch_return:
	mov	r0, r3        /* match! branch to handler                */
	pop	{r1-r4}
	bx	lr
_ltbl_bsearch_right:
	mov	r1, r3
	b	_ltbl_bsearch_start
_ltbl_bsearch_left:
	mov	r0, r3
	b	_ltbl_bsearch_start
_ltbl_bsearch_error:
	mov	r0, #0
	pop	{r1-r4}
	bx	lr
.size ltbl_bsearch, .-ltbl_bsearch

