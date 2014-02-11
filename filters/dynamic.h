#ifndef _FILTER_DYNAMIC_H_
#define _FILTER_DYNAMIC_H_

typedef struct
{
	uint32_t threshold;
	uint8_t attack;
	uint8_t release;
	uint8_t hold;
} FilterNoisegateData_t;

#endif