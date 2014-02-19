#ifndef _PACKETS_H_
#define _PACKETS_H_

#include <stdint.h>

// !!! Ensure changes to this enum are reflected in sercom.py (PacketTypes
// enum)
// !!! Ensure changes to this enum are reflected in g_ppszPacketTypes
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

	// Must be last
	PACKET_TYPE_MAX,
} PacketType_e;


#pragma pack(push, 1)
typedef struct
{
	uint32_t ident;
	uint8_t type;
	uint8_t size;
} PacketHeader_t;
#pragma pack(pop)


typedef void (*PacketCallback_t)(const PacketHeader_t *pHdr, const uint8_t *pPayload);

#define PACKET_SIZE_COMPARATOR_BIT (1<<7) // comparator bit set: size should be >=, not set: size should be exact
#define PACKET_SIZE_EXACT(size) (size & ~PACKET_SIZE_COMPARATOR_BIT)
#define PACKET_SIZE_MIN(size) (size | PACKET_SIZE_COMPARATOR_BIT)

typedef struct
{
	PacketCallback_t pfnCallback;
	bool bLocksChain;
	uint8_t nPacketSize;
} PacketHandler_t;

extern const char *g_ppszPacketTypes[];

void packet_static_assertions(void);
void packet_probe_send(void);
void packet_reset_wait(void);
void packet_print_send(const char *pszLine, size_t size);
void packet_filter_list_send(void);

void packet_loop(void);
void packet_reset_receive(const PacketHeader_t *pHdr, const uint8_t *pPayload);

#pragma pack(push, 1)
typedef struct
{
	uint8_t nStage;
	uint8_t iFilterType;
	uint8_t flags;
	float flMixPerc;
} FilterCreatePacket_t;
#pragma pack(pop)

void packet_filter_create_receive(const PacketHeader_t *pHdr, const uint8_t *pPayload);

#pragma pack(push, 1)
typedef struct
{
	uint8_t nStage;
	uint8_t nBranch;
} FilterDeletePacket_t;
#pragma pack(pop)

void packet_filter_delete_receive(const PacketHeader_t *pHdr, const uint8_t *pPayload);

#pragma pack(push, 1)
typedef struct
{
	uint8_t nStage;
	uint8_t nBranch;
	uint8_t iBit;
	bool bEnable;
} FilterFlagPacket_t;
#pragma pack(pop)

void packet_filter_flag_receive(const PacketHeader_t *pHdr, const uint8_t *pPayload);

#pragma pack(push, 1)
typedef struct
{
	uint8_t nStage;
	uint8_t nBranch;
	uint8_t iOffset;
} FilterMod_t;
#pragma pack(pop)

void packet_filter_mod_receive(const PacketHeader_t *pHdr, const uint8_t *pPayload);

#endif
