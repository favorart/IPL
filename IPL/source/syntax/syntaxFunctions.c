#include "header.h"

#include "syntaxVars.h"
#include "syntaxFor.h"

#include "interpreter.h"
#include "interpreterLexical.h"
#include "interpreterSyntax.h"
//----------------------------------------------------------------------------------------------------
typedef   myvar* (*pfunc_fill)(names_table*,list_pmv*);
void      sa_vars             (names_table *Names_table, list_pmv *Sentence)
{ unsigned int var_decl = Names_table->pUsrConf->Syntax.variables_declaration;
  pfunc_fill arr_pfunc_fill[] = { sa_vars_fill_c, sa_vars_fill_py, sa_vars_fill_pas };
  myvar *cur;

  cur = laNextExact (Sentence);
  if( cur->rwtype == BGN_IDRW )
  { laNextWord (Sentence); // BGN_IDRW--
    while( laNextExact (Sentence)->rwtype != END_IDRW )
    { arr_pfunc_fill [ var_decl ] (Names_table,Sentence);
      saSeparate (Names_table,Sentence,SA_VARSC_CODE);  // ;--
    }
    saEndWord (cur,laNextWord (Sentence)); // END_IDRW--
  }
  else arr_pfunc_fill [ var_decl ] (Names_table,Sentence);
}
int       sa_vars_func_decl   (names_table *Names_table, list_pmv *Sentence, list_pmv *FormalPars)
{ pfunc_fill arr_pfunc_fill[] = { sa_vars_fill_c, sa_vars_fill_py, sa_vars_fill_pas };
  myvar *cur, *obrace; rwenum end; int brace, field;

  brace = (( Names_table->pUsrConf->Syntax.at_func_always_braces                                   ) ||
           ( Names_table->pUsrConf->Syntax.at_vars_pas_sepr_type == IPL_SYNTAX_VAR_PAS_SEPR_BRACE  ) ||
           ( Names_table->pUsrConf->Syntax.variables_declaration == IPL_SYNTAX_VAR_DECL_C_TYPE     ) ||
           ( Names_table->pUsrConf->Syntax.variables_declaration == IPL_SYNTAX_VAR_DECL_PYTHON     ));
  field =  ( Names_table->pUsrConf->Syntax.at_vars_pas_sepr_type == IPL_SYNTAX_VAR_PAS_SEPR_FIELD && !brace);

  obrace = laNextWord (Sentence); 
       if( ((obrace->rwtype ==  VARS_IDRW) ||
            (obrace->rwtype == FIELD_IDRW)) && field ) end =  DO_IDRW; // --:
  else if(  (obrace->rwtype ==   BGN_IDRW)  && brace ) end = END_IDRW; // --(
  else SetCorrectError(myvar_name (obrace),ERR_NAME);

  cur = laNextExact (Sentence);
  while( (cur->rwtype != end) && (cur->rwtype != RESL_IDRW) )
  { myvar *par = arr_pfunc_fill [ Names_table->pUsrConf->Syntax.variables_declaration ] (Names_table,Sentence);
    list_pmv_insert (FormalPars,-1,&par);

    cur = laNextExact (Sentence);
    if( (cur->rwtype != end) && (cur->rwtype != RESL_IDRW) )
    { saSeparate (Names_table,Sentence,SA_VARSC_CODE); cur = laNextExact (Sentence); }
  }

  if( brace ) saEndWord (obrace,laNextWord (Sentence)); // --)
  return  brace;
}
//void    sa_vars_func_py     (names_table *Names_table, list_pmv *Sentence, myvar *func)
//{ // _def f (a="",b=(),c=[],d=.,e) // types: e - int ,d - flt, c - cnt , b - jnt, a - str 
    // { c = a + b } _return c }
//----------------------------------------------------------------------------------------------------
list_pmv* sa_program   (names_table *Names_table, list_pmv *Sentence, list_pmv *PostfixSentence)
{ myvar *cur; int brace, field;
  
  cur = sa_vars_create_name (Names_table,Sentence);
  if( cur->vtype != Variable )
   SetCorrectError(cur->name,ERR_NAME);
  list_pmv_insert (PostfixSentence,-1,&cur);

  cur->type = Int; cur->value.ivalue = 0;
  sa_vars_copy_2_base (Names_table,cur);

  brace = ((Names_table->pUsrConf->Syntax.at_vars_pas_sepr_type == IPL_SYNTAX_VAR_PAS_SEPR_BRACE ) ||
           (Names_table->pUsrConf->Syntax.variables_declaration == IPL_SYNTAX_VAR_DECL_C_TYPE    ) ||
           (Names_table->pUsrConf->Syntax.variables_declaration == IPL_SYNTAX_VAR_DECL_PYTHON    ));
  field =  (Names_table->pUsrConf->Syntax.at_vars_pas_sepr_type == IPL_SYNTAX_VAR_PAS_SEPR_FIELD  );

  cur = laNextWord (Sentence); 
  if( !(((cur->rwtype == FIELD_IDRW) && field) || ((cur->rwtype == BGN_IDRW) && brace)) )
   SetCorrectError(myvar_name (cur),ERR_SYNX);
  if( cur->rwtype == BGN_IDRW ) saEndWord (cur,laNextWord (Sentence)); // --)

  return PostfixSentence;
}
list_pmv* sa_input     (names_table *Names_table, list_pmv *Sentence, list_pmv *PostfixSentence)
{ myvar *cur, *sen, *obrace;
 
  sen = saNewSentence ();
  obrace = laNextExact (Sentence);
  if( obrace->rwtype == BGN_IDRW )
  { laNextWord (Sentence); // --(

    cur = laNextExact (Sentence);
    if((cur->vtype == Reg_Word)&&(cur->rwtype == INTIN_IDRW || cur->rwtype == FLTIN_IDRW || cur->rwtype == STRIN_IDRW))
    { laNextWord (Sentence); list_pmv_insert ( sen->value.lvalue,-1,&cur );
      
      saSeparate (Names_table,Sentence,SA_FUNCT_PARS);
    }

    cur = laNextExact (Sentence);
    if((cur->vtype == Reg_Word)&&(cur->rwtype == INTIN_IDRW || cur->rwtype == FLTIN_IDRW || cur->rwtype == STRIN_IDRW))
     SetCorrectError (cur->name,ERR_SYNX);

    while( cur->vtype == Variable )
    { laNextWord (Sentence); list_pmv_insert ( sen->value.lvalue,-1,&cur );

      saSeparate (Names_table,Sentence,SA_FUNCT_PARS);
      cur = laNextExact (Sentence);
    }

    if( !sen->value.lvalue->len )
     SetCorrectError (cur->name,ERR_PAR);

    list_pmv_insert ( PostfixSentence,-1,&sen );
    saEndWord (obrace,laNextExact (Sentence));
    sa_resulted (Names_table,Sentence,PostfixSentence);
  }
  else
  { cur = laNextExact (Sentence);
    if(  (cur-> vtype == Variable) || ((cur-> vtype == Reg_Word) &&
         (cur->rwtype == INTIN_IDRW || cur->rwtype == FLTIN_IDRW || cur->rwtype == STRIN_IDRW))
      )
    {        laNextWord (Sentence);
        list_pmv_insert ( sen->value.lvalue,-1,&cur );
        list_pmv_insert ( PostfixSentence  ,-1,&sen );
    }
    else SetCorrectError (cur->name,ERR_PAR);
  }
  return PostfixSentence;
}
list_pmv* sa_if_else   (names_table *Names_table, list_pmv *Sentence, list_pmv *PostfixSentence)
{ myvar *cur;

  saGetExpr  (Names_table,Sentence,PostfixSentence,0);
  if( Names_table->pUsrConf->Syntax.at_if_else_using_then )
  { cur = laNextWord (Sentence); // --THEN
    if( cur->rwtype != THEN_IDRW )
     SetCorrectError (cur->name,ERR_SYNX);
  }
  // тело if
  cur = laNextExact (Sentence);                     // *** внимание: костыль ***
  saGetStat (Names_table,Sentence,PostfixSentence,SA_OTHER_CODE);
  if( cur->rwtype != BGN_IDRW )                     // *** внимание: костыль ***
   saSeparate (Names_table,Sentence,SA_OTHER_CODE); // *** внимание: костыль ***

  cur = laNextExact (Sentence);
  while ( cur->rwtype == ELIF_IDRW )
  { cur = laNextWord (Sentence);
    list_pmv_insert (PostfixSentence,-1,&cur); // ++ELIF_IDRW
   
    saGetExpr (Names_table,Sentence,PostfixSentence,0);
    if( Names_table->pUsrConf->Syntax.at_if_else_using_then )
    { cur = laNextWord (Sentence);
      if( cur->rwtype != THEN_IDRW )
       SetCorrectError (cur->name,ERR_SYNX);
    }
    cur = laNextExact (Sentence);  // *** внимание: костыль ***
    saGetStat (Names_table,Sentence,PostfixSentence,SA_OTHER_CODE);

    if( cur->rwtype != BGN_IDRW )                   // *** внимание: костыль ***
     saSeparate (Names_table,Sentence,SA_OTHER_CODE);  // *** внимание: костыль ***

    cur = laNextExact (Sentence);
  }

  if( cur->rwtype == ELSE_IDRW )
  { cur = laNextWord (Sentence);
    list_pmv_insert (PostfixSentence,-1,&cur);
    saGetStat (Names_table,Sentence,PostfixSentence,SA_OTHER_CODE);
  }

  return PostfixSentence;
}
list_pmv* sa_while     (names_table *Names_table, list_pmv *Sentence, list_pmv *PostfixSentence)
{ saGetExpr (Names_table,Sentence,PostfixSentence,0);

  if( Names_table->pUsrConf->Syntax.at_for_while_using_do )
   laNextWord (Sentence); // --do
 
  saGetStat (Names_table,Sentence,PostfixSentence,SA_OTHER_CODE);

  return PostfixSentence;
}
list_pmv* sa_for       (names_table *Names_table, list_pmv *Sentence, list_pmv *PostfixSentence)
{ 
  // at_for_always_new_var;   /* 0 = NO, another = YES */ // => exFor
  // at_for_each_container;   /* 0 = NO, another = YES */ // => exFor
  // at_for_expr_condition;   /* 0 = NO, another = YES */ // => sa_for
  // at_for_c_style_syntax;   /* 0 = NO, another = YES */ // => sa_for

  if( Names_table->pUsrConf->Syntax.at_for_c_style_syntax )
   sa_for_header_ctype (Names_table,Sentence,PostfixSentence);
  else
   sa_for_header_usual (Names_table,Sentence,PostfixSentence);

  { myvar *cur = laNextExact (Sentence);

    if( Names_table->pUsrConf->Syntax.at_for_while_using_do )
     if (cur->rwtype != DO_IDRW ) 
      SetCorrectError (myvar_name (cur),ERR_SYNX);
     else laNextWord (Sentence); // --do
    else if( cur->rwtype == DO_IDRW )
     SetCorrectError (myvar_name (cur),ERR_SYNX);
  }

  // тело цикла _for
  saGetStat (Names_table,Sentence,PostfixSentence,SA_OTHER_CODE); // --<stat>

  return PostfixSentence;
} 
//----------------------------------------------------------------------------------------------------
