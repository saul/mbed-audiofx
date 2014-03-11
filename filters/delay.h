/*
 *	HAPR Project 2014
 *	Group 6 - Tom Bryant (TB) & Saul Rennison (SR)
 *
 *	File created by:	SR
 *	File modified by:	TB & SR
 *	File debugged by:	TB & SR
*/


#ifndef _FILTER_DELAY_H_
#define _FILTER_DELAY_H_


// Structure used to hold delay data
typedef struct
{
	uint16_t nDelay;		///< Length of delay (in samples [0-9999])
	float flDelayMixPerc;	///< Mix level of the delayed sample float [0-1]
} FilterDelayData_t;


int16_t filter_delay_apply(int16_t input, void *pUnknown);
void filter_delay_debug(void *pUnknown);
void filter_delay_create(void *pUnknown);
int16_t filter_delay_feedback_apply(int16_t input, void *pUnknown);

#endif
