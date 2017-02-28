#ifndef _ERROR_H
#define _ERROR_H

#include <setjmp.h>

typedef struct _list  list;
struct _list;
//-----------------------------------------------------------------------------------------
typedef enum { ERR_NO = 0, ERR_NAME,  ERR_TYPE,  ERR_VAR,   ERR_CAST,  ERR_ZERO, 
               ERR_MANY,   ERR_MEM,   ERR_FILE,  ERR_END,   ERR_CONF,  ERR_PTR,
               ERR_TERM,   ERR_RET,   ERR_PAR,   ERR_FUNC,  ERR_SGN,   ERR_SYNX,
               ERR_NPAR,   ERR_SEPR,  ERR_INTR,  ERR_FINC,  ERR_NJNT             } terr;
typedef enum { WRG_NO = 0, WRG_NORET, WRG_CNVRT, WRG_TCAST, WRG_DEPTH, WRG_NORSL } twrg;

typedef struct _error error;
struct _error
{ terr           err; // номер ошибки
  unsigned int  line; // номер строки, в которой найдена ошибка
  jmp_buf       jump; // буфер для перехода в место, где будет обработка ошибоки
  
  // вывод конкретного ошибочного слова
  // unsigned int offset_line; // индекс в списке слов 
  // list*         words_line; // указатель на список слов
};
#define InitError() do { memset( CurError(),0,sizeof(error) ); } while(0)
//-----------------------------------------------------------------------------------------
error*  CurError(void);

// Обработчики ошибок
void    SetError   (terr err);
void    SetWarning (char *buf, twrg wrg);

void    SetCorrectError(char *buf, terr err);
//-----------------------------------------------------------------------------------------
#endif          
                