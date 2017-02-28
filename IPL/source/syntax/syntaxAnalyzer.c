#include "Header.h"
#include "lexicalCheck.h"
#include "interpreter.h"
#include "interpreterLexical.h"
#include "interpreterSyntax.h"

#include "pmvStack.h"

#define  defaultSyntaxAnalyzer  saDefault
//-------------------------------------------------------------------------------
INTEGER  toTypeInt (myvar* from)
{ INTEGER i=0;
  switch (from->type)
  { case Int: i = from->value.ivalue;                       break;
    case Str: sscanf (from->value.svalue,"%d",&i);          break;
    case Flt: i = (INTEGER)(from->value.fvalue);            break;
    default : SetCorrectError (myvar_name (from),ERR_TYPE); break;
  }
  return i;
}
char*    toTypeStr (myvar* from, char* str)
{ switch (from->type)
  { case Int: sprintf (str,I_OUTLINE,from->value.ivalue); break;
    case Str:  strcpy (str,          from->value.svalue); break;
    case Flt: sprintf (str,F_OUTLINE,from->value.fvalue); break;
    default : SetCorrectError(myvar_name(from),ERR_TYPE); break;
  }
  return str;
}
FLOAT    toTypeFlt (myvar* from)
{ FLOAT f=0.;
  switch (from->type)
  { case Int: f = (FLOAT)(from->value.ivalue);              break;
    case Str: sscanf (from->value.svalue,F_OUTLINE,&f);     break;
    case Flt: f = from->value.fvalue;                       break;
    default : SetCorrectError (myvar_name (from),ERR_TYPE); break;
  }
  return f;
}
//----------------------------------------------------------------------------------------------------
void  saDefault (names_table *Names_table, list_pmv *Sentence, list_pmv *PostfixSentence)
{ myvar *cur = laNextExact (Sentence);

  if( !Names_table->Flg.ProgramWord && (cur->rwtype != UFNC_IDRW) && (cur->rwtype != PROG_IDRW) )  
   SetCorrectError (myvar_name (cur),ERR_SYNX);

  if( cur->rwtype == PROG_IDRW ) 
  { laNextWord (Sentence);
    if(  Names_table->Flg.ProgramWord ) SetCorrectError (myvar_name (cur),ERR_SYNX);
    else Names_table->Flg.ProgramWord = 1;

    list_pmv_insert (PostfixSentence,-1,&cur);
    sa_program (Names_table,Sentence,PostfixSentence);
  }
  else
  { sa_postfix (Names_table,Sentence,PostfixSentence,EPROG_IDRW);
    saSeparate (Names_table,Sentence,SA_OTHER_CODE);
  }
}
//-------------------------------------------------------------------------------
#define _DEBUG_
#ifdef  _DEBUG_ //---------------------------------------------------------------
#define  MAX(a,b) ((a>b)?(a):(b))

#include <time.h>
FILE**  _log_     (void)
{ static FILE* log; return &log; }
void    _log_init (void)
{ FILE *flog = NULL; char str[STRLEN];

  struct tm *Tm;
  time_t t = time (&t); // Get time in seconds
  Tm  = localtime (&t); // Convert time to struct tm form 
  sprintf (str,"log\\log_file %s [%d-%d-%d %d-%d-%d].txt",(*_Names_table())->pUsrConf->script,
           Tm->tm_mday,Tm->tm_mon+1,Tm->tm_year+1900,Tm->tm_hour,Tm->tm_min,Tm->tm_sec);

  if( !(flog = fopen (str,"w")) )
   SetCorrectError (str,ERR_FILE);
  //{ perror (NULL); exit (1); }

  setbuf (flog,NULL); // отключить буферизацию
  *_log_() = flog;
}
//----------------------------------
int   myvar_print (myvar *pvar);
int   print_stack (stack_pmv *Stack);
char* regwords_get_rwenum_name   (unsigned int i);
//----------------------------------
int   myvar_print (myvar *pvar)
{ if( !pvar ) { fprintf (*_log_(),"NULL"); return 1; }
  fprintf (*_log_(),"\"%10s\"  ",myvar_name (pvar));
  switch (pvar->vtype)
  { case Unknow   : fprintf (*_log_(),"Unknow    "); break;
    case Variable : fprintf (*_log_(),"Variable  "); break;
    case Constant : fprintf (*_log_(),"Constant  "); break;
    case Reg_Word : fprintf (*_log_(),"Reg_Word  "); break;
    case Pre_Comp : fprintf (*_log_(),"Pre_Comp  "); break;
  }
  regwords_get_rwenum_name (pvar->rwtype);
  switch (pvar->type)
  { case Not : fprintf (*_log_(),"Not  "); break;
    case Int : fprintf (*_log_(),"Int  "); break;
    case Str : fprintf (*_log_(),"Str  "); break;
    case Flt : fprintf (*_log_(),"Flt  "); break;
    case Cnt : fprintf (*_log_(),"Cnt  "); break;
    case Jnt : fprintf (*_log_(),"Jnt  "); break;
    case Sgn : fprintf (*_log_(),"Sgn  "); break;
  }
  return 0;
}
//----------------------------------
int   print_status  (list_pmv *PostfixSentence, stack_pmv *Stack, int s)
{ size_t i, j, n = PostfixSentence->len;

  if( !s && Stack )
  { print_stack (Stack); fprintf (*_log_(),"\nPostfixSentence - %u:\n",n); }
  for(i=0; i<n; i++)
  { myvar *pvar = *list_pmv_exact (PostfixSentence,i);
    for(j=0; j<s; j++) fprintf (*_log_()," "); myvar_print (pvar);

    if( pvar && pvar->vtype == Pre_Comp )
    { fprintf (*_log_(),":\n"); print_status (pvar->value.lvalue,NULL,s+1); }
    else fprintf (*_log_(),"|\n");
  }
  return 0;
}
int   print_stack   (stack_pmv *Stack)
{ size_t i, n = Stack->len;

  fprintf (*_log_(), "\nStack - %u:\n",n);
  for (i = 0U; i < n; ++i)
  { myvar *pvar = Stack->content[i];
    myvar_print (pvar);
    fprintf (*_log_(), "|\n");
  }
  return 0;
}
#endif //------------------------------------------------------------------------
//-------------------------------------------------------------------------------
// приоритет операций
int Precedence (rwenum op) 
{ int result = 0;
  switch (op)
  { case BGN_IDRW:
     result = 1; break;
    case BND_ARTH_IDRW:
     result = 2; break;
    case AND_ARTH_IDRW:  case  OR_ARTH_IDRW://case XOR_ARTH_IDRW:   
     result = 3; break;
    case   L_ARTH_IDRW:  case   G_ARTH_IDRW:  case   E_ARTH_IDRW:
    case  LE_ARTH_IDRW:  case  GE_ARTH_IDRW:  case  NE_ARTH_IDRW:
     result = 4; break;
    case ADD_ARTH_IDRW:  case SUB_ARTH_IDRW:
     result = 5; break;
    case MUL_ARTH_IDRW:  case DIV_ARTH_IDRW:  case MOD_ARTH_IDRW:
     result = 6; break;
    case NOT_ARTH_IDRW:
     result = 7; break;
    default:
     result = 0; break;
  }
  return result; 
}
// Преобразование из Infix в обратную польскую нотацию
list_pmv* sa_postfix  (names_table *Names_table, list_pmv *Sentence, list_pmv *PostfixSentence, int flagEnd) 
{ int flag_obrace = 0, flag_sign = 0, flag_prev_var = 0; // флаги значения пред.символа
  int flag_obrace_count = 0;
  myvar *cur, *null = constants_lookup (ZERO_FLT_NAME,NULL,HASH_ADD_DISABLE,Flt);
  //-------------------------
  stack_pmv Stack; //---cтэк операций
  stack_pmv_create (&Stack,BUFLEN);
  //-------------------------
  cur = laNextWord (Sentence);

  // пропустить пустое выражение 
  if( (cur->rwtype == SEMIC_IDRW) || (cur->rwtype == COMMA_IDRW) ||
     ((cur->rwtype != EPROG_IDRW) && (cur->rwtype == (rwenum)flagEnd)) )
  { list_pmv_insert (Sentence,0,&cur); goto END; }

  // если унарная операция - минус или логическое отрицание
  if( cur->rwtype == SUB_ARTH_IDRW || cur->rwtype == NOT_ARTH_IDRW ) 
   // то записываем в выходную строку 0, чтобы получилась бинарная операция.
   list_pmv_insert (PostfixSentence,0,&null); 

  // выражение - пока рядом не будут 2 переменных
  while( !( flag_prev_var && ((cur->type != Sgn) || (cur->rwtype == BGN_IDRW)) )) 
  { if( cur->rwtype == BGN_IDRW ) // Если очеpедной символ - '('
    { stack_pmv_push (&Stack,&cur); // то заталкиваем её в стек.

      flag_prev_var = 0;
      flag_obrace   = 1;
      flag_sign     = 0;

      flag_obrace_count++;
    }
    else if( cur->rwtype == END_IDRW ) // Если очеpедной символ - ')'
    { if( !Stack.len ) // если не находим открывающейся 
       SetCorrectError (myvar_name (cur),ERR_NAME);

      if( sa_cnt_index__pre (Names_table,Sentence,PostfixSentence,&Stack,cur) )
      { flag_prev_var = 1;
        flag_obrace   = 0;
        flag_sign     = 0;      
      }
      else
      { while( Stack.content[Stack.len-1]->rwtype != BGN_IDRW )
        { // то выталкиваем из стека в выходную стpоку все знаки опеpаций
          // до ближайшей откpывающей скобки
          { myvar* pvar = stack_pmv_pop (&Stack);
            list_pmv_insert ( PostfixSentence,0,&(pvar) );
          }

          if( !Stack.len ) // если нет открывающейся 
           SetCorrectError(myvar_name (cur),ERR_NAME);
        }
        // Удаляем из стека саму откpывающую скобку.
        stack_pmv_pop(&Stack);

        flag_prev_var = 1;
        flag_obrace   = 0;
        flag_sign     = 0;
      }
      flag_obrace_count--;
    }
    else if( cur->type == Sgn )  // Если следующий символ - знак опеpации , то:
    { if( cur->rwtype == BND_ARTH_IDRW /* = */ && PostfixSentence->len == 0)
       SetError(ERR_VAR);
      
      if( sa_cnt_index__pre (Names_table,Sentence,PostfixSentence,&Stack,cur) )
      { flag_prev_var = 1;
        flag_obrace   = 0;
        flag_sign     = 0;     
      }
      else
      { if(!Stack.len)  // если стек пуст
         stack_pmv_push(&Stack,&cur); // записываем в него опеpацию
        else // если не пуст
         // если пpиоpитет поступившей опеpации больше пpиоpитета опеpации на веpшине стека
         if( Precedence(Stack.content[Stack.len-1]->rwtype) < Precedence(cur->rwtype) ) 
          stack_pmv_push(&Stack,&cur);   // заталкиваем поступившую опеpацию на стек   
         else   // если пpиоpитет меньше                           
         { while((Stack.len)&&(Precedence(Stack.content[Stack.len-1]->rwtype)>=Precedence(cur->rwtype)))
           // пеpеписываем в выходную стpоку все опеpации с большим или pавным пpиоpитетом
           { myvar* pvar = stack_pmv_pop(&Stack);
             list_pmv_insert (PostfixSentence,0,&(pvar) );
           }
           stack_pmv_push(&Stack,&cur); // записываем в стек поступившую опеpацию
         } 

        flag_prev_var = 0;
        flag_obrace   = 0;
        flag_sign     = 1;
      }
    }
    else // если константа, переменная, регулярное слово - не знак операции
    { list_pmv one_word = {0};
      list_pmv_insert (Sentence,0,&cur);
      // записываем его в выходную строку
      sa_one_word     ( Names_table,Sentence,&one_word );
      
      if( PostfixSentence->len )
      {   one_word.last->next        = PostfixSentence->first;
        PostfixSentence->first->prev =         one_word.last;
        PostfixSentence->first       =         one_word.first;

        PostfixSentence->len += one_word.len;
      }
      else *PostfixSentence = one_word;
      //----------------

      flag_prev_var = 1;
      flag_obrace   = 0;
      flag_sign     = 0;
    }

#ifdef  _DEBUG_ //---------------------------------------------------------------
    print_status (PostfixSentence,&Stack,0);
#endif //------------------------------------------------------------------------

    if( cur->rwtype == EPROG_IDRW )
    { if( laNextExact (Sentence) )
       cur = laNextWord (Sentence); 
     
      break; // !!! КОНЕЧНОЕ СЛОВО !!!
    } 

    // Пеpеход к следующему символу входной стpоки
    cur = laNextWord (Sentence);

    // Минус первый символ или унарное логическое отрицание
    if( flag_obrace && ( cur->rwtype == SUB_ARTH_IDRW || cur->rwtype == NOT_ARTH_IDRW ))
    { list_pmv_insert (PostfixSentence,0,&null); }
    // Если снова знак опеpации , то:
    else if( flag_sign && (cur->type == Sgn) && (cur->rwtype != BGN_IDRW) ) 
     SetError (ERR_NAME);

    // если достигнуто слово окончания выражения
    if( flagEnd && (flagEnd==(int)cur->rwtype) )
     if( flagEnd==(int)END_IDRW && !flag_obrace_count )
      break;

  } // end while

  // записываем символ начала следующего выражения обратно во входную строку
  list_pmv_insert (Sentence,0,&cur);

  // Пеpеписываем все опеpации из стека в выходную стpоку
  while(Stack.len) 
  { myvar* pvar = stack_pmv_pop(&Stack);
    list_pmv_insert (PostfixSentence,0,&(pvar) );
  }

END:;
  // очищаем память стэка
  stack_pmv_free(&Stack);
  // возвращаем алгебраическое выражение в правильном порядке
  return PostfixSentence;
}
//-------------------------------------------------------------------------------
list_pmv* sa_one_word (names_table *Names_table, list_pmv *Sentence, list_pmv *PostfixSentence)
{ myvar* cur = laNextWord (Sentence);
  //-----------------------------------------------
  if( (cur->type == Sgn) && ((cur->rwtype == SUB_ARTH_IDRW) || (cur->rwtype == NOT_ARTH_IDRW)) ) 
  { myvar* num = laNextWord (Sentence);
    
    // обработка унарных операторов - и ~
    if( num->type == Int ) 
    {      if( cur->rwtype == SUB_ARTH_IDRW ) num->value.ivalue = -num->value.ivalue; // -
      else if( cur->rwtype == NOT_ARTH_IDRW ) num->value.ivalue = !num->value.ivalue; // !
    }
    else if( num->type == Flt )
    {      if( cur->rwtype == SUB_ARTH_IDRW ) num->value.fvalue = -num->value.fvalue; // -
      else if( cur->rwtype == NOT_ARTH_IDRW ) num->value.fvalue = !num->value.fvalue; // !
    }
    cur = num; list_pmv_insert (PostfixSentence,-1,&cur);
  }
  else if( cur->vtype == Reg_Word )
  { list_pmv_insert (PostfixSentence,-1,&cur);
    // если регулярное слово - подбираем параметры: обычным или особым путём
    switch ( cur->rwtype )
    { case ADD_CNT_IDRW : case DEL_CNT_IDRW : case CLR_CNT_IDRW : case FND_CNT_IDRW :
      case SRT_CNT_IDRW : list_pmv_extract (PostfixSentence, -1, &cur);
                          sa_cnt_method_pre (Names_table,Sentence,PostfixSentence,cur);   break;
      case    NEED_IDRW :
      case    VARS_IDRW : list_pmv_extract  (PostfixSentence, -1, NULL);
                          sa_vars           (Names_table, Sentence);                      break;
      case      IF_IDRW : sa_if_else        (Names_table, Sentence, PostfixSentence);     break;
      case   WHILE_IDRW : sa_while          (Names_table, Sentence, PostfixSentence);     break;
      case     FOR_IDRW : sa_for            (Names_table, Sentence, PostfixSentence);     break;
      case      IN_IDRW : sa_input          (Names_table, Sentence, PostfixSentence);     break;
      case    UFNC_IDRW : list_pmv_extract  (PostfixSentence, -1, NULL);
                          sa_user_func_decl (Names_table, Sentence, cur);                 break;
      case   EPROG_IDRW :                                                                 break;
      default           : if( !cur->value.rvalue )  SetCorrectError (myvar_name (cur),ERR_NAME);
                          saGetStat (Names_table,Sentence,PostfixSentence,SA_FUNCT_PARS); break;
    }
  }
  else if( cur->vtype == Variable )
  { if( cur->type == Fnc )
    { list_pmv_insert (PostfixSentence,-1,&cur); // <<< user_func_call
      // если пользовательская функция - параметры вызова
      saGetStat  (Names_table,Sentence,PostfixSentence,SA_FUNCT_PARS);
    }
    else if( (cur->type == Cnt) || (cur->type == Jnt) )
    { list_pmv_insert (PostfixSentence,-1,&cur);  // <<< container|joint
      // если контейнер|структура - проверить на функции|на вызов полей
      sa_cnt_method (Names_table,Sentence,PostfixSentence);
    }
    else list_pmv_insert (PostfixSentence,-1,&cur);
  }
  else list_pmv_insert (PostfixSentence,-1,&cur);
  //-----------------------------------------------
  return PostfixSentence;
}
list_pmv* sa_resulted (names_table *Names_table, list_pmv *Sentence, list_pmv *PostfixSentence)
{ myvar *cur = laNextExact (Sentence);

  if( cur && cur->rwtype == END_IDRW )
  {       laNextWord  (Sentence);

    cur = laNextExact (Sentence);
    if( cur && cur->rwtype == RESL_IDRW )
    { if( Names_table->pUsrConf->Syntax.at_stat_return_result )
      { cur = laNextWord (Sentence);
        list_pmv_insert ( PostfixSentence,-1,&cur );

        cur = saNewSentence ();
        sa_one_word ( Names_table,Sentence,cur->value.lvalue );
        list_pmv_insert ( PostfixSentence,-1,&cur );
      }
      else SetCorrectError (cur->name,ERR_NAME);
    }
  }
  
  return PostfixSentence;
}
//----------------------------------------------------------------------------------------------------
void  saItemDest (names_table *Names_table)
{
  size_t i = 0;
  char *ch = Names_table->Flg.DepthOfItem;

  while( !(*ch) )
  { if( isDecPoint (*ch) )
      i++;
    ch++;
  }

  if ( i == 1 || (Names_table->Flg.DepthOfStat < 2) )
  {
    Names_table->Flg.DepthOfItem = ZERO_STR_NAME;
  }
  else if( i>1 )
  {
    size_t  p1, p2;
    ch = Names_table->Flg.DepthOfItem;    
  
    p1 = strlen (ch) - 1;
    ch[p1] = 0;
    
    p2 = strrchr (ch, *Names_table->Lex.Alphabet[ALPH_DEC_POINT]) - ch + 1;
    ch[p2] = 0;

    // 2.2.1. --> 2.2.
    Names_table->Flg.DepthOfItem = constants_lookup (ch,NULL,HASH_ADD_DISABLE,Not)->name;

    ch[p1] = *Names_table->Lex.Alphabet[ALPH_DEC_POINT];
    ch[p2] = *Names_table->Lex.Alphabet[ALPH_DEC_POINT];
  }
}
void  saItemStat (names_table *Names_table, list_pmv *Sentence, int *flagItem, unsigned int *noItem)
{ (*noItem)++;
  //--- производим проверку на пункты ---
  if( *flagItem || (*noItem == 1) )
  { 
    size_t n, val;
    myvar *item;
    char *ch = Names_table->Flg.DepthOfItem;
    
    item = laNextExact (Sentence);
    if( *noItem == 1 )
    {
      if ( (item->vtype == Constant) && !item->type )
      { *flagItem = 1; }
      else 
      { return; }
    }
    else if( (item->vtype != Constant) || item->type )
     SetCorrectError (myvar_name (item),ERR_SYNX);

    laNextWord (Sentence);
    if( n = strlen (ch) )
     if( *noItem > 1 )
     { ch[n-1] = 0;                                                            // 1.2.1. --> 1.2.1  
       n = strrchr (ch, *Names_table->Lex.Alphabet[ALPH_DEC_POINT]) - ch + 1;  // 1.2.1  --> 1.2.
       if( n<0 )
         n=0;
     }
    
    if( strncmp (ch,item->name,n) )  // 1.2.   ==  1.2.XXX
     SetCorrectError (myvar_name (item), ERR_SYNX);

    sscanf (item->name + n, I_OUTLINE, &val);
    if( val != *noItem ) // XXX == XXX
     SetCorrectError (myvar_name (item),ERR_SYNX);

    n = strlen (ch);
    if( n && (*noItem > 1) )
     ch[n] = *Names_table->Lex.Alphabet[ALPH_DEC_POINT];
    Names_table->Flg.DepthOfItem = item->name;
  }
  //---------------------------------------------
}
void  saSeparate (names_table *Names_table, list_pmv *Sentence, int  flagPars)
{ myvar *cur;
 
  int flag_semi = Names_table->pUsrConf->Syntax.at_exp_end_semicolons;
  int flag_comm = Names_table->pUsrConf->Syntax.at_func_params_commas;

  if( Names_table->Flg.GlobalComma ) flagPars = SA_FUNCT_PARS;

  cur = laNextExact (Sentence);
  if( cur && cur->rwtype != END_IDRW )
  { if( (flagPars == SA_FUNCT_PARS) && flag_comm )
    { if( (cur->rwtype == COMMA_IDRW) || /* ??? */ (cur->rwtype == AND_ARTH_IDRW) )
       laNextWord (Sentence); // --,
      else SetCorrectError (myvar_name(cur),ERR_SEPR);
    }
    else if( ((flagPars == SA_OTHER_CODE) && flag_semi) )
    { if( (cur->rwtype == SEMIC_IDRW) ) 
       laNextWord (Sentence); // --;
      else if( strcmp (cur->name,STR_SYMB_ENDOT) && !Names_table->Flg.GlobalEnDot ) // '.'?
       SetCorrectError (myvar_name(cur),ERR_SEPR);
    }
    else if( flagPars == SA_VARSC_CODE )
    { if( (cur->rwtype == SEMIC_IDRW) || (cur->rwtype == COMMA_IDRW) ) 
       laNextWord (Sentence); // --;,
      else if( strcmp (cur->name,STR_SYMB_ENDOT) && !Names_table->Flg.GlobalEnDot ) // '.'?
       SetCorrectError (myvar_name(cur),ERR_SEPR);
    }
    Names_table->Flg.GlobalEnDot = 0;
  }  
}
//----------------------------------------------------------------------------------------------------
void  saGetWord  (names_table *Names_table, list_pmv *Sentence, list_pmv *PostfixSentence)
{ myvar *obrace = laNextExact (Sentence);
  if( obrace->rwtype == BGN_IDRW )
   laNextWord (Sentence); // --(
  else obrace = NULL;
  //---*** <word> ***-----------------------------
  { myvar *cur = saNewSentence ();
    sa_one_word ( Names_table,Sentence,cur->value.lvalue );
    list_pmv_insert ( PostfixSentence,-1,&cur );
  }
  //----------------------------------------------
  if( obrace )
  { saEndWord (obrace,laNextExact (Sentence)); // --)
    sa_resulted (Names_table,Sentence,PostfixSentence);
  }
}
void  saGetExpr  (names_table *Names_table, list_pmv *Sentence, list_pmv *PostfixSentence, int flagResl)
{ myvar *obrace = laNextExact (Sentence);
  if( obrace->rwtype == BGN_IDRW )
   laNextWord (Sentence); // --(
  else obrace = NULL;
  //---*** <exp> ***------------------------------
  { myvar *cur = saNewSentence ();
    sa_postfix (Names_table,Sentence,cur->value.lvalue,END_IDRW);
    list_pmv_insert ( PostfixSentence,-1,&cur );
  }
  //----------------------------------------------
  if( obrace )
  { saEndWord (obrace,laNextExact (Sentence)); // --)
    if( flagResl ) sa_resulted (Names_table,Sentence,PostfixSentence);
    else laNextWord (Sentence);
  }
}
void  saGetStat  (names_table *Names_table, list_pmv *Sentence, list_pmv *PostfixSentence, int flagPars)
{ myvar *cur, *sen, *obrace, *cbrace; unsigned int noItem = 0; int flagItem = 0; int flag_changed = 0;

  if( (flagPars == SA_OTHER_CODE) && ((++Names_table->Flg.DepthOfStat) > SYNTAX_RECOMMEND_DEPTH) )
   SetWarning (myvar_name(laNextExact(Sentence)),WRG_DEPTH);
  //---*** <stat> ***-----------------------------
  sen = saNewSentence ();

  obrace = laNextExact (Sentence);
  if( obrace->rwtype == BGN_IDRW )
  { laNextWord (Sentence); // --(

    if( (flagPars == SA_FUNCT_PARS) && !Names_table->Flg.GlobalComma )
    { Names_table->Flg.GlobalComma = 1; flag_changed = 1; }

    do
    { if( flagPars == SA_OTHER_CODE )
       saItemStat (Names_table,Sentence,&flagItem,&noItem);

      cur = saNewSentence();
      sa_postfix (Names_table,Sentence,cur->value.lvalue,END_IDRW);
      list_pmv_insert ( sen->value.lvalue,-1,&cur );

      saSeparate (Names_table,Sentence,flagPars);
      if( flagPars == SA_OTHER_CODE )
       saItemDest (Names_table);

      cbrace = laNextExact (Sentence); 
    } while ( cbrace->rwtype != END_IDRW );
    saEndWord (obrace,cbrace);

    if( flag_changed ) Names_table->Flg.GlobalComma = 0;

    list_pmv_insert ( PostfixSentence,-1,&sen );
    sa_resulted (Names_table,Sentence,PostfixSentence);
  }
  else
  { cur = laNextExact (Sentence);
    if( (flagPars == SA_FUNCT_PARS) && Names_table->pUsrConf->Syntax.at_func_always_braces )
     SetCorrectError (myvar_name(cur),ERR_SYNX);
   
    cur = saNewSentence ();
    sa_postfix (Names_table,Sentence,cur->value.lvalue,END_IDRW);
    list_pmv_insert ( sen->value.lvalue,-1,&cur );

    //saSeparate  (Names_table,Sentence,SA_OTHER_CODE); // потому что _program "1";
    list_pmv_insert ( PostfixSentence,-1,&sen );
  }
  //----------------------------------------------
  if( (flagPars == SA_OTHER_CODE) && Names_table->Flg.DepthOfStat ) Names_table->Flg.DepthOfStat--;
}
//----------------------------------------------------------------------------------------------------
void  saEndWord  (myvar *OBrace, myvar *CBrace)
{ if( OBrace->sytype != CBrace->sytype )
   SetCorrectError (myvar_name (CBrace),ERR_SYNX);
  if( ((OBrace->rwtype ==    BGN_IDRW) && (CBrace->rwtype !=    END_IDRW)) ||
      ((OBrace->rwtype == OINDEX_IDRW) && (CBrace->rwtype != CINDEX_IDRW)) ||
      ((OBrace->rwtype != OINDEX_IDRW) && (OBrace->rwtype !=    BGN_IDRW)) )
   SetCorrectError (myvar_name (OBrace),ERR_SYNX);

  if( !strcmp (CBrace->name,STR_SYMB_ENDOT) ) (*_Names_table())->Flg.GlobalEnDot = 1;
}
//----------------------------------------------------------------------------------------------------
