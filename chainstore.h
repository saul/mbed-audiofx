#ifndef _CHAINSTORE_H_
#define _CHAINSTORE_H_

#define STORE_IDENT ('C' | ('H' << 8) | ('S' << 16) | ('T' << 24))
#define STORE_VERSION 1

typedef struct
{
	uint32_t ident;
	uint8_t iVersion;
	uint8_t nStages;
} ChainStoreHeader_t;

typedef struct
{
	uint8_t nBranches;
} ChainStoreStageHeader_t;

typedef struct
{
	uint8_t filter;
	uint8_t flags;
	float flMixPerc;
	uint8_t nParams;
} ChainStoreBranchHeader_t;

typedef struct
{
	uint8_t iOffset;
	uint8_t nSize;
} ChainStoreParam_t;

void chainstore_save(const char *pszPath);
void chainstore_restore(const char *pszPath);

#endif
