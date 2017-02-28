#ifndef _NAME_LIST_H
#define _NAME_LIST_H

#include "defines.h"
//------------------------------------
#ifndef NAMELEN
#error NAMELEN must be defined.
#else
typedef struct _name name;
struct _name { char name[NAMELEN]; };
#endif
//------------------------------------
#define   GENERIC_LIST_POSTFIX  nm
#define   GENERIC_LIST_TYPE     name
#include "generic_list.h"
#undef    GENERIC_LIST_TYPE
#undef    GENERIC_LIST_POSTFIX
//------------------------------------
#endif
