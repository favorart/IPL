#include "header.h"
#include "interpreter.h"
#include "interpreterExecute.h"
#include "mvList.h"

//-----------------------------------------------------------------------------------------
void exSentenceConvert (exSentence *PreCompSentence,list_pmv *ExecSentence,names_table *Names_table)
{ PreCompSentence->Names_table   = Names_table;
  PreCompSentence->exec_sentence = ExecSentence;
  PreCompSentence->len = &PreCompSentence->exec_sentence->len;

  PreCompSentence->make_sentence = (list_pmv*)calloc( 1,sizeof(list_pmv) );
  if( !PreCompSentence->make_sentence ) SetError (ERR_MEM);
}
void exSentenceCollect (exSentence *PreCompSentence,list_pmv *CopySentence,names_table *Names_table)
{ if( PreCompSentence->make_sentence->len )
  { myvar var = {0}, *pvar = list_mv_insert (Names_table->pSentences,-1,&var);

    pvar->vtype        = Pre_Comp;
    pvar->value.lvalue = PreCompSentence->make_sentence;

    list_pmv_insert (CopySentence,-1,&pvar);
    PreCompSentence->make_sentence = NULL;
  }
  else 
  { free (PreCompSentence->make_sentence);
    PreCompSentence->make_sentence = NULL;
  }
}
//-----------------------------------------------------------------------------------------
myvar*  exSentenceExact      (exSentence *PreCompSentence, int order)
{ if( PreCompSentence->exec_sentence->len )
  {      if( order == ORDER_CORRECT )
     return PreCompSentence->exec_sentence->first->value;
    else if( order == ORDER_REVERSE )
     return PreCompSentence->exec_sentence->last ->value;
  }
  return NULL;
}
myvar*  exSentenceExtract    (exSentence *PreCompSentence, int order)
{ list_pmv_node *cur_node = NULL;

       if( order == ORDER_CORRECT )
  { cur_node = PreCompSentence->exec_sentence->first;

    PreCompSentence->exec_sentence->len--;
    PreCompSentence->exec_sentence->first        = PreCompSentence->exec_sentence->first->next;
    if( PreCompSentence->exec_sentence->len )
     PreCompSentence->exec_sentence->first->prev = NULL;
    else
     PreCompSentence->exec_sentence->last        = NULL;
    //------------------
    cur_node->prev = PreCompSentence->make_sentence->last;
    cur_node->next = NULL;

    if( PreCompSentence->make_sentence->len )
     PreCompSentence->make_sentence->last->next = cur_node;
    else
     PreCompSentence->make_sentence->first      = cur_node;
    PreCompSentence->make_sentence->last        = cur_node;
    PreCompSentence->make_sentence->len++;
    //------------------
  }
  else if( order == ORDER_REVERSE )
  { cur_node = PreCompSentence->exec_sentence->last;

    PreCompSentence->exec_sentence->len--;
    PreCompSentence->exec_sentence->last = PreCompSentence->exec_sentence->last->prev;
    if( PreCompSentence->make_sentence->len )
     PreCompSentence->exec_sentence->last->next = NULL;
    else
     PreCompSentence->make_sentence->first      = NULL;
    //------------------
    cur_node->next = PreCompSentence->make_sentence->first;
    cur_node->prev = NULL;

    if( PreCompSentence->make_sentence->len )
     PreCompSentence->make_sentence->first->prev = cur_node;
    else
     PreCompSentence->make_sentence->last        = cur_node;
    PreCompSentence->make_sentence->first        = cur_node;
    PreCompSentence->make_sentence->len++;
    //------------------
  }
  else SetError (ERR_PTR);
  return cur_node->value;
}
myvar*  exSentenceInsertList (exSentence *PreCompSentence, list_pmv *List)
{ myvar* cur = exSentenceExtract ( PreCompSentence,ORDER_CORRECT );

  if( cur && cur->vtype == Pre_Comp && cur->value.lvalue )
  { free ( cur->value.lvalue );
    cur->value.lvalue = List;
  }
  else SetError (ERR_PTR);

  return cur;
}
myvar*  exSentenceSub        (exSentence *PreCompSentence, list_pmv *Parameters)
{ myvar *cur = exSentenceExact (PreCompSentence,ORDER_CORRECT);

  if( cur->vtype == Pre_Comp )
  { exSentence SubSentence = {0};
    exSentenceConvert (&SubSentence,cur->value.lvalue,PreCompSentence->Names_table);

    cur = exInterpreter (&SubSentence,Parameters,NULL);
    if( SubSentence.make_sentence->len )
     exSentenceInsertList ( PreCompSentence,SubSentence.make_sentence );
    else 
    { free (SubSentence.make_sentence);
      list_pmv_extracts (PreCompSentence->exec_sentence,0);
    }
  }
  else 
  { cur = exSentenceExtract (PreCompSentence,ORDER_CORRECT);
    if(Parameters) list_pmv_insert (Parameters,-1,&cur);
  }
  return cur;
}
myvar*  exSentenceUndo       (exSentence *PreCompSentence)
{ list_pmv_node *cur_node = PreCompSentence->make_sentence->last;

  if(PreCompSentence->make_sentence->len == 0)
   return NULL;
  else if(PreCompSentence->make_sentence->len == 1)
  { PreCompSentence->make_sentence->last  = NULL;
    PreCompSentence->make_sentence->first = NULL;
  
    PreCompSentence->make_sentence->len--;
  }
  else
  { PreCompSentence->make_sentence->last = PreCompSentence->make_sentence->last->prev;
    PreCompSentence->make_sentence->last->next = NULL;

    PreCompSentence->make_sentence->len--;
  }
  //------------------
  if( !PreCompSentence->exec_sentence->len )
  { PreCompSentence->exec_sentence->first = cur_node; 
    PreCompSentence->exec_sentence->last  = cur_node;
    PreCompSentence->exec_sentence->len++;
  }
  else
  { cur_node->next = PreCompSentence->exec_sentence->first;
    cur_node->prev = NULL;

    PreCompSentence->exec_sentence->first->prev = cur_node;
    PreCompSentence->exec_sentence->first       = cur_node;

    PreCompSentence->exec_sentence->len++;
  }
  //------------------
  return cur_node->value;
}
//-----------------------------------------------------------------------------------------