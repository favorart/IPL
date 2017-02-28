#ifndef _SA_VARS_H
#define _SA_VARS_H

#include "Header.h"
#include "Interpreter.h"
//----------------------------------------------------------------------------------------------------
myvar* sa_vars_create_name  (names_table *Names_table, list_pmv *Sentence);
void   sa_vars_copy_2_base  (names_table *Names_table, myvar *pvar);
//----------------------------------------------------------------------------------------------------
myvar* sa_vars_fill_c       (names_table *Names_table, list_pmv *Sentence);
myvar* sa_vars_fill_py      (names_table *Names_table, list_pmv *Sentence);
myvar* sa_vars_fill_pas     (names_table *Names_table, list_pmv *Sentence);
//----------------------------------------------------------------------------------------------------
#endif
