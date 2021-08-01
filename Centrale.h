/* Header per funzioni e strutture utilizzate da:
 * Centrale.c
 */

#define SEM_PATH "/sem_Server"
#define SEM_PATHThread "/MutexThread"
#define SEM_PATHLista "/MutexLista"

#define SEM_PATHMutexPararm "/Sem_MutexPararm"

#define SEM_PATHPrimoRis "/Sem_PrimoRis"
#define SEM_PATHwaitRisCont "/waitRisCont"



#define MAX_THREAD_CLIENT 4 //MAX numero di client che il server Centrale può gestire concorrentemente

short primoRistorante = TRUE;

#define BACKLOG_SERVER 1000 //dimensione della coda di attesa

sem_t * sem_Server;         //Concorrenza limitata per evitare la proliferazione dei thread
sem_t * MutexThread;        //Mutua esclusione la creazione di un thread e il passaggio dei parametri.
sem_t * MutexLista;         //Mutua esclusione per la registrazione della lista.
sem_t * MutexParamThread;   //Mutua esclusione per la struct pararm
sem_t * sem_PrimoRis;       //Semaforo di segnalazione per controllare che sia stato inserito o meno il primo ristorante nella lista.
sem_t * Mutexwait;          //Semaforo MUTEX per la variabile che tiene conto del numero di thread in attesa per la ricezione del primo ris.

ushort n_waitPrimoRis = 0;

struct parametriThreadServer    paramThread[MAX_THREAD_CLIENT];
Ristoranti *headRis, *tailRis;


void rimuoviSemaforiNome(){
    sem_unlink(SEM_PATH);
    sem_unlink(SEM_PATHThread);
    sem_unlink(SEM_PATHLista);
    sem_unlink(SEM_PATHPrimoRis);
    sem_unlink(SEM_PATHMutexPararm);
    sem_unlink(SEM_PATHwaitRisCont);
}

 void handler_SIGPIPE_SERVER(int signo){
    if (signo != SIGPIPE) 
        return;
    rimuoviSemaforiNome();
    rimuoviThreads(paramThread,MAX_THREAD_CLIENT);
}


void handler_sigintCENTRALE(int signo){
    if(signo != SIGINT)
        return;

    rimuoviSemaforiNome();
    rimuoviThreads(paramThread, MAX_THREAD_CLIENT);

    exit(EXIT_SUCCESS);
}

void comunicazioneUtenteRis(char * nome, int threadid, int connfd);
void registraRis(char nome[MSG_MAXLEN], int threadid, int connfd);

void * handler_Server(void *arg){
    //Memorizzo le informazioni che ho passato a questo thread
    struct parametriThreadServer * ptr = (struct parametriThreadServer *)arg;
    
    int threadid = ptr->n_thread;
    int connfd = (int)ptr->connfd;
    int n_porta = ptr->n_porta;
    
    sem_post(MutexThread);

    char nome[MSG_MAXLEN];
    if (FullRead(connfd,nome,MSG_MAXLEN) == -1){
        printf("[Thrd%d] chiuso: Client(fd%d) disconnesso.\n", threadid, connfd);

        sem_wait(MutexParamThread); 
        paramThread[threadid].tid = 0; 
        sem_post(MutexParamThread); 

        sem_post(sem_Server);   
        pthread_exit(NULL);
    } 
    
    if (strncmp(nome,delimitatoreUtente, 2 * sizeof(char)) == 0)
        comunicazioneUtenteRis(nome, threadid, connfd);

    else if(strncmp(nome,delimitatoreRistorante, 2 * sizeof(char)) == 0)
        registraRis(nome, threadid, connfd);
    
    sem_wait(MutexParamThread); 
    paramThread[threadid].tid = 0; 
    sem_post(MutexParamThread); 

    printf("[Thrd%d]: gestito Client(fd%d), mi chiudo.\n", threadid, connfd);
    
    sem_post(sem_Server);
    pthread_exit(NULL);
}



//Thread x: gestione comunicazione tra UTENTE e RISTORANTE
void comunicazioneUtenteRis(char * nome, int threadid, int connfd){
    float   fdRis;
    int     ristrovato;
    char    input[MSG_MAXLEN];

    Menu    * headMenu,     * tailMenu;
    Ordine  * headOrdine,   * tailOrdine;

    tailMenu = headMenu = (Menu *)Calloc(1,sizeof(Menu));
    
    nome = rimuoviDelimitatore(nome);
    printf("[Thrd%d]: gestisce Utente(\"%s\" fd%d).\n",threadid, nome, connfd);
        
    //Se non è stato inserito un ristorante il thread resta in attesa.
    if(primoRistorante == TRUE){
        sem_wait(Mutexwait);
        n_waitPrimoRis++;
        sem_post(Mutexwait);
        sem_wait(sem_PrimoRis);
    }
    inviaLista(connfd,headRis); //Invia la lista Ristoranti
      
    //Acquisizione del riscelto e controllo della presenza nella lista Ristoranti 
    ristrovato = FALSE;
    do{
        printf("[Thrd%d]: Attendo una scelta da Utente(\"%s\" fd%d).\n",threadid,nome, connfd);
        if (FullRead(connfd,input,MSG_MAXLEN) == -1){  printf("[Thrd%d]: Client(fd%d) disconnesso.\n",threadid, connfd);    close(connfd); sem_post(sem_Server); sem_wait(MutexParamThread);paramThread[threadid].tid = 0; sem_post(MutexParamThread);pthread_exit(NULL);}
        ristrovato = trovaRis(input, headRis, &fdRis);
        FullWrite((int)connfd,&ristrovato,sizeof(short));
    }while(!ristrovato);

        
    //Richiesta del menu
    int servizio = 1;
    FullWrite((int)fdRis,&servizio,sizeof(int));

    printf("[Thrd%d]: In ricezione del menu da Ris(fd%d)..\n",threadid,(int)fdRis);

    short esito;
    if ((esito = leggi_registraLista(fdRis,headMenu,tailMenu)) == -1){ 
        printf("[Thrd%d]: Ris(fd%d) disconnesso. Il thread si chiude.\n",threadid,(int)fdRis);
        sem_wait(MutexLista); 
        cancellaNodo(fdRis, headRis, &tailRis);
        if (headRis == tailRis)
            primoRistorante = TRUE;
        sem_post(MutexLista);
            
        close(fdRis);  close(connfd);  freeLista(headMenu); 

        sem_wait(MutexParamThread); 
        paramThread[threadid].tid = 0; 
        sem_post(MutexParamThread); 
            
        sem_post(sem_Server);
        pthread_exit(NULL);
    }

    
    if(headMenu->next == NULL){     //Al ristorante è vietato scrivere NULL all'elemento successivo della testa, 
        printf("[Thrd%d]: Lista del menu vuota. Il thread si chiude.\n",threadid);
        close(connfd);              //al più potrebbe premere ctrl+C il quale è gestito però dalla funzione leggi_registraLista 
        sem_post(sem_Server);       //che renderà vero questo controllo per cui lo rendo noto chiudendo il file descriptor.

        close(connfd);  
        freeLista(headMenu); 
        sem_wait(MutexParamThread); 
        paramThread[threadid].tid = 0; 
        sem_post(MutexParamThread); 
        pthread_exit(NULL); 
    }

    printf("[Thrd%d]: Invia la lista all'utente(%d).\n",threadid,connfd);
    //Invia la lista Menu all'utente
    inviaLista(connfd,headMenu);
        

    //Legge l'ordine dall'utente.
    tailOrdine = headOrdine = (Ordine *) Calloc(1, sizeof(Ordine));
    if(leggi_registraLista(connfd,headOrdine,tailOrdine) == -1){
        printf("[Thrd%d]: L'utente(%d) si è disconnesso. Il thread si chiude.\n",threadid,connfd);
        close(connfd);
        freeLista(headMenu); // cancello quello già eventualmente registrato.
        freeLista(headOrdine);
        sem_wait(MutexParamThread); 
        paramThread[threadid].tid = 0; 
        sem_post(MutexParamThread);  
            
        sem_post(sem_Server);
        pthread_exit(NULL);
    }

    servizio = 2;
    FullWrite(fdRis,&servizio,sizeof(int));

    inviaLista(fdRis,headOrdine);
       
    //registro le INFO UTENTE e le invio al RIS. il Ris le invierà al RIDER.
    Utente  *Cliente = (Utente *) Calloc(1,sizeof(Utente));
    strcpy(Cliente->nome,nome);
    Cliente->numero = connfd;
        
    //INVIO INFO UTENTE a RIS.
    FullWrite(fdRis,(Utente *)Cliente,sizeof(Utente));
      

    printf("[Thrd%d]: in ricezione di un rider del Ris(fd%d) per l'ordine Utente(\"%s\" fd%d).\n",threadid,(int)fdRis ,nome, connfd);
        
    Rider * rider = (Rider *)Calloc (1,sizeof(Rider));

    //Leggo le INFO RIDER assegnato al cliente
    if (FullRead(fdRis,(Rider *)rider,sizeof(Rider)) == -1){
        printf("[Thrd%d]: Mittente disconnesso[1].\n", threadid);
        sem_wait(MutexLista); 
        cancellaNodo(fdRis, headRis, &tailRis);
        if (headRis == tailRis) //Se il nodo cancellato è il primo dopo il nodo sentinella significa che bisognerà reinserire il primo ristorante nuovamente.
            primoRistorante = TRUE;
            
        sem_post(MutexLista);
        close(fdRis);
        close(connfd);
        freeLista(headMenu); // cancello i nodi eventualmente inseriti prima del crash
        sem_post(sem_Server);

        sem_wait(MutexParamThread); 
        paramThread[threadid].tid = 0; 
        sem_post(MutexParamThread); 
        pthread_exit(NULL);
    }
        
    FullWrite(connfd,(Rider *)rider,sizeof(Rider));//Invio INFO RIDER all'UTENTE
    //Se non ci sono rider dopo aver avvertito l'utente bisogna chiudere il connfd del server.
    if(rider->numero == -1){
        printf("[Thrd%d]: Nessun rider di Ris(%d) per l'Utente(%d).\n", threadid,(int)fdRis,connfd);
        close(connfd);
        freeLista(headMenu); 
        sem_post(sem_Server);

        sem_wait(MutexParamThread); 
        paramThread[threadid].tid = 0; 
        sem_post(MutexParamThread); 
        pthread_exit(NULL);
    }

    int esito_consegna = FALSE;
    if (FullRead(fdRis,&esito_consegna,sizeof(esito_consegna)) == -1){
        printf("[Thrd%d]: Mittente disconnesso[2].\n", threadid);
        sem_wait(MutexLista); 
        cancellaNodo(fdRis, headRis, &tailRis);
        if (headRis == tailRis) //Se il nodo cancellato è il primo dopo il nodo sentinella significa che bisognerà reinserire il primo ristorante nuovamente.
            primoRistorante = TRUE;
            sem_post(MutexLista);
            close(fdRis);
            close(connfd);
            freeLista(headMenu); 
            sem_post(sem_Server);

            sem_wait(MutexParamThread); 
            paramThread[threadid].tid = 0; 
            sem_post(MutexParamThread); 
            pthread_exit(NULL);
        }
        printf("[Thrd%d]: esito dell'ordine dell'Utente(%s fd%d): %s\n", threadid, nome, connfd, esito_consegna ? "Ordine consegnato." : "Ordine non consegnato.");
        FullWrite(connfd, &esito_consegna, sizeof(esito_consegna)); 

        freeLista(headOrdine);
        freeLista(headMenu); 
        close(connfd);  // N.B: fdRis non viene chiuso perché ne perderei la comunicazione per qualsiasi utente.
}

//Thread y: registra i RISTORANTI CONNESSI
void registraRis(char * nome, int threadid, int connfd){
    int     fdTmp; 
    float   fdtrovato;
    Menu *headMenu, *tailMenu;
    tailMenu = headMenu = (Menu *)Calloc(1,sizeof(Menu));

    nome = rimuoviDelimitatore(nome);
    printf("[Thrd%d]: gestisce Ris(\"%s\" fd%d).\n",threadid, nome, connfd);
    while(TRUE){
        if (trovaRis(nome,headRis,(float *) &fdtrovato)){//TRUE = nome già presente in lista
            fdTmp = (int)fdtrovato;
            FullWrite(connfd,&fdTmp,sizeof(int));//Comunico che il nome già esiste nella lista (NON E' Univoco)
              
        }
        else{
            fdTmp = -1;
            FullWrite(connfd,&fdTmp,sizeof(int));
       
            break;
        }
        printf("[Thrd%d]: In attesa di un nome valido...\n", threadid);
        if (FullRead(connfd,nome,MSG_MAXLEN) == -1){
            printf("[Thrd%d]: Il mittente ha chiuso la connessione.\n", threadid);
            close(connfd);
            freeLista(headMenu);
            sem_post(sem_Server);
            
            sem_wait(MutexParamThread); 
            paramThread[threadid].tid = 0; 
            sem_post(MutexParamThread); 
            pthread_exit(NULL);
        }
        nome = rimuoviDelimitatore(nome);
    }
    //Aggiungo in mutua eslcusione il ristorante alla lista.

    sem_wait(MutexLista);
    primoRistorante = (headRis->next == NULL); // se l'elemento che registro 
    //è il primo, sblocco un eventuale thread in attesa.
    registraLista(nome, connfd, &tailRis);
        
    //Se il primo ristorante è stato appena aggiunto, sblocca il thread che gestisce l'utente.
    if(primoRistorante){
        sem_wait(Mutexwait);
        for(int i=0; i<n_waitPrimoRis; i++)
            sem_post(sem_PrimoRis);
        sem_post(Mutexwait);
        primoRistorante = FALSE;
    }
    printf("[Thrd%d]: Ristorante(fd%d) registrato correttamente nella lista ristoranti.\n", threadid, connfd);
    sem_post(MutexLista);
}