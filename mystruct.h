//
//  mystruct.h
//  max-external
//
/**
 *  @author Tom     Berkmann    375851,<br>
 *  @author Olivier Faure  Brac 382122,<br>
 *  @author Hamed   Habibi Fard 385540,<br>
 *  @date   10/26/18.<br>
 *  @brief  The object structure for our external with its state variables<br>
 *  @var    sampling rate <br>
 *  @var    samp_rate_ms <br>
 *  @var    delay_length_as_samples <br>
 *  @var    delay_length_as_bytes <br>
 *  @var    delay_time <br>
 *  @var    feedback <br>
 *  @var    maximum_delay_time <br>
 *  @var    delay_line <br>
 *  @var    *write_ptr <br>
 *  @var    *read_ptr  <br>
 *  @var    frac_delay <br>
 *  @var    int_delay  <br>
 *  @var    fractional_difference <br>
 *  @var    delaytime_connection_status <br>
 *  @var    feedback_connection_status  <br>
 */

#include "ext.h"  /**< must be included first */
#include "z_dsp.h"
#include "ext_obex.h"

#ifndef mystruct_h
#define mystruct_h

/* The object structure for our external with it's state variables  */
typedef struct _delaylama {
    
    t_pxobject audio_obj;            /**< must be the first line in our object structure(MAX/MSP rule!) */
    
    float sampling_rate;            /**< sampling rate */
    float samp_rate_ms;            /**<  sampling rate (milliseconds/MAX standard) */
    long delay_length_as_samples; /**< length of the delay line in samples */
    long delay_length_as_bytes;  /**<   length of delay line in bytes     */
    
    float delay_time;           /**< the current delay time */
    float feedback;            /**<   feedback multiplier */
    
    float maximum_delay_time; /**< maximum delay time */
    
    
    float *delay_line; /**< our delay line itself */
    
   /**
    *   we only need to keep track of the write pointer cause the read pointer will always follow the write
    *   pointer by a number of samples determined by the delay line
    */
    
    float *write_ptr; /**< write pointer into delay line */
    float *read_ptr; /**<  read pointer into delay line  */
    
    
    
    
    
    float frac_delay; /**< the fractional  delay time */
    long int_delay; /**<   the integer delay time    */
    float fractional_difference; /**< the fractional difference between the fractional and integer delay times */
    
    short delaytime_connection_status ; /**< the connection status of this inlet */
    short feedback_connection_status  ; /**< the connection status of this inlet  */
    
} t_delaylama;

#endif /* mystruct_h */
