#include "Header.h"
#include "interpreter.h"
#include "lexicalCheck.h"
#include "interpreterExecute.h"
#include "executeInOut.h"

//-------------------------------------------------------------------------------
myvar* fscanf_constants (FILE*  fin, names_table *Names_table, myvar *cur)
{ myvar var = {0}, *rsl = NULL;
  var.vtype = Constant;
  
  switch(cur->rwtype)
  { case INTIN_IDRW: // целочисленная константа
     var.type = Int;
      fscanf (     fin,I_OUTLINE,&var.value.ivalue);
     sprintf (var.name,I_OUTLINE, var.value.ivalue);
     rsl = constants_lookup (var.name,&var,HASH_ADD_ENABLE,var.type);
     break;
    
    case FLTIN_IDRW: // вещественная константа
     var.type = Flt;
      fscanf (     fin,F_OUTLINE,&var.value.fvalue);
     sprintf (var.name,F_OUTLINE, var.value.fvalue);
     rsl = constants_lookup (var.name,&var,HASH_ADD_ENABLE,var.type);
     break;
     
    case STRIN_IDRW: // строковая константа
     var.type = Str;
     var.value.svalue = (char*)malloc( sizeof(char)*STRLEN );
     if( !var.value.svalue ) SetError (ERR_MEM);
     fscanf ( fin,S_LEN_OUTLINE,var.value.svalue );
     rsl = constants_lookup (var.value.svalue,&var,HASH_ADD_ENABLE,var.type);
     break;

    default: SetCorrectError ( cur->name,ERR_PAR );
 }

 return rsl;
}
myvar* cnt_const_fscanf (FILE*  fin, names_table *Names_table, char  *buf)
{ int flag_minus = 0; myvar var = {0};
  var.vtype = Constant; 

  if( fscanf (fin,S_OUTLINE,buf) != 1 )
  { SetError (ERR_NPAR); return NULL; }

  flag_minus = (*buf == SYMB_MINUS);
  if( isInteger (buf + flag_minus) )
  { var.type = Int; sscanf (buf,I_OUTLINE,&var.value.ivalue);
    strcpy (var.name,buf);
  }
  else if( isFloatPoint (buf + flag_minus) )
  { var.type = Flt; sscanf (buf,F_OUTLINE,&var.value.fvalue);
    strcpy (var.name,buf);
  }
  else 
  { var.type = Str;
    if( !(var.value.svalue = (char*)malloc( sizeof(char*)*(strlen(buf)+1) )) )
    { SetError (ERR_MEM); return NULL; }
    strcpy(var.value.svalue,buf);
  }
  return constants_lookup (myvar_name (&var),&var,HASH_ADD_ENABLE,var.type);
}
//-------------------------------------------------------------------------------
void       joint_fscanf (FILE*  fin, names_table *Names_table, myvar *jnt)
{ unsigned int i; char buf[STRLEN], *sepr;

  sepr = ( Names_table->pUsrConf->Syntax.at_vars_cnt_sepr_type 
           == IPL_SYNTAX_VAR_CNT_SEPR_BRACE ) ? ("[") : ("(");
  fprintf (Names_table->pUsrConf->f_out,"%s number of joint's elements = ",sepr);

  if( fscanf (fin,I_OUTLINE,&jnt->value.jvalue->len) != 1 )
  { SetCorrectError (myvar_name (jnt),ERR_NPAR); goto END; }
  if( !jnt->value.jvalue->len )
  { SetCorrectError (myvar_name (jnt),ERR_PAR ); goto END; }

  for(i=0; i<jnt->value.jvalue->len; i++)
  { myvar *pkey, *pval;

    pkey = cnt_const_fscanf (fin,Names_table,buf);
    fscanf (fin,":");
    pval = cnt_const_fscanf (fin,Names_table,buf);

    if( !pkey || !pval ) goto END;
    joint_insert (jnt->value.jvalue,pkey,pval);

    if( Names_table->pUsrConf->Syntax.at_func_params_commas 
        && (i != jnt->value.jvalue->len+1) )
     fscanf (fin,",");
  }

  sepr = ( Names_table->pUsrConf->Syntax.at_vars_cnt_sepr_type 
           == IPL_SYNTAX_VAR_CNT_SEPR_BRACE ) ? ("]") : (")");
  fprintf (Names_table->pUsrConf->f_out," %s",sepr);
END:;
}
void   container_fscanf (FILE*  fin, names_table *Names_table, myvar *cnt)
{ unsigned int i; char buf[STRLEN], *sepr;

  sepr = ( Names_table->pUsrConf->Syntax.at_vars_cnt_sepr_type 
           == IPL_SYNTAX_VAR_CNT_SEPR_BRACE ) ? ("(") : ("[");
  fprintf (Names_table->pUsrConf->f_out,"%s number of container's elements = ",sepr);

  if( fscanf (fin,I_OUTLINE,cnt->value.cvalue->len) != 1 )
  { SetCorrectError (myvar_name (cnt),ERR_NPAR); goto END; }
  if( !(*cnt->value.cvalue->len) )
  { SetCorrectError (myvar_name (cnt),ERR_PAR ); goto END; }

  for(i=0; i<(*cnt->value.cvalue->len); i++)
  { myvar *pvar = cnt_const_fscanf (fin,Names_table,buf);
    if( !pvar ) goto END;
    container_insert (cnt->value.cvalue,i,pvar);

    if( Names_table->pUsrConf->Syntax.at_func_params_commas
        && (i != (*cnt->value.cvalue->len)+1) )
     fscanf (fin,",");
  }
  
  sepr = ( Names_table->pUsrConf->Syntax.at_vars_cnt_sepr_type 
           == IPL_SYNTAX_VAR_CNT_SEPR_BRACE ) ? (")") : ("]");
  fprintf (Names_table->pUsrConf->f_out," %s",sepr);
END:;
}
//-------------------------------------------------------------------------------
void      joint_fprintf (FILE* fout, names_table *Names_table, myvar* jnt)
{ joint_node *cur; char *sepr; joint_iter iter = {0}; int flag = 0;

  sepr = ( Names_table->pUsrConf->Syntax.at_vars_cnt_sepr_type 
           == IPL_SYNTAX_VAR_CNT_SEPR_BRACE ) ? ("[") : ("(");
  fprintf (fout,"%s number of joint's elements = %u\n",sepr,jnt->value.jvalue->len);
  
  while( cur = joint_next (jnt->value.jvalue,&iter) )
  { if( flag )
    { sepr = (Names_table->pUsrConf->Syntax.at_func_params_commas) ? (", "):(" ");
      fprintf(fout,sepr);
    }
    else flag = 1;

    switch (cur->key->type)
    { case Int:           fprintf (fout,I_OUTLINE,cur->key->value.ivalue); break;
      case Str:           fprintf (fout,S_OUTLINE,cur->key->value.svalue); break;
      case Flt:           fprintf (fout,F_OUTLINE,cur->key->value.fvalue); break;
      case Cnt: container_fprintf (fout,Names_table,cur->key);             break;
      case Jnt:     joint_fprintf (fout,Names_table,cur->key);             break;
       default:   SetCorrectError (myvar_name (cur->key),ERR_NAME);        break;
    }
    fprintf (fout," : ");

    switch (cur->val->type)
    { case Int:           fprintf (fout,I_OUTLINE,cur->val->value.ivalue); break;
      case Str:           fprintf (fout,S_OUTLINE,cur->val->value.svalue); break;
      case Flt:           fprintf (fout,F_OUTLINE,cur->val->value.fvalue); break;
      case Cnt: container_fprintf (fout,Names_table,cur->val);             break;
      case Jnt:     joint_fprintf (fout,Names_table,cur->val);             break;
       default:   SetCorrectError (myvar_name (cur->val),ERR_NAME);        break;
    }
  }
  sepr = ( Names_table->pUsrConf->Syntax.at_vars_cnt_sepr_type 
           == IPL_SYNTAX_VAR_CNT_SEPR_BRACE ) ? ("]") : (")");
  fprintf (fout," %s\n",sepr);
}
void  container_fprintf (FILE* fout, names_table *Names_table, myvar* cnt)
{ unsigned int i; char *sepr;

  sepr = ( Names_table->pUsrConf->Syntax.at_vars_cnt_sepr_type 
           == IPL_SYNTAX_VAR_CNT_SEPR_BRACE ) ? ("(") : ("[");
  fprintf (fout,"%s number of container's elements = %u\n",sepr,*cnt->value.cvalue->len);
  for(i=0; i<(*cnt->value.cvalue->len);i++)
  { myvar *cur = *container_exact (cnt->value.cvalue,i);

    switch (cur->type)
    { case Int:           fprintf (fout,I_OUTLINE,cur->value.ivalue); break;
      case Str:           fprintf (fout,S_OUTLINE,cur->value.svalue); break;
      case Flt:           fprintf (fout,F_OUTLINE,cur->value.fvalue); break;
      case Cnt: container_fprintf (fout,Names_table,cur);             break;
      case Jnt:     joint_fprintf (fout,Names_table,cur);             break;
       default:   SetCorrectError (myvar_name (cur), ERR_NAME);       break;
    }

    if( i != ((*cnt->value.cvalue->len)+1) )
    { sepr = (Names_table->pUsrConf->Syntax.at_func_params_commas) ? (" ") : (", ");
      fprintf (fout,sepr);
    }
  }
  sepr = ( Names_table->pUsrConf->Syntax.at_vars_cnt_sepr_type 
           == IPL_SYNTAX_VAR_CNT_SEPR_BRACE ) ? (")") : ("]");
  fprintf (fout," %s\n",sepr);
}
//-------------------------------------------------------------------------------