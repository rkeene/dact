/* 

timer.c module, written by Stuart Inglis 

This has MAX_TIMERS-2 timers, as timer 0 contains the timing
stats for the entire program, and cannot be either started nor
stopped.  Note that the real time for timer 0 will not be accuate
unless TimerInit is called at the start of the program.

*/

#define __EXTENSIONS__

#include <stdlib.h>
#include <sys/times.h>
#include <limits.h>

#ifndef CLK_TCK
#define CLK_TCK 1
#endif

#include "timer.h"
#include "utils.h"


static struct tms *timebuffer=NULL;

static struct {
    clock_t real;
    clock_t user;
    clock_t sys;
    int running;
} timers[MAX_TIMERS];



void TimerStart(int tn)
{
    if(timebuffer==NULL)
	error("TimerStart","using TimerInit() before calling this routine","");

    if((tn<0) || (tn>=(MAX_TIMERS)))
	error("TimerStart","timer number out of range","");

    if(timers[tn].running){
	fprintf(stderr,"TimerStart: timer %d already running. exiting...\n",tn);
	exit(1);
    }

    if(tn==0)
	error("TimerStart","cannot 'start' timer 0, this is the main program timer! use another...","");
	
    
    timers[tn].real=times(timebuffer);
    timers[tn].user=timebuffer->tms_utime;
    timers[tn].sys=timebuffer->tms_stime;
    timers[tn].running=1;
}


void TimerGet(int tn,float *real, float *sys, float *user)
{
    clock_t tempreal;

    if(timebuffer==NULL)
	error("TimerGet","using TimerInit() before calling this routine","");

    if((tn<0) || (tn>=(MAX_TIMERS)))
	error("TimerGet","timer number out of range","");


    tempreal=times(timebuffer);

    if(timers[tn].running==1){
	*real=(float)(tempreal-timers[tn].real)/(CLK_TCK);
	*user=(timebuffer->tms_utime-timers[tn].user)/(float)CLK_TCK;
	*sys=(timebuffer->tms_stime-timers[tn].sys)/(float)CLK_TCK;
    }
    else {
	*real=(timers[tn].real)/(float)CLK_TCK;
	*user=(timers[tn].user)/(float)CLK_TCK;
	*sys=(timers[tn].sys)/(float)CLK_TCK;
    }
}


void TimerPrint(int tn, FILE *fp)
{
    float real,sys,user;

    if(fp==NULL)
      return;

    if(timebuffer==NULL)
	error("TimerPrint","using TimerInit() before calling this routine","");

    if((tn<0) || (tn>=(MAX_TIMERS)))
	error("TimerPrint","timer number out of range","");

    TimerGet(tn,&real,&sys,&user);

    fprintf(fp,"Time_user:           %.2f\nTime_sys:            %.2f\nTime_real:           %.2f\n",user,sys,real);
}

void TimerStop(int tn)
{
    clock_t tempreal;

    if(timebuffer==NULL)
	error("TimerStop","using TimerInit() before calling this routine","");
    
    if((tn<0) || (tn>=(MAX_TIMERS)))
	error("TimerStop","timer number out of range","");

    if(tn==0)
	error("TimerStop","cannot 'stop' timer 0, this is the main program timer! use another...","");

    tempreal=times(timebuffer);

    timers[tn].real=tempreal-timers[tn].real;
    timers[tn].user=timebuffer->tms_utime-timers[tn].user;
    timers[tn].sys=timebuffer->tms_stime-timers[tn].sys;
    timers[tn].running=0;
}


void TimerInit(void)
{
    int i;
    
    timebuffer=(struct tms*)malloc(sizeof(struct tms));
    if(timebuffer==NULL)
	error("TimerInits","cannot allocate timebuffer","");
    
    for(i = 0; i < MAX_TIMERS; i++){  
	timers[i].real=0;
	timers[i].user=0;
	timers[i].sys=0;
	timers[i].running=0;
    }
    /* setup the main program timer, timers[0] */
    timers[0].real=times(timebuffer);
    timers[0].running=1;
}





void TimerString(int tn, char *s)
{
    float real,sys,user;
    int sec_left;


    if(timebuffer==NULL)
	error("TimerFree","using TimerInit() before calling this routine","");

    TimerGet(tn,&real,&sys,&user);
    sec_left=(int)real;
    if(sec_left>86400){
	int days;

	days=sec_left/86400;
	if(days>999) days=999;
	sprintf(s,"%dD:%02dH",days,(sec_left%86400)/3600);
    }
    else
	sprintf(s,"%d:%02d:%02d",(sec_left%86400)/3600, (sec_left%3600)/60, (sec_left%60));
}





void TimerFree(void)
{
    if(timebuffer==NULL)
	error("TimerFree","using TimerInit() before calling this routine","");
    
    free(timebuffer);
}


