#ifndef NODO_H
#define NODO_H
#include "QString"
#include "QList"

using namespace std;

class Nodo
{
public:
    Nodo(int _linea, int _columna, QString _token, QString _valor);
    int linea, columna;
    QString token;
    QString valor;
    QList<Nodo*> hijo;

    void add(Nodo *h);
};

#endif // NODO_H
