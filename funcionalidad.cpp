#include "funcionalidad.h"

Funcionalidad::Funcionalidad()
{

}

void Funcionalidad::Ejecutar(Nodo *temp){
    if(temp->token == "L_COMANDO"){
        if(temp->hijo.size() == 1){
            Ejecutar(temp->hijo.takeFirst());
        }
    }
    else if(temp->token == "mkdisk")
    {//Si viene la creacion de un disco

        QHash<QString,QString> par = parametros(temp->hijo.first());//Tomamos todos nuestros parametros

        if(!par.contains("size")){
            cout << "Falta el parametro SIZE en la creacion de disco | linea: " << temp->linea << " columna: " << temp->columna << endl;
        }
        else if(!par.contains("path")){
            cout << "Falta el parametro PATH en la creacion de disco | linea: " << temp->linea << " columna: " << temp->columna << endl;
        }else{
            int tamano = 1024*1024;//Tamano por defecto del disco
            FILE *file;
            file = fopen(par.value("path").toUtf8().constData(),"wb+");

            if(par.contains("unit") && par.value("unit") == "K"){//si cambian el tipo de medida
                tamano = 1024;
            }
            tamano = tamano * par.value("size").toInt();
            fseek(file,tamano,SEEK_SET);
            fputc('\0',file);

            //------------------Informacion del disco-------------------
            MBR information;

            fseek(file,0,SEEK_SET);//nos posicionamos al inicio
            information.mbr_tamano = tamano;

            time_t t = time(0);
            string tiempo = ctime(&t);
            strcpy(information.mbr_fecha_creacion,tiempo.c_str());

            information.mbr_disk_signature = rand();

            if(par.contains("fit"))
            {
                strcpy(information.disk_fit,par.value("fit").toUtf8().constData());
            }
            else
            {
                strcpy(information.disk_fit,"FF");
            }

            //Seteamos el tamaño del disco a cada posicion del arreglo de particiones para el orden
            for(int x = 0; x < 4; x++){
                information.mbr_partition[x].part_start = tamano;
            }


            fwrite(&information, sizeof (struct MBR), 1, file);

            fclose(file);


            //--------------------------COPIA---------------------------------------
            QString path_c = par.value("path").split(".")[0];
            path_c += "_ra1.disk";
            file = fopen(path_c.toUtf8().constData(),"wb+");
            fseek(file,tamano,SEEK_SET);
            fputc('\0',file);
            fseek(file,0,SEEK_SET);//nos posicionamos al inicio
            fwrite(&information, sizeof (struct MBR), 1, file);
            fclose(file);
        }
    }
    else if(temp->token == "rmdisk"){
        QHash<QString,QString> par = parametros(temp->hijo.first());//Tomamos todos nuestros parametros
        //QHash<QString,QString> par2 = parametros(temp);
        remove(par.value("path").toUtf8().constData());
    }
    else if(temp->token == "fdisk")
    {//Crear Particion----------------------------------------------------------------
        QHash<QString,QString> par = parametros(temp->hijo.first());//Tomamos todos nuestros parametros
        if(!par.contains("delete"))
        {

            if(!par.contains("size"))
            {
                cout << "Falta el parametro SIZE en la creacion de disco | linea: " << temp->linea << " columna: " << temp->columna << endl;
            }
            else if(!par.contains("path"))
            {
                cout << "Falta el parametro PATH en la creacion de disco | linea: " << temp->linea << " columna: " << temp->columna << endl;
            }
            else if(!par.contains("name"))
            {
                cout << "Falta el parametro NAME en la creacion de disco | linea: " << temp->linea << " columna: " << temp->columna << endl;
            }
            else{
                MBR_Partition information_part;
                EBR ext;

                int tamano = 1024;
                if(par.contains("unit")){//si cambian el tipo de medida
                    if(par.value("unit") == "M")
                    {
                        tamano = 1024*1024;
                    }
                    else if(par.value("unit") == "B")
                    {
                        tamano = 1;
                    }
                }
                information_part.part_size = tamano*par.value("size").toInt();
                ext.part_size = tamano*par.value("size").toInt();

                if(par.contains("type"))
                {
                    if(par.value("type") == "P"){
                       information_part.part_type = 'P';
                    }
                    else if(par.value("type") == "E"){
                        information_part.part_type = 'E';
                    }
                    else if(par.value("type") == "L"){
                        information_part.part_type = 'L';
                    }
                }
                else{
                    information_part.part_type = 'P';
                }

                if(par.contains("fit"))
                {
                    strcpy(information_part.part_fit,par.value("fit").toUtf8().constData());
                    strcpy(ext.part_fit,par.value("fit").toUtf8().constData());
                }
                else {
                    strcpy(information_part.part_fit,"WF");
                    strcpy(ext.part_fit,"WF");
                }

                strcpy(information_part.part_name,par.value("name").toUtf8().constData());
                strcpy(ext.part_name,par.value("name").toUtf8().constData());

                //Insertar particion---------------------------------------------------------------------------------
                FILE *file;
                file = fopen(par.value("path").toUtf8().constData(),"rb+");

                MBR information;
                fread(&information,sizeof(struct MBR),1,file);

                string fitAux(information.disk_fit);
                int start;

                if(information_part.part_type != 'L')
                {
                    if(information.mbr_partition[0].part_size != 0){//si el primero no esta vacio hacemos el proceso
                        if(fitAux == "FF")
                        {
                            int aux1, aux2 = 0;//Variables con las que nos moveremos en el disco
                            aux1 = sizeof(struct MBR);//aux1 se movera al inicio de cada particion

                            for(int x = 0; x < 4; x++){
                                aux2 = information.mbr_partition[x].part_start;//aux2 se movera entre los espacios libres

                                if((aux2 - aux1) >= information_part.part_size)
                                {
                                    start = aux1;
                                    information_part.part_start = start;

                                    int indice = -1;
                                    for(int x = 0; x < 4; x++)//Buscaremos un espacio libre
                                    {
                                        if(information.mbr_partition[x].part_size == 0)
                                        {
                                            indice = x;
                                            break;
                                        }
                                    }

                                    if(indice != -1)
                                    {
                                        information.mbr_partition[indice] = information_part;
                                        //Ordenamos el arreglo
                                        int n = 4;
                                        bubbleSort(information.mbr_partition,n);

                                        fseek(file,0,SEEK_SET);//Nos movemos al inicio del disco
                                        fwrite(&information,sizeof(struct MBR),1,file);//escribimos la nueva informacion
                                    }
                                    else {
                                        cout << "No pueden crearse mas particiones, elimine una de las existentes" << endl;
                                    }

                                    break;
                                }
                                else{
                                    aux1 += information.mbr_partition[x].part_size;//movemos aux1
                                }
                            }
                        }
                        else if(fitAux == "BF")//Best Fit
                        {
                            int bestStart = information.mbr_tamano;
                            int aux1, aux2 = 0;//Variables con las que nos moveremos en el disco
                            aux1 = sizeof(struct MBR);//aux1 se movera al inicio de cada particion
                            start = 0;

                            for(int x = 0; x < 4; x++)
                            {
                                aux2 = information.mbr_partition[x].part_start;//aux2 se movera entre los espacios libres
                                int resta = aux2 - aux1;
                                if(((resta) >= information_part.part_size) && ((resta) < bestStart))                            {
                                    start = aux1;
                                    bestStart = resta;
                                }
                                aux1 = aux2 + information.mbr_partition[x].part_size;//movemos aux1

                            }

                            information_part.part_start = start;

                            int indice = -1;
                            for(int x = 0; x < 4; x++)//Buscaremos un espacio libre
                            {
                                if(information.mbr_partition[x].part_size == 0)
                                {
                                    indice = x;
                                    break;
                                }
                            }

                            if(indice != -1)
                            {
                                information.mbr_partition[indice] = information_part;
                                //Ordenamos el arreglo
                                int n = 4;
                                bubbleSort(information.mbr_partition,n);

                                fseek(file,0,SEEK_SET);//Nos movemos al inicio del disco
                                fwrite(&information,sizeof(struct MBR),1,file);//escribimos la nueva informacion
                            }
                            else {
                                cout << "No pueden crearse mas particiones, elimine una de las existentes" << endl;
                            }
                        }
                        else if (fitAux == "WF") //Worst Fit
                        {
                            int worstStart = 0;
                            int aux1, aux2 = 0;//Variables con las que nos moveremos en el disco
                            aux1 = sizeof(struct MBR);//aux1 se movera al inicio de cada particion
                            start = 0;

                            for(int x = 0; x < 4; x++)
                            {
                                aux2 = information.mbr_partition[x].part_start;//aux2 se movera entre los espacios libres
                                int resta = aux2 - aux1;
                                if(((resta) >= information_part.part_size) && ((resta) > worstStart) && aux2 < information.mbr_tamano)                            {
                                    start = aux1;
                                    worstStart = resta;
                                }
                                aux1 = aux2 + information.mbr_partition[x].part_size;//movemos aux1

                            }

                            information_part.part_start = start;

                            int indice = -1;
                            for(int x = 0; x < 4; x++)//Buscaremos un espacio libre
                            {
                                if(information.mbr_partition[x].part_size == 0)
                                {
                                    indice = x;
                                    break;
                                }
                            }

                            if(indice != -1)
                            {
                                information.mbr_partition[indice] = information_part;
                                //Ordenamos el arreglo
                                int n = 4;
                                bubbleSort(information.mbr_partition,n);

                                fseek(file,0,SEEK_SET);//Nos movemos al inicio del disco
                                fwrite(&information,sizeof(struct MBR),1,file);//escribimos la nueva informacion
                            }
                            else {
                                cout << "No pueden crearse mas particiones, elimine una de las existentes" << endl;
                            }
                        }
                    }
                    else{//si esta vacio no existe ninguna particion
                        start = sizeof (struct MBR);
                        information_part.part_start = start;
                        information.mbr_partition[0] = information_part;
                        fseek(file,0,SEEK_SET);//Nos movemos al inicio del disco
                        fwrite(&information,sizeof(struct MBR),1,file);//escribimos la nueva informacion
                    }

                    if(information_part.part_type == 'E')
                    {
                        fseek(file,sizeof(struct MBR)+1,SEEK_SET);//nos movemos a la posicion de la extendida
                        EBR information_ext[20];

                        for(int x = 0; x < 20; x++)
                        {
                            information_ext[x].part_start = information_part.part_start + information_part.part_size;
                        }

                        fwrite(&information_ext,sizeof(information_ext),1,file);

                        //COPIA--------------------------------
                        QString path_c = par.value("path").split(".")[0];
                        path_c += "_ra1.disk";
                        FILE *fil = fopen(path_c.toUtf8().constData(),"wb+");
                        fseek(fil,sizeof(struct MBR)+1,SEEK_SET);
                        fwrite(&information_ext,sizeof (information_ext),1,fil);
                        fclose(fil);
                    }
                }
                else
                {
                    bool s = true;
                    EBR extAux[20];
                    for (MBR_Partition part : information.mbr_partition) {
                        if(part.part_type == 'E')
                        {
                            fseek(file,sizeof(struct MBR)+1,SEEK_SET);
                            fread(&extAux,sizeof(extAux),1,file);

                            string fitExt(part.part_fit);

                            if(extAux[0].part_size != 0)
                            {//si el primero no esta vacio hacemos el proceso
                                if(fitExt == "FF")
                                {
                                    int aux1, aux2 = 0;//Variables con las que nos moveremos en el disco
                                    aux1 = part.part_start;//aux1 se movera al inicio de cada particion

                                    for(int x = 0; x < 20; x++){
                                        aux2 = extAux[x].part_start;//aux2 se movera entre los espacios libres

                                        if((aux2 - aux1) >= ext.part_size)
                                        {
                                            start = aux1;
                                            ext.part_start = start;
                                            extAux[x].part_next = start;

                                            int indice = -1;
                                            for(int x = 0; x < 20; x++)//Buscaremos un espacio libre
                                            {
                                                if(extAux[x].part_size == 0)
                                                {
                                                    indice = x;
                                                    break;
                                                }
                                            }

                                            if(indice != -1)
                                            {
                                                extAux[indice] = ext;
                                                //Ordenamos el arreglo
                                                int n = 20;
                                                bubbleSort(extAux,n);

                                                fseek(file,sizeof (struct MBR) + 1,SEEK_SET);//Nos movemos al inicio del disco
                                                fwrite(&extAux,sizeof(extAux),1,file);//escribimos la nueva informacion
                                            }
                                            else {
                                                cout << "No pueden crearse mas particiones, elimine una de las existentes" << endl;
                                            }

                                            break;
                                        }
                                        else{
                                            aux1 += extAux[x].part_size;//movemos aux1
                                        }
                                    }
                                }
                                else if(fitAux == "BF")//Best Fit
                                {
                                    int bestStart = part.part_start+part.part_size;
                                    int aux1, aux2 = 0;//Variables con las que nos moveremos en el disco
                                    aux1 = part.part_start;//aux1 se movera al inicio de cada particion
                                    start = 0;

                                    for(int x = 0; x < 20; x++)
                                    {
                                        aux2 = extAux[x].part_start;//aux2 se movera entre los espacios libres
                                        int resta = aux2 - aux1;
                                        if(((resta) >= ext.part_size) && ((resta) < bestStart))
                                        {
                                            start = aux1;
                                            bestStart = resta;
                                        }
                                        aux1 = aux2 + extAux[x].part_size;//movemos aux1

                                    }

                                    ext.part_start = start;


                                    int indice = -1;
                                    for(int x = 0; x < 20; x++)//Buscaremos un espacio libre
                                    {
                                        if(extAux[x].part_size == 0)
                                        {
                                            indice = x;
                                            break;
                                        }
                                    }

                                    if(indice != -1)
                                    {
                                        extAux[indice] = ext;
                                        extAux[indice-1].part_next = start;
                                        //Ordenamos el arreglo
                                        int n = 20;
                                        bubbleSort(extAux,n);

                                        fseek(file,sizeof (struct MBR)+1,SEEK_SET);//Nos movemos al inicio del disco
                                        fwrite(&extAux,sizeof(extAux),1,file);//escribimos la nueva informacion
                                    }
                                    else {
                                        cout << "No pueden crearse mas particiones, elimine una de las existentes" << endl;
                                    }
                                }
                                else if (fitAux == "WF") //Worst Fit
                                {
                                    int worstStart = 0;
                                    int aux1, aux2 = 0;//Variables con las que nos moveremos en el disco
                                    aux1 = part.part_start;//aux1 se movera al inicio de cada particion
                                    start = 0;

                                    for(int x = 0; x < 20; x++)
                                    {
                                        aux2 = extAux[x].part_start;//aux2 se movera entre los espacios libres
                                        int resta = aux2 - aux1;
                                        if(((resta) >= ext.part_size) && ((resta) > worstStart) && aux2 < (part.part_start+part.part_size))
                                        {
                                            start = aux1;
                                            worstStart = resta;
                                        }
                                        aux1 = aux2 + extAux[x].part_size;//movemos aux1

                                    }

                                    ext.part_start = start;

                                    int indice = -1;
                                    for(int x = 0; x < 20; x++)//Buscaremos un espacio libre
                                    {
                                        if(extAux[x].part_size == 0)
                                        {
                                            indice = x;
                                            break;
                                        }
                                    }

                                    if(indice != -1)
                                    {
                                        extAux[indice] = ext;
                                        extAux[indice-1].part_next = start;
                                        //Ordenamos el arreglo
                                        int n = 4;
                                        bubbleSort(extAux,n);

                                        fseek(file,sizeof (struct MBR)+1,SEEK_SET);//Nos movemos al inicio del disco
                                        fwrite(&extAux,sizeof(extAux),1,file);//escribimos la nueva informacion
                                    }
                                    else {
                                        cout << "No pueden crearse mas particiones, elimine una de las existentes" << endl;
                                    }
                                }
                            }
                            else{//si esta vacio no existe ninguna particion
                                start = part.part_start;
                                ext.part_start = start;
                                extAux[0] = ext;
                                fseek(file,sizeof(struct MBR)+1,SEEK_SET);//Nos movemos al inicio del disco
                                fwrite(&extAux,sizeof(extAux),1,file);//escribimos la nueva informacion
                            }

                            QString path_c = par.value("path").split(".")[0];
                            path_c += "_ra1.disk";
                            FILE *f;
                            f = fopen(path_c.toUtf8().constData(),"wb+");
                            fwrite(&extAux,sizeof(extAux),1,f);//escribimos la nueva informacion
                            fclose(f);

                            s = false;
                        }


                    }

                    if(s)
                    {
                        cout << "No existe ninguna particion extendida para la incersion de una logica" << endl;
                    }
                }

                fclose(file);

                //COPIA-----------------------------------------------------------------------------------------
                QString path_c = par.value("path").split(".")[0];
                path_c += "_ra1.disk";
                file = fopen(path_c.toUtf8().constData(),"wb+");
                fwrite(&information,sizeof(struct MBR),1,file);//escribimos la nueva informacion
                fclose(file);
            }

        }
        else
        {
            if(!par.contains("path"))
            {
                cout << "Falta el parametro PATH en la creacion de disco | linea: " << temp->linea << " columna: " << temp->columna << endl;
            }
            else if(!par.contains("name"))
            {
                cout << "Falta el parametro NAME en la creacion de disco | linea: " << temp->linea << " columna: " << temp->columna << endl;
            }
            else{

                MBR_Partition nuevaPar;
                MBR information;
                FILE *file;
                file = fopen(par.value("path").toUtf8().constData(),"rb+");
                fread(&information,sizeof (struct MBR),1,file);

                string nameCon(par.value("name").toStdString());
                bool s1 = true;
                bool s2 = true;

                for(int x = 0; x< 4; x++)//buscaremos la particion
                {
                    string nameDisk(information.mbr_partition[x].part_name);
                    if(nameCon == nameDisk){
                        nuevaPar.part_start = information.mbr_tamano;
                        information.mbr_partition[x] = nuevaPar;
                        bubbleSort(information.mbr_partition,4);

                        fseek(file,0,SEEK_SET);
                        fwrite(&information,sizeof(struct MBR),1,file);
                        s1 = false;
                        s2 = false;

                        //COPIA-----------------------------------------------------------------------------------------
                        QString path_c = par.value("path").split(".")[0];
                        path_c += "_ra1.disk";
                        FILE *f;
                        f = fopen(path_c.toUtf8().constData(),"wb+");
                        fwrite(&information,sizeof(struct MBR),1,f);//escribimos la nueva informacion
                        fclose(f);

                        break;
                    }
                }



                if(s1)
                {
                    for(MBR_Partition part : information.mbr_partition)
                    {
                        if(part.part_type == 'E')
                        {
                            EBR ext[20];
                            fseek(file,sizeof (struct MBR)+1,SEEK_SET);
                            fread(&ext,sizeof (ext),1,file);
                            EBR nuevoExt;
                            for(int x = 0; x< 20; x++)//buscaremos la particion
                            {
                                string nameDisk(ext[x].part_name);
                                if(nameCon == nameDisk){
                                    nuevoExt.part_start = part.part_start+part.part_size;
                                    ext[x] = nuevoExt;
                                    bubbleSort(ext,20);

                                    fseek(file,sizeof (struct MBR)+1,SEEK_SET);
                                    fwrite(&ext,sizeof(ext),1,file);
                                    s2 = false;

                                    QString path_c = par.value("path").split(".")[0];
                                    path_c += "_ra1.disk";
                                    FILE *f;
                                    f = fopen(path_c.toUtf8().constData(),"wb+");
                                    fseek(f,sizeof (struct MBR)+1,SEEK_SET);
                                    fwrite(&ext,sizeof(ext),1,f);//escribimos la nueva informacion
                                    fclose(f);

                                    break;
                                }
                            }
                        }
                    }
                }

                if(s2)
                {
                    cout << "La particion " << par.value("name").toStdString() << " no existe" << endl;
                }

                fclose(file);
            }
        }
    }
    else if(temp->token == "exec"){
        QHash<QString,QString> par = parametros(temp->hijo.first());//Tomamos todos nuestros parametros
        if(!par.contains("path"))
        {
            cout << "Falta el parametro PATH en la creacion de disco | linea: " << temp->linea << " columna: " << temp->columna << endl;
        }
        else
        {
            string cadena;
            ifstream datos(par.value("path").toStdString());
            if(datos.fail()){
            cout<<"IJOLE"<<endl;
            }else{
            while(!datos.eof()){
            getline(datos,cadena); //con esta funcion tomas la linea(limitada por \n)
            EjecutarComando(cadena);
            //cout<<cadena<<endl;
            }
            }
        }

    }
    else if(temp->token == "mount")
    {
        QHash<QString,QString> par = parametros(temp->hijo.first());
        if(!par.contains("path"))
        {
            cout << "Falta el parametro PATH en montar| linea: " << temp->linea << " columna: " << temp->columna << endl;
        }
        else if(!par.contains("name"))
        {
            cout << "Falta el parametro NAME en montar | linea: " << temp->linea << " columna: " << temp->columna << endl;
        }
        else{
        MBR information;
        FILE *file;
        file = fopen(par.value("path").toUtf8().constData(), "rb+");
        fread(&information,sizeof(struct MBR),1,file);


        for(int x = 0; x < 4; x++)
        {
            QString n(information.mbr_partition[x].part_name);
            if(n == par.value("name"))
            {
                for(int m = 0; m < 27; m++)
                {
                    if(mount[m].path == "")
                    {
                        mount[m].path = par.value("path");//GUardamos el path en la letra que sera del disco
                        mount[m].numero[0].montado = true;
                        mount[m].numero[0].particion = information.mbr_partition[x];

                        information.mbr_partition[x].part_status = '1';
                        fseek(file,0,SEEK_SET);
                        fwrite(&information,sizeof (struct MBR),1,file);
                        break;
                    }
                    else if(mount[m].path == par.value("path"))
                    {
                        for(int n = 0; n < 100; n++)
                        {
                            if(mount[m].numero[n].montado == false)
                            {
                                mount[m].numero[n].montado = true;
                                mount[m].numero[n].particion = information.mbr_partition[x];

                                information.mbr_partition[x].part_status = '1';
                                fseek(file,0,SEEK_SET);
                                fwrite(&information,sizeof (struct MBR),1,file);
                                break;
                            }
                        }
                        break;
                    }
                }

                break;
            }
        }

        fclose(file);

        }
    }
    else if(temp->token == "login")
    {
        QHash<QString,QString> par = parametros(temp->hijo.first());//Tomamos todos nuestros parametros

        /*if(!par.contains("usr"))
        {
            cout << "Falta el parametro USR en crear Login | linea: " << temp->linea << " columna: " << temp->columna << endl;
        }
        else if(!par.contains("pwd"))
        {
            cout << "Falta el parametro NAME en crear reporte | linea: " << temp->linea << " columna: " << temp->columna << endl;
        }
        else */if(!par.contains("id"))
        {
            cout << "Falta el parametro ID en crear Login | linea: " << temp->linea << " columna: " << temp->columna << endl;
        }
        else
        {
            int partLetra = ((int) par.value("id").split("")[3].toStdString()[0]) - 97;
            int partNum = par.value("id").split("")[4].toInt() - 1;

            if(mount[partLetra].numero[partNum].montado)
            {
                usuario.pathDisco = mount[partLetra].path;
                usuario.particion = mount[partLetra].numero[partNum].particion;
            }
        }
    }
    else if(temp->token == "mkfs")//Formatear particion
    {
        QHash<QString,QString> par = parametros(temp->hijo.first());//Tomamos todos nuestros parametros

        if(!par.contains("id"))
        {
            cout << "Falta el parametro ID en formatear particion | linea: " << temp->linea << " columna: " << temp->columna << endl;
        }
        else{
            formatearParticion(par);
        }

    }
    else if(temp->token == "mkdir")
    {
        QHash<QString,QString> par = parametros(temp->hijo.first());//Tomamos todos nuestros parametros
        if(!par.contains("path"))
        {
            cout << "Falta el parametro PATH en crear reporte | linea: " << temp->linea << " columna: " << temp->columna << endl;
        }
        else{
            QString nuevoPath = par.value("path");
            nuevoPath.replace("\"","");
            QStringList path = nuevoPath.split('/');

            int innodoPadre;

            if(!par.contains("p"))
            {
                QString name = path.last();
                path.removeLast();
                path.removeFirst();

                innodoPadre = buscarInnodo(path);
                if(innodoPadre != -1)
                {
                    crearCarpeta(innodoPadre,name,path);
                }
                else {
                    cout << "Una carpeta no existe" << endl;
                }
            }
            else
            {
                QStringList temp;
                for(int x = 1; x < path.size(); x++)
                {
                    for(int y = 1; y <= x; y++)
                    {
                        temp.append(path.value(y));
                    }

                    innodoPadre = buscarInnodo(temp);
                    if(innodoPadre == -1)
                    {
                        QString name = temp.last();
                        temp.removeLast();
                        innodoPadre = buscarInnodo(temp);
                        crearCarpeta(innodoPadre,name,temp);
                    }

                    temp.clear();
                    cout << "";
                }
            }
        }
    }
    else if (temp->token == "mkfile")
    {
        QHash<QString,QString> par = parametros(temp->hijo.first());//Tomamos todos nuestros parametros
        if(!par.contains("path"))
        {
            cout << "Falta el parametro PATH en crear reporte | linea: " << temp->linea << " columna: " << temp->columna << endl;
        }
        else
        {
            QString nuevoPath = par.value("path");
            nuevoPath.replace("\"","");
            QStringList path = nuevoPath.split('/');

            char contenidoSize[64];
            char contenidoCont[64];

            for(int x = 0; x < 64; x++)
            {
                contenidoSize[x] = NULL;
                contenidoCont[x] = NULL;
            }

            int innodoPadre;

            if(!par.contains("p"))
            {
                QString name = path.last();
                path.removeLast();
                path.removeFirst();

                innodoPadre = buscarInnodo(path);
                if(innodoPadre != -1)
                {

                    if(par.contains("cont"))
                    {
                        QString contAux = par.value("cont");
                        contAux.replace("\"","");

                        string contS = contAux.toStdString();
                        const char *c = contS.c_str();
                        FILE *fileAux;
                        fileAux = fopen(c,"r");
                        if(!fileAux)
                        {
                            cout << "Nel" << endl;
                        }
                        else
                        {
                            int num = 0;
                            bool a = true;
                            while(!feof(fileAux))
                            {
                                fread(&contenidoCont[num],sizeof(char),1,fileAux);
                                num++;
                                if(num > 63)
                                {
                                    crearArchivo(innodoPadre,name,path,contenidoCont);
                                    for(int x = 0; x < 64; x++){
                                        contenidoCont[x] = NULL;
                                    }
                                    num = 0;
                                    if(a)
                                    {
                                        path.append(name);
                                        innodoPadre = buscarInnodo(path);
                                        a = false;
                                    }
                                }

                            }
                            crearArchivo(innodoPadre,name,path,contenidoCont);
                        }
                        fclose(fileAux);
                    }
                    else if(!par.contains("cont"))
                    {
                        if(par.contains("size"))
                        {
                            int num = 0;
                            for(int x = 0; x < par.value("size").toInt(); x++)
                            {
                                contenidoSize[x] = '0' + num;
                                num++;
                                if(num > 9)
                                {
                                    num = 0;
                                }
                            }
                            crearArchivo(innodoPadre,name,path,contenidoSize);
                        }
                    }

                }
                else {
                    cout << "Una carpeta no existe" << endl;
                }
            }
            else
            {
                QString name = path.last();
                path.removeLast();

                QStringList temp;
                for(int x = 1; x < path.size(); x++)
                {
                    for(int y = 1; y <= x; y++)
                    {
                        temp.append(path.value(y));
                    }

                    innodoPadre = buscarInnodo(temp);
                    if(innodoPadre == -1)
                    {
                        QString name = temp.last();
                        temp.removeLast();
                        innodoPadre = buscarInnodo(temp);
                        crearCarpeta(innodoPadre,name,temp);
                    }

                    temp.clear();
                    cout << "";
                }

                //---------------------------------------------------------------------
                path.removeFirst();
                innodoPadre = buscarInnodo(path);
                if(innodoPadre != -1)
                {

                    if(par.contains("cont"))
                    {
                        QString contAux = par.value("cont");
                        contAux.replace("\"","");

                        string contS = contAux.toStdString();
                        const char *c = contS.c_str();
                        FILE *fileAux;
                        fileAux = fopen(c,"r");
                        if(!fileAux)
                        {
                            cout << "Nel" << endl;
                        }
                        else
                        {
                            int num = 0;
                            bool a = true;
                            while(!feof(fileAux))
                            {
                                fread(&contenidoCont[num],sizeof(char),1,fileAux);
                                num++;
                                if(num > 63)
                                {
                                    crearArchivo(innodoPadre,name,path,contenidoCont);
                                    for(int x = 0; x < 64; x++){
                                        contenidoCont[x] = NULL;
                                    }
                                    num = 0;
                                    if(a)
                                    {
                                        path.append(name);
                                        innodoPadre = buscarInnodo(path);
                                        a = false;
                                    }
                                }

                            }
                            crearArchivo(innodoPadre,name,path,contenidoCont);
                        }
                        fclose(fileAux);
                    }
                    else if(!par.contains("cont"))
                    {
                        if(par.contains("size"))
                        {
                            int num = 0;
                            for(int x = 0; x < par.value("size").toInt(); x++)
                            {
                                contenidoSize[x] = '0' + num;
                                num++;
                                if(num > 9)
                                {
                                    num = 0;
                                }
                            }
                            crearArchivo(innodoPadre,name,path,contenidoSize);
                        }
                    }

                }
                else {
                    cout << "Una carpeta no existe" << endl;
                }
            }
        }
    }
    else if (temp->token == "pause")
    {
        cout << "Presione cualquier tecla para continuar" << endl;
        getchar();
    }
    else if (temp->token == "cat")
    {
        QHash<QString,QString> par = parametros(temp->hijo.first());//Tomamos todos nuestros parametros
        if(!par.contains("file"))
        {
            cout << "Falta el parametro FILE en cat | linea: " << temp->linea << " columna: " << temp->columna << endl;
        }
        else
        {
            QStringList path = par.value("file").split('/');
            path.removeFirst();
            int innodoArchivo = buscarInnodo(path);
            QString cont = obtenerContenidoArchivo(innodoArchivo);
            cout << cont.toStdString() << endl;

        }
    }
    else if(temp->token == "rep"){
        QHash<QString,QString> par = parametros(temp->hijo.first());//Tomamos todos nuestros parametros

        if(!par.contains("path"))
        {
            cout << "Falta el parametro PATH en crear reporte | linea: " << temp->linea << " columna: " << temp->columna << endl;
        }
        else if(!par.contains("name"))
        {
            cout << "Falta el parametro NAME en crear reporte | linea: " << temp->linea << " columna: " << temp->columna << endl;
        }
        else if(!par.contains("id"))
        {
            cout << "Falta el parametro ID en crear reporte | linea: " << temp->linea << " columna: " << temp->columna << endl;
        }
        else
        {
            int partLetra = ((int) par.value("id").split("")[3].toStdString()[0]) - 97;
            int partNum = par.value("id").split("")[4].toInt() - 1;
            QString url = par.value("path").split(".")[0];
            QString ext = par.value("path").split(".")[1];

            /*QStringList pathAux = pathArchivo(par.value("path"));
            int g = pathAux.size();

            QString k = pathAux.value(2);*/

            if(mount[partLetra].numero[partNum].montado)
            {

                MBR_Partition particion =  mount[partLetra].numero[partNum].particion;

                FILE *file;

                file = fopen(mount[partLetra].path.toUtf8().constData(),"rb+");
                MBR information2;
                fread(&information2,sizeof(struct MBR),1,file);
                fclose(file);

                ofstream out;
                url.append(".dot");

                string ur = url.toStdString();
                const char *u = ur.c_str();
                out.open(u);

                if(par.value("name") == "mbr")
                {
                    out << "digraph G{ \n";
                    out << "node [shape=plaintext] \n";
                    out << "struct1 [label=<\n";
                    out << "<TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\">\n";
                    out << "<tr><td>Nombre</td><td>Valor</td></tr>\n";

                    out << "<tr><td>mbr_tamano</td><td>";
                    out << information2.mbr_tamano;
                    out << "</td></tr>\n";

                    out << "<tr><td>mbr_fecha_creacion</td><td>";
                    out << information2.mbr_fecha_creacion;
                    out << "</td></tr>\n";

                    out << "<tr><td>mbr_disk_signature</td><td>";
                    out << information2.mbr_disk_signature;
                    out <<"</td></tr>\n";

                    out << "<tr><td>mbr_fit</td><td>";
                    out << information2.disk_fit;
                    out << "</td></tr>\n";

                    for(int x = 0; x < 4; x++)
                    {
                        if(information2.mbr_partition[x].part_status != '0')
                        {
                            out << "<tr><td>part_status_" << x << "</td><td>";
                            out << information2.mbr_partition[x].part_status;
                            out << "</td></tr>\n";

                            out << "<tr><td>part_type_" << x << "</td><td>";
                            out << information2.mbr_partition[x].part_type;
                            out << "</td></tr>\n";

                            out << "<tr><td>part_fit_" << x << "</td><td>";
                            out << information2.mbr_partition[x].part_fit[0];
                            out << information2.mbr_partition[x].part_fit[1];
                            out << "</td></tr>\n";

                            out << "<tr><td>part_start_" << x << "</td><td>";
                            out << information2.mbr_partition[x].part_start;
                            out << "</td></tr>\n";

                            out << "<tr><td>part_size_" << x << "</td><td>";
                            out << information2.mbr_partition[x].part_size;
                            out << "</td></tr>\n";

                            out << "<tr><td>part_name_" << x << "</td><td>";
                            out << information2.mbr_partition[x].part_name;
                            out << "</td></tr>\n";
                        }
                    }

                    out << "</TABLE>>];\n";
                    out << "}";

                }
                else if(par.value("name") == "disk")
                {
                    out << "digraph G{";
                    out << "node [shape=record];";
                    out << "struct1 [label=\"";

                    out << "MBR";

                    for(int x = 0; x < 4; x++)
                    {
                        if(information2.mbr_partition[x].part_size != 0)
                        {
                            out << "|";
                            if(information2.mbr_partition[x].part_type == 'P')
                            {
                                out << "Primaria";
                            }else{
                                out << "{Extendida";
                                fseek(file,sizeof (struct MBR)+1,SEEK_SET);
                                EBR k[20];
                                fread(&k,sizeof (k),1,file);
                                for(int x = 0; x < 20; x++)
                                {
                                    if(k[x].part_size != 0){
                                        out << "|";
                                        out << "EBR|";
                                        out << "Logica";
                                    }
                                }
                                out << "}";
                            }
                        }
                    }

                    out << "\"]";
                    out << "}";
                }
                else if(par.value("name") == "sb")
                {
                    SuperBloque sb;

                    file = fopen(mount[partLetra].path.toUtf8().constData(),"rb+");
                    fseek(file,particion.part_start,SEEK_SET);
                    fread(&sb,sizeof (struct SuperBloque),1,file);

                    char t;

                    fseek(file,sb.s_bm_inode_start,SEEK_SET);

                    for(int v = 0; v < sb.s_free_inodes_count;v++){
                        fread(&t,sizeof (char),1,file);
                    }
                    cout << "";
                    fclose(file);

}
                else if (par.value("name") == "tree") {
                    SuperBloque block;
                    char byte;

                    out << "digraph G{ \n";
                    out << "rankdir=LR; \n";

                    file = fopen(mount[partLetra].path.toUtf8().constData(),"rb+");
                    fseek(file,mount[partLetra].numero[partNum].particion.part_start,SEEK_SET);
                    fread(&block,sizeof (struct SuperBloque),1,file);

                    innodoTree(0,0, -1, out,file,block);

                    fclose(file);
                    out << "}\n";
                }
                else if(par.value("name") == "inode")
                {
                    SuperBloque block;
                    file = fopen(mount[partLetra].path.toUtf8().constData(),"rb+");
                    fseek(file,mount[partLetra].numero[partNum].particion.part_start,SEEK_SET);
                    fread(&block,sizeof (struct SuperBloque),1,file);

                    char byte;
                    Innodo innodo;
                    int ultimoInodo = 0;

                    out << "digraph G{ \n";
                    out << "rankdir=LR; \n";

                    fseek(file,block.s_bm_inode_start,SEEK_SET);
                    for(int x = 0; x < block.s_inodes_count; x++)
                    {
                        fread(&byte,sizeof (char),1,file);
                        if(byte == '1')
                        {
                            ultimoInodo = x;
                        }
                    }

                    for(int x = 0; x < block.s_inodes_count; x++)
                    {
                        fseek(file,block.s_bm_inode_start,SEEK_SET);
                        fseek(file,x*sizeof(char),SEEK_CUR);
                        fread(&byte,sizeof (char),1,file);

                        if(byte == '1')
                        {
                            fseek(file,block.s_inode_start,SEEK_SET);
                            fseek(file,x*block.s_inode_size,SEEK_CUR);
                            fread(&innodo,block.s_inode_size,1,file);

                            out << "innode_" << x;
                            out << " [fillcolor=darkturquoise,style=\"filled,bold\",shape=record, label=\"Inodo_" << x << " ";
                            out << "| {i_uid | " << innodo.i_uid << "}";
                            out << "| {i_gid | " << innodo.i_gid << "}";
                            out << "| {i_size | " << innodo.i_size << "}";
                            out << "| {i_atime | " << innodo.i_atime << "}";
                            out << "| {i_ctime | " << innodo.i_ctime << "}";
                            out << "| {i_mtime | " << innodo.i_mtime << "}";
                            out << "| {i_type | " << innodo.i_type << "}";
                            out << "| {i_perm | " << innodo.i_perm << "}";

                            for(int x = 0; x < 15; x++)
                            {
                                out << "| {AD " << x << "| " << innodo.i_block[x] << "}";
                            }

                            out << "\"];";
                            out << "\n";

                            if(x == ultimoInodo)
                            {
                                break;
                            }

                            out << "innode_" << x << " -> " << "innode_" << x+1;
                            out << "\n";
                        }
                    }

                    fclose(file);
                    out << "}\n";
                }
                else if(par.value("name") == "block")
                {
                    SuperBloque block;
                    BloqueArchivo archivo;
                    BloqueCarpeta carpeta;
                    BloqueApuntador apuntadorSimple;
                    BloqueApuntador apuntadorDoble;

                    file = fopen(mount[partLetra].path.toUtf8().constData(),"rb+");
                    fseek(file,mount[partLetra].numero[partNum].particion.part_start,SEEK_SET);
                    fread(&block,sizeof (struct SuperBloque),1,file);

                    char byte;
                    Innodo innodo;
                    int ultimoInodo = 0;

                    out << "digraph G{ \n";
                    out << "rankdir=LR; \n";

                    fseek(file,block.s_bm_inode_start,SEEK_SET);
                    for(int x = 0; x < block.s_inodes_count; x++)
                    {
                        fread(&byte,sizeof (char),1,file);
                        if(byte == '1')
                        {
                            ultimoInodo = x;
                        }
                    }

                    int anterior = -1;
                    int posicion = 0;

                    for(int x = 0; x < ultimoInodo; x++)
                    {
                        fseek(file,block.s_bm_inode_start,SEEK_SET);
                        fseek(file,x*sizeof(char),SEEK_CUR);
                        fread(&byte,sizeof (char),1,file);

                        if(byte == '1')
                        {
                            fseek(file,block.s_inode_start,SEEK_SET);
                            fseek(file,x*block.s_inode_size,SEEK_CUR);
                            fread(&innodo,block.s_inode_size,1,file);

                            for(int posI = 0; posI < 12; posI++)
                            {
                                posicion = innodo.i_block[posI];
                                if(posicion != -1)
                                {
                                    fseek(file,block.s_block_start,SEEK_SET);
                                    fseek(file,posicion*block.s_block_size,SEEK_CUR);
                                    if(innodo.i_type == '0')
                                    {
                                        fread(&carpeta,block.s_block_size,1,file);
                                        out << "block_" << posicion;
                                        out << " [fillcolor=chartreuse2,style=\"filled,bold\",shape=record, label=\"Bloque_" << posicion << " ";

                                        for(int x = 0; x < 4; x++)
                                        {
                                            out << "| {" << carpeta.b_content[x].b_name <<"| " << carpeta.b_content[x].b_innodo << "}";
                                        }

                                        out << "\"];";
                                        out << "\n";

                                    }
                                    else
                                    {
                                        fread(&archivo,block.s_block_size,1,file);
                                        out << "block_" << posicion;
                                        out << " [fillcolor=gold,style=\"filled,bold\",shape=record, label=\"Bloque_" << posicion << " ";
                                        out << "| {";
                                        for(int x = 0; x < 64; x++)
                                        {
                                            if(archivo.b_content[x] != NULL)
                                            {
                                                out << archivo.b_content[x];
                                            }
                                        }
                                        out << "}";

                                        out << "\"];";
                                        out << "\n";
                                    }

                                    if(anterior != -1)
                                    {
                                        out << "block_" << anterior << " -> " << "block_" << posicion << ";";
                                        out << "\n";

                                    }
                                    anterior = posicion;
                                }
                            }

                            if(innodo.i_block[12] != -1)
                            {
                                fseek(file,block.s_block_start,SEEK_SET);
                                fseek(file,innodo.i_block[12]*block.s_block_size,SEEK_CUR);
                                fread(&apuntadorSimple,block.s_block_size,1,file);

                                out << "block_" << innodo.i_block[12];
                                out << " [fillcolor=coral,style=\"filled,bold\",shape=record, label=\"Bloque_" << innodo.i_block[12] << " ";
                                for(int x = 0; x < 16; x++)
                                {
                                    out << "| {" << apuntadorSimple.b_pointers[x] << "}";
                                }

                                out << "\"];";
                                out << "\n";

                                if(anterior != -1)
                                {
                                    out << "block_" << anterior << " -> " << "block_" << innodo.i_block[12] << ";";
                                    out << "\n";

                                }
                                anterior = innodo.i_block[12];

                                for(int posicionAp : apuntadorSimple.b_pointers)
                                {
                                    if(posicionAp != -1)
                                    {
                                        fseek(file,block.s_block_start,SEEK_SET);
                                        fseek(file,posicionAp*block.s_block_size,SEEK_CUR);
                                        if(innodo.i_type == '0')
                                        {
                                            fread(&carpeta,block.s_block_size,1,file);
                                            out << "block_" << posicionAp;
                                            out << " [fillcolor=chartreuse2,style=\"filled,bold\",shape=record, label=\"Bloque_" << posicionAp << " ";

                                            for(int x = 0; x < 4; x++)
                                            {
                                                out << "| {" << carpeta.b_content[x].b_name <<"| " << carpeta.b_content[x].b_innodo << "}";
                                            }

                                            out << "\"];";
                                            out << "\n";

                                        }
                                        else
                                        {
                                            fread(&archivo,block.s_block_size,1,file);
                                            out << "block_" << posicionAp;
                                            out << " [fillcolor=gold,style=\"filled,bold\",shape=record, label=\"Bloque_" << posicionAp << " ";
                                            out << "| {";
                                            for(int x = 0; x < 64; x++)
                                            {
                                                if(archivo.b_content[x] != NULL)
                                                {
                                                    out << archivo.b_content[x];
                                                }
                                            }
                                            out << "}";

                                            out << "\"];";
                                            out << "\n";
                                        }

                                        if(anterior != -1)
                                        {
                                            out << "block_" << anterior << " -> " << "block_" << posicionAp << ";";
                                            out << "\n";

                                        }
                                        anterior = posicionAp;
                                    }
                                }
                            }

                            if(innodo.i_block[13] != -1)
                            {
                                fseek(file,block.s_block_start,SEEK_SET);
                                fseek(file,innodo.i_block[13]*block.s_block_size,SEEK_CUR);
                                fread(&apuntadorDoble,block.s_block_size,1,file);

                                out << "block_" << innodo.i_block[13];
                                out << " [fillcolor=coral,style=\"filled,bold\",shape=record, label=\"Bloque_" << innodo.i_block[12] << " ";
                                for(int x = 0; x < 16; x++)
                                {
                                    out << "| {" << apuntadorDoble.b_pointers[x] << "}";
                                }

                                out << "\"];";
                                out << "\n";

                                if(anterior != -1)
                                {
                                    out << "block_" << anterior << " -> " << "block_" << innodo.i_block[13] << ";";
                                    out << "\n";

                                }
                                anterior = innodo.i_block[13];

                                for(int posicionApD : apuntadorDoble.b_pointers)
                                {
                                   if(posicionApD != -1)
                                   {
                                       fseek(file,block.s_block_start,SEEK_SET);
                                       fseek(file,posicionApD*block.s_block_size,SEEK_CUR);
                                       fread(&apuntadorSimple,block.s_block_size,1,file);

                                       out << "block_" << posicionApD;
                                       out << " [fillcolor=coral,style=\"filled,bold\",shape=record, label=\"Bloque_" << posicionApD << " ";
                                       for(int x = 0; x < 16; x++)
                                       {
                                           out << "| {" << posicionApD << "}";
                                       }

                                       out << "\"];";
                                       out << "\n";

                                       if(anterior != -1)
                                       {
                                           out << "block_" << anterior << " -> " << "block_" << posicionApD << ";";
                                           out << "\n";

                                       }
                                       anterior = posicionApD;

                                       for(int posicionAp : apuntadorSimple.b_pointers)
                                       {
                                           if(posicionAp != -1)
                                           {
                                               fseek(file,block.s_block_start,SEEK_SET);
                                               fseek(file,posicionAp*block.s_block_size,SEEK_CUR);
                                               if(innodo.i_type == '0')
                                               {
                                                   fread(&carpeta,block.s_block_size,1,file);
                                                   out << "block_" << posicionAp;
                                                   out << " [fillcolor=chartreuse2,style=\"filled,bold\",shape=record, label=\"Bloque_" << posicionAp << " ";

                                                   for(int x = 0; x < 4; x++)
                                                   {
                                                       out << "| {" << carpeta.b_content[x].b_name <<"| " << carpeta.b_content[x].b_innodo << "}";
                                                   }

                                                   out << "\"];";
                                                   out << "\n";

                                               }
                                               else
                                               {
                                                   fread(&archivo,block.s_block_size,1,file);
                                                   out << "block_" << posicionAp;
                                                   out << " [fillcolor=gold,style=\"filled,bold\",shape=record, label=\"Bloque_" << posicionAp << " ";
                                                   out << "| {";
                                                   for(int x = 0; x < 64; x++)
                                                   {
                                                       if(archivo.b_content[x] != NULL)
                                                       {
                                                           out << archivo.b_content[x];
                                                       }
                                                   }
                                                   out << "}";

                                                   out << "\"];";
                                                   out << "\n";
                                               }

                                               if(anterior != -1)
                                               {
                                                   out << "block_" << anterior << " -> " << "block_" << posicionAp << ";";
                                                   out << "\n";

                                               }
                                               anterior = posicionAp;
                                           }
                                       }
                                   }
                                }
                            }
                        }
                    }

                    fclose(file);
                    out << "}\n";
                }
                else if(par.value("name") == "bm_inode")
                {
                    char byte;
                    int contador = 1;

                    SuperBloque block;
                    file = fopen(mount[partLetra].path.toUtf8().constData(),"rb+");
                    fseek(file,mount[partLetra].numero[partNum].particion.part_start,SEEK_SET);
                    fread(&block,sizeof (struct SuperBloque),1,file);
                    fseek(file,block.s_bm_inode_start,SEEK_SET);

                    for(int x = 0; x < block.s_inodes_count; x++)
                    {
                        fread(&byte,sizeof (char),1,file);
                        out << byte;
                        out << "    ";
                        if(contador > 19)
                        {
                            contador = 0;
                            out << "\n";
                        }
                        contador++;
                    }
                    return;
                }
                else if(par.value("name") == "bm_block")
                {
                    char byte;
                    int contador = 1;

                    SuperBloque block;
                    file = fopen(mount[partLetra].path.toUtf8().constData(),"rb+");
                    fseek(file,mount[partLetra].numero[partNum].particion.part_start,SEEK_SET);
                    fread(&block,sizeof (struct SuperBloque),1,file);
                    fseek(file,block.s_bm_block_start,SEEK_SET);

                    for(int x = 0; x < block.s_blocks_count; x++)
                    {
                        fread(&byte,sizeof (char),1,file);
                        out << byte;
                        out << "    ";
                        if(contador > 19)
                        {
                            contador = 0;
                            out << "\n";
                        }
                        contador++;
                    }
                    return;
                }


                QString comandoS = "dot -T";
                comandoS.append(ext);
                comandoS.append(" ");
                comandoS.append(u);
                comandoS.append(" -o ");
                comandoS.append(par.value("path").toUtf8().constData());
                //cout << comandoS.toStdString() << endl;
//ERROR--------------------------------------------------------------------------------------------------------------
                string g = comandoS.toStdString();
                const char* command = g.c_str();
                //cout<<command<<endl;
                system(command);
            }
        }
    }
}

void Funcionalidad::innodoTree(int padre, int actual, int posicion, ofstream &out, FILE *file, SuperBloque block)
{
    Innodo innodo;
    fseek(file,block.s_inode_start,SEEK_SET);
    fseek(file,actual*block.s_inode_size,SEEK_CUR);
    fread(&innodo,block.s_inode_size,1,file);

    out << "innode_" << actual;
    out << " [fillcolor=darkturquoise,style=\"filled,bold\",shape=record, label=\"Inodo_" << actual << " ";
    out << "| {i_uid | " << innodo.i_uid << "}";
    out << "| {i_gid | " << innodo.i_gid << "}";
    out << "| {i_size | " << innodo.i_size << "}";
    out << "| {i_atime | " << innodo.i_atime << "}";
    out << "| {i_ctime | " << innodo.i_ctime << "}";
    out << "| {i_mtime | " << innodo.i_mtime << "}";
    out << "| {i_type | " << innodo.i_type << "}";
    out << "| {i_perm | " << innodo.i_perm << "}";

    for(int x = 0; x < 15; x++)
    {
        if(innodo.i_block[x] == -1)
        {
            out << "| {AD " << x << "| " << innodo.i_block[x] << "}";
        }
        else {
            out << "| {AD" << x << "| <i" << actual << "ad" << x << "> " << innodo.i_block[x] << "}";
        }
    }

    out << "\"];";
    out << "\n";

    if(posicion != -1)
    {
        //conectamos el bloque con el inodo-------------------------------------------------------------
        out << "block_" << padre << ":b" << padre << "p" << posicion << " -> " << "innode_" << actual << ";";
        out << "\n";
    }

    for(int x = 0; x < 12; x++)
    {
        if(innodo.i_block[x] != -1)
        {
            blockTree(actual,innodo.i_block[x],x, innodo.i_type, out,file,block,false);
        }
    }

    if(innodo.i_block[12] != -1)
    {
        blockTree(actual,innodo.i_block[12],12, '2', out,file,block,false);
    }

    if(innodo.i_block[13] != -1)
    {
        blockTree(actual,innodo.i_block[13],13, '3', out,file,block,false);
    }

    if(innodo.i_block[14] != -1)
    {
        blockTree(actual,innodo.i_block[14],14, '4', out,file,block,false);
    }

}

void Funcionalidad::blockTree(int padre, int actual, int posicion, char tipo, ofstream &out, FILE *file, SuperBloque block, bool esAp)
{
    fseek(file,block.s_block_start,SEEK_SET);
    fseek(file,actual*block.s_block_size,SEEK_CUR);
    if(tipo == '0')
    {
        BloqueCarpeta carpeta;
        fread(&carpeta,block.s_block_size,1,file);

        out << "block_" << actual;
        out << " [fillcolor=chartreuse2,style=\"filled,bold\",shape=record, label=\"Bloque_" << actual << " ";

        for(int x = 0; x < 4; x++)
        {
            out << "| {" << carpeta.b_content[x].b_name <<"| <b"<< actual << "p" << x << "> " << carpeta.b_content[x].b_innodo << "}";
        }

        out << "\"];";
        out << "\n";

        //conectamos el inodo con el bloque-------------------------------------------------------------
        if(!esAp)
        {
            out << "innode_" << padre << ":i" << padre << "ad" << posicion << " -> " << "block_" << actual << ";";
            out << "\n";

            for(int x = 0; x < 4; x++)
            {
                if(posicion == 0)
                {
                    if(x >= 2){
                        if(carpeta.b_content[x].b_innodo != -1)
                        {
                            innodoTree(actual,carpeta.b_content[x].b_innodo,x,out,file,block);
                        }
                    }
                }
                else {
                    if(carpeta.b_content[x].b_innodo != -1)
                    {
                        innodoTree(actual,carpeta.b_content[x].b_innodo,x,out,file,block);
                    }
                }
            }

        }
        else
        {
            out << "block_" << padre << ":b" << padre << "p" << posicion << " -> " << "block_" << actual << ";";
            out << "\n";

            for(int x = 0; x < 4; x++)
            {
                if(carpeta.b_content[x].b_innodo != -1)
                {
                    innodoTree(actual,carpeta.b_content[x].b_innodo,x,out,file,block);
                }
            }
        }
    }
    else if (tipo == '1')
    {
        BloqueArchivo archivo;
        fread(&archivo,block.s_block_size,1,file);

        out << "block_" << actual;
        out << " [fillcolor=gold,style=\"filled,bold\",shape=record, label=\"Bloque_" << actual << " ";

        out << "| {" << "<b"<< actual << "p" << 0 << "> ";
        for(int x = 0; x < 64; x++)
        {
            if(archivo.b_content[x] != NULL)
            {
                out << archivo.b_content[x];
            }
        }
        out << "}";

        out << "\"];";
        out << "\n";

        //conectamos el inodo con el bloque-------------------------------------------------------------
        if(!esAp)
        {
            out << "innode_" << padre << ":i" << padre << "ad" << posicion << " -> " << "block_" << actual << ";";
        }
        else
        {
            out << "block_" << padre << ":b" << padre << "p" << posicion << " -> " << "block_" << actual << ";";
        }
        out << "\n";
    }
    else if (tipo == '2')
    {
        BloqueApuntador apuntador;
        fread(&apuntador,block.s_block_size,1,file);

        out << "block_" << actual;
        out << " [fillcolor=coral,style=\"filled,bold\",shape=record, label=\"Bloque_" << actual << " ";

        for(int x = 0; x < 16; x++)
        {
            out << "| {"  <<" <b"<< actual << "p" << x << "> " << apuntador.b_pointers[x] << "}";
        }

        out << "\"];";
        out << "\n";

        //conectamos el inodo con el bloque-------------------------------------------------------------
        if(!esAp)
        {
            out << "innode_" << padre << ":i" << padre << "ad" << posicion << " -> " << "block_" << actual << ";";
        }
        else
        {
            out << "block_" << padre << ":b" << padre << "p" << posicion << " -> " << "block_" << actual << ";";
        }
        out << "\n";

        for(int x = 0; x < 16; x++)
        {
            if(apuntador.b_pointers[x] != -1)
            {
                blockTree(actual,apuntador.b_pointers[x],x,'0',out,file,block,true);
            }
        }
    }
    else if (tipo == '3')
    {
        BloqueApuntador apuntador;
        fread(&apuntador,block.s_block_size,1,file);

        out << "block_" << actual;
        out << " [fillcolor=coral,style=\"filled,bold\",shape=record, label=\"Bloque_" << actual << " ";

        for(int x = 0; x < 16; x++)
        {
            out << "| {"  <<" <b"<< actual << "p" << x << "> " << apuntador.b_pointers[x] << "}";
        }

        out << "\"];";
        out << "\n";

        //conectamos el inodo con el bloque-------------------------------------------------------------
        out << "innode_" << padre << ":i" << padre << "ad" << posicion << " -> " << "block_" << actual << ";";
        out << "\n";

        for(int x = 0; x < 16; x++)
        {
            if(apuntador.b_pointers[x] != -1)
            {
                blockTree(actual,apuntador.b_pointers[x],x,'2',out,file,block,true);
            }
        }
    }
}


QHash<QString,QString> Funcionalidad::parametros(Nodo *temp)
{
    QHash<QString,QString> par;

    if(temp->hijo.size() == 2){
        par = parametros(temp->hijo.first());
        par.insert(temp->hijo.last()->token, temp->hijo.last()->valor);

    }else if(temp->hijo.size() == 1){
        par.insert(temp->hijo.first()->token, temp->hijo.first()->valor);
    }

    return par;
}

void Funcionalidad::Graficar(Nodo *temp){
    string grafo;
    int contador;
    grafo = "";
    grafo = grafo + "digraph AST{ ";
    grafo = grafo + "nodo0[label=\"token: " + temp->token.toStdString() + "\\n valor: "+ temp->valor.toStdString() + "\"];\r\n";
    contador = 1;
    Graficar("nodo0", temp, grafo,contador);
    grafo = grafo + "}";

    FILE *file;
    file = fopen("Arbol.txt","w");

    if(file == NULL)
       {
          printf("Error!");
          exit(1);
       }

    int n = grafo.length();
    char c[n+1];
    strcpy(c,grafo.c_str());

    fprintf(file,c);
    fclose(file);

}

void Funcionalidad::Graficar(string padre, Nodo *temp, string &grafo, int &contador){
    for(Nodo *hijo : temp->hijo)
    {
        string nameChild = "nodo" + to_string(contador);
        grafo = grafo + nameChild + "[label=\"token: " + hijo->token.toStdString() + "\\n valor: "+ hijo->valor.toStdString() + "\"];\n";
        grafo = grafo + padre + "->" + nameChild + ";\r\n";
        contador++;
        Graficar(nameChild, hijo, grafo, contador);
    }
}



void Funcionalidad::swap(MBR_Partition *xp, MBR_Partition *yp)
{
    MBR_Partition temp = *xp;
    *xp = *yp;
    *yp = temp;
}

// A function to implement bubble sort
void Funcionalidad::bubbleSort(MBR_Partition arr[], int n)
{
    int i, j;
    for (i = 0; i < n-1; i++){

    // Last i elements are already in place
    for (j = 0; j < n-i-1; j++){
        if (arr[j].part_start > arr[j+1].part_start)
            swap(&arr[j], &arr[j+1]);
    }
    }
}

void Funcionalidad::swap(EBR *xp, EBR *yp)
{
    EBR temp = *xp;
    *xp = *yp;
    *yp = temp;
}

// A function to implement bubble sort
void Funcionalidad::bubbleSort(EBR arr[], int n)
{
    int i, j;
    for (i = 0; i < n-1; i++){

    // Last i elements are already in place
    for (j = 0; j < n-i-1; j++){
        if (arr[j].part_start > arr[j+1].part_start)
            swap(&arr[j], &arr[j+1]);
    }
    }
}

/* Function to print an array */
void printArray(int arr[], int size)
{
    int i;
    for (i = 0; i < size; i++)
        cout << arr[i] << " ";
    cout << endl;
}

void Funcionalidad::EjecutarComando(string n){
    if(n != "")
    {
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

                   Graficar(raiz);
                   Ejecutar(raiz);
               }else{
                   cout<<"Se encontraron errores en el comando especificado"<<endl;
               }
    }
}


//METODOS FASE 2--------------------------------------------------------------------------------------------------------

void Funcionalidad::formatearParticion(QHash<QString,QString> par)
{
    int partLetra = ((int) par.value("id").split("")[3].toStdString()[0]) - 97;
    int partNum = par.value("id").split("")[4].toInt() - 1;

    if(mount[partLetra].numero[partNum].montado)//vamos a la posicion montada
    {
        MBR_Partition particion = mount[partLetra].numero[partNum].particion;
        //SUPER BLOQUE-----------------------------------------------------------------------------------------
        SuperBloque block;
        int Num_estructuras;//variable para lso inodos y bloques
        double n, nn;

        if(par.contains("fs"))//tipo de sistema de archivos
        {
            if(par.value("fs") == "3fs")
            {
                block.s_filesystem_type = 3;
                n = (particion.part_size - sizeof(struct SuperBloque));
                nn = (4 + sizeof(struct Innodo) + sizeof(struct Journal) + 3*(sizeof(struct BloqueCarpeta)));
            }
            else{
                block.s_filesystem_type = 2;
                n = (particion.part_size - sizeof(struct SuperBloque));
                nn = (4 + sizeof(struct Innodo) + 3*(sizeof(struct BloqueCarpeta)));
            }
        }
        else{
            block.s_filesystem_type = 2;
            n = (particion.part_size - sizeof(struct SuperBloque));
            nn = (4 + sizeof(struct Innodo) + 3*(sizeof(struct BloqueCarpeta)));
        }

        n = n/nn;
        Num_estructuras = floor(n);

        block.s_inodes_count = Num_estructuras;
        block.s_blocks_count = 3*Num_estructuras;
        block.s_free_inodes_count = Num_estructuras;
        block.s_free_blocks_count = 3*Num_estructuras;

        time_t t = time(0);
        string tiempo = ctime(&t);

        strcpy(block.s_mtime,tiempo.c_str());
        block.s_umtime[0] = '-';
        block.s_umtime[1] = '1';

        block.s_mnt_count = 1;
        block.s_magic = 201700584;
        block.s_inode_size = sizeof (struct Innodo);
        block.s_block_size = sizeof(struct BloqueCarpeta);
        block.s_first_ino = 1;
        block.s_first_blo = 0;

        block.s_bm_inode_start = particion.part_start + sizeof (struct SuperBloque);
        if(block.s_filesystem_type == 3)
        {
            block.s_bm_inode_start = particion.part_start + sizeof (struct SuperBloque) + sizeof (struct Journal);
        }
        block.s_bm_block_start = block.s_bm_inode_start + Num_estructuras*sizeof(char);
        block.s_inode_start = block.s_bm_block_start + 3*Num_estructuras*sizeof (char);
        block.s_block_start = block.s_inode_start + Num_estructuras*sizeof (struct Innodo);
        //-----------------------------------------------------------------------------------------------------

        //Bitmaps---------------------------------------------------------------------------------------------
        char bit = '0';
        char bit1 = '1';

        MBR information;
        FILE *file;
        string g = mount[partLetra].path.toStdString();
        file = fopen(g.c_str(),"rb+");

        fseek(file,block.s_bm_inode_start,SEEK_SET);

        fwrite(&bit1,sizeof(char),1,file);

        for(int n = 1; n < Num_estructuras; n++)//bitmap inodos
        {
            fwrite(&bit,sizeof(char),1,file);
        }

        for(int n = 0; n < 3*Num_estructuras; n++)//bitmap bloques
        {
            fwrite(&bit,sizeof(char),1,file);
        }

        //Innodo root-------------------------------------------------------------------------------
        Innodo in;
        in.i_uid = 1;
        in.i_gid = 1;
        in.i_size = 0;

        t = time(0);
        tiempo = ctime(&t);
        for(int x = 0; x < 30; x++)
        {
            in.i_atime[x] = NULL;
            in.i_ctime[x] = NULL;
            in.i_mtime[x] = NULL;
        }

        for(int y = 0; y < tiempo.size(); y++)
        {
            if(tiempo[y] != NULL && tiempo[y] != '\n')
            {
                in.i_atime[y] = tiempo[y];
                in.i_ctime[y] = tiempo[y];
                in.i_mtime[y] = tiempo[y];
            }
        }

        //strcpy(in.i_atime,tiempo.c_str());
        //strcpy(in.i_ctime,tiempo.c_str());
        //strcpy(in.i_mtime,tiempo.c_str());

        for(int x = 0; x < 15; x++){
            in.i_block[x] = -1;
        }

        in.i_type = '0';
        in.i_perm = 700;

        //---------------------------------------------
        in.i_block[0] = block.s_first_blo;
        marcarBloqueLIbre(block,file);
        block.s_first_blo = primerBloqueLIbre(block,file);
        block.s_free_blocks_count--;

        BloqueCarpeta carpetaNueva;
        for(int g = 0; g < 12; g++)
        {
            carpetaNueva.b_content[0].b_name[g] = NULL;
            carpetaNueva.b_content[1].b_name[g] = NULL;
            carpetaNueva.b_content[2].b_name[g] = NULL;
            carpetaNueva.b_content[3].b_name[g] = NULL;
        }

        int actual,padre;

        padre = 0;
        actual = 0;

        carpetaNueva.b_content[0].b_name[0] = '.';
        carpetaNueva.b_content[0].b_innodo = actual;

        carpetaNueva.b_content[1].b_name[0] = '.';
        carpetaNueva.b_content[1].b_name[1] = '.';
        carpetaNueva.b_content[1].b_innodo = padre;//padre

        char cont[64];

        for(int x = 0; x < 64; x++)
        {
            cont[x] = NULL;
        }
        string c = "1,G,root \n 1,U,root,root,123 \n ";
        for(int x = 0; x < c.size(); x++)
        {
            cont[x] = c[x];
        }

        int nInnodo = block.s_first_ino;//primer innodo libre
        marcarInnodoLibre(block,file);//marcamos que se uso
        block.s_first_ino = primerInnodoLibre(block,file);//guardamos el nuevo innodo libre
        block.s_free_inodes_count--;

        carpetaNueva.b_content[2].b_innodo = nInnodo;
        string name = "users.txt";

        for(int g = 0; g < 12; g++)
        {
            carpetaNueva.b_content[2].b_name[g] = NULL;
        }
        for(int z = 0; z < name.size(); z++)
        {
            carpetaNueva.b_content[2].b_name[z] = name[z];
        }

        Innodo nuevo = nuevoInnodo('1');

        //creamos el primer bloque con padre y actual-------------------------------------------------------
        nuevo.i_block[0] = block.s_first_blo;
        marcarBloqueLIbre(block,file);
        block.s_first_blo = primerBloqueLIbre(block,file);
        block.s_free_blocks_count--;


        BloqueArchivo archivoNuevo;
        for(int g = 0; g < 64; g++)
        {
            archivoNuevo.b_content[g] = cont[g];
        }

        fseek(file,block.s_inode_start,SEEK_SET);
        fseek(file,carpetaNueva.b_content[2].b_innodo*block.s_inode_size,SEEK_CUR);
        fwrite(&nuevo,sizeof (struct Innodo),1,file);

        fseek(file,block.s_block_start,SEEK_SET);
        fseek(file,nuevo.i_block[0]*block.s_block_size,SEEK_CUR);
        fwrite(&archivoNuevo,block.s_block_size,1,file);

        fseek(file,block.s_block_start,SEEK_SET);
        fseek(file,in.i_block[0]*block.s_block_size,SEEK_CUR);
        fwrite(&carpetaNueva,block.s_block_size,1,file);
        //---------------------------------------------

        fseek(file,particion.part_start,SEEK_SET);
        fwrite(&block,sizeof(struct SuperBloque),1,file);

        fseek(file,block.s_inode_start,SEEK_SET);
        fwrite(&in,sizeof (struct Innodo),1,file);



        fclose(file);

    }
    else {
        cout << "La particion no esta montada" << endl;
    }
}

int Funcionalidad::buscarInnodo(QStringList pathAux)
{
    //pathAux.removeLast();
    //pathAux.removeFirst();

    string url = usuario.pathDisco.toStdString();
    const char *u = url.c_str();

    FILE *file;
    file = fopen(u,"rb+");

    Innodo innodo;
    MBR mbr;
    SuperBloque block;

    fread(&mbr, sizeof (struct MBR),1,file);
    fseek(file,usuario.particion.part_start,SEEK_SET);
    fread(&block,sizeof (struct SuperBloque),1,file);
    fseek(file,block.s_inode_start,SEEK_SET);
    fread(&innodo,block.s_inode_size,1,file);

    int x = -1;

    x = buscarInnodo(0, pathAux,0,block,file);

    fclose(file);
    return x;
}

int Funcionalidad::buscarInnodo(int numInnodo, QStringList path,int posicionPath, SuperBloque block, FILE *file)
{
    int numRetorno = -1;//variable que indicara si encontramos el innodo
    if(posicionPath < path.size())//si llegamos a la ultima pos esa devolvemos
    {
        Innodo innodo;
        fseek(file,block.s_inode_start,SEEK_SET);
        fseek(file, numInnodo*block.s_inode_size,SEEK_CUR);
        fread(&innodo,block.s_inode_size,1,file);//tomamos el innodo

        for(int x = 0; x < 12; x++)//Moverse en los bloques directos
        {
            if(innodo.i_block[x] != -1)//buscaremos unicamente en los bloques usados
            {
                int num = innodo.i_block[x];
                fseek(file,block.s_block_start,SEEK_SET);//inicio del area bloques
                fseek(file, num*block.s_block_size,SEEK_CUR);// nos movemos al bloque

                    BloqueCarpeta carpeta;
                    fread(&carpeta,block.s_block_size,1,file);//leemos el bloque

                    for(int y = 0; y < 4; y++)
                    {

                        QString name = "";
                        for(char c : carpeta.b_content[y].b_name)
                        {
                            if(c != NULL)
                            {
                            name.append(c);
                            }
                        }

                        if(name == path.value(posicionPath))//si la carpeta contiene lo que buscamos
                        {
                            posicionPath++;
                            numRetorno = buscarInnodo(carpeta.b_content[y].b_innodo,path,posicionPath,block,file);

                            if(numRetorno != -1)//si ya encontramos el innodo retornamos el numero
                            {
                                return numRetorno;
                            }
                        }
                    }
            }
        }

        if(numRetorno == -1 && innodo.i_block[12] != -1)//Bloque de apuntadores simple
        {
            BloqueApuntador apuntador;
            int numAp = innodo.i_block[12];
            fseek(file,block.s_block_start,SEEK_SET);//inicio del area bloques
            fseek(file, numAp*block.s_block_size,SEEK_CUR);// nos movemos al bloque
            fread(&apuntador,block.s_block_size,1,file);

            for(int x = 0; x < 16; x++)
            {
                if(apuntador.b_pointers[x] != -1)
                {
                    int numCarpeta = apuntador.b_pointers[x];
                    fseek(file,block.s_block_start,SEEK_SET);//inicio del area bloques
                    fseek(file, numCarpeta*block.s_block_size,SEEK_CUR);// nos movemos al bloque

                        BloqueCarpeta carpeta;
                        fread(&carpeta,block.s_block_size,1,file);//leemos el bloque

                        for(int y = 0; y < 4; y++)
                        {

                            QString name = "";
                            for(char c : carpeta.b_content[y].b_name)
                            {
                                if(c != NULL)
                                {
                                name.append(c);
                                }
                            }

                            if(name == path.value(posicionPath))//si la carpeta contiene lo que buscamos
                            {
                                posicionPath++;
                                numRetorno = buscarInnodo(carpeta.b_content[y].b_innodo,path,posicionPath,block,file);

                                if(numRetorno != -1)//si ya encontramos el innodo retornamos el numero
                                {
                                    return numRetorno;
                                }
                            }
                        }
                }
            }

        }

        if(numRetorno == -1 && innodo.i_block[13] != -1)
        {
            BloqueApuntador apuntadorDoble;
            int numApDoble = innodo.i_block[13];
            fseek(file,block.s_block_start,SEEK_SET);//inicio del area bloques
            fseek(file, numApDoble*block.s_block_size,SEEK_CUR);// nos movemos al bloque
            fread(&apuntadorDoble,block.s_block_size,1,file);

            for(int posD = 0; posD < 16; posD++)
            {
                if(apuntadorDoble.b_pointers[posD] != -1)
                {
                    BloqueApuntador apuntador;
                    int numAp = apuntadorDoble.b_pointers[posD];
                    fseek(file,block.s_block_start,SEEK_SET);//inicio del area bloques
                    fseek(file, numAp*block.s_block_size,SEEK_CUR);// nos movemos al bloque
                    fread(&apuntador,block.s_block_size,1,file);

                    for(int x = 0; x < 16; x++)
                    {
                        if(apuntador.b_pointers[x] != -1)
                        {
                            int numCarpeta = apuntador.b_pointers[x];
                            fseek(file,block.s_block_start,SEEK_SET);//inicio del area bloques
                            fseek(file, numCarpeta*block.s_block_size,SEEK_CUR);// nos movemos al bloque

                                BloqueCarpeta carpeta;
                                fread(&carpeta,block.s_block_size,1,file);//leemos el bloque

                                for(int y = 0; y < 4; y++)
                                {

                                    QString name = "";
                                    for(char c : carpeta.b_content[y].b_name)
                                    {
                                        if(c != NULL)
                                        {
                                        name.append(c);
                                        }
                                    }

                                    if(name == path.value(posicionPath))//si la carpeta contiene lo que buscamos
                                    {
                                        posicionPath++;
                                        numRetorno = buscarInnodo(carpeta.b_content[y].b_innodo,path,posicionPath,block,file);

                                        if(numRetorno != -1)//si ya encontramos el innodo retornamos el numero
                                        {
                                            return numRetorno;
                                        }
                                    }
                                }
                        }
                    }
                }
            }
        }
    }
    else{
        return numInnodo;
    }
    return numRetorno;
}

void Funcionalidad::crearCarpeta(int numInnodoPadre, QString name, QStringList path)
{
    string url = usuario.pathDisco.toStdString();
    const char *u = url.c_str();
    FILE *file;
    file = fopen(u,"rb+");

    Innodo innodo;
    SuperBloque block;

    fseek(file,usuario.particion.part_start,SEEK_SET);
    fread(&block,sizeof (struct SuperBloque),1,file);
    fseek(file,block.s_inode_start,SEEK_SET);
    fseek(file,numInnodoPadre*sizeof (struct Innodo),SEEK_CUR);
    fread(&innodo,block.s_inode_size,1,file);//Tomamos el innodo donde crearemos la carpeta

    //Primero buscamos si no existe ya la carpeta a crear---------------------------------------------------------
    string nameAux = name.toStdString();
    for(int n = 0; n < 12; n++)
    {
        if(innodo.i_block[n] != -1)
        {
            BloqueCarpeta carpeta;
            fseek(file,block.s_block_start,SEEK_SET);
            fseek(file,innodo.i_block[n]*block.s_block_size,SEEK_CUR);
            fread(&carpeta,block.s_block_size,1,file);//

            if(n == 0)
            {
                for(int pos = 2; pos < 4; pos++)
                {
                    if(carpeta.b_content[pos].b_innodo != -1)
                    {
                        if(carpeta.b_content[pos].b_name == nameAux)
                        {
                            fclose(file);
                            cout << "la carpeta " << nameAux << " ya existe" << endl;
                            return;
                        }
                    }
                }
            }
            else
            {
                for(int pos = 0; pos < 4; pos++)
                {
                    if(carpeta.b_content[pos].b_innodo != -1)
                    {
                        if(carpeta.b_content[pos].b_innodo != -1)
                        {
                            if(carpeta.b_content[pos].b_name == nameAux)
                            {
                                fclose(file);
                                cout << "la carpeta " << nameAux << " ya existe" << endl;
                                return;
                            }
                        }
                    }
                }
            }
        }
    }
    //------------------------------------------------------------------------------------------------------------

    bool indirecto = true;
    int espacioDisponible = -1;
    for(int x = 0; x < 12; x++)//iremos a travez de los bloques directos buscando espacio
    {
        if(innodo.i_block[x] != -1)//si existe un bloque creado
        {
            BloqueCarpeta carpeta;
            fseek(file,block.s_block_start,SEEK_SET);
            fseek(file,innodo.i_block[x]*block.s_block_size,SEEK_CUR);
            fread(&carpeta,block.s_block_size,1,file);//

            for(int y = 0; y < 4; y++)
            {
                if(carpeta.b_content[y].b_innodo == -1)//si hay espacio en el bloque
                {
                    int nInnodo = block.s_first_ino;//primer innodo libre
                    marcarInnodoLibre(block,file);//marcamos que se uso
                    block.s_first_ino = primerInnodoLibre(block,file);//guardamos el nuevo innodo libre
                    block.s_free_inodes_count--;

                    carpeta.b_content[y].b_innodo = nInnodo;
                    string n = name.toStdString();

                    for(int g = 0; g < 12; g++)
                    {
                        carpeta.b_content[y].b_name[g] = NULL;
                    }
                    for(int z = 0; z < n.size(); z++)
                    {
                        carpeta.b_content[y].b_name[z] = n[z];
                    }

                    Innodo nuevo = nuevoInnodo('0');

                    //creamos el primer bloque con padre y actual-------------------------------------------------------
                    nuevo.i_block[0] = block.s_first_blo;
                    marcarBloqueLIbre(block,file);
                    block.s_first_blo = primerBloqueLIbre(block,file);
                    block.s_free_blocks_count--;

                    BloqueCarpeta carpetaNueva;
                    for(int g = 0; g < 12; g++)
                    {
                        carpetaNueva.b_content[0].b_name[g] = NULL;
                        carpetaNueva.b_content[1].b_name[g] = NULL;
                        carpetaNueva.b_content[2].b_name[g] = NULL;
                        carpetaNueva.b_content[3].b_name[g] = NULL;
                    }

                    int actual,padre;

                    if(path.size() <= 1)
                    {
                        actual = 0;
                        padre = 0;
                    }
                    else
                    {
                        actual = buscarInnodo(path);
                        path.removeLast();
                        padre = buscarInnodo(path);
                    }

                    carpetaNueva.b_content[0].b_name[0] = '.';
                    carpetaNueva.b_content[0].b_innodo = actual;

                    carpetaNueva.b_content[1].b_name[0] = '.';
                    carpetaNueva.b_content[1].b_name[1] = '.';
                    carpetaNueva.b_content[1].b_innodo = padre;//padre

                    fseek(file,block.s_block_start,SEEK_SET);
                    fseek(file,nuevo.i_block[0]*block.s_block_size,SEEK_CUR);
                    fwrite(&carpetaNueva,block.s_block_size,1,file);
                    //--------------------------------------------------------------------------------------------------

                    fseek(file,block.s_inode_start,SEEK_SET);
                    fseek(file,nInnodo*block.s_inode_size,SEEK_CUR);
                    fwrite(&nuevo,block.s_inode_size,1,file);//escribimos el nuevo innodo

                    fseek(file,block.s_block_start,SEEK_SET);
                    fseek(file,innodo.i_block[x]*block.s_block_size,SEEK_CUR);
                    fwrite(&carpeta,block.s_block_size,1,file);//escribimos el bloque carpeta actualizada

                    fseek(file,usuario.particion.part_start,SEEK_SET);
                    fwrite(&block,sizeof(struct SuperBloque),1,file);
                    fclose(file);
                    return;
                }
            }

        }
        else if(innodo.i_block[x] == -1 && espacioDisponible == -1)
        {
            espacioDisponible = x;
            indirecto = false;
        }
    }

    if(espacioDisponible != -1)
    {
        //creamos un bloque carpeta-----------------------------------------------------------
        BloqueCarpeta carpeta;
        fseek(file,block.s_block_start,SEEK_SET);
       int numBloque = block.s_first_blo;
       marcarBloqueLIbre(block,file);
       block.s_first_blo = primerBloqueLIbre(block, file);
       block.s_free_blocks_count--;

        for(int g = 0; g < 12; g++)
        {
            carpeta.b_content[0].b_name[g] = NULL;
            carpeta.b_content[1].b_name[g] = NULL;
            carpeta.b_content[2].b_name[g] = NULL;
            carpeta.b_content[3].b_name[g] = NULL;
        }
        //-----------------------------------------------------------------------------------

        //marcamos que usamos un bloque------------------------------------------------------
        innodo.i_block[espacioDisponible] = numBloque;
        //-----------------------------------------------------------------------------------
        //insertamos el nuevo innodo---------------------------------------------------------
        int nInnodo = block.s_first_ino;//primer innodo libre
        marcarInnodoLibre(block,file);//marcamos que se uso
        block.s_first_ino = primerInnodoLibre(block,file);//guardamos el nuevo innodo libre
        block.s_free_inodes_count--;

        carpeta.b_content[0].b_innodo = nInnodo;
        string n = name.toStdString();
        for(int z = 0; z < n.size(); z++)
        {
            carpeta.b_content[0].b_name[z] = n[z];
        }

        Innodo nuevo = nuevoInnodo('0');

        //------------------------------------------------------------------------------------------
        nuevo.i_block[0] = block.s_first_blo;
        marcarBloqueLIbre(block,file);
        block.s_first_blo = primerBloqueLIbre(block,file);
        block.s_free_blocks_count--;

        BloqueCarpeta carpetaNueva;
        for(int g = 0; g < 12; g++)
        {
            carpetaNueva.b_content[0].b_name[g] = NULL;
            carpetaNueva.b_content[1].b_name[g] = NULL;
            carpetaNueva.b_content[2].b_name[g] = NULL;
            carpetaNueva.b_content[3].b_name[g] = NULL;
        }

        int actual,padre;

        if(path.size() <= 1)
        {
            actual = 0;
            padre = 0;
        }
        else
        {
            actual = buscarInnodo(path);
            path.removeLast();
            padre = buscarInnodo(path);
        }

        carpetaNueva.b_content[0].b_name[0] = '.';
        carpetaNueva.b_content[0].b_innodo = actual;

        carpetaNueva.b_content[1].b_name[0] = '.';
        carpetaNueva.b_content[1].b_name[1] = '.';
        carpetaNueva.b_content[1].b_innodo = padre;//padre

        fseek(file,block.s_block_start,SEEK_SET);
        fseek(file,nuevo.i_block[0]*block.s_block_size,SEEK_CUR);
        fwrite(&carpetaNueva,block.s_block_size,1,file);
        //------------------------------------------------------------------------------------------

        fseek(file,block.s_inode_start,SEEK_SET);
        fseek(file,numInnodoPadre*block.s_inode_size,SEEK_CUR);
        fwrite(&innodo,block.s_inode_size,1,file);//escribimos el innodo actualizado

        fseek(file,block.s_inode_start,SEEK_SET);
        fseek(file,nInnodo*block.s_inode_size,SEEK_CUR);
        fwrite(&nuevo,block.s_inode_size,1,file);//escribimos el nuevo innodo

        fseek(file,block.s_block_start,SEEK_SET);
        fseek(file,numBloque*block.s_block_size,SEEK_CUR);
        fwrite(&carpeta,block.s_block_size,1,file);//escribimos el bloque carpeta actualizada

        fseek(file,usuario.particion.part_start,SEEK_SET);
        fwrite(&block,sizeof(struct SuperBloque),1,file);
        //-------------------------------------------------------------------------------------
        fclose(file);
        return;
    }

    if(indirecto)
    {
        bool inDoble = true;
        bool inTriple = true;

        if(innodo.i_block[12] != -1)//Bloque simple directo
        {
            BloqueApuntador apuntador;
            fseek(file,block.s_block_start,SEEK_SET);
            fseek(file,innodo.i_block[12]*block.s_block_size,SEEK_CUR);
            fread(&apuntador,block.s_block_size,1,file);

            for(int x = 0; x < 16; x++)
            {
                if(apuntador.b_pointers[x] != -1)
                {

                    BloqueCarpeta carpeta;
                    fseek(file,block.s_block_start,SEEK_SET);
                    fseek(file,apuntador.b_pointers[x]*block.s_block_size,SEEK_CUR);
                    fread(&carpeta,block.s_block_size,1,file);//

                    for(int y = 0; y < 4; y++)
                    {
                        if(carpeta.b_content[y].b_innodo == -1)//si hay espacio en el bloque
                        {
                            inDoble = false;
                            inTriple = false;

                            int nInnodo = block.s_first_ino;//primer innodo libre
                            marcarInnodoLibre(block,file);//marcamos que se uso
                            block.s_first_ino = primerInnodoLibre(block,file);//guardamos el nuevo innodo libre
                            block.s_free_inodes_count--;

                            carpeta.b_content[y].b_innodo = nInnodo;
                            string n = name.toStdString();

                            for(int g = 0; g < 12; g++)
                            {
                                carpeta.b_content[y].b_name[g] = NULL;
                            }
                            for(int z = 0; z < n.size(); z++)
                            {
                                carpeta.b_content[y].b_name[z] = n[z];
                            }

                            Innodo nuevo = nuevoInnodo('0');

                            //creamos el primer bloque con padre y actual-------------------------------------------------------
                            nuevo.i_block[0] = block.s_first_blo;
                            marcarBloqueLIbre(block,file);
                            block.s_first_blo = primerBloqueLIbre(block,file);
                            block.s_free_blocks_count--;

                            BloqueCarpeta carpetaNueva;
                            for(int g = 0; g < 12; g++)
                            {
                                carpetaNueva.b_content[0].b_name[g] = NULL;
                                carpetaNueva.b_content[1].b_name[g] = NULL;
                                carpetaNueva.b_content[2].b_name[g] = NULL;
                                carpetaNueva.b_content[3].b_name[g] = NULL;
                            }

                            int actual,padre;

                            if(path.size() <= 1)
                            {
                                actual = 0;
                                padre = 0;
                            }
                            else
                            {
                                actual = buscarInnodo(path);
                                path.removeLast();
                                padre = buscarInnodo(path);
                            }

                            carpetaNueva.b_content[0].b_name[0] = '.';
                            carpetaNueva.b_content[0].b_innodo = actual;

                            carpetaNueva.b_content[1].b_name[0] = '.';
                            carpetaNueva.b_content[1].b_name[1] = '.';
                            carpetaNueva.b_content[1].b_innodo = padre;//padre

                            fseek(file,block.s_block_start,SEEK_SET);
                            fseek(file,nuevo.i_block[0]*block.s_block_size,SEEK_CUR);
                            fwrite(&carpetaNueva,block.s_block_size,1,file);
                            //--------------------------------------------------------------------------------------------------

                            fseek(file,block.s_inode_start,SEEK_SET);
                            fseek(file,nInnodo*block.s_inode_size,SEEK_CUR);
                            fwrite(&nuevo,block.s_inode_size,1,file);//escribimos el nuevo innodo

                            fseek(file,block.s_block_start,SEEK_SET);
                            fseek(file,apuntador.b_pointers[x]*block.s_block_size,SEEK_CUR);
                            fwrite(&carpeta,block.s_block_size,1,file);//escribimos el bloque carpeta actualizada

                            fseek(file,usuario.particion.part_start,SEEK_SET);
                            fwrite(&block,sizeof(struct SuperBloque),1,file);
                            fclose(file);
                            return;
                        }
                    }
                }
                else if(apuntador.b_pointers[x] == -1 && espacioDisponible == -1){
                    espacioDisponible = x;
                    inDoble = false;
                    inTriple = false;
                }
            }

            if(espacioDisponible != -1)
            {
                //creamos un bloque carpeta-----------------------------------------------------------
               BloqueCarpeta carpeta;
               fseek(file,block.s_block_start,SEEK_SET);
               int numBloque = block.s_first_blo;
               marcarBloqueLIbre(block,file);
               block.s_first_blo = primerBloqueLIbre(block, file);
               block.s_free_blocks_count--;

                for(int g = 0; g < 12; g++)
                {
                    carpeta.b_content[0].b_name[g] = NULL;
                    carpeta.b_content[1].b_name[g] = NULL;
                    carpeta.b_content[2].b_name[g] = NULL;
                    carpeta.b_content[3].b_name[g] = NULL;
                }
                //-----------------------------------------------------------------------------------

                //marcamos que usamos un bloque------------------------------------------------------
                apuntador.b_pointers[espacioDisponible] = numBloque;
                //-----------------------------------------------------------------------------------
                //insertamos el nuevo innodo---------------------------------------------------------
                int nInnodo = block.s_first_ino;//primer innodo libre
                marcarInnodoLibre(block,file);//marcamos que se uso
                block.s_first_ino = primerInnodoLibre(block,file);//guardamos el nuevo innodo libre
                block.s_free_inodes_count--;

                carpeta.b_content[0].b_innodo = nInnodo;
                string n = name.toStdString();
                for(int z = 0; z < n.size(); z++)
                {
                    carpeta.b_content[0].b_name[z] = n[z];
                }

                Innodo nuevo = nuevoInnodo('0');

                //------------------------------------------------------------------------------------------
                nuevo.i_block[0] = block.s_first_blo;
                marcarBloqueLIbre(block,file);
                block.s_first_blo = primerBloqueLIbre(block,file);
                block.s_free_blocks_count--;

                BloqueCarpeta carpetaNueva;
                for(int g = 0; g < 12; g++)
                {
                    carpetaNueva.b_content[0].b_name[g] = NULL;
                    carpetaNueva.b_content[1].b_name[g] = NULL;
                    carpetaNueva.b_content[2].b_name[g] = NULL;
                    carpetaNueva.b_content[3].b_name[g] = NULL;
                }

                int actual,padre;

                if(path.size() <= 1)
                {
                    actual = 0;
                    padre = 0;
                }
                else
                {
                    actual = buscarInnodo(path);
                    path.removeLast();
                    padre = buscarInnodo(path);
                }

                carpetaNueva.b_content[0].b_name[0] = '.';
                carpetaNueva.b_content[0].b_innodo = actual;

                carpetaNueva.b_content[1].b_name[0] = '.';
                carpetaNueva.b_content[1].b_name[1] = '.';
                carpetaNueva.b_content[1].b_innodo = padre;//padre

                fseek(file,block.s_block_start,SEEK_SET);
                fseek(file,nuevo.i_block[0]*block.s_block_size,SEEK_CUR);
                fwrite(&carpetaNueva,block.s_block_size,1,file);
                //------------------------------------------------------------------------------------------

                fseek(file,block.s_block_start,SEEK_SET);
                fseek(file,innodo.i_block[12]*block.s_block_size,SEEK_CUR);
                fwrite(&apuntador,block.s_block_size,1,file);//escribimos el innodo actualizado

                fseek(file,block.s_inode_start,SEEK_SET);
                fseek(file,nInnodo*block.s_inode_size,SEEK_CUR);
                fwrite(&nuevo,block.s_inode_size,1,file);//escribimos el nuevo innodo

                fseek(file,block.s_block_start,SEEK_SET);
                fseek(file,numBloque*block.s_block_size,SEEK_CUR);
                fwrite(&carpeta,block.s_block_size,1,file);//escribimos el bloque carpeta actualizada

                fseek(file,usuario.particion.part_start,SEEK_SET);
                fwrite(&block,sizeof(struct SuperBloque),1,file);
                //-------------------------------------------------------------------------------------
                fclose(file);
                return;
            }

        }
        else {
            inDoble = false;
            inTriple = false;
            //Apuntador---------------------------------------------------------------------------
            BloqueApuntador apuntador = nuevoApuntador();//nuestro bloque de apuntadores simple
            int numBloqueApuntador = block.s_first_blo;
            marcarBloqueLIbre(block,file);
            block.s_first_blo = primerBloqueLIbre(block, file);
            block.s_free_blocks_count--;

            innodo.i_block[12] = numBloqueApuntador;

            //Carpeta-----------------------------------------------------------------------------
            BloqueCarpeta carpeta;
            int numBloque = block.s_first_blo;
            marcarBloqueLIbre(block,file);
            block.s_first_blo = primerBloqueLIbre(block, file);
            block.s_free_blocks_count--;

            apuntador.b_pointers[0] = numBloque;

            for(int g = 0; g < 12; g++)
            {
                carpeta.b_content[0].b_name[g] = NULL;
                carpeta.b_content[1].b_name[g] = NULL;
                carpeta.b_content[2].b_name[g] = NULL;
                carpeta.b_content[3].b_name[g] = NULL;
            }

            //insertamos el nuevo innodo---------------------------------------------------------
            int nInnodo = block.s_first_ino;//primer innodo libre
            marcarInnodoLibre(block,file);//marcamos que se uso
            block.s_first_ino = primerInnodoLibre(block,file);//guardamos el nuevo innodo libre
            block.s_free_inodes_count--;

            carpeta.b_content[0].b_innodo = nInnodo;
            string n = name.toStdString();
            for(int z = 0; z < n.size(); z++)
            {
                carpeta.b_content[0].b_name[z] = n[z];
            }

            Innodo nuevo = nuevoInnodo('0');

            //------------------------------------------------------------------------------------------
            nuevo.i_block[0] = block.s_first_blo;
            marcarBloqueLIbre(block,file);
            block.s_first_blo = primerBloqueLIbre(block,file);
            block.s_free_blocks_count--;

            BloqueCarpeta carpetaNueva;
            for(int g = 0; g < 12; g++)
            {
                carpetaNueva.b_content[0].b_name[g] = NULL;
                carpetaNueva.b_content[1].b_name[g] = NULL;
                carpetaNueva.b_content[2].b_name[g] = NULL;
                carpetaNueva.b_content[3].b_name[g] = NULL;
            }

            int actual,padre;

            if(path.size() <= 1)
            {
                actual = 0;
                padre = 0;
            }
            else
            {
                actual = buscarInnodo(path);
                path.removeLast();
                padre = buscarInnodo(path);
            }

            carpetaNueva.b_content[0].b_name[0] = '.';
            carpetaNueva.b_content[0].b_innodo = actual;

            carpetaNueva.b_content[1].b_name[0] = '.';
            carpetaNueva.b_content[1].b_name[1] = '.';
            carpetaNueva.b_content[1].b_innodo = padre;//padre

            fseek(file,block.s_block_start,SEEK_SET);
            fseek(file,nuevo.i_block[0]*block.s_block_size,SEEK_CUR);
            fwrite(&carpetaNueva,block.s_block_size,1,file);
            //------------------------------------------------------------------------------------------

            fseek(file,block.s_block_start,SEEK_SET);
            fseek(file,numBloqueApuntador*block.s_block_size,SEEK_CUR);
            fwrite(&apuntador,block.s_block_size,1,file);

            fseek(file,block.s_inode_start,SEEK_SET);
            fseek(file,numInnodoPadre*block.s_inode_size,SEEK_CUR);
            fwrite(&innodo,block.s_inode_size,1,file);//escribimos el innodo actualizado

            fseek(file,block.s_inode_start,SEEK_SET);
            fseek(file,nInnodo*block.s_inode_size,SEEK_CUR);
            fwrite(&nuevo,block.s_inode_size,1,file);//escribimos el nuevo innodo

            fseek(file,block.s_block_start,SEEK_SET);
            fseek(file,numBloque*block.s_block_size,SEEK_CUR);
            fwrite(&carpeta,block.s_block_size,1,file);//escribimos el bloque carpeta actualizada

            fseek(file,usuario.particion.part_start,SEEK_SET);
            fwrite(&block,sizeof(struct SuperBloque),1,file);
            //-------------------------------------------------------------------------------------
            fclose(file);
            return;
        }

        if(innodo.i_block[13] != -1 && inDoble)
        {
            BloqueApuntador apuntadorDoble;
            fseek(file,block.s_block_start,SEEK_SET);
            fseek(file,innodo.i_block[13]*block.s_block_size,SEEK_CUR);
            fread(&apuntadorDoble,block.s_block_size,1,file);

            for(int posD = 0; posD < 16; posD++)
            {
                if(apuntadorDoble.b_pointers[posD] != -1)//apuntador doble contiene algo
                {
                    BloqueApuntador apuntador;
                    fseek(file,block.s_block_start,SEEK_SET);
                    fseek(file,apuntadorDoble.b_pointers[posD]*block.s_block_size,SEEK_CUR);
                    fread(&apuntador,block.s_block_size,1,file);

                    for(int x = 0; x < 16; x++)
                    {
                        if(apuntador.b_pointers[x] != -1)
                        {
                            BloqueCarpeta carpeta;
                            fseek(file,block.s_block_start,SEEK_SET);
                            fseek(file,apuntador.b_pointers[x]*block.s_block_size,SEEK_CUR);
                            fread(&carpeta,block.s_block_size,1,file);//

                            for(int y = 0; y < 4; y++)
                            {
                                if(carpeta.b_content[y].b_innodo == -1)//si hay espacio en el bloque
                                {
                                    inTriple = false;

                                    int nInnodo = block.s_first_ino;//primer innodo libre
                                    marcarInnodoLibre(block,file);//marcamos que se uso
                                    block.s_first_ino = primerInnodoLibre(block,file);//guardamos el nuevo innodo libre
                                    block.s_free_inodes_count--;

                                    carpeta.b_content[y].b_innodo = nInnodo;
                                    string n = name.toStdString();

                                    for(int g = 0; g < 12; g++)
                                    {
                                        carpeta.b_content[y].b_name[g] = NULL;
                                    }
                                    for(int z = 0; z < n.size(); z++)
                                    {
                                        carpeta.b_content[y].b_name[z] = n[z];
                                    }

                                    Innodo nuevo = nuevoInnodo('0');

                                    //creamos el primer bloque con padre y actual-------------------------------------------------------
                                    nuevo.i_block[0] = block.s_first_blo;
                                    marcarBloqueLIbre(block,file);
                                    block.s_first_blo = primerBloqueLIbre(block,file);
                                    block.s_free_blocks_count--;

                                    BloqueCarpeta carpetaNueva;
                                    for(int g = 0; g < 12; g++)
                                    {
                                        carpetaNueva.b_content[0].b_name[g] = NULL;
                                        carpetaNueva.b_content[1].b_name[g] = NULL;
                                        carpetaNueva.b_content[2].b_name[g] = NULL;
                                        carpetaNueva.b_content[3].b_name[g] = NULL;
                                    }

                                    int actual,padre;

                                    if(path.size() <= 1)
                                    {
                                        actual = 0;
                                        padre = 0;
                                    }
                                    else
                                    {
                                        actual = buscarInnodo(path);
                                        path.removeLast();
                                        padre = buscarInnodo(path);
                                    }

                                    carpetaNueva.b_content[0].b_name[0] = '.';
                                    carpetaNueva.b_content[0].b_innodo = actual;

                                    carpetaNueva.b_content[1].b_name[0] = '.';
                                    carpetaNueva.b_content[1].b_name[1] = '.';
                                    carpetaNueva.b_content[1].b_innodo = padre;//padre

                                    fseek(file,block.s_block_start,SEEK_SET);
                                    fseek(file,nuevo.i_block[0]*block.s_block_size,SEEK_CUR);
                                    fwrite(&carpetaNueva,block.s_block_size,1,file);
                                    //--------------------------------------------------------------------------------------------------

                                    fseek(file,block.s_inode_start,SEEK_SET);
                                    fseek(file,nInnodo*block.s_inode_size,SEEK_CUR);
                                    fwrite(&nuevo,block.s_inode_size,1,file);//escribimos el nuevo innodo

                                    fseek(file,block.s_block_start,SEEK_SET);
                                    fseek(file,apuntador.b_pointers[x]*block.s_block_size,SEEK_CUR);
                                    fwrite(&carpeta,block.s_block_size,1,file);//escribimos el bloque carpeta actualizada

                                    fseek(file,usuario.particion.part_start,SEEK_SET);
                                    fwrite(&block,sizeof(struct SuperBloque),1,file);
                                    fclose(file);
                                    return;
                                }
                            }
                        }
                        else if(apuntador.b_pointers[x] == -1 && espacioDisponible == -1){
                            espacioDisponible = x;
                            inDoble = false;
                            inTriple = false;
                        }
                    }

                    if(espacioDisponible != -1)
                    {
                        //creamos un bloque carpeta-----------------------------------------------------------
                       BloqueCarpeta carpeta;
                       fseek(file,block.s_block_start,SEEK_SET);
                       int numBloqueCarpeta = block.s_first_blo;
                       marcarBloqueLIbre(block,file);
                       block.s_first_blo = primerBloqueLIbre(block, file);
                       block.s_free_blocks_count--;

                        for(int g = 0; g < 12; g++)
                        {
                            carpeta.b_content[0].b_name[g] = NULL;
                            carpeta.b_content[1].b_name[g] = NULL;
                            carpeta.b_content[2].b_name[g] = NULL;
                            carpeta.b_content[3].b_name[g] = NULL;
                        }
                        //-----------------------------------------------------------------------------------

                        //marcamos que usamos un bloque------------------------------------------------------
                        apuntador.b_pointers[espacioDisponible] = numBloqueCarpeta;
                        //-----------------------------------------------------------------------------------
                        //insertamos el nuevo innodo---------------------------------------------------------
                        int nInnodo = block.s_first_ino;//primer innodo libre
                        marcarInnodoLibre(block,file);//marcamos que se uso
                        block.s_first_ino = primerInnodoLibre(block,file);//guardamos el nuevo innodo libre
                        block.s_free_inodes_count--;

                        carpeta.b_content[0].b_innodo = nInnodo;
                        string n = name.toStdString();
                        for(int z = 0; z < n.size(); z++)
                        {
                            carpeta.b_content[0].b_name[z] = n[z];
                        }

                        Innodo nuevo = nuevoInnodo('0');

                        //------------------------------------------------------------------------------------------
                        nuevo.i_block[0] = block.s_first_blo;
                        marcarBloqueLIbre(block,file);
                        block.s_first_blo = primerBloqueLIbre(block,file);
                        block.s_free_blocks_count--;

                        BloqueCarpeta carpetaNueva;
                        for(int g = 0; g < 12; g++)
                        {
                            carpetaNueva.b_content[0].b_name[g] = NULL;
                            carpetaNueva.b_content[1].b_name[g] = NULL;
                            carpetaNueva.b_content[2].b_name[g] = NULL;
                            carpetaNueva.b_content[3].b_name[g] = NULL;
                        }

                        int actual,padre;

                        if(path.size() <= 1)
                        {
                            actual = 0;
                            padre = 0;
                        }
                        else
                        {
                            actual = buscarInnodo(path);
                            path.removeLast();
                            padre = buscarInnodo(path);
                        }

                        carpetaNueva.b_content[0].b_name[0] = '.';
                        carpetaNueva.b_content[0].b_innodo = actual;

                        carpetaNueva.b_content[1].b_name[0] = '.';
                        carpetaNueva.b_content[1].b_name[1] = '.';
                        carpetaNueva.b_content[1].b_innodo = padre;//padre

                        fseek(file,block.s_block_start,SEEK_SET);
                        fseek(file,nuevo.i_block[0]*block.s_block_size,SEEK_CUR);
                        fwrite(&carpetaNueva,block.s_block_size,1,file);
                        //------------------------------------------------------------------------------------------

                        fseek(file,block.s_block_start,SEEK_SET);
                        fseek(file,apuntadorDoble.b_pointers[posD]*block.s_block_size,SEEK_CUR);
                        fwrite(&apuntador,block.s_block_size,1,file);//escribimos el innodo actualizado

                        fseek(file,block.s_inode_start,SEEK_SET);
                        fseek(file,nInnodo*block.s_inode_size,SEEK_CUR);
                        fwrite(&nuevo,block.s_inode_size,1,file);//escribimos el nuevo innodo

                        fseek(file,block.s_block_start,SEEK_SET);
                        fseek(file,numBloqueCarpeta*block.s_block_size,SEEK_CUR);
                        fwrite(&carpeta,block.s_block_size,1,file);//escribimos el bloque carpeta actualizada

                        fseek(file,usuario.particion.part_start,SEEK_SET);
                        fwrite(&block,sizeof(struct SuperBloque),1,file);
                        //-------------------------------------------------------------------------------------
                        fclose(file);
                        return;
                    }
                }
                else if(apuntadorDoble.b_pointers[posD] == -1 && espacioDisponible == -1){
                    espacioDisponible = posD;
                    inDoble = false;
                    inTriple = false;
                }
            }

            if(espacioDisponible != -1)
            {
                BloqueApuntador apuntador = nuevoApuntador();//nuestro bloque de apuntadores simple
                int numBloqueApuntador = block.s_first_blo;
                marcarBloqueLIbre(block,file);
                block.s_first_blo = primerBloqueLIbre(block, file);
                block.s_free_blocks_count--;

                apuntadorDoble.b_pointers[espacioDisponible] = numBloqueApuntador;

                //creamos un bloque carpeta-----------------------------------------------------------
               BloqueCarpeta carpeta;
               fseek(file,block.s_block_start,SEEK_SET);
               int numBloque = block.s_first_blo;
               marcarBloqueLIbre(block,file);
               block.s_first_blo = primerBloqueLIbre(block, file);
               block.s_free_blocks_count--;

                for(int g = 0; g < 12; g++)
                {
                    carpeta.b_content[0].b_name[g] = NULL;
                    carpeta.b_content[1].b_name[g] = NULL;
                    carpeta.b_content[2].b_name[g] = NULL;
                    carpeta.b_content[3].b_name[g] = NULL;
                }
                //-----------------------------------------------------------------------------------

                //marcamos que usamos un bloque------------------------------------------------------
                apuntador.b_pointers[0] = numBloque;
                //-----------------------------------------------------------------------------------
                //insertamos el nuevo innodo---------------------------------------------------------
                int nInnodo = block.s_first_ino;//primer innodo libre
                marcarInnodoLibre(block,file);//marcamos que se uso
                block.s_first_ino = primerInnodoLibre(block,file);//guardamos el nuevo innodo libre
                block.s_free_inodes_count--;

                carpeta.b_content[0].b_innodo = nInnodo;
                string n = name.toStdString();
                for(int z = 0; z < n.size(); z++)
                {
                    carpeta.b_content[0].b_name[z] = n[z];
                }

                Innodo nuevo = nuevoInnodo('0');

                //------------------------------------------------------------------------------------------
                nuevo.i_block[0] = block.s_first_blo;
                marcarBloqueLIbre(block,file);
                block.s_first_blo = primerBloqueLIbre(block,file);
                block.s_free_blocks_count--;

                BloqueCarpeta carpetaNueva;
                for(int g = 0; g < 12; g++)
                {
                    carpetaNueva.b_content[0].b_name[g] = NULL;
                    carpetaNueva.b_content[1].b_name[g] = NULL;
                    carpetaNueva.b_content[2].b_name[g] = NULL;
                    carpetaNueva.b_content[3].b_name[g] = NULL;
                }

                int actual,padre;

                if(path.size() <= 1)
                {
                    actual = 0;
                    padre = 0;
                }
                else
                {
                    actual = buscarInnodo(path);
                    path.removeLast();
                    padre = buscarInnodo(path);
                }

                carpetaNueva.b_content[0].b_name[0] = '.';
                carpetaNueva.b_content[0].b_innodo = actual;

                carpetaNueva.b_content[1].b_name[0] = '.';
                carpetaNueva.b_content[1].b_name[1] = '.';
                carpetaNueva.b_content[1].b_innodo = padre;//padre

                fseek(file,block.s_block_start,SEEK_SET);
                fseek(file,nuevo.i_block[0]*block.s_block_size,SEEK_CUR);
                fwrite(&carpetaNueva,block.s_block_size,1,file);
                //------------------------------------------------------------------------------------------

                fseek(file,block.s_block_start,SEEK_SET);
                fseek(file,innodo.i_block[13]*block.s_block_size,SEEK_CUR);
                fwrite(&apuntadorDoble,block.s_block_size,1,file);//escribimos el apuntador actualizado

                fseek(file,block.s_block_start,SEEK_SET);
                fseek(file,numBloqueApuntador*block.s_block_size,SEEK_CUR);
                fwrite(&apuntador,block.s_block_size,1,file);//escribimos el apuntador actualizado

                fseek(file,block.s_inode_start,SEEK_SET);
                fseek(file,nInnodo*block.s_inode_size,SEEK_CUR);
                fwrite(&nuevo,block.s_inode_size,1,file);//escribimos el nuevo innodo

                fseek(file,block.s_block_start,SEEK_SET);
                fseek(file,numBloque*block.s_block_size,SEEK_CUR);
                fwrite(&carpeta,block.s_block_size,1,file);//escribimos el bloque carpeta actualizada

                fseek(file,usuario.particion.part_start,SEEK_SET);
                fwrite(&block,sizeof(struct SuperBloque),1,file);
                //-------------------------------------------------------------------------------------
                fclose(file);
                return;
            }
        }
        else {
            inTriple = false;
            //ApuntadorDoble----------------------------------------------------------------------
            BloqueApuntador apuntador = nuevoApuntador();//nuestro bloque de apuntadores simple
            int numBloqueApuntador = block.s_first_blo;
            marcarBloqueLIbre(block,file);
            block.s_first_blo = primerBloqueLIbre(block, file);
            block.s_free_blocks_count--;

            innodo.i_block[13] = numBloqueApuntador;

            //Apuntador---------------------------------------------------------------------------
            BloqueApuntador nuevoAp = nuevoApuntador();//nuestro bloque de apuntadores simple
            int numNuevoBloqueApuntador = block.s_first_blo;
            marcarBloqueLIbre(block,file);
            block.s_first_blo = primerBloqueLIbre(block, file);
            block.s_free_blocks_count--;

            apuntador.b_pointers[0] = numNuevoBloqueApuntador;

            //Carpeta-----------------------------------------------------------------------------
            BloqueCarpeta carpeta;
            int numBloque = block.s_first_blo;
            marcarBloqueLIbre(block,file);
            block.s_first_blo = primerBloqueLIbre(block, file);
            block.s_free_blocks_count--;

            nuevoAp.b_pointers[0] = numBloque;

            for(int g = 0; g < 12; g++)
            {
                carpeta.b_content[0].b_name[g] = NULL;
                carpeta.b_content[1].b_name[g] = NULL;
                carpeta.b_content[2].b_name[g] = NULL;
                carpeta.b_content[3].b_name[g] = NULL;
            }

            //insertamos el nuevo innodo---------------------------------------------------------
            int nInnodo = block.s_first_ino;//primer innodo libre
            marcarInnodoLibre(block,file);//marcamos que se uso
            block.s_first_ino = primerInnodoLibre(block,file);//guardamos el nuevo innodo libre
            block.s_free_inodes_count--;

            carpeta.b_content[0].b_innodo = nInnodo;
            string n = name.toStdString();
            for(int z = 0; z < n.size(); z++)
            {
                carpeta.b_content[0].b_name[z] = n[z];
            }

            Innodo nuevo = nuevoInnodo('0');

            //------------------------------------------------------------------------------------------
            nuevo.i_block[0] = block.s_first_blo;
            marcarBloqueLIbre(block,file);
            block.s_first_blo = primerBloqueLIbre(block,file);
            block.s_free_blocks_count--;

            BloqueCarpeta carpetaNueva;
            for(int g = 0; g < 12; g++)
            {
                carpetaNueva.b_content[0].b_name[g] = NULL;
                carpetaNueva.b_content[1].b_name[g] = NULL;
                carpetaNueva.b_content[2].b_name[g] = NULL;
                carpetaNueva.b_content[3].b_name[g] = NULL;
            }

            int actual,padre;

            if(path.size() <= 1)
            {
                actual = 0;
                padre = 0;
            }
            else
            {
                actual = buscarInnodo(path);
                path.removeLast();
                padre = buscarInnodo(path);
            }

            carpetaNueva.b_content[0].b_name[0] = '.';
            carpetaNueva.b_content[0].b_innodo = actual;

            carpetaNueva.b_content[1].b_name[0] = '.';
            carpetaNueva.b_content[1].b_name[1] = '.';
            carpetaNueva.b_content[1].b_innodo = padre;//padre

            fseek(file,block.s_block_start,SEEK_SET);
            fseek(file,nuevo.i_block[0]*block.s_block_size,SEEK_CUR);
            fwrite(&carpetaNueva,block.s_block_size,1,file);
            //------------------------------------------------------------------------------------------

            fseek(file,block.s_block_start,SEEK_SET);
            fseek(file,numBloqueApuntador*block.s_block_size,SEEK_CUR);
            fwrite(&apuntador,block.s_block_size,1,file);

            fseek(file,block.s_block_start,SEEK_SET);
            fseek(file,numNuevoBloqueApuntador*block.s_block_size,SEEK_CUR);
            fwrite(&nuevoAp,block.s_block_size,1,file);

            fseek(file,block.s_inode_start,SEEK_SET);
            fseek(file,numInnodoPadre*block.s_inode_size,SEEK_CUR);
            fwrite(&innodo,block.s_inode_size,1,file);//escribimos el innodo actualizado

            fseek(file,block.s_inode_start,SEEK_SET);
            fseek(file,nInnodo*block.s_inode_size,SEEK_CUR);
            fwrite(&nuevo,block.s_inode_size,1,file);//escribimos el nuevo innodo

            fseek(file,block.s_block_start,SEEK_SET);
            fseek(file,numBloque*block.s_block_size,SEEK_CUR);
            fwrite(&carpeta,block.s_block_size,1,file);//escribimos el bloque carpeta actualizada

            fseek(file,usuario.particion.part_start,SEEK_SET);
            fwrite(&block,sizeof(struct SuperBloque),1,file);
            //-------------------------------------------------------------------------------------
            fclose(file);
            return;
        }

        if(innodo.i_block[14] != -1 && inTriple)
        {

        }
        else {

        }
    }

    fclose(file);

}

void Funcionalidad::crearArchivo(int numInnodoPadre,QString name,QStringList path, char contenido[64])
{
    string url = usuario.pathDisco.toStdString();
    const char *u = url.c_str();
    FILE *file;
    file = fopen(u,"rb+");

    Innodo innodo;
    SuperBloque block;

    fseek(file,usuario.particion.part_start,SEEK_SET);
    fread(&block,sizeof (struct SuperBloque),1,file);
    fseek(file,block.s_inode_start,SEEK_SET);
    fseek(file,numInnodoPadre*sizeof (struct Innodo),SEEK_CUR);
    fread(&innodo,block.s_inode_size,1,file);//Tomamos el innodo donde crearemos el archivo
    char tipo = innodo.i_type;

    //Primero buscamos si no existe ya el archivo a crear---------------------------------------------------------
    string nameAux = name.toStdString();

    if(tipo == '0')
    {
        for(int n = 0; n < 12; n++)
        {
            if(innodo.i_block[n] != -1)
            {
                BloqueCarpeta carpeta;
                fseek(file,block.s_block_start,SEEK_SET);
                fseek(file,innodo.i_block[n]*block.s_block_size,SEEK_CUR);
                fread(&carpeta,block.s_block_size,1,file);//

                if(n == 0)
                {
                    for(int pos = 2; pos < 4; pos++)
                    {
                        if(carpeta.b_content[pos].b_innodo != -1)
                        {
                            if(carpeta.b_content[pos].b_name == nameAux)
                            {
                                fclose(file);
                                cout << "el archivo " << nameAux << " ya existe" << endl;
                                return;
                            }
                        }
                    }
                }
                else
                {
                    for(int pos = 0; pos < 4; pos++)
                    {
                        if(carpeta.b_content[pos].b_innodo != -1)
                        {
                            if(carpeta.b_content[pos].b_innodo != -1)
                            {
                                if(carpeta.b_content[pos].b_name == nameAux)
                                {
                                    fclose(file);
                                    cout << "el archivo " << nameAux << " ya existe" << endl;
                                    return;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    //------------------------------------------------------------------------------------------------------------

    //Si estamos por crearla o ya esta creada
    if(tipo == '0')
    {
        bool indirecto = true;
        int espacioDisponible = -1;
        for(int x = 0; x < 12; x++)//iremos a travez de los bloques directos buscando espacio
        {
            if(innodo.i_block[x] != -1)//si existe un bloque creado
            {
                BloqueCarpeta carpeta;
                fseek(file,block.s_block_start,SEEK_SET);
                fseek(file,innodo.i_block[x]*block.s_block_size,SEEK_CUR);
                fread(&carpeta,block.s_block_size,1,file);//

                for(int y = 0; y < 4; y++)
                {
                    if(carpeta.b_content[y].b_innodo == -1)//si hay espacio en el bloque
                    {
                        int nInnodo = block.s_first_ino;//primer innodo libre
                        marcarInnodoLibre(block,file);//marcamos que se uso
                        block.s_first_ino = primerInnodoLibre(block,file);//guardamos el nuevo innodo libre
                        block.s_free_inodes_count--;

                        carpeta.b_content[y].b_innodo = nInnodo;
                        string n = name.toStdString();

                        for(int g = 0; g < 12; g++)
                        {
                            carpeta.b_content[y].b_name[g] = NULL;
                        }
                        for(int z = 0; z < n.size(); z++)
                        {
                            carpeta.b_content[y].b_name[z] = n[z];
                        }

                        Innodo nuevo = nuevoInnodo('1');

                        //creamos el primer bloque con padre y actual-------------------------------------------------------
                        nuevo.i_block[0] = block.s_first_blo;
                        marcarBloqueLIbre(block,file);
                        block.s_first_blo = primerBloqueLIbre(block,file);
                        block.s_free_blocks_count--;


                        BloqueArchivo archivoNuevo;
                        for(int g = 0; g < 64; g++)
                        {
                            archivoNuevo.b_content[g] = contenido[g];
                        }

                        fseek(file,block.s_block_start,SEEK_SET);
                        fseek(file,nuevo.i_block[0]*block.s_block_size,SEEK_CUR);
                        fwrite(&archivoNuevo,block.s_block_size,1,file);
                        //--------------------------------------------------------------------------------------------------

                        fseek(file,block.s_inode_start,SEEK_SET);
                        fseek(file,nInnodo*block.s_inode_size,SEEK_CUR);
                        fwrite(&nuevo,block.s_inode_size,1,file);//escribimos el nuevo innodo

                        fseek(file,block.s_block_start,SEEK_SET);
                        fseek(file,innodo.i_block[x]*block.s_block_size,SEEK_CUR);
                        fwrite(&carpeta,block.s_block_size,1,file);//escribimos el bloque carpeta actualizada

                        fseek(file,usuario.particion.part_start,SEEK_SET);
                        fwrite(&block,sizeof(struct SuperBloque),1,file);
                        fclose(file);
                        return;
                    }
                }

            }
            else if(innodo.i_block[x] == -1 && espacioDisponible == -1)
            {
                espacioDisponible = x;
                indirecto = false;
            }
        }

        if(espacioDisponible != -1)
        {
            //creamos un bloque carpeta-----------------------------------------------------------
            BloqueCarpeta carpeta;
            fseek(file,block.s_block_start,SEEK_SET);
           int numBloque = block.s_first_blo;
           marcarBloqueLIbre(block,file);
           block.s_first_blo = primerBloqueLIbre(block, file);
           block.s_free_blocks_count--;

            for(int g = 0; g < 12; g++)
            {
                carpeta.b_content[0].b_name[g] = NULL;
                carpeta.b_content[1].b_name[g] = NULL;
                carpeta.b_content[2].b_name[g] = NULL;
                carpeta.b_content[3].b_name[g] = NULL;
            }
            //-----------------------------------------------------------------------------------

            //marcamos que usamos un bloque------------------------------------------------------
            innodo.i_block[espacioDisponible] = numBloque;
            //-----------------------------------------------------------------------------------
            //insertamos el nuevo innodo---------------------------------------------------------
            int nInnodo = block.s_first_ino;//primer innodo libre
            marcarInnodoLibre(block,file);//marcamos que se uso
            block.s_first_ino = primerInnodoLibre(block,file);//guardamos el nuevo innodo libre
            block.s_free_inodes_count--;

            carpeta.b_content[0].b_innodo = nInnodo;
            string n = name.toStdString();
            for(int z = 0; z < n.size(); z++)
            {
                carpeta.b_content[0].b_name[z] = n[z];
            }

            Innodo nuevo = nuevoInnodo('1');

            //------------------------------------------------------------------------------------------
            nuevo.i_block[0] = block.s_first_blo;
            marcarBloqueLIbre(block,file);
            block.s_first_blo = primerBloqueLIbre(block,file);
            block.s_free_blocks_count--;

            BloqueArchivo archivoNuevo;
            for(int g = 0; g < 64; g++)
            {
                archivoNuevo.b_content[g] = contenido[g];
            }


            fseek(file,block.s_block_start,SEEK_SET);
            fseek(file,nuevo.i_block[0]*block.s_block_size,SEEK_CUR);
            fwrite(&archivoNuevo,block.s_block_size,1,file);
            //------------------------------------------------------------------------------------------

            fseek(file,block.s_inode_start,SEEK_SET);
            fseek(file,numInnodoPadre*block.s_inode_size,SEEK_CUR);
            fwrite(&innodo,block.s_inode_size,1,file);//escribimos el innodo actualizado

            fseek(file,block.s_inode_start,SEEK_SET);
            fseek(file,nInnodo*block.s_inode_size,SEEK_CUR);
            fwrite(&nuevo,block.s_inode_size,1,file);//escribimos el nuevo innodo

            fseek(file,block.s_block_start,SEEK_SET);
            fseek(file,numBloque*block.s_block_size,SEEK_CUR);
            fwrite(&carpeta,block.s_block_size,1,file);//escribimos el bloque carpeta actualizada

            fseek(file,usuario.particion.part_start,SEEK_SET);
            fwrite(&block,sizeof(struct SuperBloque),1,file);
            //-------------------------------------------------------------------------------------
            fclose(file);
            return;
        }

        if(indirecto)
        {

        }
    }
    else if (tipo == '1')
    {
        bool indirecto = true;
        int espacioDisponible = -1;
        for(int x = 1; x < 12; x++)//iremos a travez de los bloques directos buscando espacio
        {
            if(innodo.i_block[x] == -1)
            {
                indirecto = false;

                innodo.i_block[x] = block.s_first_blo;
                marcarBloqueLIbre(block,file);
                block.s_first_blo = primerBloqueLIbre(block,file);
                block.s_free_blocks_count--;

                BloqueArchivo archivoNuevo;
                for(int g = 0; g < 64; g++)
                {
                    archivoNuevo.b_content[g] = contenido[g];
                }


                fseek(file,block.s_block_start,SEEK_SET);
                fseek(file,innodo.i_block[x]*block.s_block_size,SEEK_CUR);
                fwrite(&archivoNuevo,block.s_block_size,1,file);

                fseek(file,block.s_inode_start,SEEK_SET);
                fseek(file,numInnodoPadre*block.s_inode_size,SEEK_CUR);
                fwrite(&innodo,block.s_inode_size,1,file);//escribimos el innodo actualizado

                fseek(file,usuario.particion.part_start,SEEK_SET);
                fwrite(&block,sizeof(struct SuperBloque),1,file);
                fclose(file);
                return;
            }
        }
    }
    fclose(file);
}

QString Funcionalidad::obtenerContenidoArchivo(int numInnodo)
{
    QString contenido = "";

    string url = usuario.pathDisco.toStdString();
    const char *u = url.c_str();
    FILE *file;
    file = fopen(u,"rb+");

    Innodo innodo;
    SuperBloque block;
    fseek(file,usuario.particion.part_start,SEEK_SET);
    fread(&block,sizeof (struct SuperBloque),1,file);
    fseek(file,block.s_inode_start,SEEK_SET);
    fseek(file,numInnodo*block.s_inode_size,SEEK_CUR);
    fread(&innodo,block.s_inode_size,1,file);

    for(int x = 0; x < 15; x++)
    {
        if(innodo.i_block[x] != -1)
        {
            BloqueArchivo archivo;
            fseek(file,block.s_block_start,SEEK_SET);
            fseek(file,innodo.i_block[x]*block.s_block_size,SEEK_CUR);
            fread(&archivo,block.s_block_size,1,file);
            QString contAux(archivo.b_content);
            contenido.append(contAux);
        }
    }
    fclose(file);

    return contenido;
}

//Metodos de ayuda-----------------------------------------------------------------------------
QStringList Funcionalidad::pathArchivo(QString path){
    QStringList lista = path.split("/");
    return lista;
}

int Funcionalidad::primerBloqueLIbre(SuperBloque block, FILE *file)
{
    char byte;
    fseek(file,block.s_bm_block_start,SEEK_SET);

    for(int x = 0; x < block.s_blocks_count; x++)
    {
        fread(&byte,sizeof (char),1,file);
        if(byte == '0')
        {
            return x;
        }
    }
}

int Funcionalidad::primerInnodoLibre(SuperBloque block, FILE *file)
{
    char byte;
    fseek(file,block.s_bm_inode_start,SEEK_SET);

    for(int x = 0; x < block.s_inodes_count; x++)
    {
        fread(&byte,sizeof (char),1,file);
        if(byte == '0')
        {
            return x;
        }
    }
}

void Funcionalidad::marcarBloqueLIbre(SuperBloque block, FILE *file)
{
    char byte;
    fseek(file,block.s_bm_block_start,SEEK_SET);

    for(int x = 0; x < block.s_blocks_count; x++)
    {
        fread(&byte,sizeof (char),1,file);
        if(byte == '0')
        {
            byte = '1';
            fseek(file,-sizeof (char),SEEK_CUR);
            fwrite(&byte,sizeof (char),1,file);
            return;
        }
    }
}

void Funcionalidad::marcarInnodoLibre(SuperBloque block, FILE *file)
{
    char byte;
    fseek(file,block.s_bm_inode_start,SEEK_SET);

    for(int x = 0; x < block.s_inodes_count; x++)
    {
        fread(&byte,sizeof (char),1,file);
        if(byte == '0')
        {
            byte = '1';
            fseek(file,-sizeof (char),SEEK_CUR);
            fwrite(&byte,sizeof (char),1,file);
            return;
        }
    }
}

BloqueApuntador Funcionalidad::nuevoApuntador()
{
    BloqueApuntador apuntador;

    for(int x = 0; x < 16; x++)
    {
        apuntador.b_pointers[x] = -1;
    }

    return apuntador;
}

Innodo Funcionalidad::nuevoInnodo(char tipo)
{
    Innodo in;
    time_t t = time(0);
    string tiempo = ctime(&t);

    in.i_uid = 1;
    in.i_gid = 1;
    in.i_size = 0;

    t = time(0);
    tiempo = ctime(&t);

    for(int x = 0; x < 30; x++)
    {
        in.i_atime[x] = NULL;
        in.i_ctime[x] = NULL;
        in.i_mtime[x] = NULL;
    }

    for(int y = 0; y < tiempo.size(); y++)
    {
        if(tiempo[y] != NULL && tiempo[y] != '\n')
        {
            in.i_atime[y] = tiempo[y];
            in.i_ctime[y] = tiempo[y];
            in.i_mtime[y] = tiempo[y];
        }
    }

    /*
    strcpy(in.i_atime,tiempo.c_str());
    strcpy(in.i_ctime,tiempo.c_str());
    strcpy(in.i_mtime,tiempo.c_str());
    */

    for(int x = 0; x < 15; x++){
        in.i_block[x] = -1;
    }

    in.i_type = tipo;
    in.i_perm = 700;

    return in;
}




