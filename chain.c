/*
 * chain.c - Filter chain
 *
 * Defines structures and functions to manage and manipulate the filter chain.
 */

#include <stdlib.h>

#include "dbg.h"
#include "chain.h"


ChainStageHeader_t *g_pChainRoot = NULL;


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
	ChainStageHeader_t *pHdr = (ChainStageHeader_t *)calloc(1, sizeof(ChainStageHeader_t));
	dbg_assert(pHdr, "unable to allocate chain stage");

	return pHdr;
}


/*
 * stage_free
 *
 * Deallocates a chain stage.
 */
void stage_free(ChainStageHeader_t *pStageHdr)
{
	dbg_assert(pStageHdr, "cannot free NULL stage");

	StageBranch_t *pBranch = pStageHdr->pFirst;

	while(pBranch)
	{
		StageBranch_t *pNextBranch = pBranch->pNext;
		branch_free(pBranch);
		pBranch = pNextBranch;
	}

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
int32_t stage_apply(const ChainStageHeader_t *pStageHdr, int32_t iSample)
{
	dbg_assert(pStageHdr->nBranches > 0, "stage has no branches");

	const StageBranch_t *pBranch = pStageHdr->pFirst;
	dbg_assert(pBranch, "stage has no branches (NULL pFirst)");

	// Is this a simple one stage branch?
	if(pStageHdr->nBranches == 1)
	{
		if(!(pBranch->flags & BRANCHFLAG_ENABLED))
			return iSample;

		// If we are using BRANCHFLAG_FULL_MIX, skip the floating point
		// multiplication to save some clock cycles
		if(pBranch->flags & BRANCHFLAG_FULL_MIX)
			return pBranch->pFilter->pfnApply(iSample, pBranch->pUnknown);

		return pBranch->pFilter->pfnApply(iSample, pBranch->pUnknown) * pBranch->flMixPerc;
	}

	// Does this stage have multiple parallel branches that must be mixed?
	else
	{
		uint32_t iResult = 0;
		bool bAnyEnabled = false;

		while(pBranch)
		{
			if(!(pBranch->flags & BRANCHFLAG_ENABLED))
			{
				pBranch = pBranch->pNext;
				continue;
			}

			bAnyEnabled = true;
			iResult += pBranch->pFilter->pfnApply(iSample, pBranch->pUnknown) * pBranch->flMixPerc;

			pBranch = pBranch->pNext;
		}

		if(!bAnyEnabled)
			return iSample;

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
		const char *pszLinePrefix = ANSI_COLOR_RED;
		if(pBranch->flags & BRANCHFLAG_ENABLED)
			pszLinePrefix = ANSI_COLOR_GREEN;

		dbg_printf("%s  - #%d: filter=%s" ANSI_COLOR_RESET ", flags=%x, mixperc=%.3f, data=%p", pszLinePrefix, ++i, pBranch->pFilter->pszName, pBranch->flags, pBranch->flMixPerc, (void *)pBranch->pUnknown);

		if(pBranch->pFilter->pfnDebug)
		{
			dbg_printn(" -> ", -1);
			pBranch->pFilter->pfnDebug(pBranch->pUnknown);
		}

		dbg_printn("\r\n", -1);

		pBranch = pBranch->pNext;
	}
}
#pragma GCC diagnostic pop


/*
 * stage_get_branch
 *
 * @returns branch of index `nBranch` of stage `pStageHdr`
 */
StageBranch_t *stage_get_branch(const ChainStageHeader_t *pStageHdr, uint8_t nBranch)
{
	uint8_t i = 0;
	StageBranch_t *pBranch = pStageHdr->pFirst;

	while(pBranch && i < nBranch)
	{
		pBranch = pBranch->pNext;
		i++;
	}

	if(i != nBranch)
	{
		dbg_warning("unable to get branch %u of stage\r\n", nBranch);
		return NULL;
	}

	return pBranch;
}


/*
 * branch_alloc
 *
 * Allocates a stage branch.
 *
 * @returns pointer to the header for this chain stage
 */
 StageBranch_t *branch_alloc(Filter_e iFilterType, uint8_t flags, float flMixPerc, void **ppUnknown)
{
	dbg_assert(iFilterType < NUM_FILTERS, "invalid filter type");

	// Allocate branch
	StageBranch_t *pBranch = (StageBranch_t *)calloc(1, sizeof(StageBranch_t));
	dbg_assert(pBranch, "unable to allocate branch");

	// Initialise the branch
	pBranch->pFilter = &g_pFilters[iFilterType];
	pBranch->flags = flags;
	pBranch->flMixPerc = flMixPerc;

	// Allocate filter data
	pBranch->pUnknown = calloc(1, pBranch->pFilter->nFilterDataSize);
	dbg_assert(pBranch->pUnknown, "unable to allocate data for filter %s", pBranch->pFilter->pszName);

	if(ppUnknown)
		*ppUnknown = pBranch->pUnknown;

	return pBranch;
}


/*
 * branch_free
 *
 * Deallocates a branch and its data.
 */
void branch_free(StageBranch_t *pBranch)
{
	dbg_assert(pBranch, "cannot free NULL branch");
	free(pBranch->pUnknown);
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
int16_t chain_apply(int16_t iSample)
{
	// Upscale 12-bit ADC sample to 32-bit
	int32_t iIntermediate = iSample << 19;

	const ChainStageHeader_t *pStageHdr = g_pChainRoot;

	while(pStageHdr)
	{
		if(pStageHdr->nBranches > 0)
			iIntermediate = stage_apply(pStageHdr, iIntermediate);

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
void chain_debug(void)
{
	dbg_printf(" === chain_debug(%p) ===\r\n", (void *)g_pChainRoot);

	uint i = 0;
	const ChainStageHeader_t *pStageHdr = g_pChainRoot;

	while(pStageHdr)
	{
		dbg_printf("\r\n#%d: ", ++i);
		stage_debug(pStageHdr);
		pStageHdr = pStageHdr->pNext;
	}

	dbg_printn("\r\n", -1);
}


/*
 * chain_get_stage
 *
 * @returns stage of index `nStage` of chain
 */
ChainStageHeader_t *chain_get_stage(uint8_t nStage)
{
	uint8_t i = 0;
	const ChainStageHeader_t *pStageHdr = g_pChainRoot;

	while(pStageHdr && i < nStage)
	{
		pStageHdr = pStageHdr->pNext;
		i++;
	}

	if(i != nStage)
	{
		dbg_warning("unable to get stage %u of chain\r\n", nStage);
		return NULL;
	}

	return (ChainStageHeader_t *)pStageHdr;
}


/*
 * chain_free
 *
 * Deallocates entire chain
 */
void chain_free(void)
{
	ChainStageHeader_t *pStageHdr = g_pChainRoot;

	while(pStageHdr)
	{
		ChainStageHeader_t *pNextStage = pStageHdr->pNext;
		stage_free(pStageHdr);
		pStageHdr = pNextStage;
	}
}
