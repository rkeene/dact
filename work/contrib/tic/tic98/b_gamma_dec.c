#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>

#include "arithcode.h"

#include "globals.h"

#include "tic98.h"
#include "b_gamma.h"

int 
main()
{
  int pos,num;
  INTCODETYPE enc,size;

  assert(BUFFER_SIZE>=1);

#ifndef Q_CODER
  globals_init();
#endif

  START_DEC();
  enc=INTCODE_INIT();
  size=INTCODE_INIT();

  while(1){
    num=BUFFER_SIZE-INTCODE_DECODE_LL(size);
    for(pos=0;pos<num;pos++){
      fprintf(stdout,"%lld\n",INTCODE_DECODE_LL(enc));
    }
    if(num!=BUFFER_SIZE)
      break;
  }

  INTCODE_FREE(enc);
  INTCODE_FREE(size);

  FINISH_DEC();

  return 0;
}
