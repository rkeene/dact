/*****************************************************************************
 *
 *   Author:   Stuart Inglis     <singlis@waikato.ac.nz>
 *
 *
 * timing routines
 *
 *****************************************************************************/
#ifndef __TIMER_H
#define __TIMER_H
#include <stdio.h>

#define MAX_TIMERS	5

void TimerInit(void);
void TimerFree(void);

void TimerStart(int tn);
void TimerStop(int tn);
void TimerGet(int tn,float *real, float *user, float *cpu);
void TimerPrint(int tn, FILE *fp);
void TimerString(int tn, char *s);

#endif	       
