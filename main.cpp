#include <QCoreApplication>
#include <iostream>
#include "parser.h"
#include "scanner.h"
#include <stdlib.h>
#include<fstream>
#include "QFile"
#include "QHash"
#include "QString"
#include <QTextStream>
#include <funcionalidad.h>
#include <ctime>

using namespace std;




extern int yyparse();
extern Nodo *raiz;
extern int linea;
extern int columna;
extern int yylineno;
Funcionalidad *f = new Funcionalidad();

void EjecutarComando(string n){
            QString datos = QString::fromUtf8(n.c_str());

               QFile file("temp.txt"); //SE CREA UN ARCHIVO TEMPORAL PARA COMPILARLO
                   if ( file.open( file.WriteOnly ) ) { //BUFFER PARA EL TEXTO QUE SE DESEA COMPILAR
                        QTextStream stream1( &file );
                         stream1 << datos;
               }

                   const char* x = "temp.txt";
                   FILE* input = fopen(x, "r");
                   yyrestart(input);
                   if(yyparse()==0){

                       f->Graficar(raiz);
                       f->Ejecutar(raiz);
                   }else{
                       cout<<"Se encontraron errores en el comando especificado"<<endl;
                   }
}


int main(int argc, char *argv[])
{
        QCoreApplication a(argc, argv);
            char cadena1[500];
            while(true){
                cout<<"Ingrese el comando para ejecutar"<<endl;
                cin.getline(cadena1,sizeof (cadena1));
                EjecutarComando(cadena1);

            }

        //char t = 'z';
        //cout << (int)t - 97 << endl;
        //cout << (char)(25+97) << endl;

            /*time_t t = time(0);
            string tiempo = ctime(&t);
            char tiempo_char[30];
            strcpy(tiempo_char,tiempo.c_str());
            cout << tiempo_char;*/


}

