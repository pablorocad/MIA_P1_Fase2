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

            if(mount[partLetra].numero[partNum].montado)
            {

                MBR_Partition particion =  mount[partLetra].numero[partNum].particion;

                FILE *file;

                file = fopen(mount[partLetra].path.toUtf8().constData(),"rb+");
                MBR information2;
                fread(&information2,sizeof(struct MBR),1,file);
                fclose(file);

                ofstream out;

                out.open("/home/pablorocad/Documentos/Rep/dis.dot");

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
                    fread(&t,sizeof (char),1,file);

                    cout << "";
                    fclose(file);

                }

                QString ext = par.value("path").split(".")[1];
                //const char* ext = e.c_str();

                QString comandoS = "dot -T";
                comandoS.append(ext);
                comandoS.append(" /home/pablorocad/Documentos/Rep/dis.dot");
                comandoS.append(" -o ");
                comandoS.append(par.value("path").toUtf8().constData());
                //cout << comandoS.toStdString() << endl;
//ERROR--------------------------------------------------------------------------------------------------------------
                string g = comandoS.toStdString();
                const char* command = g.c_str();
                cout<<command<<endl;
                system(command);
            }


        }

        /*MBR information;
        FILE *file;
        file = fopen(par.value("path").toUtf8().constData(),"rb");
        fread(&information,sizeof(struct MBR),1,file);

        cout << "------MBR-----" << endl;
        cout << "tamano: " << information.mbr_tamano << endl;
        cout << "fecha: " << information.mbr_fecha_creacion << endl;
        cout << "signature: " << information.mbr_disk_signature << endl;
        cout << "fit: " << information.disk_fit << endl;
        cout << "--------------" << endl;

        EBR ext[20];
        fseek(file,sizeof (struct MBR)+1,SEEK_SET);
        fread(&ext,sizeof(ext),1,file);
        cout << "-----------------";

        fclose(file);*/


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
                   Funcionalidad *f;
                   f->Graficar(raiz);
                   f->Ejecutar(raiz);
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

        block.s_mnt_count = 1;
        block.s_magic = 201700584;
        block.s_inode_size = sizeof (struct Innodo);
        block.s_block_size = sizeof(struct BloqueCarpeta);
        block.s_firts_ino = 1;
        block.s_first_blo = 1;

        block.s_bm_inode_start = particion.part_start + sizeof (struct SuperBloque);
        if(block.s_filesystem_type == 3)
        {
            block.s_bm_inode_start = particion.part_start + sizeof (struct SuperBloque) + sizeof (struct Journal);
        }
        block.s_bm_block_start = block.s_bm_inode_start + Num_estructuras*sizeof(char);
        block.s_inode_start = block.s_bm_block_start + 3*Num_estructuras*sizeof (char);
        block.s_block_start = block.s_inode_start + Num_estructuras*sizeof (struct Innodo);
        //-----------------------------------------------------------------------------------------------------

        char bit = '0';
        char bit1 = '1';

        MBR information;
        FILE *file;
        string g = mount[partLetra].path.toStdString();
        file = fopen(g.c_str(),"rb+");

        fseek(file,particion.part_start,SEEK_SET);
        fwrite(&block,sizeof(struct SuperBloque),1,file);

        fwrite(&bit1,sizeof(char),1,file);
        fwrite(&bit,sizeof(char),Num_estructuras-1,file);//bitmap inodos
        fwrite(&bit1,sizeof(char),1,file);
        fwrite(&bit,sizeof(char),3*Num_estructuras-1,file);//bitmap bloques

        //Innodo root y bloque root-------------------------------------------------------------------------------
        Innodo in;
        in.i_uid = 1;
        in.i_gid = 1;
        in.i_size = 0;

        t = time(0);
        tiempo = ctime(&t);
        strcpy(in.i_atime,tiempo.c_str());
        strcpy(in.i_ctime,tiempo.c_str());
        strcpy(in.i_mtime,tiempo.c_str());

        for(int x = 0; x < 15; x++){
            in.i_block[x] = -1;
        }

        in.i_type = 0;
        in.i_perm = 700;



        fclose(file);

    }
    else {
        cout << "La particion no esta montada" << endl;
    }
}


