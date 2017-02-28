#include "Header.h"
#include "Interpreter.h"
//-------------------------------------------------------------------------------
error* CurError (void)
{ static error Error; return &Error; }
void   SetError (terr err)
{ SetCorrectError(NULL, err); }

void   SetWarning      (char *buf, twrg wrg)
{ FILE *f_err = (*_Names_table())->pUsrConf->f_err;
  fprintf (f_err,"\nWarning: line %d word \"%s\"\n", CurError()->line, buf );

  switch(wrg)
  { case WRG_NO:   
     break;
    case WRG_NORET: fprintf (f_err,"Function is not return a value in the arithmetic sentence.\n");
     break;
    case WRG_CNVRT: fprintf (f_err,"Conversion this variable to this type does not save it's value.\n");
     break;
    case WRG_TCAST: fprintf (f_err,"There is will be type cast.\n");
     break;
    case WRG_DEPTH: fprintf (f_err,"There are too many nested blocks in program.\n");
     break;
    case WRG_NORSL: fprintf (f_err,"Instruction does no effect.\n");
     break;
  }

}
void   SetCorrectError (char *buf, terr err)
{ FILE *f_err;

  if( *_Names_table() ) 
  { f_err = (*_Names_table())->pUsrConf->f_err;
    fprintf (f_err, "\nERROR: The program can't be executed correctly.\n\
There is an error with \"%s\" on the line %d\n", buf, CurError()->line );
  }
  else f_err = stderr;

  switch(err)
  { case ERR_NO:   
     break;
    case ERR_NAME: fprintf(f_err,"Invalid instruction word.\n"); 
     break;
    case ERR_TYPE: fprintf(f_err,"This type of operand is illegal in this operation.\n"); 
     break;
    case ERR_VAR:  fprintf(f_err,"Left operand must be a variable.\n"); 
     break;
    case ERR_CAST: fprintf(f_err,"Type cast is illegal.\n"); 
     break;
    case ERR_ZERO: fprintf(f_err,"Dividing by zero.\n"); 
     break;
    case ERR_MANY: fprintf(f_err,"Too many instructions in parameters.\n"); 
     break;
    case ERR_MEM:  fprintf(f_err,"Allocation memory is failed.\n");
     break;
    case ERR_FILE: fprintf(f_err,"File can't be opened.\n");
     break;
    case ERR_END:  fprintf(f_err,"Unexpected end of file.\n");
     break;
    case ERR_CONF: fprintf(f_err,"Incorrect config file.\n");
     break;
    case ERR_PTR:  fprintf(f_err,"Function got invalid pointer parameter.\n");
     break;
    case ERR_TERM: fprintf(f_err,"Unexpected end of script file.\nPlease, write word of terminating at the end of program.\n");
     break;
    case ERR_RET:  fprintf(f_err,"Instruction must have return value.\n");
     break;
    case ERR_PAR:  fprintf(f_err,"Different type of parameter.\n");
     break;
    case ERR_FUNC: fprintf(f_err,"Function already has definition.\n");
     break;
    case ERR_SGN:  fprintf(f_err,"Unexpected sign.\n");
     break;
    case ERR_SYNX: fprintf(f_err,"Unexpected word - there is syntax error.\n");
     break;
    case ERR_NPAR: fprintf(f_err,"There are not parameters of function.\n");
     break;
    case ERR_SEPR: fprintf(f_err,"Separator must be here.\n");
     break; 
    case ERR_INTR: fprintf(f_err,"Incorrect interpreter-struct initialization.\n");
     break;
    case ERR_FINC: fprintf(f_err,"Error while reading script file.\n");
     break;
    case ERR_NJNT: fprintf(f_err,"There are no such key in IPL joint.\n");
     break;
  }

  //system("pause"); exit(1);
  CurError()->err = err;

  char memoryBlock[sizeof (jmp_buf)] = { 0 };
  if ( memcmp (CurError ()->jump, memoryBlock, sizeof (jmp_buf)) )
    longjmp (CurError()->jump, 1);
}
//-------------------------------------------------------------------------------