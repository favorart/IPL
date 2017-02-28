#include "Header.h"
#include "Interpreter.h"
//--------------------
#include "InterpreterSyntax.h"
#include "InterpreterPreCompile.h"
//--------------------
#include "plnList.h"
#include "mvList.h"

//#define _IN_OUTPUT_
//#define _DEBUG_
#ifdef  _DEBUG_ //---DEBUG--------------------------
FILE** _log_ (void);
int   myvar_print (myvar *pvar);
int   hash_fwrite (hash_table *Hash, FILE* f_out);
#endif  //------------------------------------------

//-------------------------------------------------------------------------------
/* FL_OFFSET */ size_t  hash        (hash_table* Hash, char *name); 
/* FL_NONODE */ size_t  hash_index  (hash_table* Hash, char *name, int flag_var)
{ size_t  i, h;
  hash_node* node = NULL;

  if ( !name )
  { SetError (ERR_PTR);
    return -1;
  }

 // if( !strcmp ("None",name) )\
  { i= 1; }
  h = hash (Hash,name);
  for(i=0, node = Hash->content[h]; node; node = node->next, i++)
  { if( !strcmp (name,myvar_name (&node->value)) &&
       (!flag_var || (node->value.type == flag_var)) )
     return i;
  }
  return -1;
}
                myvar*  hash_rindex (hash_table* Hash, int offset, int   nonode)
{
  size_t i;
  hash_node* node = Hash->content[offset];

  //{ myvar *cur; hash_iter iter = {0};
  //  while( cur = hash_next (Hash,&iter) )
  //   printf ("%u,%s\n",iter.i,myvar_name (cur));
  //}
  if ( offset == 639 )
    offset = 639;
  for ( i = 0U; i < nonode; i++ )
  {
    if ( !node )
      return NULL;
    node = node->next;
  }
  return &node->value;
}
//-------------------------------------------------------------------------------
int    hash_fwrite  (hash_table *Hash, FILE* f_out)
{ myvar* cur; hash_iter iter = {0};

  fwrite (&Hash->size,sizeof(int),1,f_out);   // << HashSize
  fwrite (&Hash->len ,sizeof(int),1,f_out);   // << HashLen
  while( cur = hash_next (Hash,&iter) )
   myvar_fwrite (cur,f_out);                  // << myvar
  return 0;
}
int    hash_fread   (hash_table *Hash, FILE* f_inc)
{ unsigned int i, len; myvar var = {0}; tenum flag_var;

  if( fread (&len,sizeof(int),1,f_inc) != 1 )                             // >> HashSize
  { SetError (ERR_END); return 1; }
  if( Hash->size != len )
  { hash_free (Hash);  Hash->size = len;  Hash->len = 0;
    Hash->content = (hash_node**)malloc( sizeof(hash_node*)*Hash->size );
    if( !Hash->content ) { SetError (ERR_MEM); return 1; }    
  }
  if( fread (&len,sizeof(int),1,f_inc) != 1 )                             // >> HashLen
  { SetError (ERR_END); return 1; }
  for(i=0; i<len; i++)
  { myvar_fread (&var,f_inc);                                             // >> myvar
    flag_var  = ( var.vtype == Variable ) ? ( (tenum)0 ) :( var.type );
    //if( !strcmp ("None",var.name) )
    //{ flag_var  = ( var.vtype == Variable ) ? ( (tenum)0 ) :( var.type ); }
    hash_lookup (Hash,myvar_name (&var),&var,HASH_ADD_ENABLE,flag_var);   // var >> Hash

    //if( !hash_lookup (Hash,myvar_name (&var),NULL,HASH_ADD_DISABLE,flag_var) )
    //{ hash_node *nnode, *node = (hash_node*)malloc( sizeof(hash_node) );
    //  if( !node ) { SetError(ERR_MEM); return 1; }
    //  node->next  = NULL;
    //  node->value =  var;

    //  i = hash (Hash,var.name);
    //  nnode = Hash->content[i];
    //          Hash->len++;
    //  // добавляется в конец списка
    //  if( !nnode ) Hash->content[i] = node;
    //  else 
    //  { while( nnode->next ) nnode = nnode->next; nnode->next = node; }
    //}
  }
  return 0;
}
//-------------------------------------------------------------------------------
void   myvar_fwrite (myvar *var, FILE* f_out)
{ fwrite (var,sizeof(myvar),1,f_out);                                     // << myvar

  if( var->type == Str )
  { size_t len = strlen (var->value.svalue) + 1;
    fwrite (&len, sizeof (len), 1, f_out);                                // << length
    fwrite (var->value.svalue,len*sizeof (char),1,f_out);                 // << string
  }
  else if( var->type == Fnc )
  {
    procedure_fwrite (var->value.pvalue,(*_Names_table()),f_out);         // << function
  }
  else if( var->type == Cnt )
  {
    size_t i;
    fwrite ( var->value.cvalue->len ,sizeof (size_t),1,f_out);            // << cnt_length
    fwrite (&var->value.cvalue->type,sizeof (size_t),1,f_out);            // << cnt_type

    for(i=0; i<(*var->value.cvalue->len); i++)                             
     myvar_fwrite (*container_exact (var->value.cvalue,i),f_out);         // << cnt_content
  }
  else if( var->type == Jnt )
  { joint_node *cur;  joint *Joint = var->value.jvalue;
    joint_iter iter = {0};

    fwrite (&Joint->len , sizeof (size_t), 1, f_out);                     // << jnt_length
    fwrite (&Joint->size, sizeof (size_t), 1, f_out);                     // << jnt_size

    while( cur = joint_next (Joint,&iter) )                               // << jnt_content
    { myvar_fwrite (cur->key, f_out);
      myvar_fwrite (cur->val, f_out);
    } // end for
  } // end else if
}
void   myvar_fread  (myvar *var, FILE* f_inc)
{ if( fread (var, sizeof (myvar), 1, f_inc) != 1 )                        // >> myvar
  { SetError (ERR_END); return; }

  if( var->type == Str )
  { unsigned int len;

    if( fread (&len,sizeof(int),1,f_inc) != 1 )                            // >> length
    { SetError (ERR_END); return; }
    if( !(var->value.svalue = (char*)malloc( sizeof(char)*len )) )
    { SetError (ERR_MEM); return; }
    if( fread (var->value.svalue,sizeof(char),len,f_inc) != len  )         // >> string
    { SetError (ERR_END); return; }
  }
  else if( var->type == Fnc )
  { var->value.pvalue = procedure_create ();
    procedure_fread (var->value.pvalue,(*_Names_table()),f_inc);           // >> function
  }
  else if( var->type == Cnt )
  { size_t i, len, type; 

    if( !(var->value.cvalue = (container*)calloc( 1,sizeof(container) )) )
    { SetError (ERR_MEM); return; }
    if( fread (&len ,sizeof(unsigned int),1,f_inc) != 1 )                  // >> cnt_length
    { SetError (ERR_END); return; }
    if( fread (&type,sizeof(unsigned int),1,f_inc) != 1 )                  // >> cnt_type
    { SetError (ERR_END); return; }

    container_create (var->value.cvalue, (conts_enum) type, 0);
    for(i=0U; i<len; i++)                             
    { myvar mv = {0}; myvar *pvar;                                         // >> cnt_content
      myvar_fread (&mv,f_inc); pvar = constants_lookup (myvar_name (&mv),&mv,HASH_ADD_ENABLE,mv.type);
      container_insert (var->value.cvalue,i,pvar);
    }
  }
  else if( var->type == Jnt )
  { unsigned int i, len, size;
  
    if( !(var->value.jvalue = (joint*)calloc( 1,sizeof(joint) )) )
    { SetError (ERR_MEM); return; }
    if( fread (&len ,sizeof(unsigned int),1,f_inc) != 1 )                  // >> jnt_length
    { SetError (ERR_END); return; }
    if( fread (&size,sizeof(unsigned int),1,f_inc) != 1 )                  // >> jnt_size
    { SetError (ERR_END); return; }

    joint_create (var->value.jvalue,size);
    for(i=0; i<len; i++)
    { myvar mv = {0}, *key, *val;                                          // >> jnt_content
      myvar_fread (&mv,f_inc); key = constants_lookup (myvar_name (&mv),&mv,HASH_ADD_ENABLE,mv.type);
      myvar_fread (&mv,f_inc); val = constants_lookup (myvar_name (&mv),&mv,HASH_ADD_ENABLE,mv.type);
      joint_insert (var->value.jvalue,key,val);
    }
  }
}
//-------------------------------------------------------------------------------
int    prepareIPCname    (names_table *Names_table, char **filename)
{ int result = 0;
  size_t filename_len;
 
  //---prepare filename----------
  filename_len = strlen (Names_table->pUsrConf->script) +  1; // +'\0' + ".ipc" - ".txt"

  if( !(*filename = (char*)malloc( sizeof(char)*filename_len )) )
  { SetError (ERR_MEM); result = 1; goto END; }

  strcpy (*filename,Names_table->pUsrConf->script);
  strcpy (*filename + (strlen(*filename)-3),"ipc"); // interpreter pre-compiled code
  //------------------------------
END:
  return result;
}
int    prepareDATname    (names_table *Names_table, char **filename, char* name_end, vhtenum vht)
{ int result = 0; char *script = Names_table->pUsrConf->script;
  //------------------------------
  if( !(*filename = (char*)malloc( sizeof(char)*BUFLEN )) )
  { SetError (ERR_MEM); result = 1; goto END; }

  strcpy (*filename,script);
  if( vht == HT_REWO )
  { script = Names_table->pUsrConf->config;
    strcpy (*filename + (strlen (*filename) - FILE_EXT_LEN + 1),script);
    strcat (*filename,name_end);
  }
  else strcpy (*filename + (strlen (*filename) - FILE_EXT_LEN),name_end);
  //------------------------------
END:
  return result;
}
//-------------------------------------------------------------------------------
int    prepareIPCfile    (names_table *Names_table, char* f_rw_mode, FILE **f)
{ int result = 0;  
  char *filename = NULL;

  if( prepareIPCname (Names_table,&filename) ) { result = 1; goto END; }
  //------------------------------
  if( !(*f = fopen (filename,f_rw_mode)) )
  { SetCorrectError (filename,ERR_FILE); result = 1; goto END; }
  //------------------------------
END:
  if(filename) { free (filename); filename = NULL; }

  return result;
}
//-------------------------------------------------------------------------------
int    hash_table_fwrite (names_table *Names_table, vhtenum vht)
{ int result = 0;
  FILE *f_out    = NULL;
  char *filename = NULL;

  char *vfname; hash_table *vhtable;
  //------------------------------
  switch (vht)
  { default      : SetError (ERR_PAR); result = 1; goto END;
    case HT_VARS : vfname = FILENAME_VARS; vhtable = Names_table->pBsValVars; break;
    case HT_CONS : vfname = FILENAME_CONS; vhtable = Names_table->pConstants; break;
    case HT_REWO : vfname = FILENAME_REWO; vhtable = Names_table->pReg_Words; break;  
  }
  //------------------------------
  prepareDATname (Names_table,&filename,vfname,vht);

  if( (vht == HT_REWO) && !checkHashConfFiles (Names_table) ) goto END;

  if( !(f_out = fopen (filename,"wb")) )
  { result = 1; goto END; }

  if(  vht == HT_REWO )    writeHashConfigFile (Names_table,f_out);

  if( hash_fwrite (vhtable,f_out) )
  { result = 1; goto END; }
  //------------------------------
END:
  if(f_out)    { fclose  (f_out); f_out    = NULL; }
  if(filename) { free (filename); filename = NULL; }

  return result;
}
int    hash_table_fread  (names_table *Names_table, vhtenum vht)
{ int result = 0;
  FILE *f_inc    = NULL;
  char *filename = NULL;

  char *vfname; hash_table *vhtable;
  //------------------------------
  switch (vht)
  { default      : SetError (ERR_PAR); result = 1; goto END;
    case HT_VARS : vfname = FILENAME_VARS; vhtable = Names_table->pBsValVars; break;
    case HT_CONS : vfname = FILENAME_CONS; vhtable = Names_table->pConstants; break;
    case HT_REWO : vfname = FILENAME_REWO; vhtable = Names_table->pReg_Words; break;  
  }
  //------------------------------
  prepareDATname (Names_table,&filename,vfname,vht);
  //------------------------------
  if( !(f_inc = fopen ( filename,"rb" )) )
  { result = 1; goto END; }

  if( (vht == HT_REWO) && checkHashConfigFile (Names_table,f_inc) )
  { result = 1; goto END; }

  if( hash_fread (vhtable,f_inc) )
  { result = 1; goto END; }

  if( vht == HT_VARS )
   hash_copy (Names_table->pBsValVars,Names_table->pVariables);
  //------------------------------
END:
  if(f_inc)    { fclose  (f_inc); f_inc    = NULL; }
  if(filename) { free (filename); filename = NULL; }

  return result;
}
//-------------------------------------------------------------------------------
int    bytecode_fwrite   (names_table *Names_table, list_pmv *Sentence, FILE* f_out)
{ size_t   i = 0;
  int result = 0;

  size_t  fileline[3] = { 0 };
  list_pln    PCLists = { 0 };

  list_pmv_node* cur_node;
  list_pln_insert (&PCLists,-1,&Sentence->first);

  { fileline[FL_VTYPE ] = Pre_Comp;
    fileline[FL_OFFSET] = 0U;
    fileline[FL_NONODE] = Sentence->len;

#ifdef _IN_OUTPUT_ //----------------------------------------
    printf ("%3d | %3d %5d %5d\n",i++,fileline[FL_VTYPE ],
                  fileline[FL_OFFSET],fileline[FL_NONODE]);
    fwrite ( fileline,sizeof(fileline),1,f_out );
#endif //----------------------------------------------------
  }
  //---write content----------------
  while( PCLists.len )
  { cur_node = list_pln_extracts (&PCLists,-1);

    while( cur_node )
    { myvar      *cur = cur_node->value;
      hash_table *HashCur = NULL;

      if( cur )
      { switch ( cur->vtype )
        { case Reg_Word: HashCur = Names_table->pReg_Words; break;
          case Variable: HashCur = Names_table->pVariables; break;
          case Constant: HashCur = Names_table->pConstants; break;
          case Pre_Comp:                                    break;
          default:    SetCorrectError (cur->name,ERR_NAME); break;
        }

        if( cur->vtype == Pre_Comp )
        { fileline[FL_VTYPE ] = Pre_Comp;
          fileline[FL_OFFSET] = 0;
          fileline[FL_NONODE] = cur->value.lvalue->len;
#ifdef _IN_OUTPUT_ //----------------------------------------
          printf ("%3d | %3d %5d %5d\n",i++,fileline[FL_VTYPE ],
                         fileline[FL_OFFSET],fileline[FL_NONODE]);
#endif //----------------------------------------------------
          list_pln_insert (&PCLists,-1,&cur_node->next);
          cur_node = cur->value.lvalue->first;
               cur = cur_node->value;
        }
        else
        { char *name = myvar_name (cur);

          fileline[FL_VTYPE ] = cur->vtype;
          fileline[FL_OFFSET] = hash       (HashCur,name);
          fileline[FL_NONODE] = hash_index (HashCur,name,cur->type);
#ifdef _IN_OUTPUT_ //----------------------------------------
          printf ("%3d | %3d %5d %5d %s\n",i++,fileline[FL_VTYPE ],
                            fileline[FL_OFFSET],fileline[FL_NONODE],
                            myvar_name (cur_node->value));
#endif //----------------------------------------------------
          cur_node = cur_node->next;
        }
      }
      else 
      { fileline[FL_VTYPE ] = 0;
        fileline[FL_OFFSET] = 0;
        fileline[FL_NONODE] = 0;

#ifdef _IN_OUTPUT_ //----------------------------------------
          printf ("%3d | %3d %5d %5d\n",i++,fileline[FL_VTYPE ],
                         fileline[FL_OFFSET],fileline[FL_NONODE]);
#endif //----------------------------------------------------
        cur_node = cur_node->next;
      }
      fwrite ( fileline, sizeof(fileline), 1, f_out );
    } // end while
  } // end while
#ifdef _IN_OUTPUT_ //----------------------------------------
  printf ("\n");
#endif //----------------------------------------------------
  return result;
}
int    bytecode_fread    (names_table *Names_table, list_pmv *Sentence, FILE* f_inc)
{ int result = 0; unsigned int i=0, must_len = 0;
  int  fileline[3] = {0};
  myvar *pvar;

  list_mv PCLists   = {0};
  myvar CurSentence = {0};
  CurSentence.vtype = Pre_Comp;
  CurSentence.value.lvalue = Sentence;

  if( fread ( &fileline,sizeof(fileline),1,f_inc ) != 1 )
  { SetError (ERR_END); result = 1; goto END; }

  if( fileline[FL_VTYPE ] != Pre_Comp )
   SetError (ERR_TERM);
  if( !(must_len = fileline[FL_NONODE]) )
   goto END;

  //---read content-----------------
  do
  { do
    { if( fread ( &fileline,sizeof(fileline),1,f_inc ) != 1 )
      { SetError (ERR_END); result = 1; goto END; }
     
      switch (fileline[FL_VTYPE])
      { case Pre_Comp: pvar = saNewSentence ();                                                              break;
        case Variable: pvar = hash_rindex (Names_table->pVariables,fileline[FL_OFFSET],fileline[FL_NONODE]); break;
        case Constant: pvar = hash_rindex (Names_table->pConstants,fileline[FL_OFFSET],fileline[FL_NONODE]); break;
        case Reg_Word: pvar = hash_rindex (Names_table->pReg_Words,fileline[FL_OFFSET],fileline[FL_NONODE]); break;
        case Unknow  : pvar = NULL;                                                                          break;
        default      : SetError (ERR_NAME);                                                                  break;
      }
      list_pmv_insert (CurSentence.value.lvalue,-1,&pvar);
#ifdef _IN_OUTPUT_ //----------------------------------------
      printf ("%3d | %3d %5d %5d %s\n",i++,fileline[FL_VTYPE ],
                       fileline[FL_OFFSET],fileline[FL_NONODE],
                       (pvar)?myvar_name (pvar):NULL);
#endif //----------------------------------------------------
      if( fileline[FL_VTYPE ] == Pre_Comp )
      { CurSentence.type = (tenum)(must_len);
        list_mv_insert (&PCLists,-1,&CurSentence);

        CurSentence = *pvar;
        must_len = fileline[FL_NONODE];
      } 
    } while( CurSentence.value.lvalue->len < must_len ); // end while

     while( CurSentence.value.lvalue->len >= must_len )
     { if( !PCLists.len ) break;
       CurSentence = list_mv_extracts (&PCLists,-1);
       must_len = (int)CurSentence.type; CurSentence.type = (tenum)(0);
     }
  } while( (PCLists.len || (CurSentence.value.lvalue->len < must_len)) );

  if( !Sentence->len ) SetError (ERR_TERM);
#ifdef _IN_OUTPUT_ //----------------------------------------
  printf ("\n");
#endif //----------------------------------------------------
END:
  return result;
}
//-------------------------------------------------------------------------------
int    writeHashScriptFile (char *filename, FILE *f_out)
{ unsigned char newSHA256Line[SHA256LEN];
  checkSHA256file (filename,NULL,newSHA256Line);
  fwrite ( newSHA256Line,sizeof(char),SHA256LEN,f_out );
  return 0;
}
int    checkHashScriptFile (char *filename, FILE *f_inc)
{ int result = 0;
  unsigned char oldSHA256Line[SHA256LEN];
  unsigned char newSHA256Line[SHA256LEN];
  //---------------------------------------------------
  if( fread ( oldSHA256Line,sizeof (char),SHA256LEN,f_inc ) < SHA256LEN )
  { result = 1; goto END; }
  if( !checkSHA256file (filename,oldSHA256Line,newSHA256Line) )
  { result = 1; goto END; }
  //---------------------------------------------------
END:;
  return result;
}

int    checkHashConfigFile (names_table *Names_table, FILE *f_inc)
{ int result = 0;
  char cnf_filename[BUFLEN];
  unsigned char oldSHA256Line[SHA256LEN];
  unsigned char newSHA256Line[SHA256LEN];  
  //---------------------------------------------------
  strcpy (cnf_filename, Names_table->pUsrConf->config);
  strcat (cnf_filename,ALPH_CONF_EXT);

  if( fread (oldSHA256Line,sizeof(char),SHA256LEN,f_inc)<SHA256LEN ) { result = 1; goto END; }
  if( ! checkSHA256file (cnf_filename,oldSHA256Line,newSHA256Line) ) { result = 1; goto END; }
  //---------------------------------------------------
  strcpy (cnf_filename,Names_table->pUsrConf->config);
  strcat (cnf_filename,REWO_CONF_EXT);

  if( fread (oldSHA256Line,sizeof(char),SHA256LEN,f_inc)<SHA256LEN ) { result = 1; goto END; }
  if( ! checkSHA256file (cnf_filename,oldSHA256Line,newSHA256Line) ) { result = 1; goto END; }
  //---------------------------------------------------
END:
  return result;
}
int    writeHashConfigFile (names_table *Names_table, FILE *f_out)
{ unsigned char newSHA256Line[SHA256LEN];
  char cnf_filename[BUFLEN];
  //---------------------------------------------------
  strcpy (cnf_filename, Names_table->pUsrConf->config);
  strcat (cnf_filename,ALPH_CONF_EXT);

  checkSHA256file   (cnf_filename,NULL,newSHA256Line);
  fwrite (newSHA256Line,sizeof(char),SHA256LEN,f_out);
  //---------------------------------------------------
  strcpy (cnf_filename,Names_table->pUsrConf->config);
  strcat (cnf_filename,REWO_CONF_EXT);

  checkSHA256file   (cnf_filename,NULL,newSHA256Line);
  fwrite (newSHA256Line,sizeof(char),SHA256LEN,f_out);
  //---------------------------------------------------
  return 0;
}

// this function used in constructor-function of names_table
int    checkHashConfFiles  (names_table *Names_table)
{ int result = 0;
  FILE *f_inc = NULL;
  char *fname = NULL;
  //------------------------------
  prepareDATname (Names_table,&fname,FILENAME_REWO,HT_REWO);
  if( !(f_inc = fopen (fname,"rb")) )
  { result = 1; goto END; }
  if( checkHashConfigFile (Names_table,f_inc) )
  { result = 1; goto END; }
  //------------------------------
END:;
  if(result)  remove (fname);
  if(fname) { free   (fname); fname = NULL; }
  if(f_inc) { fclose (f_inc); f_inc = NULL; }  

  return result;
}
//-------------------------------------------------------------------------------
int     program_fwrite  (names_table *Names_table, list_pmv *Sentence)
{ int  result = 0;
  FILE *f_out = NULL;

  if( !Sentence || !Sentence->len )
  { SetError (ERR_TERM); result = 1; goto END; }

  if( hash_table_fwrite (Names_table,HT_REWO) ) { result = 1; goto END; } //---reg_words.dat
  if( hash_table_fwrite (Names_table,HT_VARS) ) { result = 1; goto END; } //---variables.dat
  if( hash_table_fwrite (Names_table,HT_CONS) ) { result = 1; goto END; } //---constants.dat
  //------------------------------
  if( prepareIPCfile (Names_table,"wb",&f_out) )
  { result = 1; goto END; }

  writeHashScriptFile (Names_table->pUsrConf->script,f_out); //---write file-hash
  if( bytecode_fwrite (Names_table,Sentence,f_out) )
  { result = 1; goto END; }

  fwrite (&Names_table->Flg.cEnumerator,sizeof(unsigned int),1,f_out);
  fwrite (&Names_table->Flg.jEnumerator,sizeof(unsigned int),1,f_out);
  //--------------------------------
END: 
  if(f_out) { fclose (f_out); f_out = NULL; }

#ifdef  _DEBUG_
  { myvar *cur; hash_iter iter = {0};
    hash_table *Hash = Names_table->pBsValVars;
    printf ("\nBaseVariables\n");
    
    while( cur = hash_next (Hash,&iter) )
     printf ("%s\n",myvar_name (cur));

    cur = Names_table->pUserFuncs->first->value;
    printf ("\n%s\n",myvar_name (cur));
    Hash = Names_table->pUserFuncs->first->value->value.pvalue->BsValVars;
    while( cur = hash_next (Hash,&iter) )
     printf ("%s\n",myvar_name (cur));

    cur = Names_table->pUserFuncs->first->next->value;
    printf ("\n%s\n",myvar_name (cur));
    Hash = Names_table->pUserFuncs->first->next->value->value.pvalue->BsValVars;
    while( cur = hash_next (Hash,&iter) )
     printf ("%s\n",myvar_name (cur));

    cur = Names_table->pUserFuncs->first->next->next->value;
    printf ("\n%s\n",myvar_name (cur));
    Hash = Names_table->pUserFuncs->first->next->next->value->value.pvalue->BsValVars;
    while( cur = hash_next (Hash,&iter) )
     printf ("%s\n",myvar_name (cur));
  }
#endif
  return result;
}
int     program_fread   (names_table *Names_table, list_pmv *Sentence)
{ int   result = 0;
  FILE *f_inc  = NULL;
  char *filename = Names_table->pUsrConf->script;

  if( !Sentence ) { SetError (ERR_PTR); result = 1; goto END; }
  if(  Sentence->len ) list_pmv_clear (Sentence);
  //------------------------------
      hash_table_fread  (Names_table,HT_REWO);                               //---reg_words.dat
  if( hash_table_fread  (Names_table,HT_CONS)    ) { result = 1; goto END; } //---constants.dat
  if( hash_table_fread  (Names_table,HT_VARS)    ) { result = 1; goto END; } //---variables.dat  
  //------------------------------
  if( prepareIPCfile    (Names_table,"rb",&f_inc)) { result = 1; goto END; }
  if( checkHashScriptFile (filename,f_inc))        { result = 1; goto END; } //---chk file-hash
  if( bytecode_fread (Names_table,Sentence,f_inc)) { result = 1; goto END; }
  //------------------------------
  if( (fread (&Names_table->Flg.cEnumerator,sizeof(unsigned int),1,f_inc) != 1) &&
      (fread (&Names_table->Flg.jEnumerator,sizeof(unsigned int),1,f_inc) != 1) )
  { result = 1; goto END; }
  //------------------------------
END: 
  if(f_inc) { fclose (f_inc); f_inc = NULL; }
  if(result)
  { hash_clear     (Names_table->pVariables); 
    hash_clear     (Names_table->pConstants); 
    hash_clear     (Names_table->pBsValVars); 
    list_pmv_clear (Sentence);
  }
  return result;
}
//-------------------------------------------------------------------------------
void   fprintf_list (FILE* f_out, list_pmv *Sentence, names_table *Names_table)
{ list_pmv_node* cur_node;

  fprintf ( f_out, "[ ");
  for(cur_node = Sentence->first; cur_node; cur_node = cur_node->next)
  { myvar *cur = cur_node->value;
    
    if( cur->vtype != Pre_Comp )
    { char *name =  (cur->vtype == Constant && cur->type == Str) ? (cur->value.svalue) : (cur->name);
      fprintf (f_out,"%u %u %u \"%s\" ",strlen(name)+1,cur->vtype,cur->type,name);
    }
    else fprintf_list ( f_out,cur->value.lvalue,Names_table );
  } // end for
  fprintf ( f_out, "] ");
}
void    fscanf_list (FILE* f_inc, list_pmv *Sentence, names_table *Names_table)
{ myvar *cur;

  char  *buf;
  size_t buf_len = STRLEN;
  if( !(buf = (char*)malloc( sizeof(char)*buf_len ) ))
    SetError (ERR_MEM);

  *buf = 0;
  while( *buf != ']' )
  { if( fscanf (f_inc,"%c",buf)!=1 ) SetError (ERR_TERM);
   
    if( *buf == '[' )
    { if( fscanf (f_inc,"%c",buf)!=1 ) // пропускаем " "
       SetError (ERR_TERM);

      cur = saNewSentence();
      fscanf_list ( f_inc,cur->value.lvalue,Names_table );

      if( fscanf (f_inc,"%c",buf)!=1 ) // пропускаем " "
       SetError (ERR_TERM);
    }
    else
    { unsigned int len, vtype, type;
      if( fscanf (f_inc,"%u ",&len)!=1 ) // считываем остальную часть строки
       SetError (ERR_TERM);
      if( len > buf_len )
      { free (buf); buf_len = len*2;
        if( !(buf = (char*)malloc( sizeof(char)*buf_len ) ))
         SetError (ERR_MEM);
      }

      fscanf (f_inc,"%u %u \"%s\" ",&vtype,&type,buf);
      if( vtype == Constant )
       cur = constants_lookup (buf,NULL,HASH_ADD_DISABLE,type );
      else if( vtype == Variable )
       cur = variables_lookup (buf,NULL,HASH_ADD_DISABLE );
      else if( vtype == Reg_Word )
       cur = hash_lookup ( Names_table->pReg_Words,buf,NULL,HASH_ADD_DISABLE,0 );
      else SetError (ERR_PAR);
    }

    list_pmv_insert ( Sentence,-1,&cur );
  } // end while
}
//-------------------------------------------------------------------------------
void   ipc_files_remove (names_table *Names_table)
{ char *filename_s = NULL;
  char *filename_c = NULL;
  char *filename_v = NULL;
  //---prepare filename----------
  prepareIPCname (Names_table,&filename_s);
  prepareDATname (Names_table,&filename_c,FILENAME_CONS,HT_CONS);
  prepareDATname (Names_table,&filename_v,FILENAME_VARS,HT_VARS); 
  //------------------------------
  remove (filename_s);
  remove (filename_c);
  remove (filename_v);
  //------------------------------
  if(filename_s) { free(filename_s); filename_s = NULL; }
  if(filename_c) { free(filename_c); filename_c = NULL; }
  if(filename_v) { free(filename_v); filename_v = NULL; }
}
//-------------------------------------------------------------------------------
int  check (names_table *Names_table)
{ int   result = 0; FILE *f_inc  = NULL;
  char* filename = Names_table->pUsrConf->script;
            if( prepareIPCfile (Names_table,"rb",&f_inc) ) result = 1;
  if( !result ) if( checkHashScriptFile (filename,f_inc) ) result = 1;
  if( !result ) if( checkHashConfFiles     (Names_table) ) result = 1;

  if( f_inc ) { fclose (f_inc); f_inc = NULL; }
  return result;
}
//-------------------------------------------------------------------------------