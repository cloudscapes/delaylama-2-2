//
//  delaylama_float.c
//  max-external
//
//  Created by Tom     Berkmann 375851,
//  Olivier Faure  Brac 382122,
//  Hamed   Habibi Fard  385540, on 10/26/18.
//

#include "ext.h"  // must be included first
#include "z_dsp.h"
#include "ext_obex.h"
#include <stdio.h>
#include "mystruct.h"
#include "delaylama_class.h"
#include "delaylama_utility.h"


/* The float method */

void delaylama_float(t_delaylama *x, double f)
{
    /* we don't care about the first inlet(from left) cause it's recieving only signals */
    int inlet = ((t_pxobject*)x)->z_in;
    switch(inlet){
            
            /* If the float came into the second inlet it is delay time */
            
        case 1:
            if(f < 0.0 || f > x->maximum_delay_time * 1000.0)
            {
                error("delaylama~: illegal delay: %f reset to 1 ms", f);
            }
            else {
                x->delay_time = f * 0.001; // convert to seconds
                x->frac_delay = x->delay_time * x->samp_rate_ms; // convert to samples
                while(x->frac_delay < 0){
                    x->frac_delay += x->delay_length_as_samples;
                }
                x->int_delay = trunc(x->frac_delay);
                x->fractional_difference = x->frac_delay - x->int_delay;
            }
            break;
            
            /* If the float came into the third inlet it is feedback factor */
            
        case 2: x->feedback = f; break;
    }
}
