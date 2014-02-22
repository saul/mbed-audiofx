#ifndef _SAMPLES_H_
#define _SAMPLES_H_

#include <stdint.h>


extern volatile uint16_t g_iSampleCursor;
extern volatile uint16_t g_iVibratoSampleCursor;


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

typedef struct
{
	uint16_t nSamples;
	uint32_t average;
} SampleAverage_t;


uint32_t sample_get(int16_t index);
void sample_set(int16_t index, uint16_t value);
uint32_t sample_get_interpolated(float index);
uint32_t sample_get_average(uint16_t nSamples);
void sample_clear_average(void);

#endif
