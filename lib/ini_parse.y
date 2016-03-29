%{
#include <stdio.h>
#include "mls.h"
#include "ini_read2.h"

char *cur_text ="";
int linecnt =0;
int string_width =0;
int num_strings=0;

void yyerror(const char *str)
{
  printf("error: %s %d\n",str,linecnt );
}

int yywrap()
{
        return 1;
}


%}

%token OPEN CLOSE COL SEP AMP EQUAL

%union
{
    char str[500];
}

%token <str> ID  
%token <str> STRING
%token <str> CMD


%%
program: | chapter program ;

chapter:	OPEN ID CLOSE		
{ 
  TRACE(2,"Chapter %s", $2); 
  rc_add_chapter($2);
}
datalst			{ TRACE(2, "Chapter END"); }
;

datalst:	| varinit datalst ;

varinit:	ID COL			{ TRACE(2,"CMD: %s", $1 ); rc_var_add($1); }
		paramlist |
		ID EQUAL ID		{	TRACE(2,"VAR: %s=%s", $1,$3);
						rc_var_define( $1,$3 ); } 

paramlist:	def | def SEP paramlist ;

def:		singledef | doubledef ;

doubledef:	ID AMP ID		{ 
						TRACE(2,"%s & %s", $1, $3 ); 
						rc_var_set( $1, $3 );
					}
;

singledef:	ID			{	TRACE(2,"S1: %s", $1 ); 
						rc_var_set( $1, $1 );
					}
;



