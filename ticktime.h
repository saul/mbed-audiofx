#ifndef _TICKTIME_H_
#define _TICKTIME_H_

void time_init(uint32_t iResMsec);
bool time_setup(void);
float time_realtime(void);
void time_sleep(uint32_t msec);
uint32_t time_tickcount(void);
float time_tickinterval(void);

#endif
