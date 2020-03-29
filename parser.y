%{
//Area de encabezado y codigo

#include "scanner.h"
#include "nodo.h"
#include <iostream>


extern int yylineno; //linea actual donde se encuentra el parse (analisis lexico) lo maneja bison
extern int columna;//columna actual donde se encuentra el parse(analisi lexico) lo maneja bison
extern int linea;
extern char *yytext; //lexema actual donde esta el parser (analisis lexico) lo maneja BISON

extern Nodo *raiz;

int yyerror(const char* mens)
{

    std::cout << mens <<" "<<yytext<<" Linea: "<<linea<<" Columna: "<<columna<< std::endl;
    return 0;
}

%}

//error-verbose si se especifica la opcion los errores sintacticos son especificados por BISON
%defines "parser.h"
%output "parser.cpp"
%error-verbose
%locations

%union{
//se especifican los tipo de valores para los no terminales y lo terminales
char TEXT [256];
class Nodo *nodo;
}

//Terminales
%token<TEXT> numero;
%token<TEXT> cadena;
%token<TEXT> ruta;
%token<TEXT> montar;
%token<TEXT> Tigual;
%token<TEXT> Tguion;
%token<TEXT> identificador;

//Non terminales
%type<nodo> INICIO;
%type<nodo> L_COMANDO;
%type<nodo> COMANDO;
%type<nodo> L_PARAMETRO;
%type<nodo> PARAMETRO;

%start INICIO
%%


INICIO: L_COMANDO{
        raiz = new Nodo(@1.first_line, @1.first_column,"S","S");
        raiz=$1;
    };

L_COMANDO: L_COMANDO COMANDO{
                    $$=new Nodo(@1.first_line, @1.first_column,"L_COMANDO","L_COMANDO");
                    $$->add($1);
                    $$->add($2);
                }
               |COMANDO{
                   $$=new Nodo(@1.first_line, @1.first_column,"L_COMANDO","L_COMANDO");

                   $$->add($1);
               };

COMANDO: identificador L_PARAMETRO{
            $$=new Nodo(@1.first_line, @1.first_column,$1,$1);

            $$->add($2);
            };


L_PARAMETRO: L_PARAMETRO PARAMETRO{
                    $$=new Nodo(@1.first_line, @1.first_column,"L_PARAMETRO","L_PARAMETRO");
                    $$->add($1);
                    $$->add($2);
                }
                |PARAMETRO{
                    $$=new Nodo(@1.first_line, @1.first_column,"L_PARAMETRO","L_PARAMETRO");
                    $$->add($1);
                };

PARAMETRO: Tguion identificador Tigual identificador{
            $$=new Nodo(@1.first_line, @1.first_column,$2,$4);
            }
           |Tguion identificador Tigual cadena{
               $$=new Nodo(@1.first_line, @1.first_column,$2,$4);
           }
           |Tguion identificador Tigual ruta{
               $$=new Nodo(@1.first_line, @1.first_column,$2,$4);
           }
           |Tguion identificador Tigual numero{
               $$=new Nodo(@1.first_line, @1.first_column,$2,$4);
           }
           |Tguion identificador Tigual montar{
               $$=new Nodo(@1.first_line, @1.first_column,$2,$4);
           }
           |Tguion identificador{
                $$=new Nodo(@1.first_line, @1.first_column,$2,$2);
           }
           |
            {$$=new Nodo(0, 0,"","");};
