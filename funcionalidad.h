#ifndef FUNCIONALIDAD_H
#define FUNCIONALIDAD_H
#include <nodo.h>
#include "QString"
#include "QFile"
#include<fstream>
#include <QTextStream>
#include "QHash"
#include "QProcess"
#include <ctime>
#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include "scanner.h"
#include <sys/stat.h>
#include <iostream>
#include <math.h>
using namespace std;

extern int yyparse();
extern Nodo *raiz;
extern int linea;
extern int columna;
extern int yylineno;


//------------------STRUCTS-------------------------
struct EBR{
    char part_status = '0';
    char part_fit[2];
    int part_start;
    int part_size = 0;
    int part_next = -1;
    char part_name[16];
};

struct partExt{
    EBR logica[20];
};

struct MBR_Partition{
    char part_status = '0';
    char part_type = 'P';
    char part_fit[2];
    int part_start;
    int part_size = 0;
    char part_name[16];
};

struct MBR{
    int mbr_tamano;
    char mbr_fecha_creacion[30];
    int mbr_disk_signature;
    char disk_fit[2];
    MBR_Partition mbr_partition[4];
};

struct MOUNT_NUM{
    bool montado = false;
    MBR_Partition particion;
    EBR logica;
};

struct MOUNT_LETRA{
    QString path = "";
    MOUNT_NUM numero[100];
};

struct Login{
    MBR_Partition particion;
    QString user;
    QString group;
    QString pathDisco = "";
};

struct SuperBloque{
    int s_filesystem_type;
    int s_inodes_count;
    int s_blocks_count;
    int s_free_blocks_count;
    int s_free_inodes_count;
    char s_mtime[30];
    char s_umtime[30];
    int s_mnt_count;
    int s_magic;
    int s_inode_size;
    int s_block_size;
    int s_first_ino;
    int s_first_blo;
    int s_bm_inode_start;
    int s_bm_block_start;
    int s_inode_start;
    int s_block_start;
};

struct Innodo{
    int i_uid;
    int i_gid;
    int i_size;
    char i_atime[30];
    char i_ctime[30];
    char i_mtime[30];
    int i_block[15];
    char i_type;
    int i_perm;

};

struct Journal{
    bool estado = true;
    char instruccion[200];
    char fecha[30];
};

struct Content{
    char b_name[12];
    int b_innodo = -1;
};

struct BloqueCarpeta{
    Content b_content[4];
};

struct BloqueArchivo{
    char b_content[64];
};

struct BloqueApuntador{
  int b_pointers[16];
};

class Funcionalidad
{
public:
    MOUNT_LETRA mount[27];
    Login usuario;
    int numBloque;

    Funcionalidad();
    void Ejecutar(Nodo *temp);
    QHash<QString,QString> parametros(Nodo *temp);

    //METODOS FASE 1--------------------------------------------------------------------------------------------------------
    void crearDiscoNuevo(QStringList pathAux);

    //METODOS FASE 2--------------------------------------------------------------------------------------------------------
    void formatearParticion(QHash<QString,QString> par);

    int buscarInnodo(QStringList pathAux);//Metodo para buscar un innodo con un path
    int buscarInnodo(int numInnodo, QStringList path,int posicionPath, SuperBloque block, FILE *file);
    void moverArchivo(QString name, int inodoDestino, int carpetaActual);

    void crearCarpeta(int numInnodoPadre,QString name,QStringList path);//Metodo de creacion de carpeta
    void crearArchivo(int numInnodoPadre,QString name,QStringList path, char contenido[64]);//Metodo de creacion de archivo

    void Graficar(Nodo *temp);
    void Graficar(string padre, Nodo *temp, string &grafo, int &contador);

    void swap(MBR_Partition *xp, MBR_Partition *yp);
    void bubbleSort(MBR_Partition arr[], int n);

    void swap(EBR *xp, EBR *yp);
    void bubbleSort(EBR arr[], int n);

    void EjecutarComando(string n);

    QStringList pathArchivo(QString path);
    int primerBloqueLIbre(SuperBloque block, FILE *file);
    int primerInnodoLibre(SuperBloque block, FILE *file);

    void marcarBloqueLIbre(SuperBloque block, FILE *file);
    void marcarInnodoLibre(SuperBloque block, FILE *file);

    Innodo nuevoInnodo(char tipo);
    BloqueApuntador nuevoApuntador();
    Journal nuevoRegistro();
    void getFecha(char arr[30]);
    void escribirEnJournal(string comando,FILE *file);
    string getString(char arr[],int size);

    void innodoTree(int padre, int actual, int posicion, ofstream &out, FILE *file, SuperBloque block);
    void blockTree(int padre, int actual, int posicion, char tipo ,ofstream &out, FILE *file, SuperBloque block,bool esAp);
    void innodoLS(int actual, string name, ofstream &out, FILE *file, SuperBloque block);
    void blockLS(int actual, ofstream &out, FILE *file, SuperBloque block);

    QString obtenerContenidoArchivo(int numInnodo);

    void cambiarNombre(QString oldName, QString newName, int padre);
    void agregarGrupo(QString name);
    void agregarUsuario(QString usr,QString pwd,QString grp);

    int decToBinary(int n);

    QString find(int numInnodo,QString path,QString pathAnterior,QString nameFile,SuperBloque block,FILE *file);
    QString mostrarArbolFind(QStringList path,int posicionPath,SuperBloque block,int numInnodo,FILE *file);

private:

};

#endif // FUNCIONALIDAD_H
