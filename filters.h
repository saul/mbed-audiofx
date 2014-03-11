/*
 *	HAPR Project 2014
 *	Group 6 - Tom Bryant (TB) & Saul Rennison (SR)
 *	Saul Rennison Individual Part
 *
 *	File created by:	SR
 *	File modified by:	TB & SR
 *	File debugged by:	TB & SR
 *
 * filters.c - Audio effect filters
 */

#ifndef _FILTERS_H_
#define _FILTERS_H_

// Character that separates parameter info in Filter_t::pszParamFormat
#define PARAM_SEP "|"


extern Filter_t g_pFilters[];
extern const size_t NUM_FILTERS;


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
 * FilterApply_t
 *
 * Receives a 32-bit sample value (`input`) and filter data `pUnknown`.
 * `pUnknown` should be cast to the struct that holds the parameters for this
 * filter.
 */
typedef int16_t (*FilterApply_t)(int16_t input, void *pUnknown);


/*
 * FilterCallback_t
 *
 * Passes filter data as `pUnknown`. Used for parameter debugging,
 * creation callback and filter data mod callback.
 */
typedef void (*FilterCallback_t)(void *pUnknown);


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
	const char *pszParamFormat; ///< defines what parameters can be modified by the UI and which ones are saved to disk
	FilterApply_t pfnApply; ///< called to apply the filter to a sample
	FilterCallback_t pfnDebug;
	FilterCallback_t pfnCreateCallback; ///< called when a filter is created
	FilterCallback_t pfnModCallback; ///< called when filter data is modified
	uint8_t nFilterDataSize; ///< size of filter data struct
	uint8_t nNonPublicDataSize; ///< size of non-public data at start of filter data struct
} Filter_t;
#pragma pack(pop)


void filter_debug(void);

#endif
