#include "header.h"
#include "interpreter.h"
#include "interpreterExecute.h"

#define EPS 10e-14
//-----------------------------------------------------------------------------------------
int       isTrueMyVar  (myvar *cur)
{ int rsl = 0;
  if( cur->vtype == Pre_Comp ) SetError (ERR_PAR);
  switch (cur->type)
  { case Not: break;
    case Int: if(  cur->value.ivalue      )  rsl = 1;    break;
    case Str: if( *cur->value.svalue      )  rsl = 1;    break;
    case Flt: if(  cur->value.fvalue      )  rsl = 1;    break;
    case Cnt: if( *cur->value.cvalue->len )  rsl = 1;    break;
    case Jnt: if(  cur->value.jvalue->len )  rsl = 1;    break;
    default : SetCorrectError(myvar_name(cur),ERR_TYPE); break;
  }
  return rsl;
}
myvar*    isTrueExpr   (exSentence *PreCompSentence)
{ return exSentenceSub (PreCompSentence,NULL); }
//-----------------------------------------------------------------------------------------
typedef struct _for_data  for_data;
struct _for_data
{ myvar *from;
  myvar *till;
  myvar *step;
//----------------------------------
  myvar *cntr; // cur_counter
  myvar  svcn; // sav_counter
//----------------------------------
  int    rvrs; // flag
  int    elem; // flag
};
//----------------------------------
int   for_chk_flt_floor (myvar *pvar)
{ return ((pvar->type == Flt) && ((pvar->value.fvalue - floor (pvar->value.fvalue)) < EPS)); }
void  for_get_flt_value (for_data *ForData)
{ if( ForData->from->type == Int )
  { ForData->from->type = Flt;
    ForData->from->value.fvalue = (FLOAT)ForData->from->value.ivalue;
    SetWarning (myvar_name (ForData->from),WRG_TCAST);
  }
  if( ForData->till->type == Int )
  { ForData->till->type = Flt;
    ForData->till->value.fvalue = (FLOAT)ForData->till->value.ivalue;
    SetWarning (myvar_name (ForData->till),WRG_TCAST);
  }
  if( ForData->step->type == Int )
  { ForData->step->type = Flt;
    ForData->step->value.fvalue = (FLOAT)ForData->step->value.ivalue;
    SetWarning (myvar_name (ForData->step),WRG_TCAST);
  }
}
void  for_get_int_value (for_data *ForData)
{ if( ForData->from->type == Flt )
  { ForData->from->type = Int;
    ForData->from->value.ivalue = (INTEGER)ForData->from->value.fvalue;
    SetWarning (myvar_name (ForData->from),WRG_TCAST);
  }
  if( ForData->till->type == Flt )
  { ForData->till->type = Int;
    ForData->till->value.ivalue = (INTEGER)ForData->till->value.fvalue;
    SetWarning (myvar_name (ForData->till),WRG_TCAST);
  }
  if( ForData->step->type == Flt )
  { ForData->step->type = Int;
    ForData->step->value.ivalue = (INTEGER)ForData->step->value.fvalue;
    SetWarning (myvar_name (ForData->step),WRG_TCAST);
  }
}
//----------------------------------
int     isSimpleType (tenum type)
{ return (type == Int) || (type == Flt) || (type == Str); }
int     isEnumerable (tenum type)
{ return (type == Cnt) || (type == Jnt); }
//----------------------------------
myvar*  exForCounterInit (for_data *ForData)
{ int flag_nvar = (*_Names_table())->pUsrConf->Syntax.at_for_always_new_var;
  //---подготовить _счетчик---------------------------------  
  if( flag_nvar || !ForData->cntr->type ) // ???
  { ForData->svcn = *ForData->cntr; ForData->cntr->type = Not; }
  if( !ForData->elem ) ForData->cntr->vtype = Constant;
  //--------------------------------------------------------
  if( ForData->from && (ForData->from->type != Int) &&
                       (ForData->from->type != Flt) &&
                       (ForData->from->type != Cnt) &&
                       (ForData->from->type != Jnt) )
   SetCorrectError (myvar_name (ForData->from),ERR_TYPE);
  if( ForData->till && (ForData->till->type != Int) &&
                       (ForData->till->type != Flt) )
   SetCorrectError (myvar_name (ForData->till),ERR_TYPE);
  if( ForData->step && (ForData->step->type != Int) &&
                       (ForData->step->type != Flt) )
   SetCorrectError (myvar_name(ForData->step),ERR_TYPE);

  if( !ForData->from )
   ForData->from = myvar_create (ZERO_INT_NAME,Int);
  if( !ForData->step && (ForData->from->type != Cnt) )
   ForData->step = myvar_create (TRUE_INT_NAME,Int);
  //--------------------------------------------------------
  if( isEnumerable (ForData->from->type) ) // if( Cnt || Jnt )
  { if( ForData->cntr->type && (ForData->cntr->type != Int) )
     SetWarning (myvar_name (ForData->cntr),WRG_TCAST);
    ForData->cntr->type = Int; ForData->cntr->value.ivalue = 0;   
  }
  else if( ((ForData->from->type == Int) || for_chk_flt_floor (ForData->from)) &&
           ((ForData->step->type == Int) || for_chk_flt_floor (ForData->step)) )
  { for_get_int_value (ForData);
    if( ForData->cntr->type && (ForData->cntr->type != Int) )
     SetWarning (myvar_name (ForData->cntr),WRG_TCAST);
    ForData->cntr->type = Int; ForData->cntr->value.ivalue = 0;
  }
  else
  { for_get_flt_value (ForData);
    if( ForData->cntr->type && (ForData->cntr->type != Flt) )
     SetWarning (myvar_name (ForData->cntr),WRG_TCAST);
    ForData->cntr->type = Flt; ForData->cntr->value.fvalue = 0.;
  }
  //--------------------------------------------------------
  return ForData->cntr;
}
myvar*  exForCounterTerm (for_data *ForData)
{ if(  ForData->svcn.vtype ) *ForData->cntr = ForData->svcn;
  if( !ForData->elem )        ForData->cntr->vtype = Variable;
  return ForData->cntr;
}

//#define _DEBUG_
#ifdef  _DEBUG_ //---DEBUG--------------------------
FILE** _log_ (void);
int   myvar_print (myvar *pvar);
int   hash_fwrite (hash_table *Hash, FILE* f_out);
int   print_status  (list_pmv *PostfixSentence, void *Stack, int s);
#endif  //------------------------------------------

myvar*  exForHead  (exSentence *PreCompSentence, for_data *ForData)
{ myvar *cur;
  //---обработка _elem--------------------------------------
  if( (cur = exSentenceExact (PreCompSentence,ORDER_CORRECT)) && (cur->rwtype == ELEM_IDRW) )
  { ForData->elem = 1; exSentenceExtract (PreCompSentence,ORDER_CORRECT); }
  //---создание переменной счётчик--------------------------
  ForData->cntr = exSentenceExtract (PreCompSentence,ORDER_CORRECT); // --name
  //---блок _от---------------------------------------------
  if( cur = exSentenceExact (PreCompSentence,ORDER_CORRECT) )
   ForData->from = exSentenceSub (PreCompSentence,NULL);   // --<exp>/<word>
  else { ForData->from = NULL; exSentenceExtract (PreCompSentence,ORDER_CORRECT); }
  //---блок _до---------------------------------------------
  if( cur = exSentenceExact (PreCompSentence,ORDER_CORRECT) )
  { ForData->till = exSentenceSub (PreCompSentence,NULL); } // --<exp>/<word>
  else { ForData->till = NULL; exSentenceExtract (PreCompSentence,ORDER_CORRECT); }
  //---блок _шагом------------------------------------------
  if( cur = exSentenceExact (PreCompSentence,ORDER_CORRECT) )
  { ForData->step = exSentenceSub (PreCompSentence,NULL); } // --<exp>/<word>
  else { ForData->step = NULL; exSentenceExtract (PreCompSentence,ORDER_CORRECT); }
  //---обработка _rvrs--------------------------------------
  if( (cur = exSentenceExact (PreCompSentence,ORDER_CORRECT)) && (cur->rwtype == RVRS_IDRW) )
  { ForData->rvrs = 1; exSentenceExtract (PreCompSentence,ORDER_CORRECT); }
  //--------------------------------------------------------
  return ForData->cntr;
}
myvar*  exForBody  (exSentence *PreCompSentence, for_data *ForData)
{ 
  PreCompSentence->Names_table->Flg.DepthCycles++;
       if( ForData->from->type == Cnt )
  { unsigned int i,n; int rvr = ForData->rvrs;
    myvar **elem = NULL, *cx = ForData->cntr;
    container *cnt = ForData->from->value.cvalue;

    n = *cnt->len;
    for(i=0; i<n; i++)
    { if( ForData->elem )
      { elem = container_exact (cnt,(rvr)?(n-i+1):(i));
        if( isSimpleType ((*elem)->type) )
         myvar_copy_value (*elem,cx);
        else
        { cx->type  = (*elem)->type;
          cx->value = (*elem)->value;
        }
      } // end if-elem
      else cx->value.ivalue = (rvr)?(n-i+1):(i);

      exSentenceSub   (PreCompSentence, NULL);           // выполнить тело цикла
      exSentenceUndo  (PreCompSentence);                // вернуть <stat>

      if( ForData->elem )
       if( isSimpleType ((*elem)->type) )
        *elem = myvar_create_value (cx->value,cx->type);
       else
        if( (cx->type != (*elem)->type) || (cx->value.cvalue != (*elem)->value.cvalue) )
        { myvar_free (*elem);
          if( isEnumerable (cx->type) ) 
          { (*elem)->type  = cx->type;
            (*elem)->value = cx->value;
          }
          else
            (*elem) = myvar_create_value (cx->value,cx->type);
        }
    } // end for
  } // end if-cnt
  else if( ForData->from->type == Jnt )
  { myvar **elem = NULL, *cx = ForData->cntr;
    joint *jnt = ForData->from->value.jvalue;
    joint_node *cur; joint_iter iter = {0};

    if( ForData->rvrs )
     SetWarning (myvar_name (findRegWord (PreCompSentence->Names_table->pReg_Words,RVRS_IDRW)),WRG_NORSL);

    if( !ForData->elem ) cx->value.ivalue = 0;

    while( cur = joint_next (jnt,&iter) )
    { if( ForData->elem )
      { elem = &cur->val;
        if( isSimpleType ((*elem)->type) )
         myvar_copy_value (*elem,cx);
        else
        { cx->type  = (*elem)->type;
          cx->value = (*elem)->value;
        }
      } // end if-elem
      else cx->value.ivalue++;

      exSentenceSub   (PreCompSentence,NULL);           // выполнить тело цикла
      exSentenceUndo  (PreCompSentence);                // вернуть <stat>

      if( ForData->elem )
       if( isSimpleType ((*elem)->type) )
        *elem = myvar_create_value (cx->value,cx->type);
       else
        if( (cx->type != (*elem)->type) || (cx->value.cvalue != (*elem)->value.cvalue) )
        { myvar_free (*elem);
          if( isEnumerable (cx->type) ) 
          { (*elem)->type  = cx->type;
            (*elem)->value = cx->value;
          }
          else *elem = myvar_create_value (cx->value,cx->type);
        }
    } // end for
  } // end if-cnt
  else
  { // повторить (от from до till) раз с шагом step
    if( ForData->cntr->type == Int )
    { INTEGER *cntr = &ForData->cntr->value.ivalue;
      INTEGER  from =  ForData->from->value.ivalue;
      INTEGER  till =  ForData->till->value.ivalue;
      INTEGER  step =  ForData->step->value.ivalue;

      for(*cntr=from; *cntr<=till; *cntr+=step)
      { exSentenceSub   (PreCompSentence,NULL);         // выполнить тело цикла
        exSentenceUndo  (PreCompSentence);              // вернуть <stat>
      }
    } 
    else
    { FLOAT *cntr = &ForData->cntr->value.fvalue;
      FLOAT  from =  ForData->from->value.fvalue;
      FLOAT  till =  ForData->till->value.fvalue; 
      FLOAT  step =  ForData->step->value.fvalue;

      for(*cntr=from; *cntr<=till; *cntr+=step)
      { exSentenceSub   (PreCompSentence,NULL);         // выполнить тело цикла
        exSentenceUndo  (PreCompSentence);              // вернуть <stat>
      }
    }
  }
  exSentenceExtract (PreCompSentence,ORDER_CORRECT);    // --<stat>
  PreCompSentence->Names_table->Flg.DepthCycles--;

  return  ForData->cntr;
}
//-----------------------------------------------------------------------------------------
void    exForUsual (exSentence *PreCompSentence)
{ myvar *par; exSentence exSsub = {0}; for_data ForData = {0};

  par = exSentenceExtract (PreCompSentence,ORDER_CORRECT);
        exSentenceConvert (&exSsub,par->value.lvalue,PreCompSentence->Names_table);
  //--------
  exForHead (&exSsub,&ForData);
  //--------
  list_pmv_free (par->value.lvalue);
  par->value.lvalue = exSsub.make_sentence;

  exForCounterInit (&ForData);
  //--------------------------------------------------------
  exForBody (PreCompSentence,&ForData);
  //--------------------------------------------------------
  exForCounterTerm (&ForData);
}
void    exForCtype (exSentence *PreCompSentence)
{ myvar *cur, *par, *counter; exSentence exSsub = {0};
  
  par = exSentenceExtract (PreCompSentence,ORDER_CORRECT);
        exSentenceConvert (&exSsub,par->value.lvalue,PreCompSentence->Names_table );
        exSentenceExtract (&exSsub,ORDER_CORRECT); // --name

  if( cur = exSentenceExact (&exSsub,ORDER_CORRECT) )
  { counter = exSentenceSub (&exSsub,NULL); }      // --introduction

   // exSentenceExtract (&exSsub,ORDER_CORRECT);   // --condition
   // exSentenceExtract (&exSsub,ORDER_CORRECT);   // --increase

  while( !exSentenceExact (&exSsub,ORDER_CORRECT) || isTrueMyVar (isTrueExpr (&exSsub)) ) // --condition
  { exSentenceSub   (PreCompSentence,NULL); // выполнить тело цикла
    exSentenceUndo  (PreCompSentence);      // вернуть <stat>
  
    if( exSentenceExact (&exSsub,ORDER_CORRECT) )
     exSentenceSub (&exSsub,NULL); // --increase

    exSentenceUndo  (&exSsub); // вернуть increase
    exSentenceUndo  (&exSsub); // вернуть condition
  }
  exSentenceExtract (        &exSsub,ORDER_CORRECT); // --increase
  exSentenceExtract (PreCompSentence,ORDER_CORRECT); // --<stat>

  list_pmv_free (par->value.lvalue);
  par->value.lvalue = exSsub.make_sentence;
}
//-----------------------------------------------------------------------------------------