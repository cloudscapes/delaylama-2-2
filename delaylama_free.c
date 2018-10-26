//
//  delaylama_free.c
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

/* Our custom free routine */

void delaylama_free(t_delaylama *x)
{
    dsp_free((t_pxobject *) x);
    sysmem_freeptr(x->delay_line);
}
