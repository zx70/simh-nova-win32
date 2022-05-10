/* nova_panel.c    front panel functions */



/* Copyright (c) 2007-2008, John Kichury

   This software is freely distributable free of charge and without license fees with the
   following conditions:

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
   JOHN KICHURY BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
   IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

   The above copyright notice must be included in any copies of this software.

*/


#include <stdio.h>
//#include <unistd.h>
#include <ctype.h>
#include "stdio.h"
#include "string.h"
#include "sim_defs.h"
#include "nova_defs.h"
#include <frontpanel.h>
#include <signal.h>

/* lightpanel interface */

uint16 fp_address, 
       fp_data;

uint64 fp_simclock = 1;

uint8  fp_runflag, fp_defer = 0, fp_fetch=0, fp_exec=0, fp_on=1;

uint8  fp_stop = 0,
       fp_stop_switch = 0,
       fp_start_switch = 0,
       fp_reset_switch = 0;
uint16 fp_switches = 0;
static int	fp_enabled = 0;
extern uint16 M[MAXMEMSIZE];

extern int32  C, int_req;
extern uint16 fp_address, fp_data;
extern uint64 fp_simclock;
extern uint8  fp_runflag, fp_defer, fp_fetch, fp_exec;
extern uint8  fp_stop, fp_stop_switch, fp_start_switch, fp_reset_switch;


void fp_switches_callback(int state, int val);
void fp_ac_callback(int state, int val);
void fp_examine_callback(int state, int val);
void fp_reset_stop_callback(int state, int val);
void fp_start_continue_callback(int state, int val);

void panel_sig_handler(int sig)
{
 int n;
  printf("panel signal received\n");
  return;
}

int panel_control(char *cptr)
{
FILE *fd;
 int n;

 if(!strcmp(cptr,"off") || !strcmp(cptr,"OFF"))
  {   if(fp_enabled) fp_quit();
      fp_enabled = 0;
//      sim_vm_read = NULL;
      return 0;
  }

 // hack to load a core file
 if(!strcmp(cptr,"load"))
  {   if(fp_enabled) fp_quit();

     fd = fopen("core.dat","rb"); 

     fread(M,65536,1,fd);
    fclose(fd);
   return 1;
  }

 if(fp_enabled) return 0;

 if(!fp_init(cptr)) return 1;

#if 0
 if (signal (SIGUSR1, panel_sig_handler) == SIG_ERR) 
  { 
    fprintf(stderr,"error setting signal SIGUSR1\n");
  }
#endif


  fp_ignoreBindErrors(1);
  fp_bindSimclock(&fp_simclock);
  fp_bindRunFlag(&fp_runflag);

  fp_bindLight16("LED_ADDR_{15-01}",&fp_address, 1);
  fp_bindLight16("LED_DATA_{15-00}",&fp_data, 1);
  fp_bindLight32("LED_CARRY",&C, 17);
  fp_bindLight8("LED_DEFER",&fp_defer, 1);
  fp_bindLight8("LED_FETCH",&fp_fetch, 1);
  fp_bindLight8("LED_EXEC",&fp_exec, 1);
  fp_bindLight8("LED_RUN",&fp_runflag, 1);
  fp_bindLight8("LED_ON",&fp_on,1);
  fp_bindLight32("LED_ION",&int_req, 20);

  fp_addSwitchCallback("SW_AC0",fp_ac_callback,0);
  fp_addSwitchCallback("SW_AC1",fp_ac_callback,1);
  fp_addSwitchCallback("SW_AC2",fp_ac_callback,2);
  fp_addSwitchCallback("SW_AC3",fp_ac_callback,3);

  fp_bindSwitch8("SW_RESET",&fp_reset_switch,&fp_stop_switch,1);
  fp_addSwitchCallback("SW_RESET",fp_reset_stop_callback,1);
  fp_addSwitchCallback("SW_START",fp_start_continue_callback,1);

  fp_addSwitchCallback("SW_EXAMINE",fp_examine_callback,1);

  fp_bindSwitch16("SW_{15-00}",&fp_switches,&fp_switches,1);
  fp_addSwitchCallback("SW_{15-00}",fp_switches_callback,1);
  //sim_vm_read = panel_read_line;

  fp_enabled = 1;
  return 0;
}


char *panel_read_line (char *cptr, int32 size, FILE *stream)
{
char *tptr;
printf("panel_read_line:\n");

cptr = fgets (cptr, size, stream);                      /* get cmd line */
if (cptr == NULL) {
    clearerr (stream);                                  /* clear error */
    return NULL;                                        /* ignore EOF */
    }
for (tptr = cptr; tptr < (cptr + size); tptr++) {       /* remove cr or nl */
    if ((*tptr == '\n') || (*tptr == '\r') ||
        (tptr == (cptr + size - 1))) {                  /* str max length? */
        *tptr = 0;                                      /* terminate */
        break;
        }
    }
while (isspace (*cptr)) cptr++;                         /* trim leading spc */
if (*cptr == ';') *cptr = 0;                            /* ignore comment */

#if defined (HAVE_READLINE)
add_history (cptr);

#endif
return cptr;
}

