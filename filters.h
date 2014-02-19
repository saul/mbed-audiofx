/*
 * filters.c - Audio effect filters
 */

#ifndef _FILTERS_H_
#define _FILTERS_H_

/*
 * Enumeration of available filters.
 *
 * Use these enumeration values to pass into `filter_get`.
 */
typedef enum
{
	FILTER_DELAY = 0,
	FILTER_NOISE,
} Filter_e;


/*
 * SampleFilter_t
 *
 * Receives a 32-bit sample value (`input`) and filter private data `pPrivate`.
 * `pPrivate` should be cast to the struct that holds the parameters for this
 * filter.
 */
typedef uint32_t (*SampleFilter_t)(uint32_t input, void *pPrivate);


/*
 * FilterDebug_t
 *
 * Passes filter private data as `pPrivate`, this function should print the
 * parameter data.
 */
typedef void (*FilterDebug_t)(void *pPrivate);


/*
 * Filter_t
 *
 * Holds all information about a filter type (NOT a filter instance, see
 * `ChainStage_t` in chain.h)
 */
#pragma pack(push, 1)
typedef struct
{
	const char *pszName;
	const char *pszParamFormat;
	SampleFilter_t pfnApply;
	FilterDebug_t pfnDebug;
	uint8_t nPrivateDataSize; ///< size of private data struct
} Filter_t;
#pragma pack(pop)


extern Filter_t g_pFilters[];
extern const size_t NUM_FILTERS;


void filter_debug(void);

#endif
