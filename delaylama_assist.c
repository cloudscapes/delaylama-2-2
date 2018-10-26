//
//  delaylama_assist.c
//  max-external
//
//  Created by Shawn Yves on 10/26/18.
//

#include "ext.h"  // must be included first
#include "z_dsp.h"
#include "ext_obex.h"
#include <stdio.h>
#include "mystruct.h"
#include "delaylama_class.h"
#include "delaylama_utility.h"

/* The assist method */
/* this method is for the purpose of showing some information to user when he/she goes with
 the mouse over the inlets/outles
 */

void delaylama_assist(t_delaylama *x, void *b, long msg, long arg, char *dst)
{
    
    if (msg == ASSIST_INLET) {
        /* the inlet number is determined with the arg variable*/
        switch (arg) {
            case 0: sprintf(dst,"(signal) Input");
                break;
            case 1: sprintf(dst,"(signal) Delay Time");
                break;
            case 2: sprintf(dst,"(signal) Feedback");
                break;
        }
    } else if (msg == ASSIST_OUTLET) {
        /* since we have only one outlet no switching is required */
        sprintf(dst,"(signal) Output");
    }
}
