#ifndef _SA_FOR_H
#define _SA_FOR_H

typedef struct _names_table names_table;
typedef struct _list_pmv list_pmv;
//----------------------------------------------------------------------------------------------------
void   sa_for_name_check   (names_table *Names_table, list_pmv *Sentence, list_pmv *PostfixSentence);
void   sa_for_expression   (names_table *Names_table, list_pmv *Sentence, list_pmv *PostfixSentence);
void   sa_for_semic_miss   (names_table *Names_table, list_pmv *Sentence);

void   sa_for_header_usual (names_table *Names_table, list_pmv *Sentence, list_pmv *PostfixSentence);
void   sa_for_header_ctype (names_table *Names_table, list_pmv *Sentence, list_pmv *PostfixSentence);
//----------------------------------------------------------------------------------------------------
#endif
