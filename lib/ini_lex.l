%{
#include "ini_parse.tab.h"
#include "mls.h"
extern int linecnt;
#define YYCP() strncpy( yylval.str, yytext, sizeof(yylval.str)) 
%}

%%
"="			return EQUAL;
":"			return COL;
"["                     TRACE(1,"OPEN"); return OPEN;
"]"                     TRACE(1,"CLOSE"); return CLOSE;
","			TRACE(1,"SEP"); return SEP;
"&"			TRACE(1,"AMP"); return AMP;
[^ \n\t\]\[\,\"\&\:\=]+     TRACE(1,"ID1"); YYCP(); return ID;
\"[^\n\t\"]+\"		{ TRACE(1,"ID2"); 
			  ASSERT( strlen(yytext) < sizeof(yylval.str));
			  memcpy( yylval.str, yytext+1, strlen(yytext)-2 );
			  yylval.str[strlen(yytext)-2]=0; 
			  return ID;
			}
\"\"			TRACE(1,"ID3"); YYCP(); return ID;
[ \t]+                  TRACE(1,"Whitepsace"); /* ignore whitespace */
\n                      TRACE(1,"newline"); linecnt++; 
%%