#ifndef _TICKTIME_H_
#define _TICKTIME_H_

void time_init(int iResMsec);
int time_setup(void);
float time_realtime(void);
void time_sleep(int msec);
unsigned long time_tickcount(void);
float time_tickinterval(void);

#endif
