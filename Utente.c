//Header di tutti
#include "WRAPPER.h"

//Header condiviso tra tutte le entità
#include"global.h"

//Header per il client
#include "Client.h"


Ristoranti      *headRis,       *tailRis;
Menu            *headMenu,      *tailMenu;
Ordine          *headOrdine,    *tailOrdine;

int clientfd;

void handler_sigIntUtente(int signo){
   if(signo != SIGINT)
        return;

    close(clientfd);

    freeLista(headMenu);
    freeLista(headRis);
    freeLista(headOrdine);
    
    exit(EXIT_FAILURE);
}

void main(int argc, char * argv[]){
    int         id_rider;
    uint16_t    nbo_porta;
    struct      sockaddr_in servaddr; 
    char        nomeUt[MSG_MAXLEN];
    char        strIPserver[SIZEIPV4STR]; 
    char        input[MSG_MAXLEN];

    stampaTitolo("UTENTE"); 

    signal(SIGPIPE,handler_SIGPIPE);
    signal(SIGINT,handler_sigIntUtente);
    
    tailRis = headRis = (Ristoranti *)Calloc(1,sizeof(Ristoranti));
    
    clientfd    = Socket(AF_INET, SOCK_STREAM, 0); 

    Inet_pton(AF_INET,"127.0.0.1",(struct sockaddr *)&servaddr.sin_addr);
    servaddr.sin_family = AF_INET;
    nbo_porta           = htons(HBO_SERVER); //porta in NBO ottenuta dalla conversione da [Host Byte Order(HBO)] a [Network Byte Order(NBO)].
    servaddr.sin_port   = nbo_porta; 
    
    Connect(clientfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    
	if(LeggiNome(nomeUt,MSG_MAXLEN) == -1){close(clientfd); freeLista(headRis); exit(EXIT_FAILURE);}
	
	inviaNome(clientfd,delimitatoreUtente,nomeUt, MSG_MAXLEN);

   
    puts("Utente in attesa di una risposta dal server...");
    
    if (leggi_registraLista(clientfd,headRis,tailRis) == -1){ //leggiamo dalla socket i nomi dei ristoranti e li inseriamo in una lista   
        puts("Il Server Centrale disconnesso.");
        freeLista(headRis);
        exit(EXIT_FAILURE);
    }

    stampaTitolo("UTENTE");
    stampaNome(&nomeUt[2], strlen(nomeUt) - 2);

    if(!stampaLista(headRis, 0) ){
        puts("Disconnessione in corso...");
        exit(EXIT_FAILURE);
    }    
    if (sceltaRis(clientfd, input, headRis, MSG_MAXLEN) <= 0){//SCELTA DEL RISTORANTE E CONTROLLO ESISTENZA E DISPONIBILITA' DEL RISTORANTE
        close(clientfd);
        freeLista(headRis);
        exit(EXIT_FAILURE);
    }   
    
    stampaTitolo("UTENTE");
    stampaNome(&nomeUt[2], strlen(nomeUt) - 2);
    tailMenu    = headMenu  = (Menu *)Calloc(1,sizeof(Menu));
    puts("In ricezione del menu..");
    short esito;
    if ((esito = leggi_registraLista(clientfd, headMenu, tailMenu)) == -1){
        freeLista(headRis);
        freeLista(headMenu);
        
        close(clientfd);
        
        exit(EXIT_FAILURE);
    }
    stampaLista(headMenu, 1); 

    headOrdine = tailOrdine = (Ordine *)Calloc(1,sizeof(Ordine));
   
    if (componiPietanze(headMenu, tailOrdine, 1, MSG_MAXLEN) == -1){close(clientfd);freeLista(tailOrdine);freeLista(headRis);exit(EXIT_FAILURE);}
    
    system("clear");
    stampaTitolo("UTENTE"); 
    stampaNome(&nomeUt[2], strlen(nomeUt) - 2);
    puts("Ordine effettuato: ");
    stampaLista(headOrdine, 1);
    float totale_ordine = getTotale(headOrdine);
    if (totale_ordine == -1){close(clientfd);  freeLista(headRis); freeLista(headMenu); freeLista(headOrdine); exit(EXIT_FAILURE);}
    printf("Totale ordine: € %0.2f\n", totale_ordine);
    
    inviaLista(clientfd,headOrdine);
    
    puts("In attesa della conferma ordine..");

    Rider *rider = (Rider *) Calloc(1,sizeof(Rider));

    int esito_consegna = FALSE;
   if(FullRead(clientfd,rider,sizeof(Rider)) == -1){
        puts("Ordine annullato: il mittente si è disconnesso");
        close(clientfd);
        freeLista(headOrdine);
        freeLista(headMenu);
        freeLista(headRis);
        exit(EXIT_SUCCESS);
    }    
	
    if (rider->numero > 0){
        printf("Rider %s(id:%d) gestisce l'ordine.\n",rider->nome,(int)rider->numero);
        if ( FullRead(clientfd,&esito_consegna,sizeof(esito_consegna)) == -1){
            puts("Ordine annullato: il mittente si è disconnesso");
            close(clientfd);
            freeLista(headOrdine);
            freeLista(headMenu);
            freeLista(headRis);
            exit(EXIT_SUCCESS);
        }
        printf("Ordine: %s\n", esito_consegna ? "ricevuto." : "Ordine annullato. Il rider ha avuto dei problemi.");
    }
    else if(rider->numero == -1) //la funzione ritorna -1 in caso di nessun rider
        puts("Ordine annullato: Nessun rider disponibile."); 


    close(clientfd);
    freeLista(headOrdine);
    freeLista(headMenu);
    freeLista(headRis);
    exit(EXIT_SUCCESS);
}
