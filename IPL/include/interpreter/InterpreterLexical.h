#ifndef _IPL_LEXIC_H
#define _IPL_LEXIC_H

typedef struct _names_table   names_table;
typedef struct _list_pmv         list_pmv;
typedef struct _lexical_data lexical_data;
//-----------------------------------------------------------------------------------------
int   lex_create (lexical_data *Lex);
int   lex_copy   (lexical_data *scr, lexical_data *dest);
void  lex_free   (lexical_data *Lex);
//-----------------------------------------------------------------------------------------
//---*** LEXICAL ANALYSIS ***--------------------------------------------------------------
myvar*  laNextExact (list_pmv *Sentence);
myvar*  laNextWord  (list_pmv *Sentence);

int     laLexicalAnalyzer (names_table *Names_table, list_pmv *List_atoms);
int     regwords_config   (char* filename, hash_table *RegularWords);
//-----------------------------------------------------------------------------------------
#endif
