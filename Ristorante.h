/* Header per funzioni e strutture utilizzate da:
 * Ristorante.c
 */

#define sizeSTRPort 9
#define SIZEPORT 5
#define BACKLOG_RISTORANTE  300 //dimensione della coda di attesa

#define MAX_THREAD_RIDER 2 //Massimo numero di rider che il serverRis può gestire concorrentemente

sem_t   MutexThread;		//Gestione della mutua esclusione per i thread Server
sem_t   MutexParamThread;	//gestione della mutua esclusione per paramThread
sem_t   sem_Rider;			//Semaforo contatore per la concorrenza limitata dei rider
sem_t   MutexListRider;		//Gestione della mutua esclusione per la lista dei rider

 // server: per i rider. client: per il server centrale
pthread_t                       thread_client = 0; 
struct parametriThreadServer    paramThread[MAX_THREAD_RIDER];

char nomeRis[MSG_MAXLEN];

Menu *headMenu, *tailMenu;
Rider *headRider, *tailRider;

void handler_sigIntRis(int signo){
    if(signo != SIGINT)
        return;
    
    sem_close(&sem_Rider);
    sem_close(&MutexListRider);
    sem_close(&MutexThread);
    sem_close(&MutexParamThread);
    
    if (thread_client != 0)
        pthread_kill(thread_client,SIGKILL);
    
    puts("");
    exit(EXIT_SUCCESS);
}


void handler_SIGPIPE_SERVER(int signo){
    if (signo != SIGPIPE) 
        return;
        puts("SIGPIPE: errore: destinatario disconnesso. Il thread si chiude.");
        
    sem_close(&MutexListRider);
    sem_close(&MutexParamThread);
    sem_close(&sem_Rider);
   
    exit(EXIT_FAILURE);
    }


Rider * getRider(Rider *head, Rider *tail){
    if (head == NULL || head->next == NULL) // necessaro l'OR perché non posso ritornare NULL->NEXT
        return NULL;
    return head->next;
}

void * handler_risClient(void *arg){
    int * ptr = (int *)arg;
    int sockfd = * ptr;
    Ordine *headOrdine, *tailOrdine;
    uint16_t nbo_porta;
    struct sockaddr_in servaddr;
    socklen_t len;

    
    Inet_pton(AF_INET,"127.0.0.1",(struct sockaddr *)&servaddr.sin_addr);
    servaddr.sin_family = AF_INET;
    nbo_porta           = htons(HBO_SERVER); //conversione da [Host Byte Order(HBO)] a [Network Byte Order(NBO)].
    servaddr.sin_port   = nbo_porta;
    int richiesta;
    int fdTmp; // utilizzato per controllare che il nome scelto sia univoco
    len = sizeof(servaddr);

    Connect(sockfd,(struct sockaddr*)&servaddr,len);

    //Sceglie un nome che deve essere univoco
    do{
        inviaNome(sockfd, delimitatoreRistorante, nomeRis, MSG_MAXLEN);
        if (FullRead(sockfd,&fdTmp,sizeof(int) )  == -1){
            printf("[cRis]: Server centrale disconnesso. Il ristorante si chiude.\n");
            close(sockfd);
            exit(EXIT_FAILURE);
        }
        if (fdTmp != -1){
            printf("[cRis]:Nome inserito gia' esistente, riprovare.\n");
            if (LeggiNome(nomeRis,MSG_MAXLEN)==-1){close(sockfd);rimuoviThreads(paramThread,MAX_THREAD_RIDER);sem_close(&sem_Rider);sem_close(&MutexListRider);sem_close(&MutexThread); exit(EXIT_FAILURE);}
        }
    } while (fdTmp != -1); //finché il server trova un file descriptor (quindi finché il nome scelto è già in uso..)
    
    
//Gestione delle richieste che il Server Centrale può fare al client Ristorante
    while(TRUE){

        if (FullRead(sockfd,&richiesta,sizeof(int)) == -1){
            printf("[cRis]: Server centrale disconnesso. Il ristorante si chiude.\n");
            exit(EXIT_FAILURE);
        }

        switch (richiesta){
            case 1: //INVIO DEL MENU
                printf("[cRis]: Invio il menu richiesto a fd(%d)\n", sockfd);
                inviaLista(sockfd, headMenu);

                break;
            case 2: //INFO UTENTE E INFO RIDER
                tailOrdine = headOrdine = (Ordine *) Calloc(1,sizeof(Ordine));
                if (leggi_registraLista(sockfd,headOrdine,tailOrdine) == -1){
                    freeLista(headOrdine);
                    //free(tailOrdine);
                    close(sockfd);
                    exit(EXIT_FAILURE);
                }
            
                Utente * utente = (Utente*) Calloc (1,sizeof(Utente));
                //Leggo le info del cliente
                if(FullRead(sockfd, (Utente *)utente, sizeof(Utente)) == -1){
                    //Il mittente si è disconnesso
                    printf("[cRis]: Mittente disconnesso\n");
                    close(sockfd);
                    freeLista(headOrdine);
                    exit(EXIT_FAILURE);

                }
                Rider *rider = (Rider*) Calloc(1,sizeof(Rider));

                sem_wait(&MutexListRider);
                rider = getRider(headRider, tailRider);
                if (rider == NULL){
                    headRider->numero = -1;
                    //Non ci sono rider lo comunico al server
                    FullWrite(sockfd, (Rider *)headRider, sizeof(Rider));
                    sem_post(&MutexListRider);
                    continue;
                }
                else{
                    //Invio le info del rider al server
                    FullWrite(sockfd, (Rider *)rider, sizeof(Rider));// scrivo al server il rider.
                    inviaLista(rider->numero, headOrdine); //Invio l'ordine dell'utente al rider.
                    FullWrite(rider->numero, (Utente *)utente, sizeof(Utente)); // scrivo al rider la struttura del cliente.
                }
                sem_post(&MutexListRider);
                int esito_consegna = FALSE;
                if (FullRead(rider->numero, &esito_consegna, sizeof(esito_consegna)) == -1){
                    printf("[cRis]: Destinatario disconnesso");
                    close(rider->numero);
                    FullWrite(sockfd, &esito_consegna, sizeof(esito_consegna));
                }
                
                printf("[cRis]: Esito consegna del Rider(id:%d) : %s\n", (int)rider->numero, (esito_consegna ? "positivo." : "NEGATIVO."));
                FullWrite(sockfd, &esito_consegna, sizeof(esito_consegna));
                cancellaNodo(rider->numero, headRider, &tailRider);
                break;

            default:
                printf("[cRis]:Errore imprevisto: Caso non gestito(case default), il ristorante si chiude.\n");
                close(sockfd);
                freeLista(headOrdine); freeLista(headMenu); freeLista(headRider);
                exit(EXIT_FAILURE);
                break;
        }
    }
    printf("[cRis]: Errore inatteso: Il client Ristorante è uscito dal while(1). Il ristorante si chiude.");
    close(sockfd);
    pthread_exit(NULL);
}





//Registrazione di un rider connesso nella lista dei rider.
void * handler_risServer(void *arg){
    struct parametriThreadServer * ptr = (struct parametriThreadServer *)arg;
    int threadid    = ptr->n_thread;
    int riderfd     = ptr->connfd;
    int n_porta     = ptr->n_porta;
    
    char * nomeRider = (char *) Calloc(MSG_MAXLEN,sizeof(char));
    

    sem_post(&MutexThread); 
    
    printf("[Thrd%d]: gestisce il Rider(fd: %d).\n",threadid, riderfd);
   
    sem_wait(&MutexListRider);

    if(FullRead(riderfd,nomeRider,MSG_MAXLEN) == -1){
        printf("[Thrd%d]: Rider(fd%d) disconnesso. Il thread si chiude.\n", threadid, riderfd);
        sem_wait(&MutexThread);
        paramThread[threadid].tid = 0;
        sem_post(&MutexThread);
        sem_post(&MutexListRider);
        pthread_exit(NULL);
    }
    nomeRider = rimuoviDelimitatore(nomeRider);
    registraLista(nomeRider,riderfd,&tailRider);
    stampaLista(headRider, 2); 

    sem_post(&MutexListRider);
    
    sem_post(&sem_Rider);
    sem_wait(&MutexThread);
    paramThread[threadid].tid = 0;
    sem_post(&MutexThread);
    printf("[Thrd%d]: Rider(fd%d) registrato nella lista rider. Il thread si chiude.\n", threadid, riderfd);
    pthread_exit(NULL);
}