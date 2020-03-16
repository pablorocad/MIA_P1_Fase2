#include "nodo.h"

Nodo::Nodo(int _linea, int _columna, QString _token, QString _valor)
{
    token = _token;
    valor = _valor;
    linea = _linea;
    columna = _columna;
}

void Nodo::add(Nodo *h){
    hijo.append(h);
}
