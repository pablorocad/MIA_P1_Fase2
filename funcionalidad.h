#ifndef FUNCIONALIDAD_H
#define FUNCIONALIDAD_H
#include <nodo.h>
#include "QString"
#include "QFile"
#include<fstream>
#include <QTextStream>
#include "QHash"
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
};

struct MOUNT_LETRA{
    QString path = "";
    MOUNT_NUM numero[100];
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
    int s_firts_ino;
    int s_first_blo;
    int s_bm_inode_start;
    int s_bm_block_start;
    int s_inode_start;
    int s_block_start;
};

struct Innodo{
    int id_innodo;
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
    char comando[7];
    int tipo;
    char path[50];
    char contenido[64];
    char fecha[30];
};

struct Content{
    char b_name[12];
    int b_innodo;
};

struct BloqueCarpeta{
    int id_bloque_carpeta;
    Content b_content[4];
};

struct BloqueArchivo{
    int id_bloque_archivo;
    char b_content[64];
};

struct BloqueApuntador{
    int id_bloque_apuntador;
  int b_pointers[16];
};

class Funcionalidad
{
public:
    MOUNT_LETRA mount[27];

    Funcionalidad();
    void Ejecutar(Nodo *temp);
    QHash<QString,QString> parametros(Nodo *temp);

    void formatearParticion(QHash<QString,QString> par);

    void Graficar(Nodo *temp);
    void Graficar(string padre, Nodo *temp, string &grafo, int &contador);

    void swap(MBR_Partition *xp, MBR_Partition *yp);
    void bubbleSort(MBR_Partition arr[], int n);

    void swap(EBR *xp, EBR *yp);
    void bubbleSort(EBR arr[], int n);

    void EjecutarComando(string n);


private:

};

#endif // FUNCIONALIDAD_H
