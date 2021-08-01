#include "WRAPPER.h"

#include "global.h"

#include "Client.h"

int sockfd;

Ordine *headOrdine,*tailOrdine;
Utente *nodoUtente;

void handler_sigIntRider(int signo){
    if (signo != SIGINT)
        return;

    close(sockfd);
    freeLista(headOrdine);
    exit(EXIT_FAILURE);
}

void main(int argc, char * argv[]){
    int         id_rider;
    int         n_porta;    //porta (hbo) presa da input
    uint16_t    nbo_porta;

    struct      sockaddr_in servaddr; //struttura socket

    char        nomeRider[MSG_MAXLEN];


    stampaTitolo("RIDER"); 

    signal(SIGPIPE,handler_SIGPIPE);
    signal(SIGINT,handler_sigIntRider);

    tailOrdine = headOrdine = (Ordine*) Calloc(1,sizeof(Ordine));

    sockfd    = Socket(AF_INET, SOCK_STREAM, 0); 
    
    Inet_pton(AF_INET,"127.0.0.1",(struct sockaddr *)&servaddr.sin_addr);
    servaddr.sin_family = AF_INET;

    
    char * in;
    while(TRUE){ //CONNESSIONE AL RISTORANTE: La porta è inserita dal Rider, pertanto il ciclo termina quando si è connesso.
        printf("Porta Ristorante: ");

        if((in = getInput(MSG_MAXLEN)) == NULL){ puts("chiudo il programma."); exit(EXIT_FAILURE); }
        n_porta = atoi(in);
        nbo_porta           = htons(n_porta); //porta in NBO ottenuta dalla conversione da [Host Byte Order(HBO)] a [Network Byte Order(NBO)].
        servaddr.sin_port   = nbo_porta;
        connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
        if (errno == ECONNREFUSED ){
            puts("Connessione rifiutata: utilizzare un'altra porta.");
            errno = 0;
            continue;
        }
        else
            break;
    }
    
    fflush(stdin);
    if (LeggiNome(nomeRider,MSG_MAXLEN) == -1){close(sockfd); freeLista(headOrdine); exit(EXIT_FAILURE);}
    inviaNome(sockfd,"*R",nomeRider, MSG_MAXLEN);

    //Legge l'ordine dell'utente
    if (leggi_registraLista(sockfd, headOrdine, tailOrdine) == -1){
        freeLista(headOrdine);
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    stampaTitolo("RIDER");
    stampaNome(&nomeRider[2], strlen(nomeRider) - 2);
    printf("[Rider(%s): Connesso alla porta %d]\n",&nomeRider[2], n_porta);
    puts("Ordine ricevuto:");
    stampaLista(headOrdine, 1);

    nodoUtente = (Utente *)Calloc(1,sizeof(Utente));
    
    puts("In attesa dell'id dell'Utente..");
    if (FullRead(sockfd,nodoUtente,sizeof(Utente)) == -1){
        free(nodoUtente); 
        freeLista(headOrdine);
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    
    //if(TRUE) garantisce (non si sa come) che la stampa venga effettuata prima della sleep (istruzione successiva).
    if(TRUE) printf("Consegna in corso a Utente: %s(id:%d)..\n",nodoUtente->nome,(int)nodoUtente->numero);

    sleep(randomSeconds(1,7)); 
    int consegna_effettuata = TRUE;
    FullWrite(sockfd,&consegna_effettuata,sizeof(consegna_effettuata)); 
    
    printf("Ordine consegnato a %s(id:%d), mi chiudo.\n",nodoUtente->nome,(int)nodoUtente->numero);
    
    close(sockfd);
    free(nodoUtente); 
    freeLista(headOrdine);
    exit(EXIT_SUCCESS);
}