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
#include "alloc.h"
#include "string.h"
#include "assert.h"
#include "assertf.h"

#include "kernel/log.h"

#include "lib/cfa.h"
#include "lib/cfa_private.h"
#include "lib/blake2.h"

extern int memcmp(const void *s1, const void *s2, size_t n);

static inline void hash_init(cfa_hash_state_t *state)
{
	blake2s_init(state, DIGEST_SIZE_BYTES);
}

static inline void hash_update(cfa_hash_state_t *state, const cfa_node_t *node)
{
	blake2s_update(state, (uint8_t *)node, sizeof(cfa_node_t));
}

static inline void hash_digest(cfa_hash_state_t *state, uint8_t *digest)
{
	blake2s_state s;
	memcpy(&s, state, sizeof(blake2s_state));
	blake2s_final(&s, digest, DIGEST_SIZE_BYTES);
}

static inline void path_extend(cfa_path_ctx_t *curr_path, const cfa_node_t *node)
{
	debug("CFA EXTEND path: stx = 0x%08x, end = 0x%08x", node->start, node->end);
	hash_update(&(curr_path->state), node);
}

static inline void node_extend(cfa_ctx_t *ctx, const cfa_node_t *node)
{
	debug("CFA EXTEND node: stx = 0x%08x, end = 0x%08x", node->start, node->end);
	hash_update(&(ctx->state), node);
}

static cfa_path_desc_t *new_cfa_path_desc()
{
	cfa_path_desc_t *pd;

	if (!(pd = malloc(sizeof(cfa_path_desc_t)))) {
		assert(false && "cfa_path_desc_t allocation failed");
		return NULL;
	}

	pd->ctr = 1;

	return pd;
}

static cfa_loop_desc_t *new_cfa_loop_desc()
{
	cfa_loop_desc_t *ld;

	if (!(ld = malloc(sizeof(cfa_loop_desc_t)))) {
		assert(false && "cfa_loop_desc_t allocation failed");
		return NULL;
	}

	list_init(&(ld->path_list));

	return ld;
}

static cfa_path_ctx_t *new_cfa_path_ctx()
{
	cfa_path_ctx_t *pctx;

	if (!(pctx = malloc(sizeof(cfa_path_ctx_t)))) {
		assert(false && "cfa_path_ctx_t allocation failed");
		return NULL;
	}

	hash_init(&(pctx->state));

	return pctx;
}

static cfa_loop_ctx_t *new_cfa_loop_ctx(cfa_addr_t loop_entry, cfa_addr_t loop_exit)
{
	cfa_loop_ctx_t * lctx;

	if (!(lctx = malloc(sizeof(cfa_loop_ctx_t)))) {
		assert(false && "cfa_loop_ctx_t allocation failed");
		return NULL;
	}

	lctx->entry.start = loop_entry;
	lctx->exit = loop_exit;
	lctx->ret  = 0;
	lctx->desc = new_cfa_loop_desc();

	return lctx;
}

static inline cfa_addr_t next_instr_addr(cfa_addr_t addr)
{
	return addr + INSTRUCTION_LEN;
}

static inline cfa_loop_ctx_t *get_current_loop(const cfa_ctx_t *ctx)
{
	return list_head(&(ctx->loop_stack));
}

static inline cfa_path_ctx_t *get_current_path(const cfa_loop_ctx_t *curr_loop)
{
	return curr_loop == NULL ? NULL : curr_loop->path;
}

static inline bool is_event_loop_exit(const cfa_ctx_t *ctx, const cfa_event_t *event)
{
	return event->dst_addr >= (get_current_loop(ctx))->exit;
}

static inline bool is_event_in_loop(const cfa_ctx_t *ctx, const cfa_event_t *event)
{
	(void) event;
	return !list_is_empty(&(ctx->loop_stack));
}

static bool is_event_loop_entry(cfa_ctx_t *ctx, const cfa_event_t *event,
		                                const ltbl_entry_t **ltbl_ptr)
{
	*ltbl_ptr = ctx->ltbl_start;

	if (event->type == CFA_EVENT_B && event->src_addr > event->dst_addr) {

		debug("ltbl_bsearch: start: 0x%08x, end: 0x%08x, addr = 0x%08x",
		      ctx->ltbl_start, ctx->ltbl_end, event->dst_addr);

#ifdef CFA_LTBL_BSEARCH
		*ltbl_ptr = ltbl_bsearch(ctx->ltbl_start, ctx->ltbl_end, event->dst_addr);
#else  /* CFA_LTBL_BSEARCH */
		while((*ltbl_ptr) < ctx->ltbl_end && (*ltbl_ptr)->entry < event->dst_addr) {
			(*ltbl_ptr)++;
		}
#endif /* CFA_LTBL_BSEARCH */

		assertf((*ltbl_ptr)->entry == event->dst_addr,
				"unexpected loop starting at 0x%08x", event->dst_addr);

		return true;
	} else {
		(*ltbl_ptr) = NULL;
		return false;
	}
}

static cfa_path_ctx_t *path_entry(cfa_loop_ctx_t *curr_loop)
{
	debug("CFA PATH  entry: ent = 0x%08x", curr_loop->entry);
	curr_loop->path = new_cfa_path_ctx();
	return curr_loop->path;
}

static void path_exit(cfa_loop_ctx_t *curr_loop, cfa_path_ctx_t *curr_path)
{
	cfa_path_desc_t *curr_pd, *pd;
	iter_t i;

	debug("CFA PATH   exit: end = 0x%08x", curr_loop->entry.start);
	
	curr_pd = new_cfa_path_desc();
	hash_digest(&(curr_path->state), curr_pd->digest);

	free(curr_path);

	debug("curr_path : %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x, ctr = %d",
			curr_pd->digest[0],  curr_pd->digest[1],
			curr_pd->digest[2],  curr_pd->digest[3],
			curr_pd->digest[4],  curr_pd->digest[5],
			curr_pd->digest[6],  curr_pd->digest[7],
			curr_pd->digest[8],  curr_pd->digest[9],
			curr_pd->digest[10], curr_pd->digest[11],
			curr_pd->digest[12], curr_pd->digest[13],
			curr_pd->digest[14], curr_pd->digest[15],
			curr_pd->ctr);

	i = list_iter(&(curr_loop->desc->path_list));
	
	while ((pd = list_next(&i)) != NULL) {
		if (!memcmp(pd->digest, curr_pd->digest, DIGEST_SIZE_BYTES)) {
			pd->ctr = pd->ctr + 1;
			free(curr_pd);
			return;
		}
	}
	
	list_append(&(curr_loop->desc->path_list), curr_pd);
}

static cfa_loop_ctx_t *loop_entry(cfa_ctx_t *ctx, const ltbl_entry_t *ltbl_ptr)
{
	cfa_loop_ctx_t *l;
	
	debug("CFA LOOP  entry: ent = 0x%08x, ext = 0x%08x", ltbl_ptr->entry, ltbl_ptr->exit);

	l = new_cfa_loop_ctx(ltbl_ptr->entry, ltbl_ptr->exit);

	hash_digest(&(ctx->state), l->desc->digest);
	
	list_push(&(ctx->loop_stack), l);
	
	return l;
}

static cfa_loop_ctx_t *loop_entry_from_path(cfa_ctx_t *ctx, const ltbl_entry_t *ltbl_ptr, cfa_path_ctx_t *curr_path)
{
	cfa_loop_ctx_t *l;
	
	debug("CFA LOOP  entry: ent = 0x%08x, ext = 0x%08x", ltbl_ptr->entry, ltbl_ptr->exit);

	l = new_cfa_loop_ctx(ltbl_ptr->entry, ltbl_ptr->exit);
	
	hash_digest(&(curr_path->state), l->desc->digest);
	
	list_push(&(ctx->loop_stack), l);
	
	return l;
}

static void loop_exit(cfa_ctx_t *ctx)
{
	cfa_loop_ctx_t *curr_loop;

	curr_loop = (cfa_loop_ctx_t *)list_pop(&(ctx->loop_stack));
	
	debug("CFA LOOP   exit: ent = 0x%08x, ext = 0x%08x", curr_loop->entry.start, curr_loop->exit);

	list_append(&(ctx->loop_list), curr_loop->desc);

	free(curr_loop);
}

/* Secure world API */

/* Initialize the CFA algorithm and context */
uint32_t cfa_init(cfa_ctx_t *ctx,
		  const cfa_addr_t main_start, const cfa_addr_t main_end,
		  const btbl_entry_t* btbl_start, const btbl_entry_t* btbl_end,
		  const ltbl_entry_t* ltbl_start, const ltbl_entry_t* ltbl_end)
{
	iter_t i,j;
	cfa_loop_desc_t *ld;
	cfa_path_desc_t *pd;

	debug("Initializing CFA");
	debug("    main_start = 0x%.8x", main_start);
	debug("    main_end   = 0x%.8x", main_end);
	debug("    btbl_start = 0x%.8x", (uint32_t)btbl_start);
	debug("    btbl_end   = 0x%.8x", (uint32_t)btbl_end);

	/* Clean up accumulated loop and path descriptor lists */
	if (ctx->initialized) {
		i = list_iter(&(ctx->loop_list));
		while ((ld = list_next(&i)) != NULL) {
			j = list_iter(&(ld->path_list));
			while ((pd = list_next(&j)) != NULL) {
				free(pd);
			}
			list_finalize(&(ld->path_list));
			free(ld);
		}
		list_finalize(&(ctx->loop_list));
	}

	ctx->cur_node.start = main_start;

	ctx->main_start = main_start;
	ctx->main_end = main_end;

	ctx->btbl_start = btbl_start;
	ctx->btbl_end = btbl_end;

	ctx->ltbl_start = ltbl_start;
	ctx->ltbl_end = ltbl_end;

	hash_init(&(ctx->state));

	list_init(&(ctx->loop_stack));
	list_init(&(ctx->loop_list));

	ctx->initialized = true;

	return 0;
}

static const btbl_entry_t *btbl_bsearch(const btbl_entry_t *start, const btbl_entry_t *end, cfa_addr_t addr)
{
	size_t low, high, mid;
	const btbl_entry_t *e;

	high = ((uint32_t)end - (uint32_t)start) / sizeof(btbl_entry_t);
	low = 0;
	mid = (high + low) / 2;

	e = start;

	while (high != low+1) {
		e = start + mid;

		if (e->src == addr) {
			return e;
		} else if (e->src < addr){
			low = mid;
		} else{
			high = mid;
		}

		mid = (high + low) / 2;
	}

	return e;
}

static uint32_t infer_non_taken_conditionals(cfa_ctx_t *ctx, const cfa_event_t *event)
{
	/* Check for non-taken conditional branches */
	const btbl_entry_t *btbl_ptr = ctx->btbl_start;
	const btbl_entry_t *btbl_end = ctx->btbl_end;
	cfa_loop_ctx_t *curr_loop;
	cfa_path_ctx_t *curr_path;
	uint8_t err = 0;

	btbl_ptr = btbl_bsearch(ctx->btbl_start, ctx->btbl_end, ctx->cur_node.start);

	// Return early to avoid while loop below from reading from loop table
	if (btbl_ptr >= btbl_end) {
		return err;
	}
	
	curr_loop = get_current_loop(ctx);
	curr_path = get_current_path(curr_loop);

	while(btbl_ptr->src < event->src_addr) {
		ctx->cur_node.end = btbl_ptr->src;

		if (curr_path != NULL) {
			path_extend(curr_path, &(ctx->cur_node));
		} else {
			node_extend(ctx, &(ctx->cur_node));
		}

		ctx->cur_node.start = next_instr_addr(btbl_ptr->src);

		if (curr_loop != NULL && !(curr_loop->ret) &&
		    ctx->cur_node.start >= curr_loop->exit) {

			path_exit(curr_loop, curr_path);
			loop_exit(ctx);

			curr_loop = get_current_loop(ctx);
			curr_path = get_current_path(curr_loop);
		}

		btbl_ptr++;
	}

	return err;
}

/* Process an incoming CFA event */
uint32_t cfa_event(cfa_ctx_t *ctx, const cfa_event_t *event)
{
	int ret = 0;
	const ltbl_entry_t *ltbl_ptr;
	cfa_loop_ctx_t *curr_loop;
	cfa_path_ctx_t *curr_path;

	if(ctx->initialized != true) {
		return CFA_ERROR_BAD_PARAMETERS;
	}

	ret = infer_non_taken_conditionals(ctx, event);

	/* Update block end */
	ctx->cur_node.end = event->src_addr;

	if (is_event_in_loop(ctx, event)) {
		curr_loop = get_current_loop(ctx);
		curr_path = get_current_path(curr_loop);

		if (!(curr_loop->ret) && event->type == CFA_EVENT_BL) {
			curr_loop->ret = next_instr_addr(event->src_addr);
			debug("CFA LOOP   call: ent = 0x%08x, ext = 0x%08x, ret = 0x%08x", curr_loop->entry.start, curr_loop->exit, curr_loop->ret);
		}

		if (curr_loop->ret == event->dst_addr) {
			debug("CFA LOOP    ret: ent = 0x%08x, ext = 0x%08x, ret = 0x%08x", curr_loop->entry.start, curr_loop->exit);
			curr_loop->ret = 0;
		}

		assert(curr_path != NULL && "current path is null");
		path_extend(curr_path, &(ctx->cur_node));

		if (event->dst_addr == curr_loop->entry.start) {  // Start of new iteration, new path
			path_exit(curr_loop, curr_path);
			path_entry(curr_loop);
		} else if (is_event_loop_entry(ctx, event, &ltbl_ptr)) {  // Nested loop entry
			curr_loop = loop_entry_from_path(ctx, ltbl_ptr, curr_path);
			curr_path = path_entry(curr_loop);
		}  else if (!(curr_loop->ret) && is_event_loop_exit(ctx, event)) {  // End of loop reached
			path_exit(curr_loop, curr_path);
			loop_exit(ctx);
		}

	} else {  // Not currently in loop

		if (is_event_loop_entry(ctx, event, &ltbl_ptr)) {  // New loop entry
			curr_loop = loop_entry(ctx, ltbl_ptr);
			path_entry(curr_loop);
		}

		/* Extend a basic block */
		node_extend(ctx, &(ctx->cur_node));
	}

	/* Update block start for next invocation */
	ctx->cur_node.start = event->dst_addr;

	return ret;
}

/* Produce a CFA quote of the current hash value and reset hash */
uint32_t cfa_quote(cfa_ctx_t *ctx,
		   const uint8_t *user_data, const uint32_t user_data_len,
		   uint8_t *out, uint32_t *out_len)
{
	(void)ctx;
	(void)user_data;
	(void)user_data_len;
	(void)out;
	(void)out_len;

	//TODO cfa_quote

	cfa_loop_desc_t *ld;
	cfa_path_desc_t *pd;
	iter_t i, j;
	uint32_t lnum, pnum;
	uint8_t digest[DIGEST_SIZE_BYTES];

	hash_digest(&(ctx->state), digest);

	info("cfa_quote: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
			digest[0],  digest[1],
			digest[2],  digest[3],
			digest[4],  digest[5],
			digest[6],  digest[7],
			digest[8],  digest[9],
			digest[10], digest[11],
			digest[12], digest[13],
			digest[14], digest[15]);

	i = list_iter(&(ctx->loop_list));
	lnum = 0;

	while ((ld = list_next(&i)) != NULL) {
		info("loop[%03d]: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x",
			lnum++,
			ld->digest[0],  ld->digest[1],
			ld->digest[2],  ld->digest[3],
			ld->digest[4],  ld->digest[5],
			ld->digest[6],  ld->digest[7],
			ld->digest[8],  ld->digest[9],
			ld->digest[10], ld->digest[11],
			ld->digest[12], ld->digest[13],
			ld->digest[14], ld->digest[15]);

		j = list_iter(&(ld->path_list));
		pnum = 0;

		while ((pd = list_next(&j)) != NULL) {
			info("path[%03d]: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x (%d)",
					pnum++,
					pd->digest[0],  pd->digest[1],
					pd->digest[2],  pd->digest[3],
					pd->digest[4],  pd->digest[5],
					pd->digest[6],  pd->digest[7],
					pd->digest[8],  pd->digest[9],
					pd->digest[10], pd->digest[11],
					pd->digest[12], pd->digest[13],
					pd->digest[14], pd->digest[15],
					pd->ctr);
		}
	}

	ctx->initialized = false;

	return 0;
}

