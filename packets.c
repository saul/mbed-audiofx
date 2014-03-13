/*
 *	HAPR Project 2014
 *	Group 6 - Tom Bryant (TB) & Saul Rennison (SR)
 *
 *	File created by:	SR
 *	File modified by:	TB & SR
 *	File debugged by:	TB & SR
 *
 * packets.c - Packet receipt and transmission
 *
 * Defines several enumerations and functions to handle sending and receiving
 * packets via the sercom subsystem.
 */

#include <string.h>
#include <stdint.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-pedantic"
#	include "LPC17xx.h"
#	include "lpc_types.h"
#	include "lpc17xx_uart.h"
#pragma GCC diagnostic pop

#include "sercom.h"
#include "filters.h"
#include "bytebuffer.h"
#include "dbg.h"
#include "ticktime.h"
#include "packets.h"
#include "chain.h"
#include "samples.h"
#include "config.h"
#ifdef INDIVIDUAL_BUILD_SAUL
#	include "chainstore.h"
#	include "fatfs/ff.h"
#endif


/*
 * g_ppszPacketTypes
 *
 * String representations of each enum value in PacketType_e
 */
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
	"U2B_ARB_CMD",
#ifdef INDIVIDUAL_BUILD_TOM
	"B2U_ANALOG_CONTROL",
#endif
#ifdef INDIVIDUAL_BUILD_SAUL
	"B2U_STORED_LIST",
	"B2U_CHAIN_BLOB",
#endif
};


/*
 * PacketHandler_t
 *
 * Inbound packet handlers
 */
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
	{packet_cmd_receive, true, PACKET_SIZE_MIN(sizeof(CommandPacket_t))}, // U2B_ARB_CMD
#ifdef INDIVIDUAL_BUILD_TOM
	{NULL, false, 0}, // B2U_ANALOG_CONTROL
#endif
#ifdef INDIVIDUAL_BUILD_SAUL
	{NULL, false, 0}, // B2U_STORED_LIST
	{NULL, false, 0}, // B2U_CHAIN_BLOB
#endif
};

// Should each receipt of each packet be debugged?
static bool s_bDebugPacketReceipt = false;

// Should the chain be debugged to console after the chain is unlocked?
static bool s_bDebugChainAfterLock = false;


/*
 * packet_static_assertions
 *
 * Checks that g_ppszPacketTypes, g_pPacketHandlers and PacketType_e match in
 * size at compile time.
 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-pedantic"
void packet_static_assertions(void)
{
	_Static_assert(sizeof(g_ppszPacketTypes)/sizeof(g_ppszPacketTypes[0]) == PACKET_TYPE_MAX, "g_ppszPacketTypes size does not match number of packets");
	_Static_assert(sizeof(g_pPacketHandlers)/sizeof(g_pPacketHandlers[0]) == PACKET_TYPE_MAX, "g_pPacketHandlers size does not match number of packets");
}
#pragma GCC diagnostic pop


/*
 * packet_probe_send
 *
 * Sends a probe packet to the UI. The board should respond with a probe, and
 * then startup proceeds.
 */
void packet_probe_send(void)
{
	sercom_send(A2A_PROBE, NULL, 0);
}


/*
 * packet_print_send
 *
 * Sends a print packet to the UI.
 */
void packet_print_send(const char *pszLine, size_t size)
{
	sercom_send(B2U_PRINT, (const uint8_t *)pszLine, size);
}


/*
 * packet_analog_control_send
 *
 * Sends an analogue control packet to the UI.
 */
#ifdef INDIVIDUAL_BUILD_TOM
void packet_analog_control_send(uint16_t analog_value)
{
	sercom_send(B2U_ANALOG_CONTROL, (const uint8_t *)&analog_value, sizeof(analog_value));
}
#endif


/*
 * packet_filter_list_send
 *
 * Send the filter list and parameter format strings to the UI.
 */
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


/*
 * packet_stored_list_send
 *
 * Send all files in the chains/ directory on the SD card to the UI.
 */
#ifdef INDIVIDUAL_BUILD_SAUL
void packet_stored_list_send(void)
{
	FRESULT res;
	DIR dir;

	// Open the store directory in the SD card
	if((res = f_opendir(&dir, STORE_DIRECTORY)))
	{
		dbg_warning("f_opendir failed %d\r\n", res);
		return;
	}

	// Create a dynamic buffer
	byte_buffer *buf = bb_new_default(true);

	for(;;)
	{
		FILINFO fno;

		// Break on error
		if((res = f_readdir(&dir, &fno)))
		{
			dbg_warning("f_readdir failed %d\r\n", res);
			break;
		}

		// Break on end of dir
		if(!fno.fname[0])
			break;

		// Ignore dot entry
		if(fno.fname[0] == '.')
			continue;

		// Skip directories
		if(fno.fattrib & AM_DIR)
			continue;

		// Write file name and NOT the extension
		for(const char *pszName = fno.fname; *pszName && *pszName != '.'; pszName++)
			bb_put(buf, *pszName);

		bb_put(buf, 0);
	}

	// Send packet
	sercom_send(B2U_STORED_LIST, buf->buf, buf->pos);

	// Cleanup
	f_closedir(&dir);
	bb_free(buf);
}
#endif


/*
 * packet_chain_blob_send
 *
 * Send the a stored filter chain whole to the UI for it to parse.
 */
#ifdef INDIVIDUAL_BUILD_SAUL
void packet_chain_blob_send(const char *pszPath)
{
	FRESULT res;
	UINT nRead;

	FIL fh;
	if((res = f_open(&fh, pszPath, FA_READ)))
	{
		dbg_warning("f_open(%s) failed %d\r\n", pszPath, res);
		return;
	}

	// Get file size
	DWORD size = f_size(&fh);

	// We manually send the packet data instead of using sercom_send as we send
	// the file in chunks of 128 bytes

	PacketHeader_t packetHdr = {
		.ident=PACKET_IDENT,
		.type=B2U_CHAIN_BLOB,
		.size=size
	};

	// Wait until UART is unlocked
	while(g_bUARTLock);
	g_bUARTLock = true;

	// Send packet header
	UART_Send((LPC_UART_TypeDef *)LPC_UART0, (uint8_t *)&packetHdr, sizeof(packetHdr), BLOCKING);

	// Read store header
	ChainStoreHeader_t storeHdr;
	if((res = f_read(&fh, &storeHdr, sizeof(storeHdr), &nRead)) || nRead != sizeof(storeHdr))
	{
		dbg_warning("header read failed %d\r\n", res);

		g_bUARTLock = false;
		return;
	}

	if(!chainstore_header_validate(&storeHdr))
		goto error;

	// Write header
	UART_Send((LPC_UART_TypeDef *)LPC_UART0, (uint8_t *)&storeHdr, sizeof(storeHdr), BLOCKING);

	// Write remaining file data
	uint8_t pData[128];

	do
	{
		if((res = f_read(&fh, pData, sizeof(pData), &nRead)))
		{
			dbg_warning("file read failed %d\r\n", res);
			goto error;
		}

		// Write file data to UART
		UART_Send((LPC_UART_TypeDef *)LPC_UART0, pData, nRead, BLOCKING);
	}
	while(nRead > 0);

	// Unlock UART and close file
error:
	g_bUARTLock = false;
	f_close(&fh);
}
#endif


/*
 * packet_loop
 *
 * Called by the main loop to process any inbound packets. Checks their
 * validity and calls the appropriate packet receipt callback.
 */
void packet_loop(void)
{
	uint8_t *pPayload = NULL;
	PacketHeader_t *pHdr = sercom_receive_nonblock(&pPayload);

	if(!pHdr)
		return;

	const PacketHandler_t *pHandler = &g_pPacketHandlers[pHdr->type];
	if(!pHandler->pfnCallback)
	{
		dbg_warning("received packet (%s) that has no handler!\r\n", g_ppszPacketTypes[pHdr->type]);
		goto error;
	}

	// Check packet payload size
	uint16_t nPacketSize = pHandler->nPacketSize & ~PACKET_SIZE_COMPARATOR_BIT;

	if((pHandler->nPacketSize & PACKET_SIZE_COMPARATOR_BIT) && pHdr->size < nPacketSize)
	{
		dbg_warning("received packet (%s) with invalid size: got %u bytes, expected at least %u bytes\r\n", g_ppszPacketTypes[pHdr->type], pHdr->size, nPacketSize);
		goto error;
	}
	else if(!(pHandler->nPacketSize & PACKET_SIZE_COMPARATOR_BIT) && pHdr->size != nPacketSize)
	{
		dbg_warning("received packet (%s) with invalid size: got %u, expected exactly %u bytes\r\n", g_ppszPacketTypes[pHdr->type], pHdr->size, nPacketSize);
		goto error;
	}

	if(s_bDebugPacketReceipt)
		dbg_printf("Received packet %u(%s) with size %u bytes\r\n", pHdr->type, g_ppszPacketTypes[pHdr->type], pHdr->size);

	// Lock the chain if the callback requires it
	if(pHandler->bLocksChain)
	{
		dbg_assert(!g_bChainLock, "chain already locked");
		g_bChainLock = true;
	}

	pHandler->pfnCallback(pHdr, pPayload);

	// Unlock the chain
	if(pHandler->bLocksChain)
	{
		g_bChainLock = false;

		if(s_bDebugChainAfterLock)
			chain_debug();
	}

error:
	free(pPayload);
}


/*
 * packet_reset_receive
 *
 * Called on receipt of U2B_RESET. Resets board after 1 second.
 */
void packet_reset_receive(const PacketHeader_t *pHdr, const uint8_t *pPayload)
{
	dbg_printf("Received %s packet, system rebooting in 1 second...\r\n", g_ppszPacketTypes[pHdr->type]);
	time_sleep(1000);

	NVIC_SystemReset();
}


/*
 * packet_filter_create_receive
 *
 * Called on receipt of U2B_FILTER_CREATE. Creates a new branch in a specific
 * stage.
 */
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


/*
 * packet_filter_delete_receive
 *
 * Called on receipt of U2B_FILTER_DELETE. Deletes a branch in a specific
 * stage.
 */
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


/*
 * packet_filter_flag_receive
 *
 * Called on receipt of U2B_FILTER_FLAG. Updates the flags on a branch.
 */
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


/*
 * packet_filter_mod_receive
 *
 * Called on receipt of U2B_FILTER_MOD. Updates parameter data on a branch.
 */
void packet_filter_mod_receive(const PacketHeader_t *pHdr, const uint8_t *pPayload)
{
	const FilterModPacket_t *pFilterMod = (FilterModPacket_t *)pPayload;

	ChainStageHeader_t *pStageHdr = chain_get_stage(pFilterMod->nStage);
	if(!pStageHdr)
		return;

	StageBranch_t *pBranch = stage_get_branch(pStageHdr, pFilterMod->nBranch);
	if(!pBranch)
		return;

	// Calculate where to copy from
	const uint8_t *pSource = pPayload + sizeof(FilterModPacket_t);

	// Calculate number of bytes to copy
	uint16_t nToCopy = pHdr->size - sizeof(FilterModPacket_t);

	// Calculate destination in memory to copy packet payload to
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


/*
 * packet_filter_mix_receive
 *
 * Called on receipt of U2B_FILTER_MIX. Updates branch mix percentage.
 */
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


/*
 * strnlen
 *
 * Calculates length of a string up to a maximum `maxlen`.
 */
static size_t strnlen(register const char *s, size_t maxlen)
{
  register const char *e;
  size_t n;

  for (e = s, n = 0; *e && n < maxlen; e++, n++)
    ;

  return n;
}


/*
 * packet_cmd_receive
 *
 * Called on receipt of U2B_ARB_CMD.
 */
void packet_cmd_receive(const PacketHeader_t *pHdr, const uint8_t *pPayload)
{
	const CommandPacket_t *pCmd = (CommandPacket_t *)pPayload;

	uint16_t nBytesLeft = pHdr->size - sizeof(CommandPacket_t);

	// Empty command?
	if(!nBytesLeft)
		return;

	const char *pszArg = (const char *)(pCmd + 1);

	// Allocate space to hold command arguments
	const char **ppszArgs = calloc(pCmd->nArgs, sizeof(char *));
	dbg_assert(ppszArgs, "unable to allocate memory for command");

	dbg_printn("\r\n>>> ", 6);

	// Parse arguments into ppszArgs
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

		// Add argument to arguments list
		ppszArgs[i] = pszArg;
		pszArg += (nLen + 1);
	}

	dbg_printn("\r\n", 2);

	// Have we been passed some syntax?
	if(nBytesLeft != pCmd->nArgs)
	{
		dbg_warning("%u unexpected bytes left after parse\r\n", nBytesLeft - pCmd->nArgs);
		goto cleanup;
	}

	// Debug entire chain
	if(!strcmp(ppszArgs[0], "chain_debug"))
	{
		chain_debug();
	}

	// Debug all filters
	else if(!strcmp(ppszArgs[0], "filter_debug"))
	{
		filter_debug();
	}

	// Debug a specific stage
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

	// Change s_bDebugPacketReceipt variable
	else if(!strcmp(ppszArgs[0], "bDebugPacketReceipt"))
	{
		if(pCmd->nArgs != 2)
			dbg_printf("bDebugPacketReceipt = %s\r\n", s_bDebugPacketReceipt ? "true" : "false");
		else
			s_bDebugPacketReceipt = atoi(ppszArgs[1]);
	}

	// Change s_bDebugChainAfterLock variable
	else if(!strcmp(ppszArgs[0], "bDebugChainAfterLock"))
	{
		if(pCmd->nArgs != 2)
			dbg_printf("bDebugChainAfterLock = %s\r\n", s_bDebugChainAfterLock ? "true" : "false");
		else
			s_bDebugChainAfterLock = atoi(ppszArgs[1]);
	}

	// Change system volume
	else if(!strcmp(ppszArgs[0], "volume"))
	{
		if(pCmd->nArgs != 2)
			dbg_printf("volume = %.2f\r\n", g_flChainVolume);
		else
			g_flChainVolume = atof(ppszArgs[1]);
	}

	// Calculate average over a number of samples
	else if(!strcmp(ppszArgs[0], "average"))
	{
		if(pCmd->nArgs != 2)
		{
			dbg_warning("syntax: <samples>\r\n");
			goto cleanup;
		}

		uint16_t iAverage = sample_get_average(atoi(ppszArgs[1]));
		float flVolume = (iAverage * 100.0) / ADC_MAX_VALUE;
		dbg_printf("average = %.2f%%\r\n", flVolume);
	}

	// Ping!
	else if(!strcmp(ppszArgs[0], "ping"))
	{
		dbg_printf("Pong!\r\n");
	}

#ifdef INDIVIDUAL_BUILD_SAUL
	// Save the current filter chain to SD card
	else if(!strcmp(ppszArgs[0], "chain_save"))
	{
		if(pCmd->nArgs != 2)
		{
			dbg_warning("syntax: <name>\r\n");
			goto cleanup;
		}

		// In format "chains/<file>.bin"
		char pszPath[32];
		snprintf(pszPath, sizeof(pszPath), STORE_DIRECTORY "/%s.bin", ppszArgs[1]);

		// Save chain
		chainstore_save(pszPath);

		// Send stored chain list to UI
		packet_stored_list_send();
	}

	// Restore filter chain from SD card
	else if(!strcmp(ppszArgs[0], "chain_restore"))
	{
		if(pCmd->nArgs != 2)
		{
			dbg_warning("syntax: <name>\r\n");
			goto cleanup;
		}

		// In format "chains/<file>.bin"
		char pszPath[32];
		snprintf(pszPath, sizeof(pszPath), STORE_DIRECTORY "/%s.bin", ppszArgs[1]);

		// Restore chain
		chainstore_restore(pszPath);

		// Send chain blob to UI
		packet_chain_blob_send(pszPath);
	}

	// Delete a filter chain from SD card
	else if(!strcmp(ppszArgs[0], "chain_delete"))
	{
		if(pCmd->nArgs != 2)
		{
			dbg_warning("syntax: <name>\r\n");
			goto cleanup;
		}

		// In format "chains/<file>.bin"
		char pszPath[32];
		snprintf(pszPath, sizeof(pszPath), STORE_DIRECTORY "/%s.bin", ppszArgs[1]);

		// Delete chain file
		f_unlink(pszPath);

		// Send stored chain list to UI
		packet_stored_list_send();
	}
#endif

	else
		dbg_warning("unknown command\r\n");

cleanup:
	free(ppszArgs);
}
