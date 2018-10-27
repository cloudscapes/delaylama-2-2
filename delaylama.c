//
//  delaylama.c
//  max-external
// works only with max/msp 6 & 7 (64 bit versions)
// this code has not yet been tested with MAX 8!

/**
 *  @author Tom     Berkmann    375851,<br>
 *  @author Olivier Faure  Brac 382122,<br>
 *  @author Hamed   Habibi Fard 385540,<br>
 *  @date   10/26/18.<br>
 *  @brief  Main execution and method binding<br>
 */


/** The necessary Max/MSP header files */

#include "ext.h"  /**< must be included first */
#include "z_dsp.h"
#include "ext_obex.h"
#include "delaylama_utility.h"
#include "delaylama_class.h"



/** The main() function */

void ext_main(void *r)
{
    delaylama_class = class_new("delaylama~",(method)delaylama_new,(method)delaylama_free,sizeof(t_delaylama),0,A_GIMME,0);
    
    /** binding our methods */
    
    class_addmethod(delaylama_class, (method)delaylama_dsp64, "dsp64", A_CANT, 0);
    
    class_addmethod(delaylama_class, (method)delaylama_float, "float", A_FLOAT, 0);
    
    class_addmethod(delaylama_class, (method)delaylama_assist, "assist", A_CANT, 0);
   
    class_dspinit(delaylama_class);
    class_register(CLASS_BOX, delaylama_class);
    
    post("...Basic Variable Feedback Delay... Enjoy!");
}

