#include "syntaxVars.h"
#include "lexicalCheck.h"

#include "interpreterLexical.h"
#include "interpreterSyntax.h"

int     spc_num_nword (char *buf);
//----------------------------------------------------------------------------------------------------
void  container_const_name (names_table *Names_table, char name[NAMELEN])
{ sprintf (name," %u", ++Names_table->Flg.cEnumerator); }
void      joint_const_name (names_table *Names_table, char name[NAMELEN])
{ sprintf (name," %u", ++Names_table->Flg.jEnumerator); }
//----------------------------------------------------------------------------------------------------
void          sa_vars_copy_2_base (names_table *Names_table, myvar *pvar)
{ myvar var = {0}; 
  // копировать, иначе память очищается дважды ( в constants и в variables )
  // myvar_copy_value (pval,pvar);
  // копировать начальное значение для пред-компиляции
  myvar_copy  (pvar,&var);
  hash_lookup (Names_table->pBsValVars,var.name,&var,HASH_ADD_ENABLE,0);
}
void          sa_vars_comma       (names_table *Names_table, list_pmv *Sentence)
{ myvar *cur = laNextExact (Sentence);
  if( Names_table->pUsrConf->Syntax.variables_declaration == IPL_SYNTAX_VAR_DECL_C_TYPE )
   if( cur && cur->rwtype == COMMA_IDRW ) laNextWord (Sentence);
   else if( cur && cur->vtype != Reg_Word )
    SetCorrectError (myvar_name (cur),ERR_SEPR);
}
myvar*        sa_vars_create_name (names_table *Names_table, list_pmv *Sentence)
{ myvar *cur, var = {0}; char name[NAMELEN];

  *name = 0;
  do
  { cur = laNextWord (Sentence);

    if( *name )
     strcat (name,STR_SYMB_SPACE);
    strcat (name,cur->name);

    if( cur->vtype == Unknow )
     if( !var.vtype )
     { var = constants_extract (cur->name,Not); var.vtype = Variable; }
     else    constants_extract (cur->name,Not);
    else if( cur->vtype == Variable )
     if( !var.vtype ) { strcpy (var.name,cur->name); var.vtype = Variable; }

    cur = laNextExact (Sentence);
  } while( (cur->  type !=        Sgn) &&
           (cur->rwtype != SEMIC_IDRW) &&
           (cur->rwtype != COMMA_IDRW) &&
           (cur->rwtype !=  RESL_IDRW) &&
           (cur->rwtype !=  FROM_IDRW) &&
           (cur->rwtype !=  UNTL_IDRW) &&
           (cur->rwtype !=    DO_IDRW) &&
           (cur->rwtype !=  BYST_IDRW) );

  if( !var.vtype )
   SetCorrectError (myvar_name (cur),ERR_NAME);
  else strcpy (var.name,name);

  cur = variables_lookup (var.name,&var,HASH_ADD_ENABLE);
  { unsigned int i;
    for(i=0; i<Sentence->len; i++)
    { myvar *pmv = *list_pmv_exact (Sentence,i);
      if( pmv->rwtype && (pmv->vtype != Reg_Word) && 
          pmv->type &&   (pmv->type  != Sgn) )
       *list_pmv_exact (Sentence,i) = cur;
    }
  }
  //--- ??? --------------------------------------------
  if( strlen (name) > Names_table->Lex.max_rw_len )
   Names_table->Lex.max_rw_len = strlen (name);
  if( spc_num_nword (name) > Names_table->Lex.max_rw_spc )
   Names_table->Lex.max_rw_spc = spc_num_nword (name);
  //----------------------------------------------------

  return cur;
}
unsigned int  sa_vars_array_type  (names_table *Names_table, list_pmv *Sentence)
{ unsigned int result = 0;
  myvar *cur = laNextExact (Sentence);
  if( cur->rwtype == SIZE_IDRW )
  {       laNextWord  (Sentence);
    cur = laNextExact (Sentence);
    if( (cur->type != Int) || !cur->value.ivalue )
     SetCorrectError (myvar_name (cur),ERR_SYNX);

    result = cur->value.ivalue;
    laNextWord (Sentence);
  }
  return result;
}
//----------------------------------------------------------------------------------------------------
typedef struct _borders borders;
struct _borders
{ rwenum obrace;
  rwenum cbrace;
  rwenum oindex;
  rwenum cindex;
};
void    borders_create (borders *b, names_table *Names_table)
{ //--- здесь пока что все возможные скобки. при расширении списка придется использовать switch ---
  b->obrace = ( Names_table->pUsrConf->Syntax.at_vars_cnt_sepr_type ) ? (BGN_IDRW) : (OINDEX_IDRW);
  b->cbrace = ( Names_table->pUsrConf->Syntax.at_vars_cnt_sepr_type ) ? (END_IDRW) : (CINDEX_IDRW);
  b->oindex = ( Names_table->pUsrConf->Syntax.at_vars_cnt_sepr_type ) ? (OINDEX_IDRW) : (BGN_IDRW);
  b->cindex = ( Names_table->pUsrConf->Syntax.at_vars_cnt_sepr_type ) ? (CINDEX_IDRW) : (END_IDRW);
  //-----------------------------------------------------------------------------------------------
}
//----------------------------------------------------------------------------------------------------
void    sa_var_fill_container   (names_table *Names_table, list_pmv *Sentence, container* pCnt);
void    sa_var_fill_joint       (names_table *Names_table, list_pmv *Sentence,     joint* pJnt);

myvar*  sa_var_object_element   (names_table *Names_table, list_pmv *Sentence, myvar *pyvar, size_t arr_size)
{ myvar* cur; borders b;
  borders_create (&b,Names_table);

  cur = laNextExact (Sentence);
  if( cur->vtype == Constant )
  { if( (cur->type == Sgn) && (cur->rwtype == SUB_ARTH_IDRW) )
    {       laNextWord (Sentence); // --SUB
    
      cur = laNextWord (Sentence); // --value
      if( (cur->type == Int) || (cur->type == Flt) ) 
      { cur = myvar_create_value (cur->value,cur->type); }
      else SetCorrectError(myvar_name (cur),ERR_SGN);
    }
    else cur = laNextWord (Sentence); // --value
    if( pyvar ) myvar_copy_value (cur,pyvar);
  }
  else if( cur->vtype == Variable )
  { cur = laNextWord (Sentence); // --value
    if( pyvar ) myvar_copy_value (cur,pyvar);
    else  cur = myvar_create_value (cur->value,cur->type); // --value
  }
  else if( cur->rwtype == b.obrace )
  { myvar cnt = {0};
    cnt.vtype = Constant;
    cnt. type = Cnt;
    cnt.value.cvalue = (container*)malloc( sizeof(container) );
    if( !cnt.value.cvalue ) SetError(ERR_MEM);

    container_create (cnt.value.cvalue, (arr_size) ? (CONT_TYPE_ARRAY) : (CONT_TYPE_DLIST), arr_size);
    sa_var_fill_container (Names_table,Sentence,cnt.value.cvalue);

    if( pyvar ) { pyvar->type = Cnt; pyvar->value.cvalue = cnt.value.cvalue; }
    else
    { container_const_name (Names_table,cnt.name);
      cur = constants_lookup (cnt.name,&cnt,HASH_ADD_ENABLE,cnt.type); // --value
    }
  }
  else if( cur->rwtype == b.oindex )
  { myvar jnt = {0};
    jnt.vtype = Constant;
    jnt. type = Jnt;
    jnt.value.jvalue = (joint*)malloc( sizeof(joint) );
    if( !jnt.value.jvalue ) SetError(ERR_MEM);
  
    joint_create (jnt.value.jvalue,0);
    sa_var_fill_joint (Names_table,Sentence,jnt.value.jvalue);

    if( pyvar )
    { pyvar->type = Jnt; pyvar->value.jvalue = jnt.value.jvalue; }
    else
    { joint_const_name (Names_table,jnt.name);
      cur = constants_lookup (jnt.name,&jnt,HASH_ADD_ENABLE,jnt.type); // --value
    }
  }
  else SetCorrectError (myvar_name (cur),ERR_NAME);

  pyvar = (pyvar)?(pyvar):(cur);
  return pyvar;
}
void    sa_var_fill_container   (names_table *Names_table, list_pmv *Sentence, container* pCnt)
{ myvar *cur, *obrace;
  borders b;
  
  size_t arr_size = (pCnt->type == CONT_TYPE_ARRAY) ? (pCnt->value.avalue.allocated) : (0U);
  borders_create (&b,Names_table);

  obrace = laNextWord (Sentence); // --(
  if( obrace->rwtype != b.obrace )
   SetCorrectError (myvar_name (obrace),ERR_TYPE);

  cur = laNextExact (Sentence);
  while( cur->rwtype != b.cbrace )
  { cur = sa_var_object_element (Names_table, Sentence, NULL, arr_size);
    container_insert (pCnt,-1,cur);

    cur = laNextExact (Sentence);
    if( cur->rwtype != b.obrace )
     saSeparate (Names_table,Sentence,SA_FUNCT_PARS);
  } // end while
  saEndWord (obrace,laNextWord (Sentence)); // --(
}
void    sa_var_fill_joint       (names_table *Names_table, list_pmv *Sentence,     joint* pJnt)
{ myvar *cur, *obrace, *key, *val; borders b;
  borders_create (&b,Names_table);

  obrace = laNextWord (Sentence); // --(
  if( obrace->rwtype != b.oindex )
   SetCorrectError (myvar_name (obrace),ERR_TYPE);

  cur = laNextExact (Sentence);
  while( cur->rwtype != b.cindex )
  { key = sa_var_object_element (Names_table,Sentence,NULL,0);
    key->sytype = 0;

    cur = laNextWord (Sentence);
    if( cur->rwtype != FIELD_IDRW )
     SetCorrectError (myvar_name (cur),ERR_TYPE); // --:

    val = sa_var_object_element (Names_table,Sentence,NULL,0);
    joint_insert (pJnt,key,val);

    cur = laNextExact (Sentence);
    if( cur->rwtype != b.cindex )
     saSeparate (Names_table,Sentence,SA_FUNCT_PARS);
  } // end while
  saEndWord (obrace,laNextWord (Sentence)); // --(
}
//----------------------------------------------------------------------------------------------------
myvar*  sa_vars_const_integer   (names_table *Names_table, list_pmv *Sentence, myvar *pvar)
{ int flag_sub = 0;
  myvar *cur = laNextExact (Sentence);
  if( cur->rwtype == BND_ARTH_IDRW )
  {       laNextWord (Sentence); // --BND_ARTH_IDRW
    cur = laNextWord (Sentence); // --value
    if( (cur->type == Sgn) && (cur->rwtype == SUB_ARTH_IDRW) ) 
    { flag_sub = 1; cur = laNextWord (Sentence); }
    if( !((cur->vtype == Constant || cur->vtype == Variable) && (cur->type == Int)) )
     SetCorrectError (myvar_name (cur),ERR_TYPE);
    cur = myvar_create_value (cur->value,Int);
  }
  else cur = constants_lookup (ZERO_INT_NAME,NULL,HASH_ADD_DISABLE,Int);

  myvar_copy_value (cur,pvar);
  return pvar;
}
myvar*  sa_vars_const_string    (names_table *Names_table, list_pmv *Sentence, myvar *pvar)
{ myvar *cur = laNextExact (Sentence);
  if( cur->rwtype == BND_ARTH_IDRW )
  {       laNextWord (Sentence); // --BND_ARTH_IDRW
    cur = laNextWord (Sentence); // --value
    if( (cur->vtype == Constant || cur->vtype == Variable) && (cur->type == Str) )
     cur = constants_lookup (cur->value.svalue,cur,HASH_ADD_ENABLE,Str);
    else SetCorrectError(myvar_name (cur),ERR_TYPE);
  }
  else cur = constants_lookup (ZERO_STR_NAME,NULL,HASH_ADD_DISABLE,Str);
  
  myvar_copy_value (cur,pvar);
  return pvar;
}
myvar*  sa_vars_const_float     (names_table *Names_table, list_pmv *Sentence, myvar *pvar)
{ int flag_sub = 0;
  myvar *cur = laNextExact (Sentence);
  if( cur->rwtype == BND_ARTH_IDRW )
  {       laNextWord (Sentence); // --BND_ARTH_IDRW
    cur = laNextWord (Sentence); // --value
    if( cur->type == Sgn && cur->rwtype == SUB_ARTH_IDRW ) 
    { flag_sub = 1; cur = laNextWord (Sentence); }
    if( !( ((cur->vtype == Constant) || (cur->vtype == Variable)) 
        && ((cur-> type == Flt     ) || (cur-> type == Int     )) ))
     SetCorrectError(myvar_name (cur),ERR_TYPE);
    cur = myvar_create_value (cur->value,Flt);
  }
  else cur = constants_lookup (ZERO_FLT_NAME,NULL,HASH_ADD_DISABLE,Flt);

  myvar_copy_value (cur,pvar);
  return pvar;
}
myvar*  sa_vars_const_container (names_table *Names_table, list_pmv *Sentence, myvar *pvar)
{
  size_t size = (size_t) pvar->value.ivalue;
  myvar *cur = constants_lookup (ZERO_CNT_NAME, NULL, HASH_ADD_DISABLE, Cnt);
  pvar->value.cvalue = NULL;
  myvar_copy_value (cur,pvar);

  if ( size ) container_create (pvar->value.cvalue, CONT_TYPE_ARRAY, size);

  cur = laNextExact (Sentence);
  if( cur->rwtype == BND_ARTH_IDRW )
  { laNextWord (Sentence); // --BND_ARTH_IDRW
    sa_var_fill_container (Names_table,Sentence,pvar->value.cvalue); // --value
  }
  return pvar;
}
myvar*  sa_vars_const_joint     (names_table *Names_table, list_pmv *Sentence, myvar* pvar)
{ myvar *cur = constants_lookup (ZERO_JNT_NAME,NULL,HASH_ADD_DISABLE,Jnt);
  myvar_copy_value (cur,pvar);

  cur = laNextExact (Sentence);
  if( cur->rwtype == BND_ARTH_IDRW )
  { laNextWord (Sentence); // --BND_ARTH_IDRW
    sa_var_fill_joint (Names_table,Sentence,pvar->value.jvalue); // --value
  }
  return pvar;
}

typedef myvar* (*pfunc_sa_vars_const)(names_table*,list_pmv*,myvar*);
myvar*  sa_vars_const_choise    (names_table *Names_table, list_pmv *Sentence, myvar* pvar)
{ // массив указателей на функции
  pfunc_sa_vars_const arr_pfunc_sa_var_const_type[] = { NULL, sa_vars_const_integer  , sa_vars_const_string, sa_vars_const_float,
                                                        NULL, sa_vars_const_container, sa_vars_const_joint };

  return arr_pfunc_sa_var_const_type[ pvar->type ] (Names_table,Sentence,pvar);
}
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
// *** C LIKE ***
myvar*   sa_vars_fill_c          (names_table *Names_table, list_pmv *Sentence)
{ myvar *cur = NULL, *pvar;
  size_t arr_size = 0;

  myvar *type = laNextWord (Sentence); // type--
  if( type->rwtype < INT_IDRW || type->rwtype > LST_IDRW )
   SetCorrectError (type->name,ERR_NAME);

  if( (type->rwtype == ARR_IDRW) || (type->rwtype == CNT_IDRW) )
   arr_size = sa_vars_array_type (Names_table,Sentence);
  
  do
  { 
    pvar = sa_vars_create_name (Names_table,Sentence); // name--
    if( (pvar->vtype != Variable) || pvar->type )
     SetCorrectError(pvar->name,ERR_NAME);

    if ( type->rwtype == ARR_IDRW )
    {
      pvar->type = Cnt;
      pvar->value.ivalue = (INTEGER) (arr_size ? arr_size : CONT_ARRAY_DEFAULT_SIZE);
    }
    else if ( type->rwtype == LST_IDRW )
    {
      pvar->type = Cnt;
      pvar->value.ivalue = (INTEGER) (arr_size);
    }
    else if( type->rwtype != FNC_IDRW )
    { pvar->type = (tenum)(type->rwtype - INT_IDRW + 1); }

    if ( type->rwtype == FNC_IDRW )
    {
      sa_user_func_file (Names_table, Sentence, pvar);
      break;
    }
    else
    { sa_vars_const_choise (Names_table,Sentence,pvar); // value--
      sa_vars_copy_2_base  (Names_table,pvar);
      if( !Names_table->Flg.saVarsComma )
       sa_vars_comma       (Names_table,Sentence); // ,--
    }

    cur = laNextExact (Sentence);
  } while( cur && cur->vtype != Reg_Word && !Names_table->Flg.saVarsComma );
  return pvar;
}
//----------------------------------------------------------------------------------------------------
// *** PYTHON LIKE ***
myvar*   sa_vars_fill_py         (names_table *Names_table, list_pmv *Sentence)
{ myvar *cur;

  myvar *pvar = sa_vars_create_name (Names_table,Sentence); // name--
  if( (pvar->vtype != Variable) || pvar->type )
   SetCorrectError(pvar->name,ERR_NAME);
  
  cur = laNextWord (Sentence);
  if( cur->rwtype != BND_ARTH_IDRW )                        // =--
   SetCorrectError(pvar->name,ERR_SYNX);

  cur = laNextExact (Sentence);
  if( cur->rwtype == FNC_IDRW )                             // import--
  { laNextWord (Sentence); 
    sa_user_func_file (Names_table,Sentence,pvar);
  }   
  else
  { sa_var_object_element (Names_table,Sentence,pvar,0);    // value--
    sa_vars_copy_2_base   (Names_table,pvar);
  }
  return pvar;
}
//----------------------------------------------------------------------------------------------------
// *** PASCAL LIKE ***
myvar*   sa_vars_fill_pas        (names_table *Names_table, list_pmv *Sentence)
{ myvar *cur, *obrace; unsigned int arr_size = 0;
  myvar *pvar = sa_vars_create_name (Names_table,Sentence); // name--
  if( (pvar->vtype != Variable) || pvar->type )
   SetCorrectError (pvar->name,ERR_NAME);

  obrace = laNextWord (Sentence); 
  switch ( Names_table->pUsrConf->Syntax.at_vars_pas_sepr_type )
  { default: SetCorrectError(myvar_name (obrace), ERR_INTR); break;
    case IPL_SYNTAX_VAR_PAS_SEPR_BRACE :
     if( obrace->rwtype ==   BGN_IDRW ) break; // '('--
    case IPL_SYNTAX_VAR_PAS_SEPR_FIELD :
     if( obrace->rwtype == FIELD_IDRW ) break; // ':'--
     SetCorrectError(myvar_name (obrace),ERR_SYNX);
  }
  
  cur = laNextWord (Sentence); // type--
  if( cur->rwtype < INT_IDRW || cur->rwtype > LST_IDRW )
   SetCorrectError (myvar_name (cur),ERR_NAME);

  if( (cur->rwtype == ARR_IDRW) || (cur->rwtype == CNT_IDRW) )
   arr_size = sa_vars_array_type (Names_table,Sentence);

  if ( cur->rwtype == ARR_IDRW )
  {
    pvar->type = Cnt;
    pvar->value.ivalue = (INTEGER) (arr_size ? arr_size : CONT_ARRAY_DEFAULT_SIZE);
  }
  else if ( cur->rwtype == LST_IDRW )
  {
    pvar->type = Cnt;
    pvar->value.ivalue = (arr_size);
  }
  else if( cur->rwtype != FNC_IDRW )
  { pvar->type = (tenum)(cur->rwtype - INT_IDRW + 1); }

  if( Names_table->pUsrConf->Syntax.at_vars_pas_sepr_type
      == IPL_SYNTAX_VAR_PAS_SEPR_BRACE )
   saEndWord (obrace,laNextWord (Sentence)); // )--

  if ( cur->rwtype == FNC_IDRW )
  {
    sa_user_func_file (Names_table, Sentence, pvar);
    return pvar;
  }
  else
  { sa_vars_const_choise (Names_table,Sentence,pvar); } // value--

  sa_vars_copy_2_base (Names_table,pvar);
  return pvar;
}
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------
