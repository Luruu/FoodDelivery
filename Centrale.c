#include "WRAPPER.h"

#include "global.h"

#include "Server.h"

#include "Centrale.h"

  
void * handler_Server(void * arg);

void main(int argc, char *argv){
    int                 listenfd, connfd;
    int                 err_thread; //eventuale errno code
    struct sockaddr_in  servadd, clientadd;
    socklen_t           len_clientadd;
    char                str_addClient[SIZEIPV4STR];
    uint16_t            nbo_porta;


    stampaTitolo("SERVER CENTRALE");

    signal(SIGPIPE, handler_SIGPIPE_SERVER);
    signal(SIGINT,  handler_sigintCENTRALE);

    //Semafori con nome
    sem_Server              = sem_open(SEM_PATH,                    O_CREAT, 0644, MAX_THREAD_CLIENT);
    MutexThread             = sem_open(SEM_PATHThread,              O_CREAT, 0644, 1);
    MutexParamThread        = sem_open(SEM_PATHMutexPararm,         O_CREAT, 0644, 1);
    MutexLista              = sem_open(SEM_PATHLista,               O_CREAT, 0644, 1);
    sem_PrimoRis            = sem_open(SEM_PATHPrimoRis,            O_CREAT, 0644, 0);
    Mutexwait               = sem_open(SEM_PATHwaitRisCont,         O_CREAT, 0644, 1);

    tailRis =   headRis =   (Ristoranti *)Calloc(1,sizeof(Ristoranti)); //head punta al nodo sentinella della lista. coda punta all'ultimo nodo della lista.
    
    listenfd = Socket(AF_INET,SOCK_STREAM,0);

    int enable = TRUE;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)); //attiviamo l'opzione che ci permette di riutilizzare gli address locali. 

    //Struttura del Server
    nbo_porta               = htons(HBO_SERVER); //porta in [Network Byte Order(NBO)] ottenuta dalla conversione da [Host Byte Order(HBO)]
    servadd.sin_family      = AF_INET;
    servadd.sin_addr.s_addr = htonl(INADDR_ANY); //INADDR_ANY permette di rendere il server accessibile all'esterno del proprio pc
    servadd.sin_port        = nbo_porta;

    Bind(listenfd, (struct sockaddr *)&servadd, sizeof(servadd));

    Listen(listenfd, BACKLOG_SERVER);
    
    int concorrenza;
    sem_getvalue(sem_Server,&concorrenza);
    printf("Server Centrale ON: In ascolto sulla porta %d \t[concorrenza = %d]\n",HBO_SERVER, concorrenza);
    
    //indica il numero dei ristoranti e utenti connessi dall'apertura del Server Centrale
    int n_client = 0;
    inizializzaTid(paramThread, MAX_THREAD_CLIENT);

    while(TRUE){ //Accettazione e gestione della connessione (creazione thread) per i client

        /*stampaVettoreThread(paramThread,MAX_THREAD_CLIENT);
        Stampa di test per visualizzare il vettore di struct dei thread.*/
        
        sem_wait(sem_Server); //concorrenza
        sem_wait(MutexThread); //mutua esclusione
        
       
        if((n_client = ricercaThreadLibero(paramThread, MAX_THREAD_CLIENT)) == -1){ puts("Errore inatteso: nessuna locazione della struct dei thread disponibile. Il programma viene chiuso."); rimuoviSemaforiNome(); rimuoviThreads(paramThread,MAX_THREAD_CLIENT); exit(EXIT_FAILURE);}
        
        len_clientadd = sizeof(clientadd);
        paramThread[n_client].connfd      = (float)Accept(listenfd, (struct sockaddr *)&clientadd, &len_clientadd);
        
        
        paramThread[n_client].n_thread    = n_client;
        paramThread[n_client].n_porta     = HBO_SERVER;
        if((err_thread = pthread_create(&paramThread[n_client].tid,NULL,handler_Server,(struct paramThread *)&paramThread[n_client])) != 0){
            fprintf(stderr,"thread error: %s", strerror(err_thread));
            close(paramThread[n_client].connfd);
            sem_post(sem_Server);
            sem_post(MutexThread);
            continue;
        }
        inet_ntop(AF_INET,&clientadd.sin_addr, str_addClient, sizeof(str_addClient));
        printf("[Server di ascolto]: Client(%s:%d (fd%d)) affidato al thrd(%d).\n", str_addClient, ntohs(clientadd.sin_port), (int)paramThread[n_client].connfd, n_client);

       /* stampaVettoreThread(paramThread,MAX_THREAD_CLIENT);
        printf("Locazione assegnata = %d \n\n",n_client);*/
    }

    puts("Errore inatteso: Il SERVER DI ASCOLTO è uscito da un while(1)! Il programma si chiude.");
    rimuoviSemaforiNome();
    rimuoviThreads(paramThread,MAX_THREAD_CLIENT);
    exit(EXIT_FAILURE);
}