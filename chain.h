/*
 *	HAPR Project 2014
 *	Group 6 - Tom Bryant (TB) & Saul Rennison (SR)
 *
 *	File created by:	SR
 *	File modified by:	SR
 *	File debugged by:	SR
 *
 * chain.c - Filter chain
 *
 * Defines structures and functions to manage and manipulate the filter chain.
 */

#ifndef _CHAIN_H_
#define _CHAIN_H_

#include <stdint.h>
#include <stdbool.h>
#include "filters.h"


/*
 * BranchFlag_e
 *
 * These flags can be OR'd together and set as StageBranch_t::flags to define
 * specific behaviour for a chain stage.
 */
typedef enum
{
	BRANCHFLAG_NONE = 0,
	BRANCHFLAG_ENABLED = (1<<0),
	BRANCHFLAG_FULL_MIX = (1<<1),	///< ignore flMixPerc, implied 1.0 -- only valid on single branch stages
	//STAGEFLAG_UNUSED2 = (1<<2),
	//STAGEFLAG_UNUSED3 = (1<<3),
	//STAGEFLAG_UNUSED4 = (1<<4),
	//STAGEFLAG_UNUSED5 = (1<<5),
	//STAGEFLAG_UNUSED6 = (1<<6),
	//STAGEFLAG_UNUSED7 = (1<<7),
} BranchFlag_e;


/*
 * StageBranch_t
 *
 * Instance of a filter in the filter chain. Contains a pointer to the type of
 * filter and the parameters for this specific part of the chain.
 */
#pragma pack(push, 1)
typedef struct StageBranch_t
{
	const Filter_t *pFilter;		///< pointer to the filter type
	uint8_t flags;					///< `StageFlag_e`s OR'd together
	float flMixPerc;				///< value >0.0 which defines how the output of this filter is scaled
	void *pUnknown;					///< effect data (parameters)
	struct StageBranch_t *pNext;	///< next branch for this stage
} StageBranch_t;
#pragma pack(pop)


/*
 * ChainStageHeader_t
 *
 * Header of a chain stage. Defines the next link in the chain and the number
 * of branches in this link.
 */
#pragma pack(push, 1)
typedef struct ChainStageHeader_t
{
	uint8_t nBranches;					///< number of branches in this stage
	StageBranch_t *pFirst;				///< pointer to first branch in stage
	struct ChainStageHeader_t *pNext;	///< pointer to next stage
} ChainStageHeader_t;
#pragma pack(pop)


extern ChainStageHeader_t *g_pChainRoot;
extern volatile bool g_bChainLock;		///< is chain locked for modification?
extern volatile float g_flChainVolume;	///< current chain volume
extern volatile bool g_writeBack;


ChainStageHeader_t *stage_alloc();
void stage_free(ChainStageHeader_t *pStageHdr);
int16_t stage_apply(const ChainStageHeader_t *pStageHdr, int16_t iSample);
void stage_debug(const ChainStageHeader_t *pStageHdr);
StageBranch_t *stage_get_branch(const ChainStageHeader_t *pStageHdr, uint8_t nBranch);


StageBranch_t *branch_alloc(Filter_e iFilterType, uint8_t flags, float flMixPerc, void **ppUnknown);
void branch_free(StageBranch_t *pBranch);


void chain_free(void);
int16_t chain_apply(int16_t iSample);
void chain_debug();
StageBranch_t *chain_get_branch(uint8_t nStage, uint8_t nBranch);
ChainStageHeader_t *chain_get_stage(uint8_t nStage);

#endif
