#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "dbg.h"
#include "fatfs/ff.h"
#include "sd.h"
#include "filters.h"
#include "chain.h"
#include "chainstore.h"


static uint8_t param_type_size(char ch)
{
	switch(ch)
	{
	case 'c':
	case 'b':
	case 'B':
	case '?':
		return 1;

	case 'h':
	case 'H':
		return 2;

	case 'i':
	case 'I':
	case 'l':
	case 'L':
	case 'f':
		return 4;

	case 'q':
	case 'Q':
	case 'd':
		return 8;

	default:
		dbg_warning("unknown parameter type (%c)\r\n", ch);
		return 0;
	}
}


static void chainstore_save_branch(FIL *pFile, const StageBranch_t *pBranch)
{
	FRESULT res;
	UINT nWrote;

	// Calculate filter index
	ptrdiff_t iFilterIndex = pBranch->pFilter - g_pFilters;
	dbg_printf("\tbranch %s (index=%d):\r\n", pBranch->pFilter->pszName, iFilterIndex);

	// Reserve space for branch header
	DWORD iHdrCursor = f_tell(pFile);
	f_lseek(pFile, iHdrCursor + sizeof(ChainStoreBranchHeader_t));

	// Iterate filter parameters
	const char *pParam = pBranch->pFilter->pszParamFormat;
	uint8_t nParams = 0;

	while(pParam)
	{
		// Skip past parameter separator
		if(*pParam == *PARAM_SEP) pParam++;

		// When is the next parameter?
		const char *pszNextParam = strstr(pParam, PARAM_SEP);

		// Iterate parameter KV
		bool bFoundOffset = false;
		uint8_t offset = 0;
		char format = 0;

		// Jump to first KV pair
		const char *pKVPair = strstr(pParam, ";");
		uint8_t nParamNameLen = pKVPair - pParam;

		while(pKVPair && (pszNextParam == NULL || pKVPair < pszNextParam))
		{
			// Move past semi-colon
			pKVPair++;

			if(!strncmp(pKVPair, "f=", 2))
			{
				format = pKVPair[2];
			}
			else if(!strncmp(pKVPair, "o=", 2))
			{
				bFoundOffset = true;
				offset = atoi(pKVPair + 2);
			}

			// Move to next KV pair
			pKVPair = strstr(pKVPair, ";");
		}

		// Missing required KV?
		if(!bFoundOffset || !format)
			dbg_error("parameter (%.*s) for filter (%s) is missing offset or format KV\r\n", nParamNameLen, pParam, pBranch->pFilter->pszName);

		offset += pBranch->pFilter->nNonPublicDataSize;

		// Write param header
		ChainStoreParam_t param;
		param.iOffset = offset;
		param.nSize = param_type_size(format);

		if((res = f_write(pFile, &param, sizeof(param), &nWrote)))
		{
			dbg_warning("parameter header write failed %d\r\n", res);
			return;
		}

		// Write param value
		if((res = f_write(pFile, &((uint8_t *)pBranch->pUnknown)[offset], param.nSize, &nWrote)))
		{
			dbg_warning("parameter value write failed %d\r\n", res);
			return;
		}

		dbg_printf("\t\tparam #%d: name=%.*s,absoffset=%d,size=%d\r\n", nParams, nParamNameLen, pParam, offset, param.nSize);

		// Move to next parameter
		pParam = pszNextParam;
		nParams++;
	}

	ChainStoreBranchHeader_t branchHdr;
	branchHdr.filter = iFilterIndex;
	branchHdr.flags = pBranch->flags;
	branchHdr.flMixPerc = pBranch->flMixPerc;
	branchHdr.nParams = nParams;

	// Write branch header
	DWORD iEndCursor = f_tell(pFile);
	f_lseek(pFile, iHdrCursor);

	if((res = f_write(pFile, &branchHdr, sizeof(branchHdr), &nWrote)))
	{
		dbg_warning("branch header write failed %d\r\n", res);
		return;
	}

	// Jump back
	f_lseek(pFile, iEndCursor);
}


void chainstore_save(const char *pszPath)
{
	FRESULT res;
	UINT nWrote;

	if(!g_pChainRoot || !g_pChainRoot->pNext)
	{
		dbg_warning("cannot save empty chain!\r\n");
		return;
	}

	if(!(g_fSDStatus & SD_STATUS_READY))
	{
		dbg_warning("SD card not ready\r\n");
		return;
	}

	FIL fh;
	if((res = f_open(&fh, pszPath, FA_CREATE_ALWAYS | FA_WRITE)))
	{
		dbg_warning("f_open(%s) failed %d\r\n", pszPath, res);
		return;
	}

	// Reserve space for header
	f_lseek(&fh, sizeof(ChainStoreHeader_t));

	const ChainStageHeader_t *pStageHdr = g_pChainRoot;
	uint8_t nStages = 0;

	while(pStageHdr->pNext)
	{
		dbg_printf("Saving stage #%d...\r\n", nStages);
		ChainStoreStageHeader_t stageHdr;
		stageHdr.nBranches = pStageHdr->nBranches;

		if((res = f_write(&fh, &stageHdr, sizeof(stageHdr), &nWrote)))
		{
			dbg_warning("stage header write failed %d\r\n", res);
			return;
		}

		const StageBranch_t *pBranch = pStageHdr->pFirst;

		while(pBranch)
		{
			chainstore_save_branch(&fh, pBranch);
			pBranch = pBranch->pNext;
		}

		nStages++;
		pStageHdr = pStageHdr->pNext;
	}

	// Write header
	f_lseek(&fh, 0);

	ChainStoreHeader_t hdr;
	hdr.ident = STORE_IDENT;
	hdr.iVersion = STORE_VERSION;
	hdr.nStages = nStages;

	if((res = f_write(&fh, &hdr, sizeof(hdr), &nWrote)))
	{
		dbg_warning("header write failed %d\r\n", res);
		return;
	}

	f_close(&fh);

	dbg_printf(ANSI_COLOR_GREEN "Saved chain (%d stages) to \"%s\"\r\n" ANSI_COLOR_RESET, nStages, pszPath);
}


void chainstore_restore(const char *pszPath)
{
	FRESULT res;
	UINT nRead;

	FIL fh;
	if((res = f_open(&fh, pszPath, FA_READ)))
	{
		dbg_warning("f_open failed %d\r\n", res);
		return;
	}

	// Read store header
	ChainStoreHeader_t hdr;
	if((res = f_read(&fh, &hdr, sizeof(hdr), &nRead)) || nRead != sizeof(hdr))
	{
		dbg_warning("header read failed %d\r\n", res);
		return;
	}

	if(hdr.ident != STORE_IDENT)
	{
		dbg_warning("invalid ident (%.4s)\r\n", (const char *)&hdr.ident);
		return;
	}

	if(hdr.iVersion != STORE_VERSION)
	{
		dbg_warning("invalid version (%d), expected %d\r\n", hdr.iVersion, STORE_VERSION);
		return;
	}

	// Deallocate current chain
	chain_free();
	g_pChainRoot = stage_alloc();

	ChainStageHeader_t *pStageHdr = g_pChainRoot;

	for(uint8_t i = 0; i < hdr.nStages; ++i)
	{
		// Read stage header
		ChainStoreStageHeader_t storeStageHdr;
		if((res = f_read(&fh, &storeStageHdr, sizeof(storeStageHdr), &nRead)) || nRead != sizeof(storeStageHdr))
		{
			dbg_warning("stage header read failed %d\r\n", res);
			return;
		}

		StageBranch_t *pLastBranch = NULL;

		for(uint8_t j = 0; j < storeStageHdr.nBranches; ++j)
		{
			// Read branch header
			ChainStoreBranchHeader_t storeBranchHdr;
			if((res = f_read(&fh, &storeBranchHdr, sizeof(storeBranchHdr), &nRead)) || nRead != sizeof(storeBranchHdr))
			{
				dbg_warning("branch header read failed %d\r\n", res);
				return;
			}

			uint8_t *pUnknown;
			StageBranch_t *pBranch = branch_alloc(storeBranchHdr.filter, storeBranchHdr.flags, storeBranchHdr.flMixPerc, (void **)&pUnknown);

			for(uint8_t k = 0; k < storeBranchHdr.nParams; ++k)
			{
				// Read parameter data
				ChainStoreParam_t storeParam;
				if((res = f_read(&fh, &storeParam, sizeof(storeParam), &nRead)) || nRead != sizeof(storeParam))
				{
					dbg_warning("parameter header read failed %d\r\n", res);
					return;
				}

				// Check offset
				if(storeParam.iOffset < pBranch->pFilter->nNonPublicDataSize || storeParam.iOffset + storeParam.nSize > pBranch->pFilter->nFilterDataSize)
				{
					dbg_warning("invalid offset/size for parameter data\r\n");
					return;
				}

				// Read parameter data into memory
				if((res = f_read(&fh, &pUnknown[storeParam.iOffset], storeParam.nSize, &nRead)) || nRead != storeParam.nSize)
				{
					dbg_warning("parameter data read failed %d\r\n", res);
					return;
				}
			}

			// Trigger filter modified
			if(pBranch->pFilter->pfnModCallback)
				pBranch->pFilter->pfnModCallback(pBranch->pUnknown);

			if(pLastBranch)
				pLastBranch = pLastBranch->pNext = pBranch;
			else
			{
				pStageHdr->pFirst = pBranch;
				pLastBranch = pBranch;
			}
		}

		// Allocate the next stage
		pStageHdr = pStageHdr->pNext = stage_alloc();
	}

	f_close(&fh);

	dbg_printf(ANSI_COLOR_GREEN "Restored chain from \"%s\"\r\n" ANSI_COLOR_RESET, pszPath);
}
