#include "header.h"
#include "interpreter.h"
#include "interpreterExecute.h"

//-----------------------------------------------------------------------------------------
void      SetTitle (char* title);
void    exForUsual (exSentence *PreCompSentence);
void    exForCtype (exSentence *PreCompSentence);
//-----------------------------------------------------------------------------------------
//---*** EXECUTION ***---------------------------------------------------------------------
#define _DEBUG_
#ifdef  _DEBUG_ //---------------------------------
FILE**  _log_     (void);
int     print_status (list_pmv *PostfixSentence, void *Stack, int s);
#endif //------------------------------------------
myvar*  exInterpreter (exSentence *PreCompSentence, list_pmv *Parameters, int* flag_ifEnd)
{ 
  if( *PreCompSentence->len )
  { myvar *cur = exSentenceExact ( PreCompSentence,ORDER_CORRECT );
    
    if( cur->vtype == Pre_Comp )
    { while( *PreCompSentence->len )
      { cur = exSentenceSub ( PreCompSentence,Parameters );
#ifdef  _DEBUG_ //---------------------------------
        fprintf (*_log_(),"\n\nExecution:\n");
        print_status (PreCompSentence->exec_sentence,NULL,0);
#endif //------------------------------------------
      }
    }
    else if( flag_ifEnd && ((int)cur->rwtype == *flag_ifEnd) )
    { exSentenceExtract ( PreCompSentence,ORDER_CORRECT );
      *flag_ifEnd = 0; // конец программы
    }
    else
    { cur = exPostfix  ( PreCompSentence,EX_ONE_WORD );
      if(Parameters)
       list_pmv_insert ( Parameters,-1,&cur );
    }
    return cur; // OK!
  }
  return myvar_create (NONE_NOT_NAME,Not);
}
myvar*  exPostfix     (exSentence *PreCompSentence, int flag_atSen)
{ if( *PreCompSentence->len )
  { myvar *cur = exSentenceExact ( PreCompSentence,ORDER_CORRECT );
   
    if( *PreCompSentence->len > 1 )
    { if( cur->type != Sgn )
       return exOneWord ( PreCompSentence,(flag_atSen)?(EX_SENTENCE):(EX_ONE_WORD) );
      else 
      { myvar *par;
        list_pmv Parameters = {0};
        cur = exSentenceExtract ( PreCompSentence,ORDER_CORRECT ); // --Sgn

        par = exPostfix ( PreCompSentence,EX_SENTENCE ); // --Par_r
        //if( !par ) SetCorrectError (cur->name,ERR_RET);
                 list_pmv_insert ( &Parameters,0,&par );
        par = exPostfix ( PreCompSentence,EX_SENTENCE ); // --Par_l
        //if( !par ) SetCorrectError (cur->name,ERR_RET);
                 list_pmv_insert ( &Parameters,0,&par );

        cur = (*cur->value.rvalue) ( PreCompSentence->Names_table,&Parameters );
        return cur;
      }
    }
    return exOneWord ( PreCompSentence,(flag_atSen)?(EX_SENTENCE):(EX_ONE_WORD) );
  }
  return myvar_create (NONE_NOT_NAME,Not);
}
myvar*  exOneWord     (exSentence *PreCompSentence, int flag_atSen)
{ myvar *cur, *result = myvar_create (NONE_NOT_NAME,Not);

  if( cur = exSentenceExtract (PreCompSentence,ORDER_CORRECT) )
  { result = cur;

    // если регулярное слово
    if( cur->vtype == Reg_Word )
    {      if( cur->rwtype == PROG_IDRW  )
       result = exProgram (PreCompSentence);
      else if( cur->rwtype == IF_IDRW    )
       result = exIfElse  (PreCompSentence);
      else if( cur->rwtype == WHILE_IDRW )
       result = exWhile   (PreCompSentence);
      else if( cur->rwtype == FOR_IDRW   )
       result = exFor     (PreCompSentence);
      else
      { list_pmv Parameters = {0};
        myvar* par = exSentenceExact ( PreCompSentence,ORDER_CORRECT );
        if( !par || (par->vtype != Pre_Comp) )
         SetCorrectError (cur->name,ERR_NPAR);
        
        if( cur->rwtype == IN_IDRW )
        { // копируем, чтобы ничего не потерялось
          par = exSentenceExtract ( PreCompSentence,ORDER_CORRECT );
                  list_pmv_copy   ( par->value.lvalue,&Parameters );
        }
        else exSentenceSub (PreCompSentence,&Parameters); // par->value.lvalue - само возьмется

        if( !cur->value.rvalue ) SetCorrectError (cur->name,ERR_NAME);
        result  = (*cur->value.rvalue) (PreCompSentence->Names_table,&Parameters);
        // если есть возвращаемое значение, вычисляем ее и записываем ее в выходную строку
        if( !isNone (cur = exResulted (PreCompSentence)) ) result = cur; 
      }
    } // end if( reg.word )

    // если переменная
    else if( cur->vtype == Variable )
    { // если пользовательская функция
      if( cur->type == Fnc )
      { list_pmv Parameters={0}; exSentenceSub (PreCompSentence,&Parameters);
        result = ex_user_func (PreCompSentence->Names_table,&Parameters,cur);
        if( !isNone (cur = exResulted (PreCompSentence)) ) result = cur;
      }
      // если контейнерный тип данных
      else if( (cur->type == Cnt) || (cur->type == Jnt) )
      { list_pmv Parameters = {0};
        myvar *mth, *par = exSentenceExact (PreCompSentence,ORDER_CORRECT);
        
        if( !isNone (par) )
        { mth = *list_pmv_exact (par->value.lvalue,0);
          if( (mth->rwtype >= ADD_ARTH_IDRW) && (mth->rwtype <= SRT_CNT_IDRW) )
          { // в случае вызова метода контейнера
            mth = list_pmv_extracts (par->value.lvalue,0);
            if( !par->value.lvalue->len )
            { par = list_pmv_extracts (PreCompSentence->exec_sentence, 0);
                    list_pmv_insert  (PreCompSentence->make_sentence,-1,&par);
            }
            else exSentenceSub  (PreCompSentence,&Parameters);
            list_pmv_insert ((*list_pmv_exact (PreCompSentence->make_sentence,-1))->value.lvalue,0,&mth);            

            // el  = <Cnt> [ ADD_CNT_IDRW (   el & inx, el & None, ... )  |   el & inx|None  ]
            // el  = <Cnt> [ DEL_CNT_IDRW ( None & inx, el & None, ... )  | None & inx       ]
            // inx = <Cnt> [ FND_CNT_IDRW     el                                             ]
            //       <Cnt> [ CLR_CNT_IDRW                                                    ]
            //       <Cnt> [ SRT_CNT_IDRW                                                    ]

            list_pmv_insert (&Parameters,0,&cur); // кладем сам контейнер  
            if( !mth->value.rvalue ) SetCorrectError (myvar_name (mth),ERR_NAME);
            result = (*mth->value.rvalue) (PreCompSentence->Names_table,&Parameters);
            if( !isNone (cur = exResulted (PreCompSentence)) ) result = cur;
          }
          else // в случае индексации
          { exSentenceSub (PreCompSentence,&Parameters);
            list_pmv_insert (&Parameters,0,&cur);
            result = ex_cnt_index (PreCompSentence->Names_table,&Parameters);
          }
        }
        else exSentenceExtract (PreCompSentence,ORDER_CORRECT);

      } // end if( cont )
    } // end if( variable )

  } // end if( cur )

  // если функция ничего не возвращает и функция
  // находится посреди арифметического выражения
  if( isNone (result) && (flag_atSen == EX_SENTENCE) )
   // выдаем предупреждение
   SetWarning (myvar_name (cur),WRG_NORET);

  return result;
}
myvar*  exResulted    (exSentence *PreCompSentence)
{ myvar *rsl = None ();
  if( *PreCompSentence->len )
  { myvar *cur = exSentenceExact (PreCompSentence,ORDER_CORRECT);
    if( cur->rwtype == RESL_IDRW )
    {       exSentenceExtract (PreCompSentence,ORDER_CORRECT); // --RESL_IDRW
      rsl = exInterpreter     (PreCompSentence,NULL,NULL);
    }
  }
  return rsl;
}
//-----------------------------------------------------------------------------------------
myvar*  exProgram     (exSentence *PreCompSentence)
{ myvar *name = exSentenceExtract (PreCompSentence,ORDER_CORRECT);
  if( name->vtype != Variable )
   SetCorrectError (myvar_name (name),ERR_PAR);
  SetTitle (myvar_name (name));

  return None ();
}
myvar*  exIfElse      (exSentence *PreCompSentence)
{ int flag_true = 0; myvar *rsl = None (), *cur = NULL; //list_pmv Parameters = {0};

  exSentenceUndo (PreCompSentence);
  do
  {         exSentenceExtract (PreCompSentence,ORDER_CORRECT);

    if( flag_true = (int)isTrueMyVar (isTrueExpr (PreCompSentence)) ) // --<exp> == TRUE?
    {       exSentenceSub (PreCompSentence,NULL); // --<stat>
         // exInterpreter (PreCompSentence,NULL,NULL ); // --<stat> (exSentenceExtract внутри)
      rsl = exResulted    (PreCompSentence);

      while((*PreCompSentence->len) && (cur = exSentenceExact (PreCompSentence,ORDER_CORRECT)) && (cur->rwtype == ELIF_IDRW))
      { exSentenceExtract (PreCompSentence,ORDER_CORRECT); // --ELIF_IDRW
        exSentenceExtract (PreCompSentence,ORDER_CORRECT); // --<exp>
        exSentenceExtract (PreCompSentence,ORDER_CORRECT); // --<stat>

        if( (*PreCompSentence->len) && (cur = exSentenceExact (PreCompSentence,ORDER_CORRECT)) && (cur->rwtype == RESL_IDRW))
        { exSentenceExtract (PreCompSentence,ORDER_CORRECT); // --RESL_IDRW
          exSentenceExtract (PreCompSentence,ORDER_CORRECT); // --<stat>
        } // end if (resulted)
      } // end if (elif)

      if(   (*PreCompSentence->len) && (cur = exSentenceExact (PreCompSentence,ORDER_CORRECT)) && (cur->rwtype == ELSE_IDRW))
      { exSentenceExtract (PreCompSentence,ORDER_CORRECT);  // --ELSE_IDRW
        exSentenceExtract (PreCompSentence,ORDER_CORRECT);  // --<stat>

        if( (*PreCompSentence->len) && (cur = exSentenceExact (PreCompSentence,ORDER_CORRECT)) && (cur->rwtype == RESL_IDRW))
        { exSentenceExtract (PreCompSentence,ORDER_CORRECT); // --RESL_IDRW
          exSentenceExtract (PreCompSentence,ORDER_CORRECT); // --<stat>
        } // end if (resulted)
      } // end if (else)

      break;
    } // end if (true)
    else
    {       exSentenceExtract (PreCompSentence,ORDER_CORRECT);  // --<stat>
      cur = exSentenceExact   (PreCompSentence,ORDER_CORRECT);  // --ELIF_IDRW||ELSE_IDRW
    } 

  } while( cur &&( cur->rwtype == ELIF_IDRW ) );

  if( !flag_true )
   if( cur &&( cur->rwtype == ELSE_IDRW ))
   { cur = exSentenceExtract (PreCompSentence,ORDER_CORRECT);   // --ELSE_IDRW

           exSentenceSub (PreCompSentence,NULL); // --<stat>
     rsl = exResulted    (PreCompSentence);
   }

  return rsl;
}
myvar*  exWhile       (exSentence *PreCompSentence)
{ //myvar *exp, *stat, *cur, *new_cur; 

  //exp  = exSentenceExtract (PreCompSentence,ORDER_CORRECT); // --<exp>
  //stat = exSentenceExtract (PreCompSentence,ORDER_CORRECT); // --<stat>
   
  PreCompSentence->Names_table->Flg.DepthCycles++;
  // выполнить условие цикла и проверить на истинность
  while( isTrueMyVar (isTrueExpr (PreCompSentence)) )
  { exSentenceSub   (PreCompSentence,NULL); // выполнить тело цикла

    exSentenceUndo  (PreCompSentence); // вернуть <stat>
    exSentenceUndo  (PreCompSentence); // вернуть <exp>
  }
  //exSentenceExtract (PreCompSentence,ORDER_CORRECT); // --<exp>
  exSentenceExtract (PreCompSentence,ORDER_CORRECT);   // --<stat>
  PreCompSentence->Names_table->Flg.DepthCycles--;

  return exResulted (PreCompSentence);
}
myvar*  exFor         (exSentence *PreCompSentence)
{ // for [        name [             NULL] [             NULL] [             NULL]        ] <stat>  // бесконечный цикл
  // for [ [elem] name [<word>|<exp>|NULL] [<word>|<exp>|NULL] [<word>|<exp>|NULL] [rvrs] ] <stat>  // usual
  // for [        name [       <exp>|NULL] [       <exp>|NULL] [       <exp>|NULL]        ] <stat>  // ctype

  //--------------------------------------------------------
  if( PreCompSentence->Names_table->pUsrConf->Syntax.at_for_c_style_syntax )
         exForCtype (PreCompSentence);
  else   exForUsual (PreCompSentence);
  //--------------------------------------------------------
  return exResulted (PreCompSentence);
}
//-----------------------------------------------------------------------------------------
myvar*  None (void)
{ return myvar_create (NONE_NOT_NAME,Not); }
int   isNone (myvar *cur)
{ return ( !strcmp (cur->name,NONE_NOT_NAME) && (cur->type == Not) ); }
//-----------------------------------------------------------------------------------------
