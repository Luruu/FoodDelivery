#include "WRAPPER.h"

#include "global.h"

#include "Client.h"

#include "Server.h"

#include "Ristorante.h"


void * handler_risClient(void * arg);
void * handler_risServer(void * arg);



void main(int argc, char * argv[]){
    int clientfd, listenfd; //client: connessione al server centrale. listen: RisServer (connfd: è in paramThread)
    int err_thread; //eventuale errno code 
    int n_porta;
    

    struct  sockaddr_in rideradd, servadd; //struttura del server ristorante e del rider client
    socklen_t len_rideradd;
    char str_rideradd[SIZEIPV4STR];
    uint16_t nbo_porta;

    stampaTitolo("RISTORANTE");

    signal(SIGINT,  handler_sigIntRis);
    signal(SIGPIPE, handler_SIGPIPE_SERVER);

    //Semafori basati su memoria
    sem_init(&sem_Rider,        0,  MAX_THREAD_RIDER);
    sem_init(&MutexThread,      0,  1);
    sem_init(&MutexParamThread, 0,  1);
    sem_init(&MutexListRider,   0,  1);

    tailMenu    =   headMenu    =   (Menu *)Calloc(1,sizeof(Menu));
    tailRider   =   headRider   =   (Rider *)Calloc(1,sizeof(Rider));

    
    //Inizializzazione del NOME e MENU del ristorante.
    if(LeggiNome(nomeRis, MSG_MAXLEN) == -1){ freeLista(headRider); freeLista(headMenu); exit(EXIT_FAILURE); }
    
    puts("Componi Menu");
    if(componiPietanze(NULL, tailMenu, 0, MSG_MAXLEN) == -1){ freeLista(headMenu);freeLista(headRider); exit(EXIT_FAILURE); }


//--PARTE RISTORANTE CLIENT
    clientfd = Socket(AF_INET, SOCK_STREAM, 0);

    if((err_thread = pthread_create(&thread_client, NULL, &handler_risClient, (void *)&clientfd)) != 0){ fprintf(stderr,"thread client error: %s", strerror(err_thread)); close(clientfd); exit(EXIT_FAILURE);}

    
//--PARTE RISTORANTE SERVER
    listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    int enable = TRUE;  
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)); //attiviamo l'opzione che ci permette di riutilizzare gli address locali.

    //Struttura del RisServer
    n_porta                   = assegnaPorta(SIZEIPV4STR, SIZEPORT); 
    nbo_porta                 = htons(n_porta); //porta in NBO ottenuta dalla conversione da [Host Byte Order(HBO)] a [Network Byte Order(NBO)]
    servadd.sin_family        = AF_INET;
    servadd.sin_addr.s_addr   = htonl(INADDR_ANY); //INADDR_ANY indirizzo che accetta qualsiasi messaggio
    servadd.sin_port          = nbo_porta;
 
    Bind(listenfd, (struct sockaddr *)&servadd, sizeof(servadd));

    Listen(listenfd, BACKLOG_RISTORANTE);

    int concorrenza;
    sem_getvalue(&sem_Rider,&concorrenza);
    stampaTitolo("RISTORANTE");
    
    printf("[Server di ascolto RIS]: in ascolto sulla porta %d \t[concorrenza = %d] \n", n_porta, concorrenza);
    
    
    int n_rider = 0;//Indica la posizione libera nel vettore di struct dei thread
    inizializzaTid(paramThread, MAX_THREAD_RIDER);
    //Accettazione e gestione della connessione (creazione thread) per i Rider
    while(TRUE){
        sem_wait(&sem_Rider);
        sem_wait(&MutexThread);
        if((n_rider = ricercaThreadLibero(paramThread, MAX_THREAD_RIDER)) == -1) { puts("Errore inatteso: nessuna locazione della struct dei thread disponibile. Il programma viene chiuso."); close(paramThread[n_rider].connfd); sem_close(&sem_Rider); sem_close(&MutexListRider); sem_close(&MutexThread);}
        len_rideradd = sizeof(rideradd);
        paramThread[n_rider].connfd     = Accept(listenfd, (struct sockaddr *)&rideradd, &len_rideradd);
        paramThread[n_rider].n_thread   = n_rider;
        paramThread[n_rider].n_porta    = HBO_RISTORANTE;

       
        if((err_thread = pthread_create(&paramThread[n_rider].tid,NULL,&handler_risServer,(struct paramThread *)&paramThread[n_rider])) != 0){
            fprintf(stderr,"thread error: %s", strerror(err_thread));
            close(paramThread[n_rider].connfd);
            sem_post(&sem_Rider);
            sem_post(&MutexThread);
            continue;
        }
        inet_ntop(AF_INET, &rideradd.sin_addr, str_rideradd, sizeof(str_rideradd));
        printf("Rider(%s:%d fd%d) affidato al thrd(%d).\n", str_rideradd, ntohs(rideradd.sin_port), (int)paramThread[n_rider].connfd, n_rider);
    }
    sem_close(&sem_Rider);
    sem_close(&MutexListRider);
    sem_close(&MutexThread);
    exit(EXIT_SUCCESS); 
}


