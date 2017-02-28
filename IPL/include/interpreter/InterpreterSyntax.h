#ifndef _IPL_SYNTAX_H
#define _IPL_SYNTAX_H

typedef struct _names_table names_table;
typedef struct _stack_pmv stack_pmv;
typedef struct _list_pmv list_pmv;

#define SYNTAX_RECOMMEND_DEPTH   5
//-----------------------------------------------------------------------------------------
//---*** SYNTAX ANALYSIS ***---------------------------------------------------------------
#define   defaultSyntaxAnalyzer   saDefault

   myvar*  saNewSentence (void);
   void    saDefault     (names_table*,list_pmv*,list_pmv*);

list_pmv*  sa_program    (names_table*,list_pmv*,list_pmv*);
list_pmv*  sa_input      (names_table*,list_pmv*,list_pmv*);
list_pmv*  sa_if_else    (names_table*,list_pmv*,list_pmv*);
list_pmv*  sa_while      (names_table*,list_pmv*,list_pmv*);
list_pmv*  sa_for        (names_table*,list_pmv*,list_pmv*);

int    sa_user_func_decl (names_table*,list_pmv*,myvar*);
int    sa_user_func_file (names_table*,list_pmv*,myvar*);

void       sa_vars       (names_table*,list_pmv*);
list_pmv*  sa_postfix    (names_table*,list_pmv*,list_pmv*,int);
list_pmv*  sa_one_word   (names_table*,list_pmv*,list_pmv*);
list_pmv*  sa_resulted   (names_table*,list_pmv*,list_pmv*);

void       sa_cnt_method (names_table*,list_pmv*,list_pmv*);
void       sa_jnt_method (names_table*,list_pmv*,list_pmv*);
void   sa_cnt_method_pre (names_table*,list_pmv*,list_pmv*,myvar*);
int    sa_cnt_index__pre (names_table*,list_pmv*,list_pmv*,stack_pmv*,myvar*);

#define    SA_OTHER_CODE  0 
#define    SA_FUNCT_PARS  1
#define    SA_VARSC_CODE  2

void  saSeparate (names_table*,list_pmv*,int);
void  saGetStat  (names_table*,list_pmv*,list_pmv*,int);
void  saGetExpr  (names_table*,list_pmv*,list_pmv*,int);
void  saGetWord  (names_table*,list_pmv*,list_pmv*);
void  saEndWord  (myvar*,myvar*);
//-----------------------------------------------------------------------------------------
#endif
