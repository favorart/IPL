#include "Header.h"
#include "interpreter.h"
#include "LexicalCheck.h"

#include "rwidArray.h"
#include "mvList.h"

#include "interpreterExecute.h"

int  regwords_hash_table_printf (hash_table *RegularWords);
//---------------------------------------------------------------------------------------------------------------
int       regwords_get_comment (char  *buf, size_t *offset, char *str)
{ char *pchar = NULL;

  if( isQuote (*str) )
  { str = 1 + strchr (str+1,SYMB_QUOTE); }

  if( pchar = strchr (str, SYMB_LCOMM) )
  { *pchar = 0;
     pchar = strchr (buf + *offset,SYMB_NLINE);
    *offset = pchar - buf;
    return 1;
  }

  return 0;
}
int       regwords_get_id_s    (char **buf, array_id *rw_ids)
{ int result = 0;

  size_t i, j;
  
  FILE* f = NULL; long file_size;
  char  c;  unsigned int  offset;
  
   // open file stream
  if( !(f = fopen (RW_ID_FILENAME,"r")) )
  { SetCorrectError (RW_ID_FILENAME,ERR_FILE);
    result = 1;
    goto END;
  }

  // get file size
  fseek (f,0L,SEEK_END);
  file_size = ftell (f);
  fseek (f,0L,SEEK_SET);

  // alloc memory
  if( !( *buf = (char*) malloc ( sizeof(char)*(file_size+1) )) ) 
  { SetCorrectError(NULL,ERR_MEM);
    result = 1;
    goto END;
  }

  // read text from file
  fread (*buf, sizeof (*buf), file_size, f);
  (*buf)[file_size] = 0;

  // get number of regular words
  sscanf (*buf, "%u%n", &i, &offset);
  array_id_create (rw_ids, i);
  rw_ids->len = i;

  // get IDs of words
  for(j=0, i=offset, c=(*buf)[i]; c && (j!=rw_ids->len); c=(*buf)[++i])
  { 
    if( (c != SYMB_SPACE) && (c != SYMB_NLINE) )
    { 
      array_id_exact (rw_ids, j)->name  = *buf + i;       
      array_id_exact (rw_ids, j)->id    = (rwenum)(j + 1);
      
      do
      { c = (*buf)[++i]; }
      while( c != SYMB_EQUAL );
      
      j++;
      (*buf)[++i] = 0;
    }
  }
  // sorting words by characters set
  qsort (rw_ids->content, rw_ids->len, sizeof (rw_id), rw_id_cmp);

//#define _DEBUG_
#ifdef  _DEBUG_ //---DEBUG---------------------------------

  printf ("\n\nreg.word ID:\n");
  for(i=0; i<rw_ids->len; i++)
   printf("\t%3d\t%3d\t%s\n",i,array_id_exact(rw_ids,i)->id,
                               array_id_exact(rw_ids,i)->name );

#endif //--------------------------------------------------

END:;
  // free temp resources
  if( result ) if(*buf) { free (*buf); *buf = NULL; }
               if(f)    { fclose  (f);    f = NULL; }

  return 0;
}
p_action  regwords_get_action  (rwenum id)
{ p_action result;
  switch (id)
  { default:              result = NULL;         break;
//    case      PROG_IDRW : result = ex_program;   break;
    case        IN_IDRW : result = ex_input;     break;
    case       OUT_IDRW : result = ex_output;    break;
    case      SIZE_IDRW : result = ex_length;    break;
    case      TYPE_IDRW : result = ex_type;      break;
    case      CAST_IDRW : result = ex_cast;      break;
    case      RAND_IDRW : result = ex_random;    break;
//    case      UFNC_IDRW : result = ex_user_func; break;
    case  BND_ARTH_IDRW : result = ex_bind;      break;
    case  ADD_ARTH_IDRW : result = ex_add;       break;
    case  SUB_ARTH_IDRW : result = ex_sub;       break;
    case  MUL_ARTH_IDRW : result = ex_mul;       break;
    case  DIV_ARTH_IDRW : result = ex_div;       break;
    case  MOD_ARTH_IDRW : result = ex_mod;       break;
    case   OR_ARTH_IDRW : result = ex_or ;       break;
    case  AND_ARTH_IDRW : result = ex_and;       break;
    case  NOT_ARTH_IDRW : result = ex_not;       break;
 // case  XOR_ARTH_IDRW : result = ex_xor;       break;
    case    E_ARTH_IDRW : result = ex_equal;     break;
    case    L_ARTH_IDRW : result = ex_less;      break;
    case    G_ARTH_IDRW : result = ex_great;     break;
    case   NE_ARTH_IDRW : result = ex_no_eq;     break;
    case   LE_ARTH_IDRW : result = ex_ls_eq;     break;
    case   GE_ARTH_IDRW : result = ex_gt_eq;     break;
    case   ADD_CNT_IDRW : result = ex_cnt_add;   break;
    case   DEL_CNT_IDRW : result = ex_cnt_del;   break;
    case   FND_CNT_IDRW : result = ex_cnt_fnd;   break;
    case   CLR_CNT_IDRW : result = ex_cnt_clr;   break;
    case   SRT_CNT_IDRW : result = ex_cnt_srt;   break;
  }
  return result;
}
tenum     regwords_get_type    (rwenum id)
{ return (  id==     BGN_IDRW  ||  id==    END_IDRW   ||
          ((id>=BND_ARTH_IDRW) && (id<=GE_ARTH_IDRW)) ||
         /* id==  OINDEX_IDRW  ||  id== CINDEX_IDRW ||*/ id==FIELD_IDRW ) ? (Sgn) : (Not);
}
int       regwords_get_sytype  (rwenum id)
{ return ( (id ==    BGN_IDRW) || (id ==    END_IDRW) ||
           (id == OINDEX_IDRW) || (id == CINDEX_IDRW) ||
           (id ==  OCOMM_IDRW) || (id ==  CCOMM_IDRW) ); }
//---------------------------------------------------------------------------------------------------------------
rwenum  find_next_id  (char *buf, array_id *rw_ids, size_t *offset)
{ char   word_id[BUFLEN];
  size_t word_id_len = 0;
  
  rw_id *curr = NULL;
  rw_id  key  = { word_id, NO_IDRW };

  do {  // ищем word_id в тексте
   if( sscanf (buf+*offset,"%s%n",word_id,&word_id_len) == -1 )
   { SetCorrectError (word_id,ERR_CONF);
     return NO_IDRW;
   }

   *offset += word_id_len;
   regwords_get_comment (buf,offset,word_id);

   word_id_len = strlen (word_id);
   // last word cheked in upper level
  } while( (word_id_len < MIN_RW_LEN)             \
        && (word_id[word_id_len-1] != SYMB_EQUAL) ); 
  
  curr = (rw_id*) bsearch (&key, rw_ids->content, rw_ids->len, 
                           sizeof (rw_id), rw_id_cmp);
  // когда нашли
  if ( !curr )
  { SetCorrectError (word_id,ERR_CONF);
    return NO_IDRW;
  }

  return curr->id;
}
int     find_next_rw  (char *buf, char    *word_rw, size_t *offset)
{
  int result = 0, spc = 0;
  size_t len = 0U;
  
  char str[BUFLEN];
  unsigned int loffcet = 0;
  //-------------------------------------------
  *word_rw = 0;
  //---ищем word_rw в тексте---пока не нашли---
  do {
   if( sscanf (buf + *offset, "%s%n", str, &loffcet) == -1 )
   { *offset += loffcet;
     result = 0;
     break;
   }
   
   if( str[ strlen(str)-1 ] == SYMB_EQUAL )
   { result = 0;
     break;
   }
   *offset += loffcet;

   regwords_get_comment (buf, offset, str);
   if( !strcmp (str, STR_SYMB_SEPAR) )
   { result = 1;
     break;
   }

   if( strlen (word_rw) && strlen (str) )
   { strcat (word_rw,STR_SYMB_SPACE);
     spc++;
   }

   strcat (word_rw,str);
  } while (1);

  //---дополнительная информация для лексического анализатора---
  len = strlen (word_rw);
  if( spc > (*_Names_table())->Lex.max_rw_spc ) (*_Names_table())->Lex.max_rw_spc = spc;
  if( len > (*_Names_table())->Lex.max_rw_len ) (*_Names_table())->Lex.max_rw_len = len;
  //------------------------------------------------------------
  return result;
}
//---------------------------------------------------------------------------------------------------------------
int       regwords_config (char* filename, hash_table *RegularWords)
{ int result = 0;
  size_t i, offset = 0;

  FILE* f = NULL; long file_size;
  array_id rw_ids = {0};
  char *rw_id_buf = NULL, *buf = NULL;
  //--------------------
  // создание соответствия между enum и word_id
  regwords_get_id_s (&rw_id_buf,&rw_ids);

  // open file stream
  if( !(f = fopen (filename,"rb")) )
  { SetCorrectError (filename,ERR_FILE); result = 1; goto END; }

  // get file size
  fseek (f,0L,SEEK_END);
  file_size = ftell (f);
  fseek (f,0L,SEEK_SET);

  // alloc memory
  if( !( buf = (char*)malloc( sizeof(char)*(file_size+1) )) ) 
  { SetCorrectError(NULL,ERR_MEM); result = 1; goto END; }
  memset (buf,0,sizeof(char)*(file_size+1));

  // read text from file
  fread (buf,sizeof(char),file_size,f); //buf[file_size]=0;
  //--------------------
  // заполнить массив регулярных слов из файла конфигураций
  for(i=0; i<rw_ids.len; i++)
  { int sy=0, rwresl=0; rwenum rwtype;
    char word_rw[BUFLEN]; myvar var={0}; // var.type = Not;

    if( offset >= file_size-1 ) break; //  !!! типо костыля.

           rwtype = find_next_id (buf,&rw_ids,&offset);
    do {   rwresl = find_next_rw (buf,word_rw,&offset);

      var. vtype = Reg_Word;
      var.rwtype = rwtype;
      var.sytype =      (regwords_get_sytype (rwtype))?(++sy):(0);
      var.  type =       regwords_get_type   (rwtype);
      var.value.rvalue = regwords_get_action (rwtype);

      // throw out the quotes
      if( isQuote (*word_rw) )  
      { word_rw[ strlen (word_rw)-1 ] = 0;
        strcpy (var.name,word_rw+1);
      }
      else
      { ToLowerString (var.name);
        strcpy (var.name,word_rw);
      }

      { myvar *pvar = hash_lookup (RegularWords,var.name,&var,HASH_ADD_ENABLE,0);
        if( rwtype == CCOMM_IDRW ) // !!! что-то типа костыля
         list_pmv_insert ((*_Names_table())->Lex.ccomm_names,-1,&pvar);
        if( rwtype == UFNC_IDRW )
         list_pmv_insert ((*_Names_table())->Flg.FncDefNames,-1,&pvar);
      }
    } while( rwresl );
  } // end for
  //--------------------

//#define _DEBUG_
#ifdef  _DEBUG_ //---DEBUG-----------
  regwords_hash_table_printf (RegularWords);
#endif //---------------------------

END:;
  // free temp.resources
  if(rw_id_buf) { free (rw_id_buf); rw_id_buf = NULL; }
  if(buf ) { free (buf ); buf  = NULL; }
  // close file stream
  if(f) { fclose (f); f = NULL; }
  //--------------------
  return result;
}
//---------------------------------------------------------------------------------------------------------------
char* regwords_get_rwenum_name   (size_t i)
{ static char *strs[] = {
"       NO_IDRW" ,
"     PROG_IDRW" ,
"     VARS_IDRW" ,
"     NEED_IDRW" ,
"      BGN_IDRW" ,
"       IF_IDRW" ,
"    WHILE_IDRW" ,
"      FOR_IDRW" ,
"       IN_IDRW" ,
"      OUT_IDRW" ,
"     SIZE_IDRW" ,
"     TYPE_IDRW" ,
"     CAST_IDRW" ,
"     RAND_IDRW" ,
"     UFNC_IDRW" ,
"      END_IDRW" ,
"     RESL_IDRW" ,
"    EPROG_IDRW" ,
" BND_ARTH_IDRW" ,
" ADD_ARTH_IDRW" ,
" SUB_ARTH_IDRW" ,
" MUL_ARTH_IDRW" ,
" DIV_ARTH_IDRW" ,
" MOD_ARTH_IDRW" ,
"  OR_ARTH_IDRW" ,
" AND_ARTH_IDRW" ,
" NOT_ARTH_IDRW" ,
" XOR_ARTH_IDRW" ,
"   E_ARTH_IDRW" ,
"   L_ARTH_IDRW" ,
"   G_ARTH_IDRW" ,
"  NE_ARTH_IDRW" ,
"  LE_ARTH_IDRW" ,
"  GE_ARTH_IDRW" ,
"  ADD_CNT_IDRW" ,
"  DEL_CNT_IDRW" ,
"  FND_CNT_IDRW" ,
"  CLR_CNT_IDRW" ,
"  SRT_CNT_IDRW" ,
"   OINDEX_IDRW" ,
"   CINDEX_IDRW" ,
" PSTINDEX_IDRW" ,
" PREINDEX_IDRW" ,
"    FIELD_IDRW" ,
"      INT_IDRW" ,
"      STR_IDRW" ,
"      FLT_IDRW" ,
"      FNC_IDRW" ,
"      CNT_IDRW" ,
"      JNT_IDRW" ,
"      CNS_IDRW" ,
"      ARR_IDRW" ,
"      LST_IDRW" ,
"     THEN_IDRW" ,
"     ELIF_IDRW" ,
"     ELSE_IDRW" ,
"     EACH_IDRW" ,
"     ELEM_IDRW" ,
"     FROM_IDRW" ,
"     UNTL_IDRW" ,
"     BYST_IDRW" ,
"     RVRS_IDRW" ,
"       DO_IDRW" ,
"    BREAK_IDRW" ,
"    INTIN_IDRW" ,
"    STRIN_IDRW" ,
"    FLTIN_IDRW" ,
"    COMMA_IDRW" ,
"    SEMIC_IDRW" ,
"    LCOMM_IDRW" ,
"    OCOMM_IDRW" ,
"    CCOMM_IDRW" };
  return strs[i];  
}
int   regwords_hash_table_printf (hash_table *RegularWords)
{ myvar*    cur  = NULL;
  size_t    i    = 0U;
  hash_iter iter = {0};
  
  while( cur = hash_next (RegularWords,&iter) )
   printf ( "%3d  %s  %- 28s  %d  %d\n",
            i++,
            regwords_get_rwenum_name ((unsigned int)cur->rwtype),
            cur->name,
            cur->type,
            cur->sytype
          );

  return 0;
}
//---------------------------------------------------------------------------------------------------------------
 
 
 
 
 