#include "Header.h"
#include "interpreter.h"
#include "interpreterLexical.h"
#include "interpreterSyntax.h"

#include "syntaxFor.h"
#include "syntaxVars.h"

//----------------------------------------------------------------------------------------------------
void   sa_for_name_check   (names_table *Names_table, list_pmv *Sentence, list_pmv *PostfixSentence)
{ myvar *cur, *pvar;
  // <exp> = name=<exp>

  cur  = *list_pmv_exact (   PostfixSentence,-1 );
  pvar = *list_pmv_exact ( cur->value.lvalue, 0 );
  if( pvar->rwtype != BND_ARTH_IDRW )  // =?
   SetCorrectError (myvar_name(pvar),ERR_SYNX);

  pvar = *list_pmv_exact ( cur->value.lvalue,-1 );
  if( pvar->vtype != Variable )   // name?
   SetCorrectError (cur->name,ERR_VAR);
  list_pmv_insert ( PostfixSentence,(PostfixSentence->len-1),&pvar ); // --name
}
void   sa_for_expression   (names_table *Names_table, list_pmv *Sentence, list_pmv *PostfixSentence)
{ myvar *cur = laNextExact (Sentence);
  if( cur->rwtype != SEMIC_IDRW )
  { cur = saNewSentence ();
    sa_postfix (Names_table,Sentence,cur->value.lvalue,END_IDRW);
    list_pmv_insert ( PostfixSentence,-1,&cur ); // --<exp>
  }
  else { cur = NULL; list_pmv_insert (PostfixSentence,-1,&cur); }
}
void   sa_for_semic_miss   (names_table *Names_table, list_pmv *Sentence)
{ myvar *cur = laNextWord (Sentence); 
  if( cur->rwtype != SEMIC_IDRW ) SetCorrectError (cur->name,ERR_SEPR);
}

void   sa_for_header_usual (names_table *Names_table, list_pmv *Sentence, list_pmv *PostfixSentence)
{ myvar *cur, *sen; 

  int flag_each = Names_table->pUsrConf->Syntax.at_for_each_container;
  int flag_expr = Names_table->pUsrConf->Syntax.at_for_expr_condition;
  int flag_was_each = 0;
  int flag_was_cont = 0;
  // usual:
  // _for name <stat>
  // _for [ [_Elem] name [<word>|<exp>] [<word>|<exp>] [<word>|<exp>] [_Rvrs] ] <stat>
 
  sen = saNewSentence ();
  //---*** for header ***-------------
  cur = laNextExact (Sentence);
  if( cur->rwtype == EACH_IDRW )
  { if( !flag_each )
     SetCorrectError (myvar_name (cur),ERR_SYNX);
    laNextWord (Sentence); // --EACH_IDRW
    flag_was_each = 1;
  }

  cur = laNextExact (Sentence);
  if( cur->rwtype == ELEM_IDRW )
  { laNextWord (Sentence); // --ELEM_IDRW
    list_pmv_insert (sen->value.lvalue,-1,&cur);
  }

  if( Names_table->pUsrConf->Syntax.at_for_always_new_var )
  { myvar *pvar;
    cur = sa_vars_create_name (Names_table,Sentence);
    pvar = hash_lookup (Names_table->pBsValVars,myvar_name (cur),cur,HASH_ADD_DISABLE,0);
    if( !pvar )
     pvar = hash_lookup (Names_table->pBsValVars,myvar_name (cur),cur,HASH_ADD_ENABLE,0);
  }
  else
  { cur = laNextWord  (Sentence);  // --name
    if( cur->vtype != Variable )
     SetCorrectError (myvar_name (cur),ERR_VAR);
  }
  list_pmv_insert (sen->value.lvalue,-1,&cur);

  cur = laNextExact (Sentence);
  if( cur->rwtype == FROM_IDRW )
  { cur = laNextWord (Sentence); // --ENUM_FROM
    if( flag_expr ) saGetExpr (Names_table,Sentence,sen->value.lvalue,0);
    else            saGetWord (Names_table,Sentence,sen->value.lvalue);
  }
  else { cur = NULL; list_pmv_insert (sen->value.lvalue,-1,&cur); }

  cur = *list_pmv_exact (sen->value.lvalue,-1);
  cur = *list_pmv_exact (cur->value.lvalue, 0);
  if( cur->type == Cnt ) flag_was_cont = 1;

  if( flag_was_each && !flag_was_cont )
   SetCorrectError (myvar_name (cur),ERR_SYNX);

  cur = laNextExact (Sentence);
  if( cur->rwtype == UNTL_IDRW )
  { if( flag_was_cont )
     SetCorrectError (myvar_name (cur),ERR_SYNX);
    cur = laNextWord (Sentence); // --ENUM_UNTIL
    if( flag_expr ) saGetExpr (Names_table,Sentence,sen->value.lvalue,0);
    else            saGetWord (Names_table,Sentence,sen->value.lvalue);
  }
  else { cur = NULL; list_pmv_insert (sen->value.lvalue,-1,&cur); }

  cur = laNextExact (Sentence);
  if( cur->rwtype == BYST_IDRW )
  { if( flag_was_cont )
     SetCorrectError (myvar_name (cur),ERR_SYNX);
    cur = laNextWord (Sentence); // --ENUM_BYSTEP
    if( flag_expr ) saGetExpr (Names_table,Sentence,sen->value.lvalue,0);
    else            saGetWord (Names_table,Sentence,sen->value.lvalue);
  }
  else { cur = NULL; list_pmv_insert (sen->value.lvalue,-1,&cur); }

  cur = laNextExact (Sentence);
  if( cur->rwtype == RVRS_IDRW )
  { if( !flag_was_cont )
     SetCorrectError (myvar_name (cur),ERR_SYNX);
    laNextWord (Sentence); // --RVRS_IDRW
    list_pmv_insert (sen->value.lvalue,-1,&cur);
  }
  //----------------------------------
  list_pmv_insert (PostfixSentence,-1,&sen);
}
void   sa_for_header_ctype (names_table *Names_table, list_pmv *Sentence, list_pmv *PostfixSentence)
{ myvar *cur;

  // ctype:
  // _for ( [name=<exp>] ; [<exp>] ; [<exp>] ) <stat>

  cur = laNextWord (Sentence); // --(
  if( cur->rwtype != BGN_IDRW )
   SetCorrectError (myvar_name (cur),ERR_SYNX);
  //----------------------------------
  {  myvar *sen = saNewSentence ();
     //---*** for header ***------------- 
     sa_for_expression (Names_table,Sentence,sen->value.lvalue); // --<exp>
     sa_for_name_check (Names_table,Sentence,sen->value.lvalue);
     //----------------
     sa_for_semic_miss (Names_table,Sentence);                   // --;
     sa_for_expression (Names_table,Sentence,sen->value.lvalue); // --<exp>
     //----------------
     sa_for_semic_miss (Names_table,Sentence);                   // --;
     sa_for_expression (Names_table,Sentence,sen->value.lvalue); // --<exp>
     //----------------------------------
     list_pmv_insert (PostfixSentence,-1,&sen);
  }
  //----------------------------------
  cur = laNextWord (Sentence); // --)
  if( cur->rwtype != END_IDRW )
   SetCorrectError (myvar_name (cur),ERR_SYNX);
}
//----------------------------------------------------------------------------------------------------
