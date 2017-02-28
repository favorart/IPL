#ifndef _RWID_ARRAY_H
#define _RWID_ARRAY_H

#include <string.h>
//------------------------------------
typedef struct _rw_id rw_id;
struct  _rw_id { char *name;  rwenum id; };

int   rw_id_cmp (const void* a, const void* b)
{ return strcmp (((rw_id*)a)->name,((rw_id*)b)->name); }
//------------------------------------
#define   GENERIC_ARRAY_POSTFIX      id
#define   GENERIC_ARRAY_TYPE      rw_id
#include "generic_array.h"
#undef    GENERIC_ARRAY_TYPE
#undef    GENERIC_ARRAY_POSTFIX
//------------------------------------
#endif