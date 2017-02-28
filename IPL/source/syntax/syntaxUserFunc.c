#include "Header.h"

#include "syntaxVars.h"
#include "lexicalCheck.h"

#include "interpreter.h"
#include "interpreterLexical.h"
#include "interpreterSyntax.h"

#define FILE_EXTENSION      ".txt"
#define NUM_UFUNC_SYNONYMS  5
//-------------------------------------
int   sa_vars_func_decl (names_table *Names_table, list_pmv *Sentence, list_pmv *FormalPars);
int   sa_user_func_decl (names_table *Names_table, list_pmv *Sentence, myvar *func);
int   sa_user_func_file (names_table *Names_table, list_pmv *Sentence, myvar *func);
//----------------------------------------------------------------------------------------------------
int   is_end_finding (size_t *k, size_t *m, size_t n)
{ size_t  i;
  for(i=0; i<n; i++)
   if( k[i] == m[i] )
     return 1;
  return 0;
}
int     find_in_file (names_table *Names_table, myvar *func, FILE *f_inc)
{ int result = 0;

  char     c;
  char*   ch[NUM_UFUNC_SYNONYMS];
  size_t   k[NUM_UFUNC_SYNONYMS];
  size_t   m[NUM_UFUNC_SYNONYMS];
  size_t   i, n = Names_table->Flg.FncDefNames->len;

  int fl_low = Names_table->pUsrConf->Syntax.no_mean_reg_of_letter;

  if( !f_inc || !n )
  { SetError (ERR_PAR);
    result = 1;
    goto END;
  }
  for ( i = 0; i < n; i++ )
  {
    ch[i] = (*list_pmv_exact (Names_table->Flg.FncDefNames, (int) i))->name;
    m[i] = strlen (ch[i]);
  }
  
NEXT_FUNC:;
  memset (k, 0, sizeof (k) * NUM_UFUNC_SYNONYMS);
  n = Names_table->Flg.FncDefNames->len;

  // find_for_func_decl_reg_word
  while( !is_end_finding (k,m,n) )
  { c = fgetc (f_inc);
    if( fl_low ) c = tolower (c);

    for(i=0; i<n; i++)
     ( c == ch[i][k[i]] ) ? (k[i]++) : (k[i]=0);

    if( feof (f_inc) )
    { SetCorrectError (myvar_name (func),ERR_CONF);
      result = 1;
      goto END;
    }
  }

#ifdef  _DEBUG_ //---DEBUG--------------------------
  { myvar* curr;
    for(curr = hash_next((*_Names_table())->pVariables,HASH_BASE); curr;
        curr = hash_next((*_Names_table())->pVariables,HASH_NEXT) )
     printf ("%s\n",curr->name);
  }
#endif  //------------------------------------------
  
  { list_pmv ListAtoms = {0};
    long pos = ftell (f_inc);
    i = sa_user_func_decl (Names_table,&ListAtoms,func);
    list_pmv_free (&ListAtoms);

    if( i==2 ) { fseek (f_inc,pos,SEEK_SET); goto NEXT_FUNC; } 
    else if( i==1 ) result = 1;
  }

END:;
  return result;
}
void  variables_swap (names_table *Names_table, myvar *name)
{ variables *tmp;
  //-------------------------------------------------------
  tmp = Names_table->pBsValVars;
  Names_table->pBsValVars = name->value.pvalue->BsValVars;
  name->value.pvalue->BsValVars = tmp;
  //-------------------------------------------------------
  tmp = Names_table->pVariables;
  Names_table->pVariables = name->value.pvalue->Variables;
  name->value.pvalue->Variables = tmp;  
  //-------------------------------------------------------
}
//----------------------------------------------------------------------------------------------------
// подключаемый модуль
int  sa_user_func_file (names_table *Names_table, list_pmv *Sentence, myvar *func)
{ int result = 0;
  FILE *f = NULL; 
  
  char filename[ NAMELEN + sizeof(FILE_EXTENSION) ];
  myvar *cur = laNextExact (Sentence);
  
  if( (func->vtype != Variable) || func->type )
  { SetCorrectError(func->name,ERR_PAR);
    result = 1;
    goto END;
  }
  func->type = Fnc;
  
  if( cur->rwtype == FROM_IDRW )
  {       laNextWord (Sentence);
    cur = laNextWord (Sentence);

    if( cur->type != Str )
    { SetCorrectError (myvar_name (cur), ERR_TYPE);
      result = 1;
      goto END;
    }
    strcpy (filename,cur->value.svalue);
  }
  else
  { strcpy (filename,func->name);
    strcat (filename,FILE_EXTENSION); 
  }

  if( !(func->value.pvalue = procedure_init (NULL,filename,Names_table)) )
  { result = 1;
    goto END;
  }
  
  func->value.pvalue->fScriptTmp = Names_table->pfScript;
  Names_table->pfScript = NULL;
  
  func->value.pvalue->pLexTmp = (lexical_data*)malloc( sizeof(lexical_data) );
  if( !func->value.pvalue->pLexTmp )
  { SetError (ERR_MEM);
    result = 1;
    goto END;
  }

  *func->value.pvalue->pLexTmp = Names_table->Lex;
  memset (&Names_table->Lex, 0, sizeof (Names_table->Lex));
  if( lex_copy (func->value.pvalue->pLexTmp, &Names_table->Lex) )
  { result = 1; goto END; }
  
  if( !(f = fopen (filename,"rt")) ) 
  { SetCorrectError(filename,ERR_FILE);
    result = 1;
    goto END;
  }  
  Names_table->pfScript = f;

  // find func. in file
  if( find_in_file (Names_table,func,f) ) { result = 1; goto END; }

END:;
  if(f) { fclose (f); f = NULL; }
  
  if( func->value.pvalue )
  { Names_table->pfScript = func->value.pvalue->fScriptTmp;
    func->value.pvalue->fScriptTmp = NULL;

    if( func->value.pvalue->pLexTmp )
    { lex_free (&Names_table->Lex);
      Names_table->Lex = *func->value.pvalue->pLexTmp;

      free (func->value.pvalue->pLexTmp);
      func->value.pvalue->pLexTmp = NULL;
    }
  } 

  return result;
}
//----------------------------------------------------------------------------------------------------
// объявление функции
int  sa_user_func_decl (names_table *Names_table, list_pmv *Sentence, myvar *func)
{ int result = 0; myvar *name, *cur = NULL;
  int flg_rtype = ( (Names_table->pUsrConf->Syntax.variables_declaration == IPL_SYNTAX_VAR_DECL_C_TYPE) ||
                    (Names_table->pUsrConf->Syntax.variables_declaration == IPL_SYNTAX_VAR_DECL_PYTHON) );
  int flg_brace =    Names_table->pUsrConf->Syntax.at_func_always_braces;
  int flg_fvars = 0;
  //----------------------------------------------
  // *** standard style syntax ***
  //
  // >>> function <name> [fld|vars {<params>} [,|&] ] | [ ( {<params>} [,|&] ) ] [resl[fld] <type> ] [do]
  // >>> begin ... [;] end [ resulted <value> ] [;]
  //
  // <params> - like in _vars, but have some new flags )
  //    Names_table->pUsrConf->Syntax.commas_at_func_params
  //    Names_table->pUsrConf->Syntax.at_func_always_braces
  //    Names_table->pUsrConf->Syntax.variables_separate >>> if(variables_declared == PAS)
  //
  // >>> function f do next ... .
  //-----------------------------------------------
  // *** C-style syntax ***
  //
  // >>> def [<type>] <name> ( [<type0> <param0> [,|&] <type1> <param1> [,|&] ...] )
  // >>> { ... [;] } [ return <value> ] [;]
  //
  //-----------------------------------------------
  if( flg_rtype )
  { cur = laNextExact (Sentence);
    if( (cur->rwtype < INT_IDRW) || (cur->rwtype > LST_IDRW) || (cur->rwtype == FNC_IDRW) )
     cur = None ();
    else
     cur = laNextWord (Sentence); // --ret_type
  }
    
  if( func->type ) // если считываем с файла
  { name = laNextExact (Sentence);
    if( (name->vtype != Variable) || (name->type != Fnc) ||
         strcmp (name->name,func->name) )
    { result = 2; goto END; }
    laNextWord (Sentence); // --name
    name->value.pvalue->ResultType = cur;

  }
  else // если просто попали на определение функции
  { name = sa_vars_create_name (Names_table,Sentence); // --name
    if( (name->vtype != Variable) || name->type )
    { SetCorrectError(name->name,ERR_NAME); result = 1; goto END; }

    name->type = Fnc;
    if( !(name->value.pvalue = procedure_init (cur,NULL,Names_table)) )
    { result = 1; goto END; }
  }
  list_pmv_insert (Names_table->pUserFuncs,-1,&name); // <<< Fnc->name

  { size_t i, n = Sentence->len;
    myvar **elem;
    // скинуть указатель на имеющуюся таблицу переменных
    variables_swap (Names_table,name);

    for(i=0; i<n; i++)
    { elem = list_pmv_exact (Sentence,i);
      if( (*elem)->vtype == Variable )
      { *elem = variables_lookup (myvar_name (*elem),*elem,HASH_ADD_ENABLE);
        if( strcmp (myvar_name (*elem),FUNC_ARGS_NO_NAME) )
        { myvar_free_value (*elem); (*elem)->type = Not; }
      }
    }
  }
  //---обработка заголовка функции--------------------------------------
  cur = laNextExact (Sentence);
  if( ((cur->rwtype != RESL_IDRW) && !flg_rtype) ||
      ((cur->rwtype !=   DO_IDRW) && !flg_brace) )
  { Names_table->Flg.saVarsComma = 1;
    flg_fvars = sa_vars_func_decl (Names_table,Sentence,&name->value.pvalue->FormalPars); // -- {param}
    Names_table->Flg.saVarsComma = 0;
  }

  if( !flg_rtype )
  // получить тип результата функции для альтернативного синтаксиса
  { cur = laNextExact (Sentence);
    if( cur->rwtype == RESL_IDRW )
    {       laNextWord  (Sentence); // --=
      cur = laNextExact (Sentence);
      if( cur->rwtype == FIELD_IDRW )
       laNextWord  (Sentence); // --:

      cur = laNextWord (Sentence); // --rtype
      if( (cur->rwtype < INT_IDRW) || (cur->rwtype > LST_IDRW) || (cur->rwtype == FNC_IDRW) )
      { SetCorrectError (myvar_name (cur),ERR_TYPE); result = 1; goto END; }
      name->value.pvalue->ResultType = cur;
    }
    else name->value.pvalue->ResultType = None ();
  }

  if( !flg_brace && !flg_fvars )
  { cur = laNextExact (Sentence);
    if( cur->rwtype == DO_IDRW)
     laNextWord (Sentence); // --DO
    else { SetCorrectError (myvar_name (cur),ERR_SYNX); result = 1; goto END; } 
  }
  cur = laNextExact (Sentence);
  if( cur->rwtype != BGN_IDRW )
  { SetCorrectError (myvar_name (cur),ERR_SYNX); result = 1; goto END; }
  //--------------------------------------------------------------------

  //---обработка тела функции---
  saGetStat (Names_table,Sentence,&name->value.pvalue->Sentences,SA_OTHER_CODE); // --func_body
  //----------------------------
  variables_swap      (Names_table,name);   
  sa_vars_copy_2_base (Names_table,name);

END:;
  return result;
}
//----------------------------------------------------------------------------------------------------