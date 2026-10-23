//Pseudocodigo:
//Creamos un nuevo string dinamico con malloc
//Recorro el primer string y lo pongo en el nuevo dinamico
//Recorro el segundo string y lo pongo en el nuevo dinamico 
//Tengo que devolver el nuevo string 
//
//
//
//
//
//
//

int len(char* src1){//Definimos la funcion auxiliar len para poder sacar la longitud de los strings sin tener en cuenta el 0 final
    int i = 0;
    while (src1[i] != '\0') i++;
    return i;
}

char * strConcatenate(char* src1, char* src2){
    int i;
    int largo1 = len(src1);
    int largo2 = len(src2);
    int largo = largo1 + largo2;
    
    char * new = (char*) malloc (sizeof(char) * (largo + 1) );
    if (new== NULL ){
        return NULL;
    }
    for (i=0; i<largo1; i++){
        new[i] = src1[i]; 
    }
    for (i=largo1; i<largo; i++){
        new[i]= src2[i-largo1];
    }
    new[largo] = '\0';
    free (src1);
    free(src2);

    return new;
}



   