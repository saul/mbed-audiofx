#include <string.h>
#include <stdint.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-pedantic"
#	include "LPC17xx.h"
#	include "lpc_types.h"
#pragma GCC diagnostic pop

#include "sercom.h"
#include "filters.h"
#include "bytebuffer.h"
#include "dbg.h"
#include "ticktime.h"


const char *g_ppszPacketTypes[] = {
	"A2A_PROBE",
	"U2B_RESET",
	"B2U_PRINT",
	"B2U_FILTER_LIST",
	"U2B_FILTER_CREATE",
	"U2B_FILTER_DELETE",
	"U2B_FILTER_FLAG",
	"U2B_FILTER_MOD",
};


PacketHandler_t g_pPacketHandlers[] = {
	{packet_reset_receive, false, PACKET_SIZE_EXACT(0)}, // A2A_PROBE
	{packet_reset_receive, false, PACKET_SIZE_EXACT(0)}, // U2B_RESET
	{NULL, false, 0}, // B2U_PRINT
	{NULL, false, 0}, // B2U_FILTER_LIST
	{packet_filter_create_receive, true, PACKET_SIZE_EXACT(sizeof(FilterCreatePacket_t))}, // U2B_FILTER_CREATE
	{packet_filter_delete_receive, true, PACKET_SIZE_EXACT(sizeof(FilterDeletePacket_t))}, // U2B_FILTER_DELETE
	{packet_filter_flag_receive, true, PACKET_SIZE_EXACT(sizeof(FilterFlagPacket_t))}, // U2B_FILTER_FLAG
	{packet_filter_mod_receive, true, PACKET_SIZE_MIN(sizeof(FilterModPacket_t))}, // U2B_FILTER_MOD
};


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-pedantic"
void packet_static_assertions(void)
{
	_Static_assert(sizeof(g_ppszPacketTypes)/sizeof(g_ppszPacketTypes[0]) == PACKET_TYPE_MAX, "g_ppszPacketTypes size does not match number of packets");
	_Static_assert(sizeof(g_pPacketHandlers)/sizeof(g_pPacketHandlers[0]) == PACKET_TYPE_MAX, "g_pPacketHandlers size does not match number of packets");
}
#pragma GCC diagnostic pop


void packet_probe_send(void)
{
	sercom_send(A2A_PROBE, NULL, 0);
}


void packet_reset_wait(void)
{
	PacketHeader_t hdr;
	uint8_t *pPayload;

	for(;;)
	{
		if(!sercom_receive(&hdr, &pPayload))
			continue;

		// If we receive a probe or reset packet, reset the board
		if(hdr.type == U2B_RESET || hdr.type == A2A_PROBE)
			packet_reset_receive(&hdr, pPayload);

		free(pPayload);
	}
}


void packet_print_send(const char *pszLine, size_t size)
{
	sercom_send(B2U_PRINT, (const uint8_t *)pszLine, size);
}


void packet_filter_list_send(void)
{
	byte_buffer *buf = bb_new_default(true);
	bb_put(buf, NUM_FILTERS);

	for(size_t i = 0; i < NUM_FILTERS; ++i)
	{
		const Filter_t *pFilter = &g_pFilters[i];
		bb_put_string(buf, pFilter->pszName);
		bb_put_string(buf, pFilter->pszParamFormat);
	}

	sercom_send(B2U_FILTER_LIST, buf->buf, buf->pos);
	bb_free(buf);
}


void packet_loop(void)
{
	PacketHeader_t hdr;
	uint8_t *pPayload;

	if(!sercom_receive(&hdr, &pPayload))
		return;

	PacketHandler_t *pHandler = &g_pPacketHandlers[hdr.type];
	if(!pHandler->pfnCallback)
	{
		dbg_warning("Received packet (%s) that has no handler!\r\n", g_ppszPacketTypes[hdr.type]);
		free(pPayload);
		return;
	}

	// Check packet payload size
	if(((pHandler->nPacketSize & PACKET_SIZE_COMPARATOR_BIT) && hdr.size < pHandler->nPacketSize))
	{
		dbg_warning("Received packet (%s) with invalid size: got %u bytes, expected at least %u bytes\r\n", g_ppszPacketTypes[hdr.type], hdr.size, pHandler->nPacketSize);
		return;
	}
	else if((!(pHandler->nPacketSize & PACKET_SIZE_COMPARATOR_BIT) && hdr.size != pHandler->nPacketSize))
	{
		dbg_warning("Received packet (%s) with invalid size: got %u, expected exactly %u bytes\r\n", g_ppszPacketTypes[hdr.type], hdr.size, pHandler->nPacketSize);
		return;
	}

	// Lock the chain if the callback requires it
	if(pHandler->bLocksChain)
	{
		dbg_assert(!g_bChainLock, "chain already locked");
		g_bChainLock = true;
	}

	(*pHandler)(&hdr, pPayload);

	// Unlock the chain
	if(pHandler->bLocksChain)
	{
		g_bChainLock = false;
		chain_debug();
	}

	free(pPayload);
}


void packet_reset_receive(const PacketHeader_t *pHdr, const uint8_t *pPayload)
{
	dbg_printf("Received %s packet, system rebooting in 1 second...\r\n", g_ppszPacketTypes[pHdr->type]);
	time_sleep(1000);

	NVIC_SystemReset();
}


void packet_filter_create_receive(const PacketHeader_t *pHdr, const uint8_t *pPayload)
{
	const FilterCreatePacket_t *pFilterCreate = (FilterCreatePacket_t *)pPayload;

	ChainStageHeader_t *pStageHdr = chain_get_stage(g_pChainRoot, pFilterCreate->nStage);
	if(!pStageHdr)
		return;

	// Create the branch
	dbg_printf("Creating %s filter...", pFilterCreate->iFilterType);
	BranchStage_t *pBranch = branch_alloc(pFilterCreate->iFilterType, flags, flMixPerc, NULL);
	dbg_printf(" ok!\r\n");

	// Add branch to stage
	pStageHdr->nBranches++;

	if(pStageHdr->pFirst)
	{
		// Grab the last branch in this stage
		BranchStage_t *pLast = stage_get_branch(pStageHdr, pStageHdr->nBranches - 2);

		// Add new branch to end of stage
		pCurrent->pNext = pBranch;
	}
	else
	{
		// Add branch as only stage
		pStageHdr->pFirst = pBranch;

		// Create a new empty stage
		pStageHdr->pNext = stage_alloc();
	}
}


void packet_filter_delete_receive(const PacketHeader_t *pHdr, const uint8_t *pPayload)
{
	const FilterDeletePacket_t *pFilterDelete = (FilterDeletePacket_t *)pPayload;

	ChainStageHeader_t *pStageHdr = chain_get_stage(g_pChainRoot, pFilterDelete->nStage);
	if(!pStageHdr)
		return;

	StageBranch_t *pBranch = stage_get_branch(pStageHdr, pFilterDelete->nBranch);
	if(!pBranch)
		return;

	// Unlink branch from linked-list
	if(pFilterDelete->nBranch == 0)
	{
		pStageHdr->pFirst = pBranch->pNext;
	}
	else
	{
		StageBranch_t *pPrevBranch = stage_get_branch(pStageHdr, pFilterDelete->nBranch - 1);
		pPrevBranch->pNext = pBranch->pNext;
	}

	// Free branch
	branch_free(pBranch);

	pStageHdr->nBranches--;

	// If we've removed all branches from this stage, delete the stage
	// Don't delete the stage if it's the last one
	if(pStageHdr->nBranches > 0 || !pStageHdr->pNext)
		return;

	// Unlink stage from chain
	if(pFilterDelete->nStage == 0)
	{
		g_pChainRoot = pStageHdr->pNext;
	}
	else
	{
		ChainStageHeader_t *pPrevStageHdr = chain_get_stage(g_pChainRoot, pFilterDelete->nStage - 1);
		pPrevStageHdr->pNext = pStageHdr->pNext;
	}

	// Free stage
	stage_free(pStageHdr);
}


void packet_filter_flag_receive(const PacketHeader_t *pHdr, const uint8_t *pPayload)
{
	const FilterFlagPacket_t *pFilterFlag = (FilterFlagPacket_t *)pPayload;

	ChainStageHeader_t *pStageHdr = chain_get_stage(g_pChainRoot, pFilterFlag->nStage);
	if(!pStageHdr)
		return;

	StageBranch_t *pBranch = stage_get_branch(pStageHdr, pFilterFlag->nBranch);
	if(!pBranch)
		return;

	// Toggle branch flag
	if(pFilterFlag->bEnable)
		pBranch->flags |= (1<<pFilterFlag->iBit);
	else
		pBranch->flags &= ~(1<<pFilterFlag->iBit);
}


void packet_filter_mod_receive(const PacketHeader_t *pHdr, const uint8_t *pPayload)
{
	const FilterModPacket_t *pFilterMod = (FilterModPacket_t *)pPayload;

	ChainStageHeader_t *pStageHdr = chain_get_stage(g_pChainRoot, pFilterMod->nStage);
	if(!pStageHdr)
		return;

	StageBranch_t *pBranch = stage_get_branch(pStageHdr, pFilterMod->nBranch);
	if(!pBranch)
		return;

	uint8_t *pSource = pPayload + sizeof(FilterModPacket_t);
	uint8_t nToCopy = pHdr->size - sizeof(FilterModPacket_t);
	uint8_t *pDest = ((uint8_t *)pFilter->pPrivate) + iOffset;

	// Buffer overflow protection
	if(iOffset + nToCopy > pBranch->pFilter->nPrivateDataSize)
	{
		dbg_warning("blocked attempted arbitrary memory modification");
		return;
	}

	// Copy new parameter value into memory
	memcpy(pDest, pSource, nToCopy);
}
