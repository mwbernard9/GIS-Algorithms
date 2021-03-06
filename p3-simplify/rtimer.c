/*
 * 
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by Duke University
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE TRUSTEES AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE TRUSTEES OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
*/



#include <sys/time.h>
#include <sys/resource.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <assert.h>

#include "rtimer.h"

char *
/*rt_sprint(char *buf, Rtimer rt) 
{
  if(rt_w_useconds(rt) == 0) {
    sprintf(buf, "[%4.2fu (%.0f%%) %4.2fs (%.0f%%) %4.2f %.1f%%]",
	    0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
  } else {
    sprintf(buf, "[%4.2fu (%.0f%%) %4.2fs (%.0f%%) %4.2f %.1f%%]",
	    rt_u_useconds(rt)/1000000,
	    100.0*rt_u_useconds(rt)/rt_w_useconds(rt),
	    rt_s_useconds(rt)/1000000,
	    100.0*rt_s_useconds(rt)/rt_w_useconds(rt),
	    rt_w_useconds(rt)/1000000,
	    100.0*(rt_u_useconds(rt)+rt_s_useconds(rt)) / rt_w_useconds(rt));
  }
  return buf;
}*/

rt_sprint(char *buf, Rtimer rt) 
{
  if(rt_w_useconds(rt) == 0) {
    sprintf(buf, "%4.2f seconds", 0.0);
  } else {
    sprintf(buf, "%4.2f seconds", rt_w_useconds(rt)/1000000);
  }
  return buf;
}



/* prints the timer divided by k --- used when k experiments kept in
   the same timer */
char *
rt_sprint_average(char *buf, Rtimer rt, int k) {

  assert(k>0);
  if(rt_w_useconds(rt) == 0) {
    sprintf(buf, "[%4.2fu (%.0f%%) %4.2fs (%.0f%%) %4.2f %.1f%%]",
	    0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
  } else {
    sprintf(buf, "[%4.2fu (%.0f%%) %4.2fs (%.0f%%) %4.2f %.1f%%]",
	    rt_u_useconds(rt)/1000000/k,
	    100.0*rt_u_useconds(rt)/rt_w_useconds(rt),
	    rt_s_useconds(rt)/1000000/k,
	    100.0*rt_s_useconds(rt)/rt_w_useconds(rt),
	    rt_w_useconds(rt)/1000000/k,
	    100.0*(rt_u_useconds(rt)+rt_s_useconds(rt)) / rt_w_useconds(rt));
  }
  return buf;
}


/* to be called after rt_stop_and_accumulate to print the total time */
/* assumes rt.tu_usec, rt.ts_usec, rt.tw_usec have been computed
 */
char *
rt_sprint_total(char *buf, Rtimer rt) {
  if(rt.tw_usec == 0) {
    sprintf(buf, "[%4.2fu (%.0f%%) %4.2fs (%.0f%%) %4.2f %.1f%%]",
	    0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
  } else {
    sprintf(buf, "[%4.2fu (%.0f%%) %4.2fs (%.0f%%) %4.2f %.1f%%]",
	    rt.tu_usec/1000000,
	    100.0*rt.tu_usec/rt.tw_usec,
	    rt.ts_usec/1000000,
	    100.0*rt.ts_usec/rt.tw_usec,
	    rt.tw_usec/1000000,
	    100.0*(rt.tu_usec+rt.ts_usec) / rt.tw_usec);
  }
  return buf;
}
