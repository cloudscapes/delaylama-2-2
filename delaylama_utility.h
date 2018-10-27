//
//  Header.h
//  max-external
/**
 *  @author Tom     Berkmann    375851, <br>
 *  @author Olivier Faure  Brac 382122, <br>
 *  @author Hamed   Habibi Fard 385540, <br>
 *  @date   10/26/18. <br>
 *  @brief all the function declarations put together into one header file <br>
*/

#ifndef Header_h
#define Header_h

#include "ext.h"  /**< must be included first */
#include "z_dsp.h"
#include "ext_obex.h"
#include "mystruct.h"

/** Function prototypes */
void *delaylama_new(t_symbol *s, short argc, t_atom *argv);

void delaylama_free(t_delaylama *x);

void delaylama_float(t_delaylama *x, double f);

void delaylama_assist(t_delaylama *x, void *b, long msg, long arg, char *dst);



void delaylama_perform64(t_delaylama *x, t_object *dsp64, double **ins,
                         long numins, double **outs,long numouts, long n,
                         long flags, void *userparam);

void delaylama_dsp64(t_delaylama *x, t_object *dsp64, short *count, double sr, long n, long flags);


#endif /* Header_h */
