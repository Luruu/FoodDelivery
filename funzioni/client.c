void inviaNome(int sockfd, char * delimitatore,char *nome, ushort len_str){ 
    char tmp[len_str]; 
    int nwrite;
    strcpy(tmp,nome);
    strcpy(nome,delimitatore);
    strcat(nome,tmp); // -> *Uinput *U oppure "*R" nel caso del ristorante
    FullWrite(sockfd,nome,len_str);
}

short sceltaRis(int sockfd,char *input, Ristoranti * head, ushort len_str){ 
    short nome_trovato = FALSE;
    char * in;
    do{
        printf("Scegli un ristorante: ");
        if((in = getInput(len_str)) == NULL) return -1;
	    strcpy(input,in);
        FullWrite(sockfd,input,len_str);
        
		if (FullRead(sockfd,&nome_trovato,sizeof(short)) == -1) //Leggo se la scelta effettuata è corretta
            return FALSE;

	    if((nome_trovato == FALSE)) // se il nome non è stato trovato
            puts("Ristorante non trovato. Riprova nuovamente.");

        
    }while (nome_trovato == FALSE); // == TRUE è stato trovato.
    
    return TRUE;
}

short componiPietanze(struct lista * testa, struct lista *tail, int mode, ushort len_str){
    struct lista * ptr;
    int n_alimenti_inseriti = 0;
    float prezzo;
    char * in;
    while (TRUE){
        ptr = (struct lista *) Calloc(1, sizeof(struct lista));
        
        printf("Nome: [*F per terminare]: ");
        if((in = getInput(len_str)) == NULL) return -1;
        strcpy(ptr->nome,in);

        if (strcmp(toUpper(ptr->nome), delimitatoreFine) != 0){
            if (mode == 0){ // componi menu
                printf("Prezzo: ");
                if((in = getInput(len_str)) == NULL) return -1;
                ptr->numero = (float)atof(in); // prende una stringa la converte in double e con il cast diventa float.
                registraLista(ptr->nome,ptr->numero, &tail);
                n_alimenti_inseriti = 1;
            }
            else if (mode == 1){ // componi ordine
                if (trovaRis(ptr->nome,testa,&prezzo)){ //il piatto è presente.
                    registraLista(ptr->nome,prezzo, &tail);
                    n_alimenti_inseriti = 1;
                }
                else{ // se non trova alcun alimento
                    puts("Alimento non trovato. Tenta nuovamente.");
                    continue; // passa all'iterazione successiva
                } 
            }
        }   
        else if (strcmp(toUpper(ptr->nome), delimitatoreFine) == 0  && n_alimenti_inseriti == 0) // delimitatore al primo elemento
                puts("Errore: Inserire almeno un elemento.");
        else // casistica nella quale ptr->nome = *F ed è stata creata una lista di almeno un elemento 
            return TRUE;

        free(ptr); //Libero il ptr ad ogni iterazione.
    }
}

float getTotale(struct lista * head){
    struct lista * ptr = head->next;
    float totale = 0;
    if (head->next == NULL){
        puts("Errore: impossibile calcolare il totale.");
        return -1;
    }
    
    while(ptr != NULL){
        totale += ptr->numero;
        ptr = ptr->next;
    }
    return totale;
}

short LeggiNome(char * nome, ushort len_str){
    const int MAXCARATTERI = len_str - 2; // 2 caratteri sono già occupati nell'array nome per il delimitatore
    const int MINCARATTERI = 1;
    char * in;
    short nome_inserito = FALSE;
    while(!nome_inserito){
        printf("Nome: (MAX %d caratteri): ", MAXCARATTERI);
        if((in = getInput(len_str)) == NULL) return -1;
        strcpy(nome,in);
        if(strlen(nome) > MINCARATTERI && strlen(nome) < MAXCARATTERI)
            nome_inserito = TRUE;
        else
            puts("Lunghezza non rispettata. Riprovare.");
    }
    return TRUE;
}


int assegnaPorta(int sizeSTRport, int sizeport){
    int     nporta, nextporta;
    char    strporta[sizeSTRport];
    char    buffer[sizeSTRport], tmp[sizeSTRport];

    int fdFile;

    if(access("config", R_OK) != 0){ // se file config non esiste
        fdFile = open("config", O_RDWR | O_CREAT, 0600);
        FullWrite(fdFile,"Port:1080",sizeSTRport);
        nextporta = nporta = HBO_RISTORANTE;
    }
    else{
        fdFile = open("config", O_RDWR, 0600);
        FullRead(fdFile,buffer,sizeSTRport); 
        nextporta = nporta = atoi(&buffer[sizeport]); 
    }

    //Inserisco la prossima porta utilizzabile nel file per il prossimo ristorante
    snprintf(strporta, sizeSTRport, "%d", ++nextporta);
  
    strcpy(tmp,strporta); //strporta es = 1080
    strcpy(strporta,"Port:");
    strcat(strporta,tmp); 

    lseek(fdFile,0,SEEK_SET);
    FullWrite(fdFile,strporta,sizeSTRport);

    close(fdFile);

    return nporta;
}

void stampaNome(char * nome, size_t len){
    char asterischi[] = "***********";
    char * spazio_dopo = (char *)malloc(MSG_MAXLEN);
    memset(spazio_dopo,' ', strlen(asterischi) - len - 4);
    printf("\t%s\n",asterischi);
    printf("\t*  %s%s*\n",nome,spazio_dopo);
    printf("\t%s\n\n",asterischi);
    free(spazio_dopo);
}