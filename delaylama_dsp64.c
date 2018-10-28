//
//  delaylama_dsp64.c
//  max-external
//
/**
 *  @author Tom     Berkmann    375851,<br>
 *  @author Olivier Faure  Brac 382122,<br>
 *  @author Hamed   Habibi Fard 385540,<br>
 *  @date   10/26/18.<br>
 *  @brief  The 64 bit DSP method, works only with MAX/MSP 6 & 7 <br>
 *  @param x     <br>
 *  @param dsp64 <br>
 *  @param count <br>
 *  @param sr    <br>
 *  @param n     <br>
 * @param  flags <br>
 */

/**
 from MAX SDK Documentation Manual:
 
 The dsp64 method specifies the signal processing function your object defines along with its arguments. Your object's dsp64 method will be called when the MSP signal compiler is building a sequence of operations (known as the DSP Chain) that will be performed on each set of audio samples. The operation sequence consists of a pointers to functions (called perform routines) followed by arguments to those functions.
 To add an entry to the DSP chain, your dsp64 method uses the "dsp_add64" method of the DSP chain. The dsp_add64 method is passed an a pointer to your object, a pointer to a perform64 routine that calculates the samples, an optional flag which may alter behavior, and a generic pointer which will be passed on to your perform routine.
*/

#include "ext.h"  // must be included first
#include "z_dsp.h"
#include "ext_obex.h"
#include <stdio.h>
#include "mystruct.h"
#include "delaylama_class.h"
#include "delaylama_utility.h"




void delaylama_dsp64(t_delaylama *x, t_object *dsp64, short *count, double sr, long n, long flags)
{
    // In case for whatever reason the sampling rate is zero, just exit!
    
    if(!sr){
        return;
    }
    
    //Store the states of signal inlet connections if the delay time is connected or not or if our feedback is
    //...connected or not
    
    x->delaytime_connection_status = count[1];
    x->feedback_connection_status = count[2];
    
    // If the user changes the sampling rate the delayline has to be set again
    
    if(x->sampling_rate != sr){
        
        x->sampling_rate = sr ;
        
        x->delay_length_as_samples = x->sampling_rate * x->maximum_delay_time + 1;
        x->delay_length_as_bytes = x->delay_length_as_samples * sizeof(float);
        
        //new memory block allocation, if the delayline pointer is set to NULL . <br>
        //... sysmem_newptrclear() returns the memory with all of its values set to zero.
         
        
        if(x->delay_line == NULL){
            x->delay_line = (float *) sysmem_newptrclear(x->delay_length_as_bytes);
        }
        
        // else,the existing memory block gets resized. <br>
        //...sysmem_resizeptrclear() sets all of the returned memory locations to zero.
        
        
        else {
            x->delay_line = (float *)
            sysmem_resizeptrclear((void *)x->delay_line,
                                  
                                  x->delay_length_as_bytes);
        }
        
        // if the delay line is still Null just let the user know that there was a problem
        //... with reallocating memory
        
        
        if(x->delay_line == NULL){
            error("delaylama~: failed to reallocate %d bytes of memory", x->delay_length_as_bytes);
            return;
        }
        
        // Assign the write pointer to the beginning of the delayline
        //... whenever dynamic memory is initialized or resized.
        
        
        x->write_ptr = x->delay_line;
    }
    
    dsp_add64(dsp64, (t_object*)x, (t_perfroutine64)delaylama_perform64 , 0 , NULL);
    
}

