#include "Header.h"
#include "interpreter.h"
#include "interpreterLexical.h"
#include "interpreterSyntax.h"

#include "pmvStack.h"

void  sa_cnt_method_pre  (names_table *Names_table, list_pmv *Sentence, list_pmv *PostfixSentence, myvar *mth);
void  sa_cnt_special_par (names_table *Names_table, list_pmv *Sentence, list_pmv *PostfixSentence, rwenum  rw);
void  sa_cnt_method_stat (names_table *Names_table, list_pmv *Sentence, list_pmv *PostfixSentence, myvar *mth);
void  sa_cnt_method      (names_table *Names_table, list_pmv *Sentence, list_pmv *PostfixSentence);
//-------------------------------------------------------------------------------
int   sa_cnt_index__pre  (names_table *Names_table, list_pmv *Sentence, list_pmv *PostfixSentence, stack_pmv *Stack, myvar *cur)
{ int result = 0;
  myvar *sen = NULL; 
  // myvar *cur, *fld, *inx, *cnt;
  // cur = laNextWord (Sentence); // <Int>|<Flt>|<exp> (Jnt)<Str>|... 
  // fld = laNextWord (Sentence); // --          //    FIELD_IDRW
  // inx = laNextWord (Sentence); // th element  // PREINDEX_IDRW
  // cnt = laNextWord (Sentence); // <Cnt> | Jnt
  
  if( cur->rwtype == END_IDRW )
  { myvar *pmv, *ssn, *cmv = laNextExact (Sentence);

    if( cmv->rwtype == FIELD_IDRW )
    {       laNextWord (Sentence);
      cmv = laNextWord (Sentence);
      if( cmv->rwtype != PREINDEX_IDRW )
       SetCorrectError (myvar_name (cmv),ERR_SYNX);

      sen = saNewSentence ();
      ssn = saNewSentence ();
      //----------------------------------------
      list_pmv_extract (PostfixSentence,  1, &pmv);
      list_pmv_insert (ssn->value.lvalue, 0, &pmv);
      while( Stack->len )
      { myvar *sgn = stack_pmv_pop (Stack);

        if( sgn->rwtype == BGN_IDRW ) break;
        if( !Stack->len )
         SetCorrectError (myvar_name (cur),ERR_NAME);

        list_pmv_extract (PostfixSentence,  0, &pmv);
        list_pmv_insert (ssn->value.lvalue, 0, &pmv);
        list_pmv_insert (ssn->value.lvalue, 0, &sgn);
      }
      //----------------------------------------
      list_pmv_insert (sen->value.lvalue,0,&ssn);
      list_pmv_insert (PostfixSentence,  0,&sen);
        
      cmv = laNextWord (Sentence);
      if( (cmv->type != Cnt) && (cmv->type != Jnt) )
       SetCorrectError (myvar_name (cmv),ERR_SYNX);
      list_pmv_insert (PostfixSentence,0,&cmv);

      result = 1;
    } 
  }
  else if( cur->rwtype == FIELD_IDRW )
  { cur = laNextWord (Sentence);
    if( cur->rwtype != PREINDEX_IDRW )
     SetCorrectError (myvar_name (cur),ERR_SYNX);

    { myvar *pmv, *ssn;
      sen = saNewSentence ();
      ssn = saNewSentence ();
      //----------------------------------------   
      list_pmv_extract (PostfixSentence, 0,&pmv);
      list_pmv_insert (ssn->value.lvalue,0,&pmv);
      //----------------------------------------
      list_pmv_insert (sen->value.lvalue,0,&ssn);
      list_pmv_insert (PostfixSentence,  0,&sen);
    }

    cur = laNextWord (Sentence);
    if( cur->type && (cur->type != Cnt) && (cur->type != Jnt) )
     SetCorrectError (myvar_name (cur),ERR_SYNX);
    list_pmv_insert (PostfixSentence,0,&cur);

    result = 1;
  }

  if( result )
  { cur = laNextExact (Sentence);
    while( (cur->rwtype ==   OINDEX_IDRW) ||
           (cur->rwtype == PSTINDEX_IDRW) )
    { laNextWord (Sentence); // --[ | --PSTINDEX_IDRW
      saGetExpr (Names_table,Sentence,sen->value.lvalue,0); 

      if( cur->rwtype == OINDEX_IDRW )
       saEndWord (cur,laNextWord (Sentence)); // --]
      cur = laNextExact (Sentence);
    }
  }  
  return result;
  // Cnt [ {<index>} ]
}
void  sa_cnt_method_pre  (names_table *Names_table, list_pmv *Sentence, list_pmv *PostfixSentence, myvar *mth)
{ // ANY_CNT_IDRW Cnt|Jnt [(]{param}[)]
  myvar *cnt = laNextWord (Sentence); // -- Cnt
  
  if( !Names_table->pUsrConf->Syntax.at_obj_method_precall )
   SetCorrectError (myvar_name (cnt),ERR_SYNX);
  if( (cnt->type != Cnt) && (cnt->type != Jnt) )
   SetCorrectError (myvar_name (cnt),ERR_SYNX);

  list_pmv_insert (PostfixSentence,-1,&cnt); // <<< Cnt | Jnt
  sa_cnt_method_stat (Names_table,Sentence,PostfixSentence,mth); // <<< {param}
  // Cnt|Jnt [ANY_CNT_IDRW {param}]
}
//-------------------------------------------------------------------------------
void  sa_cnt_special_par (names_table *Names_table, list_pmv *Sentence, list_pmv *PostfixSentence, rwenum  rw)
{ myvar *cur, *oindex;
  
  if( rw == ADD_CNT_IDRW )
  { saGetExpr (Names_table,Sentence,PostfixSentence,0);
    oindex = laNextExact (Sentence);
    if( (oindex->rwtype == OINDEX_IDRW) || (oindex->rwtype == PSTINDEX_IDRW) )
    { laNextWord (Sentence); // --[
      saGetExpr (Names_table,Sentence,PostfixSentence,0);
      if( oindex->rwtype == OINDEX_IDRW )
       saEndWord (oindex,laNextWord (Sentence)); // --[
    }
    else { cur = None (); list_pmv_insert (PostfixSentence,-1,&cur); }
  }
  else if( rw == DEL_CNT_IDRW )
  { oindex = laNextExact (Sentence);
    if( (oindex->rwtype == OINDEX_IDRW) || (oindex->rwtype == PSTINDEX_IDRW) )
    { laNextWord (Sentence); // --[
      cur = None (); list_pmv_insert (PostfixSentence,-1,&cur);
    }
    else oindex = NULL;

    saGetExpr (Names_table,Sentence,PostfixSentence,0);
    if( !oindex )
    { cur = None (); list_pmv_insert (PostfixSentence,-1,&cur); }
    else if( oindex->rwtype == OINDEX_IDRW )
     saEndWord (oindex,laNextWord (Sentence)); // --[
  }
  else SetError (ERR_PAR);
}
void  sa_cnt_method_stat (names_table *Names_table, list_pmv *Sentence, list_pmv *PostfixSentence, myvar *mth)
{ myvar *cur, *sen;
  // el  = <Cnt> [ADD_CNT_IDRW (el'['inx']', el, ... )  |  el,inx  ]
  // el  = <Cnt> [DEL_CNT_IDRW (  '['inx']', el, ... )  |  inx     ]
  // inx = <Cnt> [FND_CNT_IDRW  el                                 ]
  //       <Cnt> [CLR_CNT_IDRW                                     ]
  //       <Cnt> [SRT_CNT_IDRW RVRS_IDRW                           ]
  
  sen = saNewSentence ();
  if( mth->rwtype == FND_CNT_IDRW )
  { //---*** <stat> ***-----------------------------
    saGetExpr (Names_table,Sentence,sen->value.lvalue,1); // --elem
    //----------------------------------------------
  }
  else if( mth->rwtype == FND_CNT_IDRW )
  { cur = laNextExact (Sentence);
    if( cur->rwtype == RVRS_IDRW )
     cur = laNextWord (Sentence);
    else cur = None ();
    list_pmv_insert (sen->value.lvalue,0,&cur);
  }
  else if( (mth->rwtype == ADD_CNT_IDRW) || (mth->rwtype == DEL_CNT_IDRW) )
   if( Names_table->pUsrConf->Syntax.at_obj_method_regular )
   { if( mth->rwtype == ADD_CNT_IDRW )
     { myvar *obrace = laNextExact (Sentence);
       //---*** <stat> ***-----------------------------
       if( obrace->rwtype == BGN_IDRW )
       { laNextWord (Sentence); // --(
    
         saGetExpr (Names_table,Sentence,sen->value.lvalue,0);     // --elem
         cur = laNextExact (Sentence);
         if( cur->rwtype != END_IDRW )
         { if( cur->rwtype == PSTINDEX_IDRW ) laNextWord (Sentence);
           else saSeparate (Names_table,Sentence,SA_FUNCT_PARS);   // --,
           saGetExpr (Names_table,Sentence,sen->value.lvalue,0);   // --index
         }
         else { cur = None (); list_pmv_insert (sen->value.lvalue,-1,&cur); }
       
         saEndWord (obrace,laNextExact (Sentence));
         sa_resulted (Names_table,Sentence,sen->value.lvalue);     // --)
       }
       else
       { saGetExpr  (Names_table,Sentence,sen->value.lvalue,1);    // --elem
         cur = None (); list_pmv_insert (sen->value.lvalue,-1,&cur);
       }
       //----------------------------------------------
     } 
     else if( mth->rwtype == DEL_CNT_IDRW )
     { //---*** <stat> ***-----------------------------
       cur = None (); list_pmv_insert (sen->value.lvalue,-1,&cur);
       saGetExpr (Names_table,Sentence,sen->value.lvalue,1);       // --index
       //----------------------------------------------
     }
   }
   else
   { myvar *obrace;
     //---*** <stat> ***-----------------------------
     obrace = laNextExact (Sentence);
     if( obrace->rwtype == BGN_IDRW )
     { laNextWord (Sentence); // --(

       do { sa_cnt_special_par (Names_table,Sentence,sen->value.lvalue,mth->rwtype); 
            saSeparate (Names_table,Sentence,SA_FUNCT_PARS);
       } while ( laNextExact (Sentence)->rwtype == END_IDRW );

       saEndWord (obrace,laNextExact (Sentence));
       sa_resulted (Names_table,Sentence,PostfixSentence); // --)
     }
     else sa_cnt_special_par (Names_table,Sentence,sen->value.lvalue,mth->rwtype);
     //----------------------------------------------
   }

  list_pmv_insert (sen->value.lvalue,0,&mth); // <<< _CNT_IDRW
  list_pmv_insert (PostfixSentence ,-1,&sen);
}
//-------------------------------------------------------------------------------
void  sa_cnt_method      (names_table *Names_table, list_pmv *Sentence, list_pmv *PostfixSentence)
{ myvar *cur = laNextExact (Sentence);

  // Cnt None
  // Cnt [ {<exp>} ]         // index
  // Cnt [_CNT_IDRW <stat>]  // method

  // method: ADD, DEL, CLR, FND, SRT
  //  index: i-th element cnt, cnt[i], cnt by number 3

  if( (cur->rwtype == OINDEX_IDRW) || (cur->rwtype == PSTINDEX_IDRW) )
  { myvar *sen = saNewSentence ();
    do
    { if( cur->rwtype == OINDEX_IDRW )
      { laNextWord (Sentence); // --'['
        saGetExpr (Names_table,Sentence,sen->value.lvalue,0);
        saEndWord (cur,laNextWord (Sentence)); // --]
      }
      else
      { laNextWord (Sentence); // --PSTINDEX_IDRW
        saGetExpr (Names_table,Sentence,sen->value.lvalue,0);  
      }
      cur = laNextExact (Sentence);
    } while( (cur->rwtype == OINDEX_IDRW) || (cur->rwtype == PSTINDEX_IDRW) );
    list_pmv_insert (PostfixSentence,-1,&sen);
  }
  else
  { int fld = 0;
    // Cnt [:] [_CNT_IDRW <stat>]
    if( cur->rwtype == FIELD_IDRW )
     if( Names_table->pUsrConf->Syntax.at_obj_method_use_fld )
     { fld = 1; laNextWord (Sentence); } // --:
     else SetCorrectError (myvar_name (cur),ERR_NAME);

    cur = laNextExact (Sentence); 
    if( (cur->rwtype >= ADD_CNT_IDRW) && (cur->rwtype <= SRT_CNT_IDRW) )
    { if( !cur->value.rvalue )
       SetCorrectError (myvar_name (cur),ERR_INTR);

      if( Names_table->pUsrConf->Syntax.at_obj_method_precall ||
         (Names_table->pUsrConf->Syntax.at_obj_method_use_fld && !fld) )
       SetCorrectError (myvar_name (cur),ERR_SYNX);

      laNextWord (Sentence);                     //   --_CNT_IDRW
      sa_cnt_method_stat (Names_table,Sentence,PostfixSentence,cur); // <<< {param}
    }
    else if( fld ) SetCorrectError (myvar_name (cur),ERR_SYNX);
    else { cur = None (); list_pmv_insert (PostfixSentence,-1,&cur); }
  }

}
//-------------------------------------------------------------------------------