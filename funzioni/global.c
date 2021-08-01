
char * toUpper(char *input){
    int i;
    for(i=0;i<strlen(input);i++)
        if((int)input[i] >= 97 && (int)input[i] <= 122) //lettera minuscola
            input[i] = input[i] ^ 32; //Applico l'operatore XOR tra input e 2^5(32) modificando il sesto bit
    return input;
}

char * getInput(ushort len_stringa){
    char * input = Calloc(1, len_stringa * sizeof(char));
    do{
        if(fgets(input, len_stringa, stdin))
            *(strchr(input, '\n')) = 0; //Restituisce l'indirizzo dove trova \n e sostituisce questo char col char \0.               
        else{
            puts("fgets Error. LETTO: NULL.");
            return NULL;
        }
    }           
    while((strcmp(input, "") == 0) || input[0] == ' '); 

    return input; 
}

short stampaLista(struct lista * head, unsigned short mode){
    
    struct lista * ptr = head->next; //Parto dall'elemento successivo alla sentinella (head).
    
    if (ptr == NULL){ 
        puts("Lista vuota: nessun elemento inserito."); 
        return FALSE;
    }

    
    if (mode == 0){ // Stampa solo il campo nome
        puts("Lista Ristoranti:");
        while (ptr != NULL){
            printf("Nome = \"%s\"\n", ptr->nome);
            ptr = ptr->next;
        }
    }
    else if (mode == 1){ //stampa anche il campo numero
        puts("Lista Pietanze:");
        while (ptr != NULL){
            
            printf("Nome = \"%s\"\t € %0.2f\n", ptr->nome, ptr->numero);
            ptr = ptr->next;
        }
    }
    else if(mode == 2){
        puts("Lista Rider:");
         while (ptr != NULL){
            printf("Nome = \"%s\"\t ID: %d\n", ptr->nome, (int)ptr->numero);
            ptr = ptr->next;
        }
    }
    puts("");
    
    return TRUE;
}

void freeLista(struct lista * head){
    struct lista * tmp = head;
    struct lista * next;

    while(tmp != NULL){
        next = tmp->next;
        free(tmp);
        tmp = next;
    } 
}

short cancellaNodo(float fd, struct lista * head, struct lista ** tail){
    struct lista *ptr = head->next;
    struct lista *precedente = head;

    while (ptr != NULL){
        if(ptr->numero == fd){
            if (ptr != *tail) // se ptr non è l'ultimo elemento;
                precedente->next = ptr->next; // taglio il collegamento tra precedente e ptr.
            else if(ptr == *tail){ // se ptr è l'ultimo elemento
                *tail = precedente;
                (*tail)->next = NULL;
            }
            free(ptr);
            return TRUE;
        }
        else{
            precedente = ptr;
            ptr = ptr->next;
        }
    }
    return FALSE;
}

void registraLista(char * nome_ris, float sockfd , struct lista ** tail){ 
    //Creo il nodo da aggiungere alla lista
    struct lista * newRis = (struct lista *) Calloc(1, sizeof(struct lista)); 
    strcpy(newRis->nome, nome_ris);
    newRis->numero = sockfd;
    
    //Aggiungo il nodo all'ultimo posto della coda
    newRis->next    = (*tail)->next;
    (*tail)->next   = newRis; 
    *tail           = newRis; 
}

void inviaLista(float sockfd, struct lista * head){
    struct lista * ptr = head->next; 
    while (ptr != NULL){
    

        FullWrite((int)sockfd, ptr, sizeof(struct lista));

        ptr = ptr->next;
    }
    ptr = (struct lista *)Calloc(1, sizeof(struct lista));

    strcpy(ptr->nome, delimitatoreFine);
    ptr->numero = 0;
   
   
    FullWrite((int)sockfd, ptr, sizeof(struct lista));

}

short leggi_registraLista(float sockfd, struct lista  * head, struct lista  * tail){
    struct lista  * input = (struct lista  *) Calloc(1, sizeof(struct lista));
    do{ 
        if (FullRead((int)sockfd,input,sizeof(struct lista)) == -1){//In questo caso il ristorante si è disconnesso.
            printf("[leggi_registraLista]: Mittente disconnesso.\n");
            return -1;
        }
        //Se non leggo il terminatore stringa inserisco il ristorante nella lista
        if(strcmp(input->nome, delimitatoreFine) != 0 && strcmp(input->nome, "") != 0){
            registraLista(input->nome, input->numero, &tail);
        }
        //Se leggo il terminatore stringa non lo inserisco ed esco dal ciclo.
    }while (strcmp(input->nome, delimitatoreFine) != 0); //Controllo ricezione del delimitatore

    return 1;
}

short trovaRis(char *input ,Ristoranti * head, float *fdRis){
    Ristoranti * ptr = head->next;

    while (ptr != NULL){   
        if(strcmp(input,ptr->nome) == 0){
            *fdRis = ptr->numero;
            return TRUE;
        }
        ptr = ptr->next;
    }
    return FALSE;
}

unsigned int randomSeconds(unsigned int min, unsigned int max){
    srand(time(NULL));
    return (rand() %  (max - min + 1)) + min;
}

void stampaTitolo(char * nomesorgente){
    system("clear");
    printf("*************************************************************************\n");
    printf("\t\t       Sorgente: %s\n",nomesorgente);
    printf("\t\t          Membri del progetto:                          \n");
    printf("\t\t      Esposito  Renato  0124/1881                       \n");
    printf("\t\t      Rubino    Luca    0124/1934                       \n");
    printf("*************************************************************************\n");
    puts("");
}



char * rimuoviDelimitatore(char * input){
    char * input_senzadelim = (char *)Calloc(MSG_MAXLEN,sizeof(char));
    memcpy(input_senzadelim,&input[2],strlen(input));
    return input_senzadelim;
}
