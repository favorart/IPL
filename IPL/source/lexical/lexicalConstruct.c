#include "header.h"
#include "lexicalCheck.h"
#include "lexicalConstruct.h"
#include "nmList.h"

//-------------------------------------------------------------------------------
int     spc_num_nword (char *buf)
{ unsigned int n; 
  for(n=0; *buf; buf++)
   if(*buf == SYMB_SPACE) n++;
  return n;
}
int     compose_nword (char *buf, list_nm *nwrd)
{ unsigned int i;
  strcpy (buf,list_nm_exact (nwrd,0)->name);
  for(i=1; i<nwrd->len; i++)
  { strcat (buf,STR_SYMB_SPACE);
    strcat (buf,list_nm_exact (nwrd,i)->name );
  }
  return 0;
}
int     extract_nword (char *buf, list_nm *nwrd)
{ 
  size_t i, n = spc_num_nword (buf) + 1U;
  for(i=0; i<n; i++)
    list_nm_extracts (nwrd, 0);
  return 0;
}
int     restore_nword (char *buf, unsigned int n)
{ unsigned int i;
  for(i=0; i<n; i++) buf[ strlen(buf) ] = SYMB_SPACE;
  return 0;
}
myvar*  lookfor_nword (char *buf)
{ myvar *rsl, *pvarr = NULL, *pvarv = NULL; char *s;

  while( !pvarr && !pvarv )
  { pvarr = isRegWord    (buf);
    pvarv = isIdentifier (buf);

    s = strrchr (buf,SYMB_SPACE); 
    if( !s ) break; else *s=0; 
  }

  if( pvarr && pvarv )
   rsl = (strlen (pvarv->name) > strlen (pvarr->name)) ? (pvarv) : (pvarr);
  else
   rsl = (pvarr) ? (pvarr) : (pvarv);

  return rsl;
}
//-------------------------------------------------------------------------------
wenum   laReWoComm_Wait (myvar* pvar, wenum wait)
{ rwenum rw = ( pvar && (pvar->rwtype == OCOMM_IDRW || 
                         pvar->rwtype == CCOMM_IDRW || 
                         pvar->rwtype == LCOMM_IDRW) ) ? (pvar->rwtype) : (0);
  if( !rw ) return wait;

       if( (wait == WAIT_NO   ) && (rw == OCOMM_IDRW) ) wait = WAIT_CCOMM;
  else if( (wait == WAIT_NO   ) && (rw == LCOMM_IDRW) ) wait = WAIT_NLINE;
  else if( (wait == WAIT_CCOMM) && (rw == CCOMM_IDRW) ) wait = WAIT_NO;
  else SetCorrectError (pvar->name,ERR_SYNX);
  return wait; 
}
//-------------------------------------------------------------------------------
int     laGrbgToVarName (names_table *Names_table, list_pmv *List_atoms)
{ list_nm *grbg = Names_table->Lex.nwords_garbage;
  if( grbg->len )
  { myvar *pvr, var = {0}; // = Unknow; var.vtype = Variable;
    compose_nword (var.name,grbg);
    extract_nword (var.name,grbg);
    pvr = constants_lookup (var.name,&var,HASH_ADD_ENABLE,Not);
    list_pmv_insert (List_atoms,-1,&pvr);
    return 1;
  }
  return 0;
}
//-------------------------------------------------------------------------------
char*   laLexIncWordBuf (lexical_data *Lex, unsigned int i)
{ if( i == Lex->word_len )
   if( !(  Lex->word_buf = (char*)realloc( Lex->word_buf,sizeof(char)*(Lex->word_len*=2) ) ))
    SetError (ERR_MEM);
  return Lex->word_buf;
}
//-------------------------------------------------------------------------------
