/*
 * chain.c - Filter chain
 *
 * Defines structures and functions to manage and manipulate the filter chain.
 */

#include <stdlib.h>

#include "dbg.h"
#include "chain.h"


 /*
  * stage_alloc
  *
  * Allocates a chain stage with `nBranches` branches.
  *
  * @returns pointer to the header for this chain stage
  */
ChainStageHeader_t *stage_alloc(uint8_t nBranches)
{
	dbg_assert(nBranches > 0, "cannot allocate stage with no filters");

	// Allocate chain stage header and enough space for each branch
	unsigned char *pBase = (unsigned char *)malloc(sizeof(ChainStageHeader_t) + (nBranches * sizeof(ChainStage_t)));
	dbg_assert(pBase, "unable to allocate chain stage");

	// Initialise the header
	ChainStageHeader_t *pHdr = (ChainStageHeader_t *)pBase;
	pHdr->nBranches = nBranches;
	pHdr->pNext = NULL;

	// Initialise each branch with empty data
	for(uint8_t i = 0; i < nBranches; ++i)
	{
		ChainStage_t *pStage = STAGE_BY_INDEX(pBase, i);
		pStage->pFilter = NULL;
		pStage->flags = STAGEFLAG_NONE;
		pStage->flMixPerc = 0.0;
		pStage->pPrivate = NULL;
	}

	return pHdr;
}


/*
 * stage_free
 *
 * Deallocates a chain stage.
 */
void stage_free(ChainStageHeader_t *pStageHdr)
{
	dbg_assert(pStageHdr, "cannot free NULL header");
	free(pStageHdr);
}


/*
 * stage_apply
 *
 * Applies each branch in a chain stage to a sample `iSample`.
 *
 * Note `iSample` should be a 32-bit sample i.e., in the range [0..4294967295))
 *
 * @returns filtered 32-bit sample
 */
uint32_t stage_apply(const ChainStageHeader_t *pStageHdr, uint32_t iSample)
{
	// Is this a simple one stage branch?
	if(pStageHdr->nBranches == 1)
	{
		ChainStage_t *pStage = STAGE_BY_INDEX(pStageHdr, 0);

		// If we are using STAGEFLAG_FULL_MIX, skip the floating point
		// multiplication to save some clock cycles
		if(pStage->flags & STAGEFLAG_FULL_MIX)
			return pStage->pFilter->pfnApply(iSample, pStage->pPrivate);

		return pStage->pFilter->pfnApply(iSample, pStage->pPrivate) * pStage->flMixPerc;
	}

	// Does this stage have multiple parallel branches that must be mixed?
	else
	{
		uint32_t iResult = 0;

		for(uint8_t i = 0; i < pStageHdr->nBranches; ++i)
		{
			ChainStage_t *pStage = STAGE_BY_INDEX(pStageHdr, i);
			iResult += pStage->pFilter->pfnApply(iSample, pStage->pPrivate) * pStage->flMixPerc;
		}

		return iResult;
	}
}


/*
 * chain_apply
 *
 * Applies each stage in an entire filter chain.
 *
 * Note `iSample` should be a 12-bit sample from the ADC.
 *
 * @returns filtered 10-bit sample
 */
uint16_t chain_apply(const ChainStageHeader_t *pRoot, uint16_t iSample)
{
	// Upscale 12-bit ADC sample to 32-bit
	uint32_t iIntermediate = iSample << 20;

	const ChainStageHeader_t *pStage = pRoot;

	while(pStage)
	{
		iIntermediate = stage_apply(pStage, iIntermediate);
		pStage = pStage->pNext;
	}

	// Downscale 32-bit intermediate value to 10-bit for DAC
	return iIntermediate >> 22;
}


/*
 * stage_debug
 *
 * Prints debug information for a single chain stage (which may have multiple
 * branches).
 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdouble-promotion"
void stage_debug(const ChainStageHeader_t *pStageHdr)
{
	dbg_printf("Stage with %d branch(es):\r\n", pStageHdr->nBranches);

	for(uint8_t i = 0; i < pStageHdr->nBranches; ++i)
	{
		const ChainStage_t *pStage = STAGE_BY_INDEX(pStageHdr, i);

		dbg_printf("  - #%d: filter=%s, flags=%x, mixperc=%.3f, private=%p", i+1, pStage->pFilter->pszName, pStage->flags, pStage->flMixPerc, (void *)pStage->pPrivate);

		if(pStage->pFilter->pfnDebug)
		{
			dbg_printn(" -> ", -1);
			pStage->pFilter->pfnDebug(pStage->pPrivate);
		}

		dbg_printn("\r\n", -1);
	}
}
#pragma GCC diagnostic pop


/*
 * chain_debug
 *
 * Prints debug information for each stage in an entire chain.
 */
void chain_debug(const ChainStageHeader_t *pRoot)
{
	dbg_printf(" === chain_debug(%p) ===\r\n", (void *)pRoot);

	uint i = 0;
	const ChainStageHeader_t *pStage = pRoot;

	while(pStage)
	{
		dbg_printf("\r\n#%d: ", ++i);
		stage_debug(pStage);
		pStage = pStage->pNext;
	}
}
