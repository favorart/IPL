#ifndef _IPL_EXECUTE_H
#define _IPL_EXECUTE_H

#include "executeSentence.h"
#define  _EPS 0.000001
//-----------------------------------------------------------------------------------------
//---*** EXECUTION ***---------------------------------------------------------------------
INTEGER  toTypeInt (myvar*);
char*    toTypeStr (myvar*,char*);
FLOAT    toTypeFlt (myvar*);

myvar*   toTypeAny (names_table *Names_table, myvar *to, myvar *from);
//------------------------------------
void     variables_swap (names_table *Names_table, myvar *name);
myvar*   findRegWord    (hash_table *RegularWords, rwenum RedWord);
//------------------------------------
int      isTrueMyVar (myvar*);
myvar*   isTrueExpr  (exSentence*);
//-----------------------------------------------------------------------------------------
myvar*   exInterpreter (exSentence*,list_pmv*,int*);
myvar*   exPostfix     (exSentence*,int);
myvar*   exOneWord     (exSentence*,int);
myvar*   exResulted    (exSentence*);

myvar*   exProgram     (exSentence*);
myvar*   exIfElse      (exSentence*);
myvar*   exWhile       (exSentence*);
myvar*   exFor         (exSentence*);

myvar*   ex_input     (names_table*,list_pmv*);
myvar*   ex_output    (names_table*,list_pmv*);
myvar*   ex_length    (names_table*,list_pmv*);
myvar*   ex_type      (names_table*,list_pmv*);
myvar*   ex_cast      (names_table*,list_pmv*);
myvar*   ex_random    (names_table*,list_pmv*);
myvar*   ex_break     (names_table*,list_pmv*);
myvar*   ex_user_func (names_table*,list_pmv*,myvar*);

myvar*   ex_bind      (names_table*,list_pmv*);
myvar*   ex_not       (names_table*,list_pmv*);
myvar*   ex_add       (names_table*,list_pmv*);
myvar*   ex_sub       (names_table*,list_pmv*);
myvar*   ex_mul       (names_table*,list_pmv*);
myvar*   ex_div       (names_table*,list_pmv*);
myvar*   ex_mod       (names_table*,list_pmv*);
myvar*   ex_and       (names_table*,list_pmv*);
myvar*   ex_or        (names_table*,list_pmv*);
//myvar* ex_xor       (names_table*,list_pmv*);

myvar*   ex_equal     (names_table*,list_pmv*);
myvar*   ex_no_eq     (names_table*,list_pmv*);
myvar*   ex_gt_eq     (names_table*,list_pmv*);
myvar*   ex_ls_eq     (names_table*,list_pmv*);
myvar*   ex_great     (names_table*,list_pmv*);
myvar*   ex_less      (names_table*,list_pmv*);

myvar*  ex_cnt_add    (names_table*,list_pmv*);
myvar*  ex_cnt_del    (names_table*,list_pmv*);
myvar*  ex_cnt_fnd    (names_table*,list_pmv*);
myvar*  ex_cnt_clr    (names_table*,list_pmv*);
myvar*  ex_cnt_srt    (names_table*,list_pmv*);
myvar*  ex_cnt_index  (names_table*,list_pmv*);
//-----------------------------------------------------------------------------------------
#endif
