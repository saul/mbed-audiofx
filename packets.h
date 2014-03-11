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

#ifndef _PACKETS_H_
#define _PACKETS_H_

#include <stdint.h>
#include <stdbool.h>

// Packet type enumeration
// ----------------------------------------------------------------------------
// !!! Ensure changes to this enum are reflected in sercom.py (PacketTypes
//     enum)
//
// !!! Ensure changes to this enum are reflected in packets.c
//     (g_ppszPacketTypes)
typedef enum
{
	A2A_PROBE = 0,		///< Board sends a probe packet on boot, waits for UI to respond with a probe
	U2B_RESET,			///< UI sends a packet to reset board
	B2U_PRINT,			///< Board debug prints to UI console
	B2U_FILTER_LIST,	///< Board sends available filters to UI
	U2B_FILTER_CREATE,	///< UI is creating a filter
	U2B_FILTER_DELETE,	///< UI is deleting a filter
	U2B_FILTER_FLAG,	///< UI is changing a filter flag
	U2B_FILTER_MOD,		///< UI is changing a filter parameter
	U2B_FILTER_MIX,		///< UI is changing a filter mix percentage
	U2B_ARB_CMD,		///< UI is sending an arbitrary command to the board (e.g., chain_dump)
#ifdef INDIVIDUAL_BUILD_TOM
	B2U_ANALOG_CONTROL, ///< Board sends analog control value
#endif
#ifdef INDIVIDUAL_BUILD_SAUL
	B2U_STORED_LIST,	///< Board sends list of possible chains to load
	B2U_CHAIN_BLOB,		///< Board sends a binary chain blob to the UI to sync up restored chain
#endif

	// Must be last
	PACKET_TYPE_MAX,
} PacketType_e;


/*
 * PacketHeader_t
 *
 * Header of each packet.
 */
#pragma pack(push, 1)
typedef struct
{
	uint32_t ident;	///< Packet identifier (should be PACKET_IDENT)
	uint8_t type;	///< Packet type, see PacketType_e enumeration above
	uint16_t size;	///< Size of packet payload in bytes (excluding header)
} PacketHeader_t;
#pragma pack(pop)


// Inbound packet handlers
// ----------------------------------------------------------------------------
typedef void (*PacketCallback_t)(const PacketHeader_t *pHdr, const uint8_t *pPayload);

#define PACKET_SIZE_COMPARATOR_BIT (1<<15) // comparator bit set: size should be >=, not set: size should be exact
#define PACKET_SIZE_EXACT(size) (size & ~PACKET_SIZE_COMPARATOR_BIT)
#define PACKET_SIZE_MIN(size) (size | PACKET_SIZE_COMPARATOR_BIT)

/*
 * PacketHandler_t
 *
 * Inbound packet handler definition.
 */
typedef struct
{
	PacketCallback_t pfnCallback;	///< Function to call when packet is received
	bool bLocksChain;				///< Should the filter chain be locked while the packet is processed?
	uint16_t nPacketSize;			///< Expected size of packet payload (use PACKET_SIZE_EXACT and PACKET_SIZE_MIN above)
} PacketHandler_t;

// All packet types
extern const char *g_ppszPacketTypes[];


// Function declarations
// ----------------------------------------------------------------------------
void packet_static_assertions(void);
void packet_loop(void);

// A2A_PROBE
// ==============================================
void packet_probe_send(void);

// B2U_PRINT
// ==============================================
void packet_print_send(const char *pszLine, size_t size);

// B2U_FILTER_LIST
// ==============================================
void packet_filter_list_send(void);

// B2U_ANALOG_CONTROL
// ==============================================
#ifdef INDIVIDUAL_BUILD_TOM
void packet_analog_control_send(uint16_t analog_value);
#endif

// B2U_STORED_LIST
// ==============================================
#ifdef INDIVIDUAL_BUILD_SAUL
void packet_stored_list_send(void);
#endif

// B2U_CHAIN_BLOB
// ==============================================
#ifdef INDIVIDUAL_BUILD_SAUL
void packet_chain_blob_send(const char *pszPath);
#endif

// U2B_RESET
// ==============================================
void packet_reset_receive(const PacketHeader_t *pHdr, const uint8_t *pPayload);

// U2B_FILTER_CREATE
// ==============================================
#pragma pack(push, 1)
typedef struct
{
	uint8_t nStage;			///< stage to create branch on
	uint8_t iFilterType;	///< type of filter (index into g_pFilters)
	uint8_t flags;			///< flags to create branch with
	float flMixPerc;		///< mix percentage of branch
} FilterCreatePacket_t;
#pragma pack(pop)

void packet_filter_create_receive(const PacketHeader_t *pHdr, const uint8_t *pPayload);

// U2B_FILTER_DELETE
// ==============================================
#pragma pack(push, 1)
typedef struct
{
	uint8_t nStage;		///< stage to delete branch from
	uint8_t nBranch;	///< branch to delete
} FilterDeletePacket_t;
#pragma pack(pop)

void packet_filter_delete_receive(const PacketHeader_t *pHdr, const uint8_t *pPayload);

// U2B_FILTER_FLAG
// ==============================================
#pragma pack(push, 1)
typedef struct
{
	uint8_t nStage;		///< stage to update branch
	uint8_t nBranch;	///< index to branch in stage
	uint8_t iBit;		///< flag bit to change
	bool bEnable;		///< true = set bit, false = clear bit
} FilterFlagPacket_t;
#pragma pack(pop)

void packet_filter_flag_receive(const PacketHeader_t *pHdr, const uint8_t *pPayload);

// U2B_FILTER_MOD
// ==============================================
#pragma pack(push, 1)
typedef struct
{
	uint8_t nStage;		///< stage to update branch
	uint8_t nBranch;	///< index to branch in stage
	uint8_t iOffset;	///< offset into filter data to overwrite
} FilterModPacket_t;
#pragma pack(pop)

void packet_filter_mod_receive(const PacketHeader_t *pHdr, const uint8_t *pPayload);

// U2B_FILTER_MIX
// ==============================================
#pragma pack(push, 1)
typedef struct
{
	uint8_t nStage;		///< stage to update branch
	uint8_t nBranch;	///< index to branch in stage
	float flMixPerc;	///< new filter mix percentage
} FilterMixPacket_t;
#pragma pack(pop)

void packet_filter_mix_receive(const PacketHeader_t *pHdr, const uint8_t *pPayload);

// U2B_ARB_CMD
// ==============================================
#pragma pack(push, 1)
typedef struct
{
	uint8_t nArgs;	///< number of command arguments proceeding this header
	              	//   arguments are NUL delimited strings
} CommandPacket_t;
#pragma pack(pop)

void packet_cmd_receive(const PacketHeader_t *pHdr, const uint8_t *pPayload);

#endif
