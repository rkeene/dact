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
  long long buf[BUFFER_SIZE];
  int pos=0,j,eof=0;
  INTCODETYPE enc,size;

  assert(BUFFER_SIZE>=1);

#ifndef Q_CODER
  globals_init();
#endif

  START_ENC();
  enc=INTCODE_INIT();
  size=INTCODE_INIT();

  while(1){
    if(scanf("%lld\n",&buf[pos])!=1) 
      eof=1;
    else
      pos++;

    if((pos==BUFFER_SIZE) || (eof)){
      INTCODE_ENCODE_LL(size,BUFFER_SIZE-pos);
      for(j=0;j<pos;j++){
	INTCODE_ENCODE_LL(enc,buf[j]);
      }
      pos=0;
    }
    if(eof) break;
  }

  INTCODE_FREE(enc);
  INTCODE_FREE(size);

  FINISH_ENC();
  
  return 0;
}
