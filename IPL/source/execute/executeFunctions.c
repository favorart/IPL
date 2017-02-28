#include "Header.h"
#include "interpreter.h"
#include "lexicalCheck.h"
#include "interpreterExecute.h"
#include "executeInOut.h"

#include <time.h>
#define _DEBUG_
#ifdef  _DEBUG_ //---DEBUG--------------------------
FILE** _log_ (void);
int   myvar_print (myvar *pvar);
int   hash_fwrite (hash_table *Hash, FILE* f_out);
#endif  //------------------------------------------

//-------------------------------------------------------------------------------
unsigned int  intlen (INTEGER val)
{ unsigned int i;
  for(i=1; val>=10; i++) { val /= 10; }
  return i;
}
unsigned int  fltlen (FLOAT   val)
{ unsigned int i;
  for(i=0; val>=1.0; i++) { val /= 10; }
  return i;
}

myvar*  findRegWord  (hash_table *RegularWords, rwenum RedWord)
{ myvar* cur; hash_iter iter = {0};
  while( cur = hash_next (RegularWords,&iter) )
   if( cur->rwtype == RedWord ) return cur;
  return NULL;
}
//-------------------------------------------------------------------------------
myvar*  ex_input     (names_table *Names_table, list_pmv* Parameters)
{ myvar  *cur, *rsl; int flag_first = 1;
  FILE   *fin = Names_table->pUsrConf->f_in;

  rsl = *list_pmv_exact (Parameters,0);
  if( rsl->vtype == Pre_Comp )
   rsl = *list_pmv_exact (rsl->value.lvalue,0);

  while( cur = list_pmv_extracts (Parameters,0) )
  { if( cur->vtype == Pre_Comp )
    { //if(cur->value.lvalue->len>1) SetCorrectError(,ERR_MANY);
      cur = list_pmv_extracts (cur->value.lvalue,0);
    }
    if( flag_first && (cur->vtype == Reg_Word) ) // если первый параметр - константа
    { rsl = fscanf_constants (fin,Names_table,cur); }
    else if( cur->vtype == Variable ) // если параметр - переменная
    { if( cur->type )
       fprintf (Names_table->pUsrConf->f_out,"%s = ",cur->name);

      switch (cur->type)
      { case Int: fscanf  (fin,I_OUTLINE,&cur->value.ivalue); break;
        case Str: cur->value.svalue = (char*)malloc(sizeof(char)*STRLEN);
                  if( !cur->value.svalue ) SetError(ERR_MEM);
                  fscanf  (fin,S_OUTLINE, cur->value.svalue); break;
        case Flt: fscanf  (fin,F_OUTLINE,&cur->value.fvalue); break;
        case Cnt: container_fscanf  (fin,Names_table,cur);    break;
        case Jnt:     joint_fscanf  (fin,Names_table,cur);    break;
         default: SetCorrectError (myvar_name(cur),ERR_NAME); break;
       } // конец switch по типам параментров
    }
    else SetCorrectError(cur->name,ERR_NAME);

    flag_first = 0; 
  } // конец цикла по параметрам ввода
  
  fflush (fin);
  return rsl;
}
myvar*  ex_output    (names_table *Names_table, list_pmv* Parameters)
{ myvar *cur, *rsl = myvar_create (NONE_NOT_NAME,Not);
  FILE  *fout = Names_table->pUsrConf->f_out;

  while( cur = list_pmv_extracts (Parameters,0) )
  { if( cur->vtype == Pre_Comp )
     cur = list_pmv_extracts (cur->value.lvalue,0);
   
   if( cur->vtype == Variable && cur->type )
     fprintf (fout,"%s = ",cur->name);
   
    switch (cur->type)
    { case Int: fprintf (fout,I_OUTLINE,cur->value.ivalue); break;
      case Str: fprintf (fout,S_OUTLINE,cur->value.svalue); break;
      case Flt: fprintf (fout,F_OUTLINE,cur->value.fvalue); break;
      case Cnt: container_fprintf   (fout,Names_table,cur); break;
      case Jnt:     joint_fprintf   (fout,Names_table,cur); break;
       default: SetCorrectError (myvar_name(cur),ERR_NAME); break;
    }
    fprintf (fout," ");
  }
  fprintf (fout,"\n");
  return rsl;
}
myvar*  ex_length    (names_table *Names_table, list_pmv* Parameters)
{ // длина/кол-во элементов/кол-во знаков
  myvar *cur, var = {0};
  var.vtype = Constant;
  var. type = Int;

  cur = list_pmv_extracts (Parameters,0);
  switch( cur->type )
  { case Int: var.value.ivalue = (INTEGER) intlen( cur->value.ivalue ); break;
    case Str: var.value.ivalue = (INTEGER) strlen( cur->value.svalue ); break;
    case Flt: var.value.ivalue = (INTEGER) fltlen( cur->value.fvalue ); break;
    case Cnt: var.value.ivalue = (INTEGER) *( cur->value.cvalue->len ); break;
    case Jnt: var.value.ivalue = (INTEGER)  ( cur->value.jvalue->len ); break;
     default: SetCorrectError(cur->name,ERR_TYPE);            break;
  }
  sprintf (var.name,I_OUTLINE,var.value.ivalue); 
  return constants_lookup (var.name,&var,HASH_ADD_ENABLE,var.type);
}
myvar*  ex_type      (names_table *Names_table, list_pmv* Parameters)
{ myvar *cur, var = {0};
  var.vtype = Constant;
  var. type = Str;

 cur = list_pmv_extracts (Parameters,0);
 if( cur->vtype == Variable )
  switch( cur->type )
  { case Int: cur = findRegWord (Names_table->pReg_Words,INT_IDRW); break;
    case Str: cur = findRegWord (Names_table->pReg_Words,STR_IDRW); break;
    case Flt: cur = findRegWord (Names_table->pReg_Words,FLT_IDRW); break;
    case Fnc: cur = findRegWord (Names_table->pReg_Words,FNC_IDRW); break;
    case Cnt: cur = findRegWord (Names_table->pReg_Words,CNT_IDRW); break;
    case Jnt: cur = findRegWord (Names_table->pReg_Words,JNT_IDRW); break;
    default :       SetCorrectError (cur->name,ERR_TYPE);           break;
  }
 else SetCorrectError (cur->name,ERR_VAR);
 return  myvar_create (cur->name,Str);
}
myvar*  ex_cast      (names_table *Names_table, list_pmv* Parameters)
{ myvar var, *cur, *type;

  if( !Parameters || (Parameters->len != 2) )
   SetCorrectError (findRegWord (Names_table->pReg_Words,CAST_IDRW)->name,ERR_NPAR);

  type = list_pmv_extracts (Parameters,0);
  cur  = list_pmv_extracts (Parameters,0);

  if( cur->vtype != Variable )
   SetCorrectError (cur->name,ERR_VAR);
  if( (type->rwtype < INT_IDRW) && (type->rwtype > LST_IDRW) )
   SetCorrectError (type->name,ERR_TYPE);

  var = *cur; memset (&cur->value,1,sizeof(tvalue));
  cur->type  = (tenum)(type->rwtype - INT_IDRW + 1);

  cur = toTypeAny (Names_table,cur,&var);

  myvar_free (&var);
  return cur;
}
myvar*  ex_random    (names_table *Names_table, list_pmv* Parameters)
{ FLOAT fmin, fmax; tvalue fval;
  
  if( (Names_table == NULL) && (Parameters == NULL) )
  { srand ((unsigned int)clock()); return NULL; }

  if( !Parameters || (Parameters->len != 2) )
   SetCorrectError (findRegWord (Names_table->pReg_Words,RAND_IDRW)->name,ERR_NPAR);

  fmin = toTypeFlt (list_pmv_extracts (Parameters,0));
  fmax = toTypeFlt (list_pmv_extracts (Parameters,0));

  if( fmin > fmax )
   SetCorrectError (findRegWord (Names_table->pReg_Words,RAND_IDRW)->name,ERR_PAR );

  fval.fvalue = ((double)rand ()) / ((double)RAND_MAX) * (fmax - fmin) + fmin;
  return myvar_create_value (fval,Flt);
}
myvar*  ex_break     (names_table *Names_table, list_pmv* Parameters)
{ return 0; }
myvar*  ex_user_func (names_table *Names_table, list_pmv* Parameters, myvar *func)
{ myvar *rsl = None (); exSentence exS = {0};
  variables_swap (Names_table,func);

  if( func->value.pvalue->FormalPars.len )
  { unsigned int i;
   
    if( Parameters->len != func->value.pvalue->FormalPars.len )
    { SetCorrectError (myvar_name (func),ERR_PAR); goto END; }
   
    for(i=0; Parameters->len; i++)
    { myvar* pvar, *frml, *fact;
     
      frml = *list_pmv_exact   (&func->value.pvalue->FormalPars,i);
      fact =  list_pmv_extracts ( Parameters,0);
      pvar =  variables_lookup ( frml->name,NULL,HASH_ADD_DISABLE);
    
      if( frml && fact && (frml->type == fact->type) )
       myvar_copy_value (fact,pvar);
      else
      {      if(fact) SetCorrectError (myvar_name (fact),ERR_TYPE);
        else if(frml) SetCorrectError (myvar_name (frml),ERR_PAR );
        else SetError (ERR_PAR);
        rsl = myvar_create (NONE_NOT_NAME,Not); goto END;
      }
    }
  }
  else
  { myvar *pargs = variables_lookup (FUNC_ARGS_NO_NAME,NULL,HASH_ADD_DISABLE);
    if( !pargs )  { SetCorrectError (FUNC_ARGS_NO_NAME,ERR_NAME); goto END; }

    while( Parameters->len )
    { myvar *pvar = list_pmv_extracts (Parameters,0);
      pvar = myvar_create_value (pvar->value,pvar->type);
      container_insert (pargs->value.cvalue,-1,pvar);
    }
  }

#ifdef  _DEBUG_ //---DEBUG--------------------------
  { myvar* cur; hash_table *Hash = Names_table->pVariables; hash_iter iter = {0};
    fprintf (*_log_ (),"\nProcedure variables:\n");
    while( cur = hash_next (Hash,&iter) )
     myvar_print (cur); 
    fprintf (*_log_ (),"\n\n");
  }
#endif  //------------------------------------------

  Names_table->Flg.DepthRecurs++;
  //------------------------------------------------------------------------
  //---выполнение функции---
        exSentenceConvert (&exS,&func->value.pvalue->Sentences,Names_table);
  rsl = exInterpreter     (&exS,NULL,NULL);
        exSentenceCollect (&exS,&func->value.pvalue->Sentences,Names_table);
  //------------------------------------------------------------------------
  Names_table->Flg.DepthRecurs--;

  if( func->value.pvalue->ResultType->rwtype )
  { rwenum  rw = func->value.pvalue->ResultType->rwtype;
    if( (rw == ARR_IDRW) || (rw == LST_IDRW) ) rw = CNT_IDRW;
    if( (rw - INT_IDRW + 1) != rsl->type )
    { SetCorrectError (myvar_name (rsl),ERR_TYPE); rsl = None (); goto END; }
  }

END:;
  variables_swap (Names_table,func);
  return rsl;
}
//-------------------------------------------------------------------------------
myvar*  ex_bind      (names_table *Names_table, list_pmv* Parameters)
{ myvar *rsl, *cur;

  rsl = list_pmv_extracts (Parameters,0);
  cur = list_pmv_extracts (Parameters,0);

  if( rsl->vtype == Variable )
   rsl = toTypeAny (Names_table,rsl,cur);
  else if( (rsl->vtype == Constant) && Names_table->Flg.ConstInCont )
  { /* внимание: костыль !!! */
    if( cur->vtype == Variable )
     rsl = myvar_create_value (cur->value,cur->type);
    else if( cur->vtype == Constant )
     rsl = cur;
    else
     SetCorrectError (myvar_name (cur),ERR_TYPE);
    //rsl = constants_lookup (cur->name,cur,HASH_ADD_ENABLE,cur->type);
    *Names_table->Flg.ConstInCont =  rsl;
     Names_table->Flg.ConstInCont = NULL;
  }
  else SetCorrectError (myvar_name (rsl),ERR_VAR);

  /*switch( rsl->type )
  { case Not: SetCorrectError (rsl->name,ERR_NAME); break;
    case Int: rsl->value.ivalue = toTypeInt(cur);   break;
    case Str: if(rsl->value.svalue) free(rsl->value.svalue); 
              rsl->value.svalue = toTypeStr(cur);   break;
    case Flt: rsl->value.fvalue = toTypeFlt(cur);   break;
    case Cnt:                                       break;
    case Jnt:                                       break;
     default: SetCorrectError (rsl->name,ERR_CAST); break;
  }*/

  return rsl;
}
myvar*  ex_not       (names_table *Names_table, list_pmv* Parameters)
{ myvar *cur; int rsl;

  // это унарный оператор - первый параметр 0 
        list_pmv_extracts ( Parameters,0 );
  cur = list_pmv_extracts ( Parameters,0 );
  rsl = !isTrueMyVar (cur);
  return myvar_create ((rsl)?(TRUE_INT_NAME):(ZERO_INT_NAME),Int);
}
myvar*  ex_add       (names_table *Names_table, list_pmv* Parameters)
{ myvar *var_l, *var_r; tvalue value;

  var_l = list_pmv_extracts (Parameters,0);
  var_r = list_pmv_extracts (Parameters,0);

  switch (var_l->type)
  { case Int: value.ivalue = var_l->value.ivalue + toTypeInt (var_r); break;
    case Flt: value.fvalue = var_l->value.fvalue + toTypeFlt (var_r); break;
    case Str:
     { char str[STRLEN]; toTypeStr (var_r,str);

       if( !var_l->value.svalue || !str )
        SetCorrectError (myvar_name (var_l),ERR_TYPE);
       else
       { size_t len = strlen (var_l->value.svalue) + strlen (str) + 1;
         if( !(value.svalue = (char*)malloc( sizeof(char)*len )) )
          SetCorrectError (myvar_name (var_l),ERR_MEM);

         strcpy (value.svalue,var_l->value.svalue);
         strcat (value.svalue,str);
       }
     }
     break;
    case Not: SetCorrectError (myvar_name (var_l),ERR_NAME); break;
    default : SetCorrectError (myvar_name (var_l),ERR_TYPE); break;
  }
  return myvar_create_value (value,var_l->type);
}
myvar*  ex_sub       (names_table *Names_table, list_pmv* Parameters)
{ myvar *var_l, *var_r; tvalue value;

  var_l = list_pmv_extracts (Parameters,0);
  var_r = list_pmv_extracts (Parameters,0);

  switch (var_l->type)
  { case Int: value.ivalue = var_l->value.ivalue - toTypeInt (var_r); break;
    case Flt: value.fvalue = var_l->value.fvalue - toTypeFlt (var_r); break;
    case Not:          SetCorrectError (myvar_name (var_l),ERR_NAME); break;
    default :          SetCorrectError (myvar_name (var_l),ERR_TYPE); break;
  }
  return myvar_create_value (value,var_l->type);
}
myvar*  ex_mul       (names_table *Names_table, list_pmv* Parameters)
{ myvar *var_l, *var_r; tvalue value;

  var_l = list_pmv_extracts (Parameters,0);
  var_r = list_pmv_extracts (Parameters,0);

  switch (var_l->type)
  { case Int: value.ivalue = var_l->value.ivalue * toTypeInt (var_r); break;
    case Flt: value.fvalue = var_l->value.fvalue * toTypeFlt (var_r); break;
    case Not:          SetCorrectError (myvar_name (var_l),ERR_NAME); break;
    default :          SetCorrectError (myvar_name (var_l),ERR_TYPE); break;
  }
  return myvar_create_value (value,var_l->type);
}
myvar*  ex_div       (names_table *Names_table, list_pmv* Parameters)
{ myvar *var_l, *var_r; tvalue value;

  var_l = list_pmv_extracts (Parameters,0);
  var_r = list_pmv_extracts (Parameters,0);

  switch (var_l->type)
  { case Int:
     { INTEGER i = toTypeInt (var_r);
       if( !i ) SetCorrectError(myvar_name (var_r),ERR_ZERO);
       value.ivalue = var_l->value.ivalue / i;
     }
     break;
    case Flt:
     { FLOAT f = toTypeFlt (var_r);
       if( !f ) SetCorrectError(var_r->name,ERR_ZERO);
       value.fvalue = var_l->value.fvalue / f;
     }
     break;
    case Not: SetCorrectError (myvar_name (var_l),ERR_NAME); break;
    default : SetCorrectError (myvar_name (var_l),ERR_TYPE); break;
  }
  return myvar_create_value (value,var_l->type);
}
myvar*  ex_mod       (names_table *Names_table, list_pmv* Parameters)
{ myvar *var_l, *var_r; tvalue value;

  var_l = list_pmv_extracts (Parameters,0);
  var_r = list_pmv_extracts (Parameters,0);

  switch (var_l->type)
  { case Int:
     { INTEGER i = toTypeInt (var_r);
       if( !i ) SetCorrectError(myvar_name (var_r),ERR_ZERO);
       value.ivalue = var_l->value.ivalue % i;
     }
     break;
    case Flt:
     { INTEGER f = (INTEGER)toTypeFlt (var_r);
       SetWarning (var_r->name,WRG_TCAST);
       if( !f ) SetCorrectError (var_r->name,ERR_ZERO);
       value.ivalue = (INTEGER)var_l->value.fvalue % f;
     }
     break;
    case Not: SetCorrectError (myvar_name (var_l),ERR_NAME); break;
    default : SetCorrectError (myvar_name (var_l),ERR_TYPE); break;
  }
  return myvar_create_value (value,var_l->type);
}
myvar*  ex_and       (names_table *Names_table, list_pmv* Parameters)
{ myvar *var_l, *var_r; int rsl;

  var_l = list_pmv_extracts (Parameters,0);
  var_r = list_pmv_extracts (Parameters,0);
 
  rsl = ( isTrueMyVar (var_l) && isTrueMyVar (var_r) );
  return myvar_create ((rsl)?(TRUE_INT_NAME):(ZERO_INT_NAME),Int);
}
myvar*  ex_or        (names_table *Names_table, list_pmv* Parameters)
{ myvar *var_l, *var_r; int rsl;

  var_l = list_pmv_extracts ( Parameters,0 );
  var_r = list_pmv_extracts ( Parameters,0 );
 
  rsl = ( isTrueMyVar (var_l) || isTrueMyVar (var_r) );
  return myvar_create ((rsl)?(TRUE_INT_NAME):(ZERO_INT_NAME),Int);
}
myvar*  ex_xor       (names_table *Names_table, list_pmv* Parameters)
{ myvar *var_l, *var_r; int rsl;

  var_l = list_pmv_extracts ( Parameters,0 );
  var_r = list_pmv_extracts ( Parameters,0 );
 
  rsl = ( isTrueMyVar (var_l) ^ isTrueMyVar (var_r) );
  return myvar_create ((rsl)?(TRUE_INT_NAME):(ZERO_INT_NAME),Int);
}

myvar*  ex_equal     (names_table *Names_table, list_pmv* Parameters)
{ myvar *var_l, *var_r; int rsl;

  var_l = list_pmv_extracts (Parameters,0);
  var_r = list_pmv_extracts (Parameters,0);
 
  rsl = !myvar_compare (&var_l,&var_r);
  return myvar_create ((rsl)?(TRUE_INT_NAME):(ZERO_INT_NAME),Int);
}
myvar*  ex_no_eq     (names_table *Names_table, list_pmv* Parameters)
{ myvar *var_l, *var_r; int rsl;

  var_l = list_pmv_extracts ( Parameters,0 );
  var_r = list_pmv_extracts ( Parameters,0 );

  rsl = myvar_compare (&var_l,&var_r);
  return myvar_create ((rsl)?(TRUE_INT_NAME):(ZERO_INT_NAME),Int);
}
myvar*  ex_gt_eq     (names_table *Names_table, list_pmv* Parameters)
{ myvar *var_l, *var_r; int rsl;

  var_l = list_pmv_extracts ( Parameters,0 );
  var_r = list_pmv_extracts ( Parameters,0 );
  rsl = (myvar_compare (&var_l,&var_r) >= 0);
  return myvar_create ((rsl)?(TRUE_INT_NAME):(ZERO_INT_NAME),Int);
}
myvar*  ex_ls_eq     (names_table *Names_table, list_pmv* Parameters)
{ myvar *var_l, *var_r; int rsl;

  var_l = list_pmv_extracts ( Parameters,0 );
  var_r = list_pmv_extracts ( Parameters,0 );
  rsl = (myvar_compare (&var_l,&var_r) <= 0);
  return myvar_create ((rsl)?(TRUE_INT_NAME):(ZERO_INT_NAME),Int);
}
myvar*  ex_great     (names_table *Names_table, list_pmv* Parameters)
{ myvar *var_l, *var_r;int rsl;

  var_l = list_pmv_extracts ( Parameters,0 );
  var_r = list_pmv_extracts ( Parameters,0 );
  rsl = (myvar_compare (&var_l,&var_r) > 0);
  return myvar_create ((rsl)?(TRUE_INT_NAME):(ZERO_INT_NAME),Int);
}
myvar*  ex_less      (names_table *Names_table, list_pmv* Parameters)
{ myvar *var_l, *var_r; int rsl;

  var_l = list_pmv_extracts ( Parameters,0 );
  var_r = list_pmv_extracts ( Parameters,0 );
  rsl = (myvar_compare (&var_l,&var_r) < 0);
  return myvar_create ((rsl)?(TRUE_INT_NAME):(ZERO_INT_NAME),Int);
}
//-------------------------------------------------------------------------------
myvar*  myvar_const  (myvar *var)
{ myvar cns = {0}, *rsl = NULL;

  if(!var) SetError(ERR_PTR);
  myvar_copy_value (var,&cns);

  cns.vtype =  Constant;
  cns. type = var->type;
  
  switch( cns.type )
  { case Int:
     sprintf (cns.name,I_OUTLINE,cns.value.ivalue);
     rsl = constants_lookup (cns.name,&cns,HASH_ADD_ENABLE,Int );
     break;

    case Str:
     rsl = constants_lookup (cns.value.svalue,&cns,HASH_ADD_ENABLE,Str );
     break;

    case Flt:
     sprintf (cns.name,F_OUTLINE,cns.value.fvalue);
     rsl = constants_lookup (cns.name,&cns,HASH_ADD_ENABLE,Flt );
     break;

    case Cnt:
     container_const_name (*_Names_table(),cns.name);
     rsl = constants_lookup (cns.name,&cns,HASH_ADD_ENABLE,Cnt );
     break;

    case Jnt:
     joint_const_name (*_Names_table(),cns.name);
     rsl = constants_lookup (cns.name,&cns,HASH_ADD_ENABLE,Jnt );
     break;

     default: SetCorrectError (var->name,ERR_TYPE);
     break;
  }
  return rsl;
}

myvar*  ex_cnt_add   (names_table *Names_table, list_pmv* Parameters)
{ int key;
  myvar *elem = NULL, *cur = NULL, **ppmv = NULL, *cont = NULL, *result = None (); 
  //  Names_table->pUsrConf->Syntax.at_obj_method_regular
  // add  [elem,None]                                      // добавить элемент  elem  в конец
  // add  [elem,indx]                                      // добавить элемент  elem по индексу  indx
  // !Names_table->pUsrConf->Syntax.at_obj_method_regular
  // add  [elem,indx , elem,None , ... ]                   // добавить элементы elem по индексам indx|None
  
  cont = list_pmv_extracts (Parameters,0);
  while( elem = list_pmv_extracts (Parameters,0) )
  { if( !elem || isNone (elem) )
     SetCorrectError (myvar_name (cont),ERR_PAR);
    elem = myvar_const (elem); // var --> const

    if( cont->type == Cnt )
    { cur = list_pmv_extracts (Parameters,0);
      key = (isNone (cur)) ? (-1) : (int)toTypeInt (cur);
             container_insert (cont->value.cvalue,key,elem);
      ppmv = container_exact  (cont->value.cvalue,key);
    }
    else if( cont->type == Jnt )
    {        joint_insert (cont->value.jvalue,cur,elem);
      ppmv = joint_exact  (cont->value.jvalue,cur);
    }
    else SetCorrectError (myvar_name (cont),ERR_TYPE);
    
    if( isNone (result) )
    { Names_table->Flg.ConstInCont = ppmv;
      result = *Names_table->Flg.ConstInCont; 
    }
    cur = NULL; elem = NULL;
  }
  return result;
}
myvar*  ex_cnt_del   (names_table *Names_table, list_pmv* Parameters)
{ myvar *cur, *cont, *result = None ();
  // el  = <Cnt> [ DEL_CNT_IDRW ( None & inx, el & None, ... )  | None & inx ]
  
  cont = list_pmv_extracts (Parameters,0);
  if( cont->type == Cnt )
  { size_t key;
   
    while( cur = list_pmv_extracts (Parameters,0) )
    { if( !cur || isNone (cur) ) // удаление по индексу
      { // получаем индекс
        key = (int)toTypeInt (list_pmv_extracts (Parameters,0));

        if( key < *cont->value.cvalue->len )
        { // собственно удаляем элемент
          cur = container_extract (cont->value.cvalue,key);
          // возвращаем первый удаленный элемент
          if( isNone (result) ) result = cur;
          else myvar_free_count (cur,MV_FREE_SELF_Y);
        }
      }
      else // удаление по значению
      { // функция работает как нахождение первого слева элемента с таким значением
        cur = container_find (cont->value.cvalue, &key, cur, myvar_compare_equal);
        if( cur )
        { // собственно удаляем элемент
          cur = container_extract (cont->value.cvalue,key);
          // возвращаем первый удаленный элемент
          if( isNone (result) ) result = cur; 
          else myvar_free_count (cur,MV_FREE_SELF_Y);
        }
        // формальная проверка на четность -
        // все параметры этой функции должны идти парами
        if( !isNone (cur = list_pmv_extracts (Parameters,0)) )
         SetCorrectError (myvar_name (cont),ERR_NPAR);
      }      
      cur = NULL; 
    }
  }
  else if( cont->type == Jnt )
  { while( cur = list_pmv_extracts (Parameters,0) )
    { if( !cur || isNone (cur) ) // удаление по индексу
      { cur = list_pmv_extracts (Parameters,0);
        // собственно удаляем элемент
        cur = joint_extract(cont->value.jvalue,cur);
        // возвращаем первый удаленный элемент
        if( isNone (result) && !cur )  result = cur;
        else  myvar_free_count (cur,MV_FREE_SELF_Y);
      }
      else // удаление по значению
      { // функция работает как нахождение первого слева элемента с таким значением
        cur = joint_find   (cont->value.jvalue,cur,myvar_compare_equal);
        if( cur )
        { // собственно удаляем элемент
          cur = joint_extract(cont->value.jvalue,cur);
          // возвращаем первый удаленный элемент
          if( isNone (result) && !cur )  result = cur;
          else  myvar_free_count (cur,MV_FREE_SELF_Y);
        }
        if( !isNone (cur = list_pmv_extracts (Parameters,0)) )
         SetCorrectError (myvar_name (cont),ERR_NPAR);
      } 
    }
  }
  else SetCorrectError (myvar_name (cont),ERR_TYPE);
  return result;
}
myvar*  ex_cnt_fnd   (names_table *Names_table, list_pmv* Parameters)
{ myvar *cur, *cont, *result = None ();
  // <cont> find <elem>  // (вернуть elem или 0)

  cont = list_pmv_extracts (Parameters,0); // cont
  cur  = list_pmv_extracts (Parameters,0);
  if( !cur || isNone (cur) )
   SetCorrectError (myvar_name (cont),ERR_PAR);

  if( cont->type == Cnt )
  { 
    size_t index;
    if ( !container_find (cont->value.cvalue, &index, cur, myvar_compare_equal) )
    {
      tvalue kval;
      kval.ivalue = (INTEGER) index;
      result = myvar_create_value (kval, Int);
    }
  }
  else if( cont->type == Jnt )
  { 
    cur = joint_find (cont->value.jvalue,cur,myvar_compare_equal);
    if( cur )
      result = cur;
  }
  else
  { SetCorrectError (myvar_name (cont),ERR_TYPE); }

  return result;
}
myvar*  ex_cnt_clr   (names_table *Names_table, list_pmv* Parameters)
{ myvar *cont = list_pmv_extracts (Parameters,0);

  if( !isEnumerable (cont->type) )
   SetCorrectError (findRegWord (Names_table->pReg_Words,CLR_CNT_IDRW)->name,ERR_TYPE);
  myvar_free_count (cont,MV_FREE_SELF_N);
  return None ();
}
myvar*  ex_cnt_srt   (names_table *Names_table, list_pmv* Parameters)
{ myvar *revr, *cont;
  int (*comp)(myvar**,myvar**);

  cont = list_pmv_extracts (Parameters,0);
  revr = list_pmv_extracts (Parameters,0);
  comp = (revr->rwtype == RVRS_IDRW)?(myvar_compare_revr):(myvar_compare);

  if(  cont->type != Cnt )
  { SetCorrectError (myvar_name (cont),ERR_TYPE);
    goto END;
  }
  if( *cont->value.cvalue->len < 2 )
    goto END;
 
  if( cont->value.cvalue->type == CONT_TYPE_ARRAY )
    qsort (  cont->value.cvalue->value.avalue.content,
            *cont->value.cvalue->len,sizeof(myvar*),
            (qsort_comp)(comp));
  else if ( cont->value.cvalue->type == CONT_TYPE_DLIST )
    list_pmv_merge_sort (&cont->value.cvalue->value.lvalue, comp);
  else
    SetCorrectError (myvar_name (cont),ERR_TYPE);
 
END:;
  return None ();
}
myvar*  ex_cnt_index (names_table *Names_table, list_pmv* Parameters)
{ unsigned int i; myvar *result  = list_pmv_extracts (Parameters,0); // cont

  for(i=0; i<Parameters->len; i++)
  { myvar *key = list_pmv_extracts (Parameters,0);
   
         if( result->type == Cnt )
    { Names_table->Flg.ConstInCont = container_exact (result->value.cvalue,(int)toTypeInt(key));
      result = (Names_table->Flg.ConstInCont) ? (*Names_table->Flg.ConstInCont) : (None ());
    }
    else if( result->type == Jnt )
    { Names_table->Flg.ConstInCont = joint_exact (result->value.jvalue,key);
      if( !Names_table->Flg.ConstInCont ) SetCorrectError (myvar_name (key),ERR_NJNT);
      result = *Names_table->Flg.ConstInCont;
    }
    else SetCorrectError (myvar_name (result),ERR_TYPE);
  }
  return (result) ? (result) : (None ());
}
//-------------------------------------------------------------------------------
//myvar*  ex_sqrt   (myvar* var)
//myvar*  ex_abs    (myvar* num)
//myvar*  ex_pow    (myvar* num, myvar* pow)
//myvar*  ex_round  (myvar* num, myvar* g_l)

//myvar*  ex_--//-- (names_table *Names_table, list_pmv* Parameters)

//myvar*  ex_pi  (myvar* num)
//myvar*  ex_exp (myvar* num, myvar* pow)
//myvar*  ex_log (myvar* num, myvar* base)
//myvar*  ex_ln  (myvar* num)

//myvar*  ex_sin (myvar* var)
//myvar*  ex_cos (myvar* var)
//myvar*  ex_tg  (myvar* var)

//myvar*  ex_arcsin (myvar* var)
//myvar*  ex_arccos (myvar* var)
//myvar*  ex_arctg  (myvar* var)
//----------------------------------------------------------------------------------------------------
