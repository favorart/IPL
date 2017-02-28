#include "header.h"
#include "interpreterPreCompile.h"
#include "interpreterExecute.h"

void  lex_free (lexical_data *Lex);
//------------------------------------
void  procedure_variables_swap (procedure *Procedure, names_table *Names_table)
{ variables *tmp;
  //-------------------------------------------------------
  tmp = Names_table->pBsValVars;
  Names_table->pBsValVars = Procedure->BsValVars;
  Procedure->BsValVars = tmp;
  //-------------------------------------------------------
  tmp = Names_table->pVariables;
  Names_table->pVariables = Procedure->Variables;
  Procedure->Variables = tmp;  
  //-------------------------------------------------------
}
//-----------------------------------------------------------------------------------------
procedure*  procedure_create (void)
{ procedure* Procedure;

  if( !(Procedure = (procedure*)malloc( sizeof(procedure) )) )
  { SetError (ERR_MEM); return NULL; }

  if( !(Procedure->BsValVars = (hash_table*)malloc( sizeof(hash_table) )) )
  { SetError (ERR_MEM); return NULL; }
  if( !(Procedure->Variables = (hash_table*)malloc( sizeof(hash_table) )) )
  { SetError (ERR_MEM); return NULL; }

      hash_create ( Procedure->BsValVars );
      hash_create ( Procedure->Variables );
  list_pmv_create (&Procedure->Sentences );
  list_pmv_create (&Procedure->FormalPars);

  Procedure->ResultType = NULL;
  Procedure->   pLexTmp = NULL; 
  Procedure->fScriptTmp = NULL;
  Procedure-> pFilename = NULL;

  return Procedure;
}
procedure*  procedure_init   (myvar *rtype, char *filename, names_table *Names_table)
{ size_t i;
  procedure* Procedure = procedure_create ();
  
  if( !Procedure ) return NULL;
  Procedure->ResultType = rtype;
  // все имена функций упомянутые выше, должны быть доступны
  for(i=0; i<Names_table->pUserFuncs->len; i++)
  { myvar var, *fnc = *list_pmv_exact (Names_table->pUserFuncs,i);

    memset (&var, 0, sizeof (myvar));
    myvar_copy (fnc, &var);
    hash_lookup (Procedure->BsValVars, var.name, &var, HASH_ADD_ENABLE, (tenum) 0);
    
    memset (&var, 0, sizeof (myvar));
    myvar_copy (fnc, &var);
    hash_lookup (Procedure->Variables, var.name, &var, HASH_ADD_ENABLE, (tenum) 0);
  }
  // буфер из которого получаем фактические параметры, если нет формальный параметров
  { myvar args = {0};
    strcpy (args.name,FUNC_ARGS_NO_NAME);
    args.vtype = Variable;
    args. type = Cnt;

    if ( !(args.value.cvalue = (container*) malloc (sizeof (container))) )
    { SetError (ERR_MEM);
      return NULL;
    }

    container_create (args.value.cvalue, CONT_TYPE_DLIST, 0);
    hash_lookup (Procedure->BsValVars,args.name,&args,HASH_ADD_ENABLE,(tenum)0);

    if ( !(args.value.cvalue = (container*) malloc (sizeof (container))) )
    { SetError (ERR_MEM);
      return NULL;
    }
    container_create (args.value.cvalue, CONT_TYPE_DLIST, 0);
    hash_lookup (Procedure->Variables,args.name,&args,HASH_ADD_ENABLE,(tenum)0);
  }

  if( filename )
  { size_t  len = strlen(filename) + 1;
    Procedure->pFilename = (char*) malloc (sizeof (*Procedure->pFilename)*len);

    if( !Procedure->pFilename ) SetError (ERR_MEM);
    strcpy (Procedure->pFilename,filename);
  }
  else
  { Procedure->pFilename = NULL; }

  return Procedure;
}
void        procedure_free   (procedure *Procedure)
{ if( !Procedure ) return;

      hash_free ( Procedure->Variables );
      hash_free ( Procedure->BsValVars );
  list_pmv_free (&Procedure->Sentences );
  list_pmv_free (&Procedure->FormalPars);
  Procedure->ResultType = NULL;

  if( Procedure->Variables  ) free (Procedure->Variables);
  if( Procedure->BsValVars  ) free (Procedure->BsValVars);

  if( Procedure->fScriptTmp ) fclose ((FILE*)Procedure->fScriptTmp);
  if( Procedure->   pLexTmp )
  { lex_free (Procedure->pLexTmp); free (Procedure->pLexTmp); }
  if( Procedure-> pFilename )      free (Procedure->pFilename);

  free (Procedure);
}
void        procedure_copy   (procedure *scr, procedure *dest)
{ if( !scr || !dest ) { SetError (ERR_PAR); return; }

      hash_copy ( scr->Variables ,  dest->Variables );
      hash_copy ( scr->BsValVars ,  dest->BsValVars );
  list_pmv_copy (&scr->Sentences , &dest->Sentences );
  list_pmv_copy (&scr->FormalPars, &dest->FormalPars);

  if( scr->pFilename )
  { size_t  len = strlen (scr->pFilename) + 1;

    if( !dest->pFilename || (strlen (dest->pFilename) < len) )
    { 
      if( dest->pFilename ) free (dest->pFilename);
      
      dest->pFilename = (char*) malloc (sizeof (*dest->pFilename)*len);
      if( !dest->pFilename ) { SetError (ERR_MEM); return; }
    }
    strcpy (dest->pFilename, scr->pFilename);
  }
  else dest->pFilename = NULL;

  dest->ResultType = scr->ResultType;
  dest->fScriptTmp = NULL;
  dest->   pLexTmp = NULL;
}
//-----------------------------------------------------------------------------------------
int         procedure_fread  (procedure *Procedure, names_table *Names_table, FILE *f_inc)
{ int result = 0;
  size_t i = 0U;
  int flag = 0;

  if( hash_fread (Procedure->BsValVars, f_inc) )
  { result = 1;
    goto END;
  }

  // hash_fread (Procedure->Variables,f_inc);
  hash_copy  (Procedure->BsValVars, Procedure->Variables);

  if( fread (&i, sizeof (i), 1, f_inc) != 1 )
  { SetError (ERR_END);
    result = 1;
    goto END;
  }

  Procedure->ResultType = NULL;
  if( i && !(Procedure->ResultType = findRegWord (Names_table->pReg_Words,(rwenum) i)) )
  { SetError (ERR_CONF);
    result = 1;
    goto END;
  }

  procedure_variables_swap (Procedure, Names_table);
  flag = 1;
  //--------------------------------------
  // запишем формальные параметры функции.
  if( bytecode_fread (Names_table,&Procedure->FormalPars,f_inc) )
  { result = 1; goto END; }

  if( fread (&i,sizeof(unsigned int),1,f_inc) != 1 )
  { SetError (ERR_END); result = 1; goto END; }
  if( i )
  { 
    if ( !(Procedure->pFilename = (char*) malloc (sizeof (*Procedure->pFilename)*i)) )
    { SetError (ERR_MEM);
      result = 1;
      goto END;
    }

    if ( fread (Procedure->pFilename, sizeof (*Procedure->pFilename), i, f_inc) != i )
    { SetError (ERR_END);
      result = 1;
      goto END;
    }
    checkHashScriptFile (Procedure->pFilename, f_inc);
  }
  else
  { Procedure->pFilename = NULL; }

  // запишем в файл код функции.
  if( bytecode_fread (Names_table, &Procedure->Sentences, f_inc) )
  { result = 1; goto END; }
  //--------------------------------------
END:;
  if( flag )
  { procedure_variables_swap (Procedure, Names_table); }

  return result;
}
int         procedure_fwrite (procedure *Procedure, names_table *Names_table, FILE *f_out)
{
  int result = 0;
  size_t i = 0U;

  // запишем в файл хэш-таблицы функции.
  hash_fwrite (Procedure->BsValVars, f_out);
  //hash_fwrite (Procedure->Variables,f_out);

  i = (Procedure->ResultType) ? ((size_t) Procedure->ResultType->rwtype) : (0);
  fwrite (&i, sizeof (i), 1, f_out);
  
  procedure_variables_swap (Procedure, Names_table);
  //--------------------------------------
  // запишем формальные параметры функции.
  bytecode_fwrite (Names_table, &Procedure->FormalPars, f_out); // list_pmv ???

  if( Procedure->pFilename )
  { i = strlen (Procedure->pFilename) + 1U;
    fwrite (&i, sizeof (i), 1, f_out);

    fwrite (Procedure->pFilename, sizeof (*Procedure->pFilename), i, f_out);
    writeHashScriptFile (Procedure->pFilename, f_out);
  }
  else
  { i = 0U;
    fwrite (&i, sizeof (i), 1, f_out);
  }

  // запишем в файл код функции.
  bytecode_fwrite (Names_table, &Procedure->Sentences, f_out);
  //--------------------------------------
  procedure_variables_swap (Procedure, Names_table);

  return result;
}
//-----------------------------------------------------------------------------------------
