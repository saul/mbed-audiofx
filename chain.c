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
 * Allocates a chain stage header.
 *
 * @returns pointer to the header for this chain stage
 */
ChainStageHeader_t *stage_alloc(void)
{
	// Allocate chain stage header
	ChainStageHeader_t *pHdr = (ChainStageHeader_t *)malloc(sizeof(ChainStageHeader_t));
	dbg_assert(pHdr, "unable to allocate chain stage");

	// Initialise the header
	pHdr->nBranches = 0;
	pHdr->pFirst = NULL;
	pHdr->pNext = NULL;

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
	dbg_assert(pStageHdr->nBranches > 0, "stage has no branches");

	const StageBranch_t *pBranch = pStageHdr->pFirst;
	dbg_assert(pBranch, "stage has no branches (NULL pFirst)");

	// Is this a simple one stage branch?
	if(pStageHdr->nBranches == 1)
	{
		if(!(pBranch->flags & STAGEFLAG_ENABLED))
			return 0;

		// If we are using STAGEFLAG_FULL_MIX, skip the floating point
		// multiplication to save some clock cycles
		if(pBranch->flags & STAGEFLAG_FULL_MIX)
			return pBranch->pFilter->pfnApply(iSample, pBranch->pPrivate);

		return pBranch->pFilter->pfnApply(iSample, pBranch->pPrivate) * pBranch->flMixPerc;
	}

	// Does this stage have multiple parallel branches that must be mixed?
	else
	{
		uint32_t iResult = 0;

		while(pBranch)
		{
			if(!(pBranch->flags & STAGEFLAG_ENABLED))
			{
				pBranch = pBranch->pNext;
				continue;
			}

			iResult += pBranch->pFilter->pfnApply(iSample, pBranch->pPrivate) * pBranch->flMixPerc;

			pBranch = pBranch->pNext;
		}

		return iResult;
	}
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

	uint8_t i = 0;
	const StageBranch_t *pBranch = pStageHdr->pFirst;

	while(pBranch)
	{
		dbg_printf("  - #%d: filter=%s, flags=%x, mixperc=%.3f, private=%p", ++i, pBranch->pFilter->pszName, pBranch->flags, pBranch->flMixPerc, (void *)pBranch->pPrivate);

		if(pBranch->pFilter->pfnDebug)
		{
			dbg_printn(" -> ", -1);
			pBranch->pFilter->pfnDebug(pBranch->pPrivate);
		}

		dbg_printn("\r\n", -1);

		pBranch = pBranch->pNext;
	}
}
#pragma GCC diagnostic pop


/*
 * branch_alloc
 *
 * Allocates a stage branch.
 *
 * @returns pointer to the header for this chain stage
 */
 StageBranch_t *branch_alloc(Filter_e iFilterType, uint8_t flags, float flMixPerc, void **ppPrivate)
{
	dbg_assert(iFilterType < NUM_FILTERS, "invalid filter type");
	dbg_assert(ppPrivate, "ppPrivate is NULL");

	// Allocate branch
	StageBranch_t *pBranch = (StageBranch_t *)malloc(sizeof(StageBranch_t));
	dbg_assert(pBranch, "unable to allocate branch");

	// Initialise the branch
	pBranch->pFilter = &g_pFilters[iFilterType];
	pBranch->flags = flags;
	pBranch->flMixPerc = flMixPerc;
	pBranch->pNext = NULL;

	// Allocate private data
	*ppPrivate = malloc(pBranch->pFilter->nPrivateDataSize);
	dbg_assert(pBranch->pPrivate, "unable to allocate private data for filter %s", pBranch->pFilter->pszName);
	pBranch->pPrivate = *ppPrivate;

	return pBranch;
}


/*
 * branch_free
 *
 * Deallocates a branch and its private data.
 */
void branch_free(StageBranch_t *pBranch)
{
	dbg_assert(pBranch, "cannot free NULL branch");
	free(pBranch->pPrivate);
	free(pBranch);
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

	const ChainStageHeader_t *pStageHdr = pRoot;

	while(pStageHdr)
	{
		iIntermediate += stage_apply(pStageHdr, iIntermediate);
		pStageHdr = pStageHdr->pNext;
	}

	// Downscale 32-bit intermediate value to 10-bit for DAC
	return iIntermediate >> 22;
}


/*
 * chain_debug
 *
 * Prints debug information for each stage in an entire chain.
 */
void chain_debug(const ChainStageHeader_t *pRoot)
{
	dbg_printf(" === chain_debug(%p) ===\r\n", (void *)pRoot);

	uint i = 0;
	const ChainStageHeader_t *pStageHdr = pRoot;

	while(pStageHdr)
	{
		dbg_printf("\r\n#%d: ", ++i);
		stage_debug(pStageHdr);
		pStageHdr = pStageHdr->pNext;
	}
}
