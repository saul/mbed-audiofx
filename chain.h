#ifndef _CHAIN_H_
#define _CHAIN_H_

#include <stdint.h>
#include "filters.h"


#define STAGE_BY_INDEX(pHdr, i) ((ChainStage_t *)((unsigned char *)pHdr + sizeof(ChainStageHeader_t) + (i * sizeof(ChainStage_t))))


#pragma pack(push, 1)
typedef struct ChainStageHeader_t
{
	uint8_t nBranches;
	struct ChainStageHeader_t *pNext;
} ChainStageHeader_t;
#pragma pack(pop)


enum StageFlag_e
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
};


#pragma pack(push, 1)
typedef struct
{
	Filter_t *pFilter;
	uint8_t flags;
	float flMixPerc;
	void *pPrivate; // effect data (parameters etc.)
} ChainStage_t;
#pragma pack(pop)


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