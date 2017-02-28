#ifndef _TIMER_H
#define _TIMER_H

#include <time.h>
//--------------------------------------------------------------------------------------------
#define TIMER_EPS 0.0000001
// таймер
#define TIMER_START(time) \
do { time = ((double)clock()/(CLOCKS_PER_SEC/1000)/1000); } while(0)
#define TIMER_STOP(time) \
	do { if( (time=( (double)clock()/(CLOCKS_PER_SEC/1000)/1000 ) - time)<TIMER_EPS ) time=0; } while(0)
//--------------------------------------------------------------------------------------------
#endif