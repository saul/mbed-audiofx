/*
 * chainstore.c - Chain loading and saving functions
 *
 * Defines functions to serialise the current filter chain and save it to disk
 * in a binary format.
 */

#ifndef _CHAINSTORE_H_
#define _CHAINSTORE_H_

// Directory where the chains are stored on SD card
#define STORE_DIRECTORY "chains"

// Identifier for the ChainStore format
#define STORE_IDENT ('C' | ('H' << 8) | ('S' << 16) | ('T' << 24))

// Current version for the ChainStore format
#define STORE_VERSION 1


/*
 * ChainStoreHeader_t
 *
 * Header for the ChainStore format.
 */
#pragma pack(push, 1)
typedef struct
{
	uint32_t ident;		///< File format identifier (should be STORE_IDENT)
	uint8_t iVersion;	///< Version of the stored chain (should be STORE_VERSION)
	uint8_t nStages;	///< Number of stages stored in the file
} ChainStoreHeader_t;
#pragma pack(pop)


/*
 * ChainStoreStageHeader_t
 *
 * Header for a filter stage.
 */
#pragma pack(push, 1)
typedef struct
{
	uint8_t nBranches;	///< Number of branches in this stage
} ChainStoreStageHeader_t;
#pragma pack(pop)


/*
 * ChainStoreBranchHeader_t
 *
 * Header for a branch of a filter stage.
 */
#pragma pack(push, 1)
typedef struct
{
	uint8_t filter;		///< Index into g_pFilters
	uint8_t flags;		///< Flags for this branch
	float flMixPerc;	///< Mix percentage for this branch
	uint8_t nParams;	///< Number of filter parameter values that follow this header
} ChainStoreBranchHeader_t;
#pragma pack(pop)


/*
 * ChainStoreParam_t
 *
 * Header for branch parameter data.
 */
#pragma pack(push, 1)
typedef struct
{
	uint8_t iOffset;	///< Offset into filter data that the proceeding value should be copied
	uint8_t nSize;		///< Size of the parameter value that follows this struct
} ChainStoreParam_t;
#pragma pack(pop)


void chainstore_save(const char *pszPath);
bool chainstore_header_validate(const ChainStoreHeader_t *pHdr);
void chainstore_restore(const char *pszPath);

#endif
