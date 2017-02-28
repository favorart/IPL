#ifndef _MYVAR_H
#define _MYVAR_H

#include "Header.h"
//--------------------------------------
#ifndef INTEGER
#define INTEGER     int
#define I_OUTLINE OUTLINE(d)
// #define INTEGER   __int64
// #define I_OUTLINE OUTLINE(ld)
// typedef signed long long INTEGER;
// #define I_OUTLINE        OUTLINE(ld)
#endif
#ifndef STRING
#define STRING        char*
#define S_OUTLINE     OUTLINE(s)
#define S_LEN_OUTLINE OUTLINE(CAT(STRLENZ,s))
#endif
#ifndef FLOAT
#define FLOAT    double
#define F_OUTLINE OUTLINE(lf)

// #define FLOAT     float
// #define F_OUTLINE OUTLINE(f)
// typedef double    FLOAT;
#endif
//-----------------------------------------------------------------------------------------
typedef enum {
     NO_IDRW  ,
//-----------------
   PROG_IDRW ,
   VARS_IDRW ,
   NEED_IDRW ,
   BGN_IDRW ,
     IF_IDRW ,
  WHILE_IDRW ,
    FOR_IDRW ,
     IN_IDRW ,
    OUT_IDRW ,
   SIZE_IDRW ,
   TYPE_IDRW ,
   CAST_IDRW ,
   RAND_IDRW ,
   UFNC_IDRW ,
    END_IDRW ,
   RESL_IDRW ,
  EPROG_IDRW ,
//-----------------
 BND_ARTH_IDRW ,
 ADD_ARTH_IDRW ,
 SUB_ARTH_IDRW ,
 MUL_ARTH_IDRW ,
 DIV_ARTH_IDRW ,
 MOD_ARTH_IDRW ,
  OR_ARTH_IDRW ,
 AND_ARTH_IDRW ,
 NOT_ARTH_IDRW ,
 XOR_ARTH_IDRW ,
   E_ARTH_IDRW ,
   L_ARTH_IDRW ,
   G_ARTH_IDRW ,
  NE_ARTH_IDRW ,
  LE_ARTH_IDRW ,
  GE_ARTH_IDRW ,
//-----------------
  ADD_CNT_IDRW ,
  DEL_CNT_IDRW ,
  FND_CNT_IDRW ,
  CLR_CNT_IDRW ,
  SRT_CNT_IDRW ,
   OINDEX_IDRW ,
   CINDEX_IDRW ,
 PSTINDEX_IDRW ,
 PREINDEX_IDRW ,
    FIELD_IDRW ,
//-----------------
    INT_IDRW ,
    STR_IDRW ,
    FLT_IDRW ,
    FNC_IDRW ,
    CNT_IDRW ,
    JNT_IDRW ,

     CNS_IDRW  ,
    ARR_IDRW ,
    LST_IDRW ,
//-----------------
   THEN_IDRW ,
   ELIF_IDRW ,
   ELSE_IDRW ,
//-----------------
   EACH_IDRW ,
   ELEM_IDRW ,
   FROM_IDRW ,
   UNTL_IDRW ,
   BYST_IDRW ,
   RVRS_IDRW  ,
//-----------------
     DO_IDRW ,
  BREAK_IDRW ,
//-----------------
   INTIN_IDRW ,
   STRIN_IDRW ,
   FLTIN_IDRW ,
//-----------------
  COMMA_IDRW ,
    SEMIC_IDRW  ,
//-----------------
  LCOMM_IDRW ,
   OCOMM_IDRW ,
  CCOMM_IDRW
//-----------------
} rwenum;
//-----------------------------------------------------------------------------------------
//---*** STRUCT MYVAR ***------------------------------------------------------------------
typedef struct _names_table names_table; struct _names_table;
typedef struct _hash_table    variables;
typedef struct _hash_table    constants;
typedef struct _myvar             myvar;
typedef struct _joint             joint;
typedef struct _container     container;
typedef struct _procedure     procedure;
typedef struct _list_pmv list_pmv; struct _list_pmv;
typedef myvar* (*p_action)(names_table*,list_pmv*);
//-----------------------------------------------------------------------------------------
typedef enum { Unknow = 0, Variable, Constant, Reg_Word, Pre_Comp } venum;
typedef enum { Not    = 0,      Int, Str, Flt, Fnc, Cnt, Jnt, Sgn } tenum;
//  +-------------------+----------------------+
//  |   0 - None        |     4 - Function     |
//  |   1 - Integer     |     5 - Container    |
//  |   2 - String      |     6 - Joint        |
//  |   3 - Float       |     7 - Sign         |
//  +-------------------+----------------------+
//-----------------------------------------------------------------------------------------
typedef union _tvalue tvalue;
union _tvalue           // значение переменной
{   INTEGER  ivalue;   // целое
       char* svalue;   // строка
      FLOAT  fvalue;   // вещественное
  procedure* pvalue;   // функция
  container* cvalue;   // контейнер ( массив / список )
      joint* jvalue;   // объединение
   p_action  rvalue;   // регулярное слово
   list_pmv* lvalue;   // выражение ИЯП
};
//----------------
struct _myvar          // переменная интерпретатора
{ char  name[NAMELEN]; // имя переменной, *name=0 если константа

   tenum   type;       // тип значения
   venum  vtype;       // тип переменной
  rwenum rwtype;       // смысл регулярного слова
  int    sytype;       // синхронизация синонимов рег.слов
  tvalue  value;       // значение переменной
};
//-----------------------------------------------------------------------------------------
// конвертировать строку в константу ( myvar )
myvar*  myvar_create       (char    *buf, tenum type);
myvar*  myvar_create_value (tvalue value, tenum type);

char*   myvar_name           (myvar *cur);
// очистить структуру myvar полностью
void    myvar_free           (myvar *var);
void    myvar_free_value     (myvar *var);

#define    MV_FREE_SELF_N     0
#define    MV_FREE_SELF_Y     1
void    myvar_free_count     (myvar *var, int flag);
// копировать структуру myvar
void    myvar_copy           (myvar *scr, myvar *dest);
void    myvar_copy_value     (myvar *scr, myvar *dest);
// сравнение структур myvar
int     myvar_compare_equal  (myvar  **a, myvar   **b);
int     myvar_compare        (myvar  **a, myvar   **b);
int     myvar_compare_revr   (myvar  **a, myvar   **b);
//-----------------------------------------------------------------------------------------
void    container_const_name (names_table *Names_table, char name[NAMELEN]);
void        joint_const_name (names_table *Names_table, char name[NAMELEN]);
//-----------------------------------------------------------------------------------------
int     isSimpleType (tenum type);
int     isEnumerable (tenum type);
//-----------------------------------------------------------------------------------------
myvar*    None (void);
int     isNone (myvar *cur);
//-----------------------------------------------------------------------------------------
#endif
