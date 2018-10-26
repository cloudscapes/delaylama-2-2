//
//  delaylama_new.c
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

/* The new instance routine */
/* The arguments to this function are used to pass user_supplied parameters*/

void *delaylama_new(t_symbol *s, short argc, t_atom *argv)
{
    /* setting some parameters to default values in case the user inputs invalid numbers */
    
    float def_max_delay = 120.0;
    float def_delay_time = 120.0;
    float feedback = 0.1;
    
    /*  Instantiate a new delaylama~ object*/
    
    t_delaylama *x = object_alloc(delaylama_class);
    
    /* creating 3 inlets*/
    
    dsp_setup((t_pxobject *)x,3) ;
    
    /* creating 1 outlet  */
    
    outlet_new((t_object *)x, "signal");
    
    /* Read user parameters from the object box  */
    
    atom_arg_getfloat(&def_max_delay, 0, argc, argv);
    atom_arg_getfloat(&def_delay_time, 1, argc, argv);
    atom_arg_getfloat(&feedback, 2, argc, argv);
    
    /* some sanity checking in case the user enters some invalid numbers!*/
    
    if(def_max_delay <= 0){
        def_max_delay = 300.0;
    }
    x->maximum_delay_time = def_max_delay * 0.001;
    
    x->delay_time = def_delay_time ;
    
    /*our delay time shouldn't be longer than the maximum delay time or less and zero! */
    
    if(x->delay_time > def_max_delay || x->delay_time <= 0.0){
        
        error("delay time is invalid: %f, will default to 1ms", x->delay_time);
        
        /*setting our delay time to a default(here 1ms) */
        
        x->delay_time = 1.0;
    }
    
    /* by setting the sampling rate to zero we force it to be invoked in our DSP method */
    
    x->sampling_rate = 0.0;
    x->feedback = feedback;
    return x;
}











