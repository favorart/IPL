#include "Header.h"
#include "InterpreterNamesTable.h"
#include "Interpreter.h"

#include "InterpreterLexical.h"
#include "InterpreterSyntax.h"
#include "InterpreterExecute.h"
#include "InterpreterPreCompile.h"

int   StartInterpret (names_table *Names_table);
//-------------------------------------------------------------------------------
// interpreter Interpreter = {0};
int   Interpret (interpreter *Interpreter)
{ int result = 0;
  names_table Names_table;
  *_Names_table() = NULL;

  if( !Interpreter ) { SetError(ERR_PTR); result = 1; goto END; }
  
  if( !Interpreter->script ) Interpreter->script = "script.txt";
  if( !Interpreter->config ) Interpreter->config = "eng";

  if( !Interpreter->f_in   ) Interpreter->f_in   = stdin;
  if( !Interpreter->f_out  ) Interpreter->f_out  = stdout;
  if( !Interpreter->f_err  ) Interpreter->f_err  = stderr;

  if( !Interpreter->newSyntaxAnalyzer )
   Interpreter->newSyntaxAnalyzer = defaultSyntaxAnalyzer;

  // создание обработчика ошибок
  InitError (); //---занулить CurError()
   CurError ()->line=1;
   ex_random (NULL,NULL);

  if(  Interpreter->Syntax.variables_declaration > IPL_SYNTAX_VAR_DECL_LRIGHT ) 
   SetError (ERR_INTR);
  if( !Interpreter->Syntax.variables_declaration )
   Interpreter->Syntax.at_exp_end_semicolons = 1;

  *_Names_table() = &Names_table;
  // создание хэш-таблиц где будут хранится все данные интерпретатора
  if( names_table_create (&Names_table,Interpreter) )
  { result = 1; goto END; }

  //----------Интерпретация----------------------------------------------------------------
  fprintf(Interpreter->f_out,
  "Start interpretation. It can takes some time to perform. Please, wait.\n");
  
  // запоминаем место для возврата в случае ошибки
  if ( !setjmp (CurError()->jump) ) // when executed, setjmp returns 0
  { StartInterpret (&Names_table); }
  else // when longjmp jumps back, setjmp returns non-NULL
  { /* место для обработки ошибок */ 
    ipc_files_remove (&Names_table);
    goto END;
  }

  fprintf(Interpreter->f_out,"\nThe program performed successfully.\n");
  //---------------------------------------------------------------------------------------
END:
  names_table_free (&Names_table);
  return result;
}
//-------------------------------------------------------------------------------
void  constants_initialize (constants *Constants)
{ myvar no = {0}; no.vtype = Constant; strcpy (no.name,NONE_NOT_NAME);
  constants_lookup (no.name,&no,HASH_ADD_ENABLE,Not);

  // True:
  myvar_create ( TRUE_INT_NAME,Int ); //---Int 
  myvar_create ( TRUE_STR_NAME,Str ); //---Flt
  myvar_create ( TRUE_FLT_NAME,Flt ); //---Str
  // Zeroes:
  myvar_create ( ZERO_INT_NAME,Int ); //---Int
  myvar_create ( ZERO_FLT_NAME,Flt ); //---Flt
  myvar_create ( ZERO_STR_NAME,Str ); //---Str
  myvar_create ( ZERO_CNT_NAME,Cnt ); //---Cnt
  myvar_create ( ZERO_JNT_NAME,Jnt ); //---Jnt
}
//-------------------------------------------------------------------------------
#define _DEBUG_
#ifdef  _DEBUG_ //---DEBUG--------------------------
FILE**  _log_     (void);
void    _log_init (void);
#endif  //------------------------------------------

int   StartInterpret (names_table *Names_table)
{ int flag_end = EPROG_IDRW; int flag = 0;
 
  list_pmv Sentence = {0};
  exSentence exPreCompSentence = {0};

//#define _DEBUG_
#ifdef  _DEBUG_ //---DEBUG--------------------------
  _log_init ();
#endif  //------------------------------------------

  if( 1 )//Names_table->pUsrConf->modePreCompile ||\
      program_fread (Names_table,&Sentence) )
  { list_pmv PreCompCode     = {0};
    list_pmv PreCompSentence = {0};

    //---Стандартные константы
    constants_initialize (Names_table->pConstants);

    // если по каким-то причинам не удалось запустить псевдо-код
    fprintf (Names_table->pUsrConf->f_out, "Interpret from origin source code.\n");
    //---------------------------------------------
    do
    { // лексический анализатор
      if( !Sentence.len )
       if( laLexicalAnalyzer (Names_table,&Sentence) ) SetError (ERR_TERM);

#ifdef  _DEBUG_ //---DEBUG--------------------------
      { unsigned int i;
        fprintf (*_log_(),"\nlexical analyze;\n");
        for(i=0; i<Sentence.len; i++)
        { myvar *cur = *list_pmv_exact (&Sentence,i);
          fprintf (*_log_(),"[%s] ",myvar_name (cur));
        }
      }
#endif  //------------------------------------------
      // синтаксический анализатор
      if( !PreCompSentence.len )
         Names_table->pUsrConf->newSyntaxAnalyzer
       ( Names_table,&Sentence,&PreCompSentence );

      // заполняем псевдо-код в структуру, понятную выполнятелю
      exSentenceConvert (&exPreCompSentence,&PreCompSentence,Names_table);
      // выполнятель
      exInterpreter     (&exPreCompSentence,NULL,&flag_end);
      // добавляем в список финального псевдо-кода
      exSentenceCollect (&exPreCompSentence,&PreCompCode    ,Names_table);

    } while ( flag_end );

    // записываем псивдо-код в текстовый файл
    //program_fwrite ( Names_table,&PreCompCode );
    //---------------------------------------------
    list_pmv_free (&PreCompCode);
    list_pmv_free (&PreCompSentence);

  } // end if
  else // если удалось запустить псевдо-код
  { fprintf(Names_table->pUsrConf->f_out,"Interpret from pre-compiled code.\n");
    //---------------------------------------------
    { //unsigned int i; myvar *lines = *list_pmv_exact (&Sentence,0);

      //for(i=0; i<lines->value.lvalue->len; i++)
      //{ myvar *line = *list_pmv_exact (lines->value.lvalue,i );
      //  // заполняем псевдо-код в структуру, понятную выполнятелю
      //  exSentenceConvert  (&exPreCompSentence,line->value.lvalue,Names_table);
      //  // выполнятель
      //  exInterpreter      (&exPreCompSentence,NULL,&flag_end);
      //}

      unsigned int i;
      for(i=0; i<Sentence.len; i++)
      { myvar *line = *list_pmv_exact (&Sentence,i );
        // заполняем псевдо-код в структуру, понятную выполнятелю
        exSentenceConvert  (&exPreCompSentence,line->value.lvalue,Names_table);
        // выполнятель
        exInterpreter      (&exPreCompSentence,NULL,&flag_end);

        // очистка пре-комп. кода
        list_pmv_free      ( exPreCompSentence.make_sentence);
                 free      ( exPreCompSentence.make_sentence);
      }
    }
     //---------------------------------------------
  } // end else

  list_pmv_free (&Sentence);
  //---------------------------------------------
  return 0;
}
//-------------------------------------------------------------------------------