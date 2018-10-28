//
//  delaylama_free.c
//  max-external
//
/**
 *  @author Tom     Berkmann    375851,<br>
 *  @author Olivier Faure  Brac 382122,<br>
 *  @author Hamed   Habibi Fard 385540,<br>
 *  @date   10/26/18.<br>
 *  @brief  Frees a delaylama object <br>
 *  @param  x                        <br>
 */

#include "ext.h"  // must be included first 
#include "z_dsp.h"
#include "ext_obex.h"
#include <stdio.h>
#include "mystruct.h"
#include "delaylama_class.h"
#include "delaylama_utility.h"

// Our custom free routine 

void delaylama_free(t_delaylama *x)
{
    dsp_free((t_pxobject *) x);
    sysmem_freeptr(x->delay_line);
}
