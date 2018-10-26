//
//  delaylama.c
//  max-external
// works only with max/msp 6 & 7 (64 bit versions)
// this code has not yet been tested with MAX 8! 
/* Created by Tom     Berkmann 375851,
              Olivier Faure  Brac 382122,
              Hamed   Habibi Fard  385540,
              on 8/20/18.
 
*/

/* The necessary Max/MSP header files */

#include "ext.h"  // must be included first
#include "z_dsp.h"
#include "ext_obex.h"

/* The object structure for our external with it's state variables  */

typedef struct _delaylama {
    
    t_pxobject audio_obj;  // must be the first line in our object structure(MAX/MSP rule!)
    
    float sampling_rate; // sampling rate
    float samp_rate_ms; //  sampling rate (milliseconds/MAX standard)
    long delay_length_as_samples; // length of the delay line in samples
    long delay_length_as_bytes; //   length of delay line in bytes
    
    float delay_time; // the current delay time
    float feedback; //   feedback multiplier
    
    float maximum_delay_time; // maximum delay time
   
 
    float *delay_line; // our delay line itself
    
    /* we only need to keep track of the write pointer cause the read pointer will always follow the write pointer by a number of samples determined by the delay line */
    
    float *write_ptr; //  write pointer into delay line
    float *read_ptr; //   read pointer into delay line
    
    
    
   
    
    float frac_delay; // the fractional  delay time
    long int_delay; // the integer delay time
    float fractional_difference; // the fractional difference between the fractional and integer delay times
    
    short delaytime_connection_status ; // the connection status of this inlet
    short feedback_connection_status  ; // the connection status of this inlet
  
} t_delaylama;

/* The class declaration */

static t_class *delaylama_class;

/* Function prototypes */

void *delaylama_new(t_symbol *s, short argc, t_atom *argv);

void delaylama_free(t_delaylama *x);

void delaylama_float(t_delaylama *x, double f);

void delaylama_assist(t_delaylama *x, void *b, long msg, long arg, char *dst);



void delaylama_perform64(t_delaylama *x, t_object *dsp64, double **ins,
                       long numins, double **outs,long numouts, long n,
                       long flags, void *userparam);

void delaylama_dsp64(t_delaylama *x, t_object *dsp64, short *count, double sr, long n, long flags);



/* The main() function */

void ext_main(void *r)
{
    delaylama_class = class_new("delaylama~",(method)delaylama_new,(method)delaylama_free,sizeof(t_delaylama),0,A_GIMME,0);
    
    /* binding our methods */
    
    class_addmethod(delaylama_class, (method)delaylama_dsp64, "dsp64", A_CANT, 0);
    
    class_addmethod(delaylama_class, (method)delaylama_float, "float", A_FLOAT, 0);
    
    class_addmethod(delaylama_class, (method)delaylama_assist, "assist", A_CANT, 0);
   
    class_dspinit(delaylama_class);
    class_register(CLASS_BOX, delaylama_class);
    
    post("Basic Variable Feedback Delay/Enjoy!");
}

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

/* Our custom free routine */

void delaylama_free(t_delaylama *x)
{
    dsp_free((t_pxobject *) x);
    sysmem_freeptr(x->delay_line);
}

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

/* The 64 bit perform routine
 
 Works only with MAX/MSP 6 and 7

 */

void delaylama_perform64(t_delaylama *x, t_object *dsp64, double **ins,
                       long numins, double **outs,long numouts, long n,
                       long flags, void *userparam)
{
    /* first inlet from left is our signal input */
    
    t_double *input = (t_double *) ins[0];
    
    /* second inlet from left is our delay time */
    
    t_double *delaytime = (t_double *) ins[1];
    
    /* third inlet from left is our feedback factor */
    
    t_double *feedback = (t_double *) ins[2];
    
    /* our external has only one ouptput */
    t_double *output = (t_double *) outs[0];
    
    /* assigning the needed variables from the object structure into local variables
       cause it's more efficient to use local variables rather than pull them off of the
       object structure every single time they are needed!
     */
    float sr = x->sampling_rate;
    float *delay_line = x->delay_line;
    float  *read_ptr  = x->read_ptr;
    float  *write_ptr = x->write_ptr;
    long delay_length_as_samples = x->delay_length_as_samples;
    
    /* the address of the beginning of the delay line plus the number of samples in the
       the delay line multiplied by the size of float.So end_of_memory will point to
       one slot beyond the last float in the delay line memory
     */
    float *end_of_memory = delay_line + delay_length_as_samples;
    
    short delaytime_connected = x->delaytime_connection_status;
    short feedback_connected =  x->feedback_connection_status;
    
    float delaytime_float = x->delay_time;
    float feedback_float = x->feedback;
    /* fractional difference used for our simple linear interpolation */
    float fractional_difference;
    float frac_delay;
    /* samp1 & samp2 are used for linear interpolation and are taken from our delay line */
    float samp1;
    float samp2;
    long int_delay;
    float samp_rate_seconds = sr / 1000.0; // convert milliseconds to seconds
    float output_sample;
    float feedback_sample;
    
    
    /*
     From here on we choose the correct DSP configuration based on which inlets are
     connected and which of them aren't.
     
     Our delay time and feedback inlets accept both floats and signals
     
     the logic of the chosen decision tree is as follows:
     
     if (both delay_time and feedback are connected){...}
     
     else if (delay_time is connected){...}
     
     else if( feedback is connected){...}
     
     else {...}
     
     */
    
    
    /* By truncating the actual delay time and subtract the truncated delay time
       from the actual delay time we get a fraction. We can use this fraction to determine
       the relative contribution of two samples, the first one at the truncated delay time slot
       and the second sample will be taken one slot beyond the first delay time slot. But why
       linear interpolation? well consider the case that we have a delay time of 69 ms. Converting
       milliseconds to samples we get 69 * 44100 / 1000 = 3042.9 samples! we can store this sample
       at delay 3042 or 3043 but not between these two points! so using linear interpolation
       instead of just rounding gives us a smoother transition.
     also note that one of the most basic and simplest forms of
     linear interpolation has been implemented here.
     */
 
    
    
    if(delaytime_connected && feedback_connected){
        while(n--){
            frac_delay = *delaytime++ * samp_rate_seconds;
            while(frac_delay > delay_length_as_samples){
                frac_delay -= delay_length_as_samples;
            }
            while(frac_delay < 0){
                frac_delay += delay_length_as_samples ;
            }
            int_delay = (int)frac_delay ;
            fractional_difference = frac_delay - int_delay ;
            read_ptr = write_ptr - int_delay ;
            while(read_ptr < delay_line){
                read_ptr += delay_length_as_samples;
            }
            
            /* here the first sample is taken for linear interpolation */
            samp1 = *read_ptr++;
            /* if our read pointer has reached beyond the end of the delay line
             it is set to the beginning of the delay line
             */
            if(read_ptr == end_of_memory){
                read_ptr = delay_line;
            }
            /* here the second sample is taken for our linear interpolation */
            samp2 = *read_ptr;
            
            output_sample = samp1 + fractional_difference * (samp2-samp1);
            feedback_sample = output_sample * *feedback++;
            
            if(fabs(feedback_sample)  < 0.0000001){
                feedback_sample = 0.0;
            }
            /* here the input sample plus feedback is written to the current write location
             on the delay line
             */
            *write_ptr++ = *input++ + feedback_sample;
            
            /* the output sample is written to the output signal vector
             & the write pointer is incremented
             */
            *output++ = output_sample;
            
            /* if our write pointer has reached beyond the end of the delay line
             it is set to the beginning of the delay line */
            
            if(write_ptr == end_of_memory){
                write_ptr = delay_line;
            }
        }
    }
    
    
    
    else if(delaytime_connected){
        
            /* while there's still some input coming */
            while(n--){
                frac_delay = *delaytime++ * samp_rate_seconds;
                /* if the fractional_differenceal delay is less than zero, set it to a default*/
                if(frac_delay < 0){
                    frac_delay = 0.;
                }
                /* if the fractional delay goes over the range, bring it back! */
                else if(frac_delay >= delay_length_as_samples) {
                    frac_delay = delay_length_as_samples - 1;
                }
                int_delay = trunc(frac_delay);
                fractional_difference = frac_delay - int_delay;
                read_ptr = write_ptr - int_delay;
                
                while(read_ptr < delay_line){
                    read_ptr += delay_length_as_samples;
                }
                /* here the first sample is taken for linear interpolation */
                samp1 = *read_ptr++;
                if(read_ptr == end_of_memory){
                    read_ptr = delay_line;
                }
                /* here the second sample is taken for our linear interpolation */
                samp2 = *read_ptr;
                output_sample = samp1 + fractional_difference * (samp2-samp1);
                feedback_sample = output_sample * feedback_float;
                
                if(fabs(feedback_sample)  < 0.0000001){
                    feedback_sample = 0.0;
                }
                
                *write_ptr++ = *input++ + feedback_sample;
                *output++ = output_sample;
                if(write_ptr == end_of_memory){
                    write_ptr = delay_line;
                }
            }
        }

    
    
    
    else if(feedback_connected){
        
        frac_delay = delaytime_float * samp_rate_seconds;
        while(frac_delay > delay_length_as_samples){
            frac_delay -= delay_length_as_samples;
        }
        while(frac_delay < 0){
            frac_delay += delay_length_as_samples;
        }
        int_delay = trunc(frac_delay);
        fractional_difference = frac_delay - int_delay;
        while(n--){
            
            read_ptr = write_ptr - int_delay;
            while(read_ptr < delay_line){
                read_ptr += delay_length_as_samples;
            }
            samp1 = *read_ptr++;
            if(read_ptr == end_of_memory){
                read_ptr = delay_line;
            }
            samp2 = *read_ptr;
            output_sample = samp1 + fractional_difference * (samp2-samp1);
            feedback_sample = output_sample * *feedback++;
            
            if(fabs(feedback_sample)  < 0.0000001){
                feedback_sample = 0.0;
            }
            
            *write_ptr++ = *input++ + feedback_sample;
            *output++ = output_sample;
            if(write_ptr == end_of_memory){
                write_ptr = delay_line;
            }
        }
    }

    
    
    
    else {
        
        frac_delay = delaytime_float * samp_rate_seconds;
        while(frac_delay > delay_length_as_samples){
            frac_delay -= delay_length_as_samples;
        }
        while(frac_delay < 0){
            frac_delay += delay_length_as_samples;
        }
        int_delay = trunc(frac_delay);
        fractional_difference = frac_delay - int_delay;
        while(n--){
            
            read_ptr = write_ptr - int_delay;
            while(read_ptr < delay_line){
                read_ptr += delay_length_as_samples;
            }
            samp1 = *read_ptr++;
            if(read_ptr == end_of_memory){
                read_ptr = delay_line;
            }
            samp2 = *read_ptr;
            output_sample = samp1 + fractional_difference * (samp2-samp1);
            feedback_sample = output_sample * feedback_float;
            
            // some sanity checking incase the user types garbage
            if(fabs(feedback_sample) < 0.0000001){
                feedback_sample = 0.0;
            }
            
            *write_ptr++ = *input++ + feedback_sample;
            *output++ = output_sample;
            if(write_ptr == end_of_memory){
                write_ptr = delay_line;
            }
        }
    }
    
    /* since the current value of our write pointer has changed in our perform routine
     it must be copied back to the object structure.
     */
    x->write_ptr = write_ptr;    
    
}
    



/* The 64 bit DSP method, works only with MAX/MSP 6 & 7 */

void delaylama_dsp64(t_delaylama *x, t_object *dsp64, short *count, double sr, long n, long flags)
{
    /*In case for whatever reason the sampling rate is zero, just exit! */
    
    if(!sr){
        return;
    }
    
    /* Store the states of signal inlet connections
       if the delay time is connected or not or if our feedback is connected or not
     */
    
    x->delaytime_connection_status = count[1];
    x->feedback_connection_status = count[2];
    
    /* If the user changes the sampling rate the delayline has to be set again */
    
    if(x->sampling_rate != sr){
        
        x->sampling_rate = sr ;
        
        x->delay_length_as_samples = x->sampling_rate * x->maximum_delay_time + 1;
        x->delay_length_as_bytes = x->delay_length_as_samples * sizeof(float);
        
        /*
         new memory block allocation, if the delayline pointer is set to NULL .
         sysmem_newptrclear() returns the memory with all of its values set to zero.
         */
        
        if(x->delay_line == NULL){
            x->delay_line = (float *) sysmem_newptrclear(x->delay_length_as_bytes);
        }
        
        /*
         else,the existing memory block gets resized. sysmem_resizeptrclear()
         sets all of the returned memory locations to zero.
         */
        
        else {
            x->delay_line = (float *)
            sysmem_resizeptrclear((void *)x->delay_line,
             
                                  x->delay_length_as_bytes);
        }
        
        /* if the delay line is still Null just let the user know that there was a problem
         with reallocating memory*/
        
        if(x->delay_line == NULL){
            error("delaylama~: failed to reallocate %d bytes of memory", x->delay_length_as_bytes);
            return;
        }
        
        /* Assign the write pointer to the beginning of the delayline
           whenever dynamic memory is initialized or resized.
         */
        
        x->write_ptr = x->delay_line;
    }
    
     dsp_add64(dsp64, (t_object*)x, (t_perfroutine64)delaylama_perform64 , 0 , NULL);
    
}

