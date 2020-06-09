/*Area defines que acepta el lenguaje*/

%option case-insensitive
%option yylineno
%option header-file="scanner.h"
%option outfile="scanner.cpp"
%option noyywrap
%option nounput

%{
    //Area de definicion de cabeceras y pariables

    #include "parser.h"
    #include <cstdlib>
    #include <string>
    #include <iostream>
    #include "nodo.h"
    #include <cstdlib>
    #include <stdio.h>

    //Variables para llevar el control de la parte lexica
    int yylex();
    void yyerror(const char *s);
    int linea=1;
    int columna=1;
    int anterior=0;

    Nodo *raiz;//Raiz donde empieza la parte lexica;

    void imprimirEnConsola (const char *mensaje,const char *tkn){
        std::cout<<mensaje<<" : "<<tkn<<std::endl;
    }

%}

/* expresiones regulares*/

numero -?[0-9]+

identificador [a-zñA-ZÑ]+

cadena "\""[^"\""]*"\""
ruta ([a-zA-Z]\:)?[a-zA-Z0-9\/s_\/.\/-\/(\/):]+

montar "vd"[a-zñA-ZÑ]{numero}

otros \t|\r|\f

%x COMENTARIO

%%
INITIAL
{
    "#"             { BEGIN COMENTARIO; }

    {otros}         {/*igore white espace*/}

    {identificador}        {columna=columna+anterior;  anterior=strlen(yytext);    strcpy(yylval.TEXT,yytext); return identificador;}    

    {numero}      {columna=columna+anterior;  anterior=strlen(yytext);    strcpy(yylval.TEXT,yytext); return numero;}

    {cadena}     {columna=columna+anterior;  anterior=strlen(yytext);    strcpy(yylval.TEXT,yytext); return cadena;}

    {ruta}       {columna=columna+anterior;  anterior=strlen(yytext);    strcpy(yylval.TEXT,yytext); return ruta;}

    {montar}  {columna=columna+anterior;  anterior=strlen(yytext);    strcpy(yylval.TEXT,yytext); return montar;}

     "="         {columna=columna+anterior;  anterior=strlen(yytext);    strcpy(yylval.TEXT,yytext); return Tigual;}

     "-"              {columna=columna+anterior;  anterior=strlen(yytext);    strcpy(yylval.TEXT,yytext); return Tguion;}

     " "             {columna=columna+anterior; anterior=strlen(yytext);  strcpy(yylval.TEXT,yytext); }

     "\n"            {linea++; columna=0;anterior=0;}
}

<COMENTARIO>"\n"       { linea++; columna = 0; BEGIN INITIAL; }
<COMENTARIO>.           {/*Ignorar*/}
<COMENTARIO><<EOF>>    {linea++; columna = 0; BEGIN INITIAL;}
%%
