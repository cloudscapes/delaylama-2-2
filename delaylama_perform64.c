//
//  delaylama_perform64.c
//  max-external
//
/**
 *  @author Tom     Berkmann    375851,<br>
 *  @author Olivier Faure  Brac 382122,<br>
 *  @author Hamed   Habibi Fard 385540,<br>
 *  @date   10/26/18.<br>
 *  @brief  The Performance Routine, works only with MAX/MSP 6 and 7 <br>
 *  @param x               <br>
 *  @param dsp64           <br>
 *  @param ins             <br>
 *  @param numins          <br>
 *  @param outs            <br>
 *  @param numouts         <br>
 *  @param n               <br>
 *  @param flags           <br>
 *  @param userparam       <br>
 *  @var   input           <br>
 *  @var   delaytime       <br>
 *  @var   feedback        <br>
 *  @var   output          <br>
 *  @var   sr              <br>
 *  @var   delay_line      <br>
 *  @var   read_ptr        <br>
 *  @var   write_ptr       <br>
 *  @var   delay_length_as_samples    <br>
 *  @var   end_of_memory              <br>
 *  @var   delaytime_connected        <br>
 *  @var   feedback_connected         <br>
 *  @var   delaytime_float            <br>
 *  @var   fractional_difference      <br>
 *  @var   frac_delay                 <br>
 *  @var   samp1                      <br>
 *  @var   samp2                      <br>
 *  @var   int_delay                  <br>
 *  @var   samp_rate_seconds          <br>
 *  @var   output_sample              <br>
 *  @var   feedback_sample            <br>
 */

#include "ext.h"  /** must be included first */
#include "z_dsp.h"
#include "ext_obex.h"
#include <stdio.h>
#include "mystruct.h"
#include "delaylama_class.h"
#include "delaylama_utility.h"




void delaylama_perform64(t_delaylama *x, t_object *dsp64, double **ins,
                         long numins, double **outs,long numouts, long n,
                         long flags, void *userparam)
{
    /** first inlet from left is our signal input */
    
    t_double *input = (t_double *) ins[0];
    
    /** second inlet from left is our delay time */
    
    t_double *delaytime = (t_double *) ins[1];
    
    /** third inlet from left is our feedback factor */
    
    t_double *feedback = (t_double *) ins[2];
    
    /** our external has only one ouptput */
    t_double *output = (t_double *) outs[0];
    
    /** assigning the needed variables from the object structure into local variables
     cause it's more efficient to use local variables rather than pull them off of the
     object structure every single time they are needed!
     */
    float sr = x->sampling_rate;
    float *delay_line = x->delay_line;
    float  *read_ptr  = x->read_ptr;
    float  *write_ptr = x->write_ptr;
    long delay_length_as_samples = x->delay_length_as_samples;
    
    /** the address of the beginning of the delay line plus the number of samples in the
     the delay line multiplied by the size of float.So end_of_memory will point to
     one slot beyond the last float in the delay line memory
     */
    float *end_of_memory = delay_line + delay_length_as_samples;
    
    short delaytime_connected = x->delaytime_connection_status;
    short feedback_connected =  x->feedback_connection_status;
    
    float delaytime_float = x->delay_time;
    float feedback_float = x->feedback;
    /** fractional difference used for our simple linear interpolation */
    float fractional_difference;
    float frac_delay;
    /** samp1 & samp2 are used for linear interpolation and are taken from our delay line */
    float samp1;
    float samp2;
    long int_delay;
    float samp_rate_seconds = sr / 1000.0; // convert milliseconds to seconds
    float output_sample;
    float feedback_sample;
    
    
    /**
     From here on we choose the correct DSP configuration based on which inlets are
     connected and which of them aren't.
     
     Our delay time and feedback inlets accept both floats and signals
     
     the logic of the chosen decision tree is as follows:
     
     if (both delay_time and feedback are connected){...}
     
     else if (delay_time is connected){...}
     
     else if( feedback is connected){...}
     
     else {...}
     
     */
    
    
    /** By truncating the actual delay time and subtract the truncated delay time
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
            
            /** here the first sample is taken for linear interpolation */
            samp1 = *read_ptr++;
            /* if our read pointer has reached beyond the end of the delay line
             it is set to the beginning of the delay line
             */
            if(read_ptr == end_of_memory){
                read_ptr = delay_line;
            }
            /** here the second sample is taken for our linear interpolation */
            samp2 = *read_ptr;
            
            output_sample = samp1 + fractional_difference * (samp2-samp1);
            feedback_sample = output_sample * *feedback++;
            
            if(fabs(feedback_sample)  < 0.0000001){
                feedback_sample = 0.0;
            }
            /** here the input sample plus feedback is written to the current write location
             on the delay line
             */
            *write_ptr++ = *input++ + feedback_sample;
            
            /** the output sample is written to the output signal vector
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
        
        /** while there's still some input coming */
        while(n--){
            frac_delay = *delaytime++ * samp_rate_seconds;
            /** if the fractional_differenceal delay is less than zero, set it to a default */
            if(frac_delay < 0){
                frac_delay = 0.;
            }
            /** if the fractional delay goes over the range, bring it back! */
            else if(frac_delay >= delay_length_as_samples) {
                frac_delay = delay_length_as_samples - 1;
            }
            int_delay = trunc(frac_delay);
            fractional_difference = frac_delay - int_delay;
            read_ptr = write_ptr - int_delay;
            
            while(read_ptr < delay_line){
                read_ptr += delay_length_as_samples;
            }
            /** here the first sample is taken for linear interpolation */
            samp1 = *read_ptr++;
            if(read_ptr == end_of_memory){
                read_ptr = delay_line;
            }
            /** here the second sample is taken for our linear interpolation */
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
            
            /* some sanity checking incase the user types garbage */
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
    
    /** since the current value of our write pointer has changed in our perform routine
     it must be copied back to the object structure.
     */
    x->write_ptr = write_ptr;
    
}
