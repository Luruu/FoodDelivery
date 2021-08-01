/* Header per funzioni e strutture utilizzate da:
 * Centrale.c - Ristorante.c - Utente.c - Rider.c
 */

#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <sys/file.h>
#include <pthread.h>
#include <math.h>

#define TRUE    1
#define FALSE   0

#define SIZEIPV4STR 16 // Massima dimensione per rappresentare un IP stringa. Esempio: "192.168.160.140" ha 15 caratteri + terminatore stringa

#define HBO_SERVER          1024 
#define HBO_RISTORANTE      1080

#define MSG_MAXLEN 100 

char * delimitatoreFine         =   "*F";
char * delimitatoreUtente       =   "*U";
char * delimitatoreRistorante   =   "*R";



struct lista{
    float numero; //Indica il fd per i Ristoranti mentre indica il prezzo per Menu e Ordine.
    char nome[MSG_MAXLEN];
    struct lista *next;
};
typedef struct lista Menu;
typedef struct lista Ordine;
typedef struct lista Ristoranti;
typedef struct lista Rider; 
typedef struct lista Utente;

//Se la FullWrite fallisce, il thread del mittente fallisce e si chiude.
void handler_SIGPIPE(int signo){
    if (signo != SIGPIPE) 
        return;
    puts("errore: destinatario disconnesso. Il processo si chiude.");
    exit(EXIT_FAILURE);
}

//Header delle funzioni di global
#include "./funzioni/global.c"


//Rende input maiuscolo
char * toUpper(char *input); 


//Permette di leggere da input una riga restituendolo senza il carattere di newline inserito da fgets.
char * getInput(ushort len_stringa); 


/*  mode = 0    Lista Ristoranti(nomi)
 *  mode = 1    Lista pietanze(nomi + prezzo): 
 *  mode = 2    Lista Rider(nomi + ID (file descriptor))*/
short stampaLista(struct lista * head, unsigned short mode);


//free per ogni nodo della lista
void freeLista(struct lista * head); 


//Ricerca il nodo che contiene fd e lo cancella.
short cancellaNodo(float fd, struct lista * head, struct lista ** tail);


//Crea una lista con i nomi dei ristoranti che si connettono al server.
void registraLista(char * nome_ris, float sockfd , struct lista ** tail);


/* Ad ogni iterazione viene inviato un elemento della lista. La lista viene ricostruita dal destinatario*/
void inviaLista(float sockfd, struct lista * head);


//Legge il MENU componendone una lista.
short leggi_registraLista(float sockfd, struct lista  * head, struct lista  * tail);


//Ricerca un ristorante. Se lo trova ritorna 1 settando il parametro di output fdRis con il file descriptor trovato.
short trovaRis(char *input ,Ristoranti * head, float *fdRis);


//Ritorna un intero random dell'intervallo [min,max]
unsigned int randomSeconds(unsigned int min, unsigned int max);


//Stampa il titolo del sorgente
void stampaTitolo(char * nomesorgente);


//Stampa il nome scelto dall'entità
void stampaNome(char * nome, size_t len);


//Rimuove un delimitatore da input
char * rimuoviDelimitatore(char * input);