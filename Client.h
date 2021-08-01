/* Header per funzioni e strutture utilizzate da:
 * Utente.c - Ristorante.c - Rider.c
 */

#include "./funzioni/client.c"

//un client invia il proprio nome ad un server
void inviaNome(int sockfd, char * delimitatore,char *nome, ushort len_str);


//un utente sceglie un ristorante
short sceltaRis(int sockfd,char *input, Ristoranti * head, ushort len_str);


/*  mode = 0    componi un menu:    ristorante compila il menu. Per cui si passa (NULL, tailMenu, 0)
 *  mode = 1    componi un ordine:  utente compila l'ordine. Per cui si passa (headMenu, tailOrdine,1)*/
short componiPietanze(struct lista * testa, struct lista *tail, int mode, ushort len_str);

//Somma gli interi memorizzati in una lista
float getTotale(struct lista * head);


//Legge un nome di un client finché non viene digitato correttamente
short LeggiNome(char * nome, ushort len_str);

/*Legge la porta disponibile dal file config e inserisce nel file la successiva porta disponibile.
 *FUNZIONE UTILIZZATA SOLO A SCOPO DI TEST DAL RISTORANTE! */
int assegnaPorta(int sizeSTRport, int sizeport);