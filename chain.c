#include <stdlib.h>

#include "dbg.h"
#include "usbcon.h"
#include "chain.h"


ChainStageHeader_t *stage_alloc(uint8_t nBranches)
{
	dbg_assert(nBranches > 0, "cannot allocate stage with no filters");

	unsigned char *pBase = (unsigned char *)malloc(sizeof(ChainStageHeader_t) + (nBranches * sizeof(ChainStage_t)));
	dbg_assert(pBase, "unable to allocate chain stage");

	ChainStageHeader_t *pHdr = (ChainStageHeader_t *)pBase;
	pHdr->nBranches = nBranches;
	pHdr->pNext = NULL;

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


void stage_free(ChainStageHeader_t *pStageHdr)
{
	dbg_assert(pStageHdr, "cannot free NULL header");
	free(pStageHdr);
}


uint32_t stage_apply(const ChainStageHeader_t *pStageHdr, uint32_t iSample)
{
	//usbcon_writef("    stage_apply(%p, %u)\r\n", (void *)pStageHdr, (unsigned int)iSample);

	if(pStageHdr->nBranches == 1)
	{
		ChainStage_t *pStage = STAGE_BY_INDEX(pStageHdr, 0);
		//usbcon_writef("    1 branch, stage=%p\r\n", (void *)pStage);

		if(pStage->flags & STAGEFLAG_FULL_MIX)
			return pStage->pFilter->pfnApply(iSample, pStage->pPrivate);
		else
			return pStage->pFilter->pfnApply(iSample, pStage->pPrivate) * pStage->flMixPerc;
	}
	else
	{
		uint32_t iResult = 0;

		for(uint8_t i = 0; i < pStageHdr->nBranches; ++i)
		{
			ChainStage_t *pStage = STAGE_BY_INDEX(pStageHdr, i);
			//usbcon_writef("    stage[%d]=%p\r\n", i, (void *)pStage);

			iResult += pStage->pFilter->pfnApply(iSample, pStage->pPrivate) * pStage->flMixPerc;
		}

		return iResult;
	}
}


// Input: 10-bit, output: 12-bit
uint16_t chain_apply(const ChainStageHeader_t *pRoot, uint16_t iSample)
{
	uint32_t iIntermediate = iSample << 20;
	//usbcon_writef("\r\nchain_apply(%p, %u):\r\n", (void *)pRoot, iSample);

	const ChainStageHeader_t *pStage = pRoot;

	while(pStage)
	{
		//usbcon_writef("  - stage %p\r\n", (void *)pStage);
		iIntermediate = stage_apply(pStage, iIntermediate);
		pStage = pStage->pNext;
	}

	return iIntermediate >> 22;
}


void stage_debug(const ChainStageHeader_t *pStageHdr)
{
	usbcon_writef("Stage with %d branch(es):\r\n", pStageHdr->nBranches);

	for(uint8_t i = 0; i < pStageHdr->nBranches; ++i)
	{
		const ChainStage_t *pStage = STAGE_BY_INDEX(pStageHdr, i);

		usbcon_writef("  - #%d: filter=%s, flags=%x, mixperc=%.3f, private=%p", i+1, pStage->pFilter->pszName, pStage->flags, pStage->flMixPerc, (void *)pStage->pPrivate);

		if(pStage->pFilter->pfnDebug)
		{
			usbcon_write(" -> ", -1);
			pStage->pFilter->pfnDebug(pStage->pPrivate);
		}

		usbcon_write("\r\n", -1);
	}
}


void chain_debug(const ChainStageHeader_t *pRoot)
{
	usbcon_writef(" === chain_debug(%p) ===\r\n", (void *)pRoot);

	uint i = 0;
	const ChainStageHeader_t *pStage = pRoot;

	while(pStage)
	{
		usbcon_writef("\r\n#%d: ", ++i);
		stage_debug(pStage);
		pStage = pStage->pNext;
	}
}