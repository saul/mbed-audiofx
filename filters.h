#ifndef _FILTERS_H_
#define _FILTERS_H_

typedef enum
{
	FILTER_DELAY = 0,
	FILTER_GAIN,
	FILTER_REVERB,
	FILTER_GATE,
	FILTER_LOW_PASS,
	FILTER_HIGH_PASS,
	FILTER_BAND_PASS,

	//FILTER_LAST, ///< not a valid filter value
} Filter_e;


typedef uint32_t (*SampleFilter_t)(uint32_t input, void *pPrivate);
typedef void (*FilterDebug_t)(void *pPrivate);
typedef uint32_t (*FilterDataValidator_t)(void *pPrivate);


#pragma pack(push, 1)
typedef struct
{
	const char *pszName;
	const char *pszDescription;
	SampleFilter_t pfnApply;
	FilterDebug_t pfnDebug;

	FilterDataValidator_t pfnValidate; // validate private data struct
	uint8_t nPrivateDataSize; // size of private data struct
} Filter_t;
#pragma pack(pop)

extern Filter_t g_pFilters[];

const Filter_t *filter_get(Filter_e idx);
void filter_debug(void);

#endif
