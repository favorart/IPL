#ifndef _INTERPRETER_H
#define _INTERPRETER_H

#include <stdio.h>
//-----------------------------------------------
#define   IPL_MODE_DBG                   0
#define   IPL_MODE_RLS                   1

#define   IPL_VIEW_ERRORS_YES            0
#define   IPL_VIEW_ERRORS_NOT            1

#define   IPL_PRE_COMPILE_YES            0
#define   IPL_PRE_COMPILE_NOT            1
//-----------------------------------------------
#define   IPL_SYNTAX_VAR_DECL_C_TYPE     0
#define   IPL_SYNTAX_VAR_DECL_PYTHON     1
#define   IPL_SYNTAX_VAR_DECL_PASCAL     2
#define   IPL_SYNTAX_VAR_DECL_VBASIC     2
//#define IPL_SYNTAX_VAR_DECL_JSCRPT     3 // NEED_VAR
#define   IPL_SYNTAX_VAR_DECL_LRIGHT     2

#define   IPL_SYNTAX_VAR_PAS_SEPR_FIELD  0
#define   IPL_SYNTAX_VAR_PAS_SEPR_BRACE  1

#define   IPL_SYNTAX_VAR_CNT_SEPR_BRACE  0
#define   IPL_SYNTAX_VAR_CNT_SEPR_INDEX  1
//-----------------------------------------------
typedef struct _names_table names_table;
typedef struct _list_pmv       list_pmv;
//-----------------------------------------------
typedef void (*ptrSyntaxAnalizer) (names_table *Names_table, list_pmv *List_Atoms, list_pmv *Result_Sentence);
//-----------------------------------------------
typedef struct _syntax_options syntaxOptions;
struct _syntax_options
{ unsigned int   no_mean_reg_of_letter; /* 0 = tolower()         */
	unsigned int   variables_declaration; /* 0 = C_TYPE            */

	unsigned int   at_vars_pas_sepr_type; /* 0 = SEPR_COLON(PASCAL)*/
	unsigned int   at_vars_cnt_sepr_type; /* 0 = CONT_BRACE        */

	// using: 
	unsigned int   at_exp_end_semicolons; /* 0 = NO, another = YES */
  unsigned int   at_stat_return_result; /* 0 = NO, another = YES */

  unsigned int   at_for_always_new_var; /* 0 = NO, another = YES */
  unsigned int   at_for_each_container; /* 0 = NO, another = YES */
  unsigned int   at_for_expr_condition; /* 0 = NO, another = YES */
  unsigned int   at_for_c_style_syntax; /* 0 = NO, another = YES */
  unsigned int   at_for_while_using_do; /* 0 = NO, another = YES */

  unsigned int   at_while_false_condit; /* 0 = NO, another = YES */
  unsigned int   at_if_else_using_then; /* 0 = NO, another = YES */

  unsigned int   at_func_always_braces; /* 0 = NO, another = YES */
	unsigned int   at_func_params_commas; /* 0 = NO, another = YES */

	unsigned int   at_obj_method_precall; /* 0 = NO, another = YES */
	unsigned int   at_obj_method_regular; /* 0 = NO, another = YES */
	unsigned int   at_obj_method_use_fld; /* 0 = NO, another = YES */
};
//-----------------------------------------------
typedef struct _IPL_interpreter interpreter;
struct _IPL_interpreter
{ int      mode;              /*  0 = INTERPRET_DBG              */
	int      modePreCompile;    /*  0 = INTERPRET_PRE_COMPILE_YES  */
	int      modeViewErrors;    /*  0 = INTERPRET_VIEW_ERRORS_YES  */

	ptrSyntaxAnalizer newSyntaxAnalyzer;          /*  0 = default  */
	syntaxOptions                Syntax;

	char* script;		 /*  0 = "script.txt"          */
	char* config;		 /*  0 = "eng"                 */

	FILE* f_in;  		 /* stream  INPUT; 0 = stdin   */
	FILE* f_out; 		 /* stream OUTPUT; 0 = stdout  */
	FILE* f_err; 		 /* stream ERRORS; 0 = stderr  */
};
//-----------------------------------------------
int  Interpret (interpreter *Interpreter);
//-----------------------------------------------
#endif

