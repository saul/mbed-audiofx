#ifndef _SAMPLES_H_
#define _SAMPLES_H_

#include <stdint.h>


extern volatile uint16_t g_iSampleCursor;


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-pedantic"
#pragma pack(push, 1)
typedef struct
{
	uint16_t a : 12;
	uint16_t b : 12;
} SamplePair_t;
#pragma pack(pop)
#pragma GCC diagnostic pop


uint16_t sample_get(int16_t index);
void sample_set(int16_t index, uint16_t value);
uint16_t sample_get_interpolated(float index);

#endif