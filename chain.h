/*
 * chain.c - Filter chain
 *
 * Defines structures and functions to manage and manipulate the filter chain.
 */

#ifndef _CHAIN_H_
#define _CHAIN_H_

#include <stdint.h>
#include "filters.h"


/*
 * Returns the `ChainStage_t` at position `i` in a chain stage `pHdr`.
 */
#define STAGE_BY_INDEX(pHdr, i) ((ChainStage_t *)((unsigned char *)pHdr + sizeof(ChainStageHeader_t) + (i * sizeof(ChainStage_t))))


 /*
  * ChainStageHeader_t
  *
  * Header of a chain stage. Defines the next link in the chain and the number
  * of branches in this link.
  *
  * `nBranches` instances of `ChainStage_t` directly follow a
  * `ChainStageHeader_t` in memory. Use `STAGE_BY_INDEX` to access a
  * `ChainStage_t` branch from a chain stage header.
  */
#pragma pack(push, 1)
typedef struct ChainStageHeader_t
{
	uint8_t nBranches;
	struct ChainStageHeader_t *pNext;
} ChainStageHeader_t;
#pragma pack(pop)


/*
 * StageFlag_e
 *
 * These flags can be OR'd together and set as ChainStage_t::flags to define
 * specific behaviour for a chain stage.
 */
typedef enum
{
	STAGEFLAG_NONE = 0,
	STAGEFLAG_FULL_MIX = (1<<0),	///< ignore flMixPerc, implied 1.0 -- only valid on single branch stages
	//STAGEFLAG_UNUSED1 = (1<<1),
	//STAGEFLAG_UNUSED2 = (1<<2),
	//STAGEFLAG_UNUSED3 = (1<<3),
	//STAGEFLAG_UNUSED4 = (1<<4),
	//STAGEFLAG_UNUSED5 = (1<<5),
	//STAGEFLAG_UNUSED6 = (1<<6),
	//STAGEFLAG_UNUSED7 = (1<<7),
} StageFlag_e;


/*
 * ChainStage_t
 *
 * Instance of a filter in the filter chain. Contains a pointer to the type of
 * filter and the parameters for this specific part of the chain.
 */
#pragma pack(push, 1)
typedef struct
{
	const Filter_t *pFilter; ///< pointer to the filter type
	uint8_t flags; ///< `StageFlag_e`s OR'd together
	float flMixPerc; ///< value >0.0 which defines how the output of this filter is scaled
	void *pPrivate; // effect data (parameters)
} ChainStage_t;
#pragma pack(pop)


/*
 * FilterChain_t
 *
 * Defines the root node of a filter chain linked-list.
 */
#pragma pack(push, 1)
typedef struct
{
	ChainStageHeader_t *pFirst;
} FilterChain_t;
#pragma pack(pop)


ChainStageHeader_t *stage_alloc(uint8_t nBranches);
void stage_free(ChainStageHeader_t *pStageHdr);
uint32_t stage_apply(const ChainStageHeader_t *pStageHdr, uint32_t iSample);
void stage_debug(const ChainStageHeader_t *pStageHdr);


uint16_t chain_apply(const ChainStageHeader_t *pRoot, uint16_t iSample);
void chain_debug(const ChainStageHeader_t *pRoot);

#endif