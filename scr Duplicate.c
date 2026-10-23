

char *str Duplicate (char* scr)
    int cont = 0;
    for(int i = 0; scr[i]!= 0; i++){
        cont += 1;
    }
    char * dupli = (char*) malloc(sizeof(char)*(cont + 1));
    int ubi= 0
    for (int i = 0; i < cont; i++){
        dupli[ubi]= src[i];
        ubi += 1
    }
    dupli[ubi] = "\0" 