#ifndef _CHAINSTORE_H_
#define _CHAINSTORE_H_

#define STORE_DIRECTORY "chains"

#define STORE_IDENT ('C' | ('H' << 8) | ('S' << 16) | ('T' << 24))
#define STORE_VERSION 1


#pragma pack(push, 1)
typedef struct
{
	uint32_t ident;
	uint8_t iVersion;
	uint8_t nStages;
} ChainStoreHeader_t;
#pragma pack(pop)


#pragma pack(push, 1)
typedef struct
{
	uint8_t nBranches;
} ChainStoreStageHeader_t;
#pragma pack(pop)


#pragma pack(push, 1)
typedef struct
{
	uint8_t filter;
	uint8_t flags;
	float flMixPerc;
	uint8_t nParams;
} ChainStoreBranchHeader_t;
#pragma pack(pop)


#pragma pack(push, 1)
typedef struct
{
	uint8_t iOffset;
	uint8_t nSize;
} ChainStoreParam_t;
#pragma pack(pop)


void chainstore_save(const char *pszPath);
bool chainstore_header_validate(const ChainStoreHeader_t *pHdr);
void chainstore_restore(const char *pszPath);

#endif
