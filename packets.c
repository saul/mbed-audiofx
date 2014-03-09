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
#include "packets.h"
#include "chain.h"
#include "samples.h"
#ifdef INDIVIDUAL_BUILD_SAUL
#	include "chainstore.h"
#endif


const char *g_ppszPacketTypes[] = {
	"A2A_PROBE",
	"U2B_RESET",
	"B2U_PRINT",
	"B2U_FILTER_LIST",
	"U2B_FILTER_CREATE",
	"U2B_FILTER_DELETE",
	"U2B_FILTER_FLAG",
	"U2B_FILTER_MOD",
	"U2B_FILTER_MIX",
	"U2B_VOLUME",
	"U2B_ARB_CMD",
#ifdef INDIVIDUAL_BUILD_TOM
	"B2U_ANALOG_CONTROL",
#endif // INDIVIDUAL_BUILD_TOM
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
	{packet_filter_mix_receive, true, PACKET_SIZE_EXACT(sizeof(FilterMixPacket_t))}, // U2B_FILTER_MIX
	{packet_volume_receive, false, PACKET_SIZE_EXACT(sizeof(VolumePacket_t))}, // U2B_VOLUME
	{packet_cmd_receive, true, PACKET_SIZE_MIN(sizeof(CommandPacket_t))}, // U2B_ARB_CMD
#ifdef INDIVIDUAL_BUILD_TOM
	{NULL, false, 0}, // B2U_ANALOG_CONTROL
#endif // INDIVIDUAL_BUILD_TOM
};


static bool s_bDebugPacketReceipt = true;
static bool s_bDebugChainAfterLock = true;


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


#ifdef INDIVIDUAL_BUILD_TOM
void packet_analog_control_send(uint16_t analog_value)
{
	sercom_send(B2U_ANALOG_CONTROL, (const uint8_t *)&analog_value, sizeof(analog_value));
}
#endif // INDIVIDUAL_BUILD_TOM


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

	const PacketHandler_t *pHandler = &g_pPacketHandlers[hdr.type];
	if(!pHandler->pfnCallback)
	{
		dbg_warning("received packet (%s) that has no handler!\r\n", g_ppszPacketTypes[hdr.type]);
		free(pPayload);
		return;
	}

	// Check packet payload size
	uint16_t nPacketSize = pHandler->nPacketSize & ~PACKET_SIZE_COMPARATOR_BIT;

	if((pHandler->nPacketSize & PACKET_SIZE_COMPARATOR_BIT) && hdr.size < nPacketSize)
	{
		dbg_warning("received packet (%s) with invalid size: got %u bytes, expected at least %u bytes\r\n", g_ppszPacketTypes[hdr.type], hdr.size, nPacketSize);
		return;
	}
	else if(!(pHandler->nPacketSize & PACKET_SIZE_COMPARATOR_BIT) && hdr.size != nPacketSize)
	{
		dbg_warning("received packet (%s) with invalid size: got %u, expected exactly %u bytes\r\n", g_ppszPacketTypes[hdr.type], hdr.size, nPacketSize);
		return;
	}

	if(s_bDebugPacketReceipt)
		dbg_printf("Received packet %u(%s) with size %u bytes\r\n", hdr.type, g_ppszPacketTypes[hdr.type], hdr.size);

	// Lock the chain if the callback requires it
	if(pHandler->bLocksChain)
	{
		dbg_assert(!g_bChainLock, "chain already locked");
		g_bChainLock = true;
	}

	pHandler->pfnCallback(&hdr, pPayload);

	// Unlock the chain
	if(pHandler->bLocksChain)
	{
		g_bChainLock = false;

		if(s_bDebugChainAfterLock)
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

	ChainStageHeader_t *pStageHdr = chain_get_stage(pFilterCreate->nStage);
	if(!pStageHdr)
		return;

	// Create the branch
	dbg_printf("Creating %u(%s) filter...", pFilterCreate->iFilterType, g_pFilters[pFilterCreate->iFilterType].pszName);
	StageBranch_t *pBranch = branch_alloc(pFilterCreate->iFilterType, pFilterCreate->flags, pFilterCreate->flMixPerc, NULL);
	dbg_printf(" ok!\r\n");

	// Add branch to stage
	pStageHdr->nBranches++;

	if(pStageHdr->pFirst)
	{
		// Grab the last branch in this stage
		StageBranch_t *pLast = stage_get_branch(pStageHdr, pStageHdr->nBranches - 2);

		// Add new branch to end of stage
		pLast->pNext = pBranch;
	}
	else
	{
		// Add branch as only stage
		pStageHdr->pFirst = pBranch;

		// Create a new empty stage
		pStageHdr->pNext = stage_alloc();
	}

	// Call creation callback
	if(pBranch->pFilter->pfnCreateCallback)
		pBranch->pFilter->pfnCreateCallback((void *)pBranch->pUnknown);
	else
		dbg_warning("filter has no creation callback, UI/board data may be out of sync!\r\n");
}


void packet_filter_delete_receive(const PacketHeader_t *pHdr, const uint8_t *pPayload)
{
	const FilterDeletePacket_t *pFilterDelete = (FilterDeletePacket_t *)pPayload;

	ChainStageHeader_t *pStageHdr = chain_get_stage(pFilterDelete->nStage);
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
		ChainStageHeader_t *pPrevStageHdr = chain_get_stage(pFilterDelete->nStage - 1);
		pPrevStageHdr->pNext = pStageHdr->pNext;
	}

	// Free stage
	stage_free(pStageHdr);
}


void packet_filter_flag_receive(const PacketHeader_t *pHdr, const uint8_t *pPayload)
{
	const FilterFlagPacket_t *pFilterFlag = (FilterFlagPacket_t *)pPayload;

	ChainStageHeader_t *pStageHdr = chain_get_stage(pFilterFlag->nStage);
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

	ChainStageHeader_t *pStageHdr = chain_get_stage(pFilterMod->nStage);
	if(!pStageHdr)
		return;

	StageBranch_t *pBranch = stage_get_branch(pStageHdr, pFilterMod->nBranch);
	if(!pBranch)
		return;

	const uint8_t *pSource = pPayload + sizeof(FilterModPacket_t);
	uint16_t nToCopy = pHdr->size - sizeof(FilterModPacket_t);
	uint8_t *pDest = ((uint8_t *)pBranch->pUnknown) + pFilterMod->iOffset + pBranch->pFilter->nNonPublicDataSize;

	// Buffer overflow protection
	if(pFilterMod->iOffset + nToCopy > pBranch->pFilter->nFilterDataSize)
	{
		dbg_warning("blocked attempted arbitrary memory modification\r\n");
		return;
	}

	// Copy new parameter value into memory
	memcpy(pDest, pSource, nToCopy);

	// Call modification callback
	if(pBranch->pFilter->pfnModCallback)
		pBranch->pFilter->pfnModCallback((void *)pBranch->pUnknown);

	// Reset last slow tick. This makes sure we reissue the "slow tick" warning
	// if the chain is still too complex.
	extern uint32_t g_ulLastLongTick;
	g_ulLastLongTick = 0;
}


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdouble-promotion"
void packet_filter_mix_receive(const PacketHeader_t *pHdr, const uint8_t *pPayload)
{
	const FilterMixPacket_t *pFilterMix = (FilterMixPacket_t *)pPayload;

	ChainStageHeader_t *pStageHdr = chain_get_stage(pFilterMix->nStage);
	if(!pStageHdr)
		return;

	StageBranch_t *pBranch = stage_get_branch(pStageHdr, pFilterMix->nBranch);
	if(!pBranch)
		return;

	if(pFilterMix->flMixPerc < 0.0f || pFilterMix->flMixPerc > 2.0f)
	{
		dbg_warning("mix perc (%.2f) not in range [0..2]\r\n", pFilterMix->flMixPerc);
		return;
	}

	pBranch->flMixPerc = pFilterMix->flMixPerc;
}
#pragma GCC diagnostic push


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdouble-promotion"
void packet_volume_receive(const PacketHeader_t *pHdr, const uint8_t *pPayload)
{
	const VolumePacket_t *pVolume = (VolumePacket_t *)pPayload;

	if(pVolume->flVolume < 0.0f || pVolume->flVolume > 2.0f)
	{
		dbg_warning("master volume (%.2f) not in range [0..2]\r\n", pVolume->flVolume);
		return;
	}

	g_flChainVolume = pVolume->flVolume;
}
#pragma GCC diagnostic pop


static size_t strnlen(register const char *s, size_t maxlen)
{
  register const char *e;
  size_t n;

  for (e = s, n = 0; *e && n < maxlen; e++, n++)
    ;

  return n;
}


void packet_cmd_receive(const PacketHeader_t *pHdr, const uint8_t *pPayload)
{
	const CommandPacket_t *pCmd = (CommandPacket_t *)pPayload;

	uint16_t nBytesLeft = pHdr->size - sizeof(CommandPacket_t);

	if(!nBytesLeft)
		return;

	const char *pszArg = (const char *)(pCmd + 1);
	const char **ppszArgs = calloc(pCmd->nArgs, sizeof(char *));
	dbg_assert(ppszArgs, "unable to allocate memory for command");

	dbg_printn("\r\n>>> ", 6);

	for(uint8_t i = 0; i < pCmd->nArgs; ++i)
	{
		size_t nLen = strnlen(pszArg, nBytesLeft);
		nBytesLeft -= nLen;

		if(nBytesLeft == 0)
		{
			dbg_warning("argument list missing NUL terminator\r\n");
			goto cleanup;
		}

		dbg_printf("%s ", pszArg);

		ppszArgs[i] = pszArg;
		pszArg += (nLen + 1);
	}

	dbg_printn("\r\n", 2);

	if(nBytesLeft != pCmd->nArgs)
	{
		dbg_warning("%u unexpected bytes left after parse\r\n", nBytesLeft - pCmd->nArgs);
		goto cleanup;
	}

	if(!strcmp(ppszArgs[0], "chain_debug"))
	{
		chain_debug();
	}
	else if(!strcmp(ppszArgs[0], "filter_debug"))
	{
		filter_debug();
	}
	else if(!strcmp(ppszArgs[0], "stage_debug"))
	{
		if(pCmd->nArgs != 2)
		{
			dbg_warning("syntax: <stage>\r\n");
			goto cleanup;
		}

		ChainStageHeader_t *pStageHdr = chain_get_stage(atoi(ppszArgs[1]));

		if(pStageHdr)
			stage_debug(pStageHdr);
	}
	else if(!strcmp(ppszArgs[0], "bDebugPacketReceipt"))
	{
		if(pCmd->nArgs != 2)
			dbg_printf("bDebugPacketReceipt = %s\r\n", s_bDebugPacketReceipt ? "true" : "false");
		else
			s_bDebugPacketReceipt = atoi(ppszArgs[1]);
	}
	else if(!strcmp(ppszArgs[0], "bDebugChainAfterLock"))
	{
		if(pCmd->nArgs != 2)
			dbg_printf("bDebugChainAfterLock = %s\r\n", s_bDebugChainAfterLock ? "true" : "false");
		else
			s_bDebugChainAfterLock = atoi(ppszArgs[1]);
	}
	else if(!strcmp(ppszArgs[0], "tom"))
	{
		dbg_printf("                  _..._\r\n");dbg_printf("               .-'     '-.\r\n");dbg_printf("              /     _    _\\\r\n");dbg_printf("             /':.  (o)  /__)\r\n");dbg_printf("            /':. .,_    |  |\r\n");dbg_printf("           |': ; /  \\   /_/\r\n");dbg_printf("           /  ;  `\"`\"    }\r\n");dbg_printf("          ; ':.,         {\r\n");dbg_printf("         /      ;        }\r\n");dbg_printf("        ; '::.   ;\\/\\ /\\ {\r\n");dbg_printf("       |.      ':. ;``\"``\\\r\n");dbg_printf("      / '::'::'    /      ;\r\n");dbg_printf("     |':::' '::'  /       |\r\n");dbg_printf("     \\   '::' _.-`;       ;\r\n");dbg_printf("     /`-..--;` ;  |       |\r\n");dbg_printf("    ;  ;  ;  ;  ; |       |\r\n");dbg_printf("    ; ;  ;  ; ;  ;        /        ,--.........,\r\n");dbg_printf("    |; ;  ;  ;  ;/       ;       .'           -='.\r\n");dbg_printf("    | ;  ;  ; ; /       /       .\\               '\r\n");dbg_printf("    |  ;   ;  /`      .\\   _,==\"  \\             .'\r\n");dbg_printf("    \\;  ; ; .'. _  ,_'\\.\\~\"   //`. \\          .'\r\n");dbg_printf("    |  ;  .___~' \\ \\- | |    /,\\ `  \\      ..'\r\n");dbg_printf("  ~ ; ; ;/  ==\"'' |`| | |       ==\"''\\.==''\r\n");dbg_printf("  ~ /; ;/=\"\"      |`| |`|   ===\"`\r\n");dbg_printf("  ~..==`     \\\\   |`| / /==\"`\r\n");dbg_printf("   ~` ~      /,\\ / /= )\")\r\n");dbg_printf("  ~ ~~         _')\")\r\n");dbg_printf("  ~ ~   _,=~\";`\r\n");dbg_printf("  ~  =~\"|;  ;|\r\n");dbg_printf("   ~  ~ | ;  |\r\n");dbg_printf("~ ~     |;|\\ |\r\n");dbg_printf("        |/  \\|\r\n");
	}
	else if(!strcmp(ppszArgs[0], "volume"))
	{
		if(pCmd->nArgs != 2)
			dbg_printf("volume = %.2f\r\n", g_flChainVolume);
		else
			g_flChainVolume = atof(ppszArgs[1]);
	}
	else if(!strcmp(ppszArgs[0], "average"))
	{
		if(pCmd->nArgs != 2)
		{
			dbg_warning("syntax: <samples>\r\n");
			goto cleanup;
		}

		uint32_t iAverage = sample_get_average(atoi(ppszArgs[1]));
		float flVolume = (iAverage * 100.0) / (((uint32_t)-1) >> 1);
		dbg_printf("average = %.2f%%\r\n", flVolume);
	}
	else if(!strcmp(ppszArgs[0], "ping"))
	{
		dbg_printf("Pong!\r\n");
	}
#ifdef INDIVIDUAL_BUILD_SAUL
	else if(!strcmp(ppszArgs[0], "chain_save"))
	{
		if(pCmd->nArgs != 2)
		{
			dbg_warning("syntax: <path>\r\n");
			goto cleanup;
		}

		chainstore_save(ppszArgs[1]);
	}
	else if(!strcmp(ppszArgs[0], "chain_restore"))
	{
		if(pCmd->nArgs != 2)
		{
			dbg_warning("syntax: <path>\r\n");
			goto cleanup;
		}

		chainstore_restore(ppszArgs[1]);
	}
#endif
	else
		dbg_warning("unknown command\r\n");

cleanup:
	free(ppszArgs);
}
