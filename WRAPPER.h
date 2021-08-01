/* Header per funzioni WRAPPER utilizzato da:
 * Centrale.c - Ristorante.c - Utente.c - Rider.c (oltre i file .h)
 */

#ifndef WRAPPER
#define WRAPPER

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>

//Legge count byte senza poter essere interrotto da una System call.
size_t FullRead(int fd, void *buf, size_t count){
    size_t nleft; // byte rimasti da leggere
    size_t nread; // byte letti
    nleft = count;

    while (nleft > 0){

        if ((nread = read(fd,buf,nleft)) < 0){// vuol dire che sono stati letti 0 byte cioè o un errore o EOF, quindi byte finiti
            if (errno == EINTR){ // se viene interrotta da una sys call si vuole ripetere continuare a reiterare (e quindi ripetere la read)
               continue;
            }
            else{ 
                exit(nread);
            }
        } 
        else if (nread == 0) {// EOF 
            return -1;  //inserito il return -1 al posto del break per poter gestire la situazione
            //break; // termina il loop perché non c'è altro da leggere
        } 
        nleft -= nread;
        buf += nread;
    }
    buf = 0;
    return(nleft);
}

//Scrive count byte senza poter essere interrotto da una System call.
ssize_t FullWrite(int fd, const void *buf, size_t count){
    size_t nleft; // n di byte che rimangonmo da scrivere
    size_t nwritten; // n di byte letti
    nleft = count; // inizializzo la quantità da scrivere ancora(nleft) alla quantità richiesta dal parametro formale count.
    
    while (nleft > 0){ // Finché ci sono byte da leggere..

        nwritten = write(fd,buf,nleft);
        if ((nwritten) < 0){// se la write torna 0 è stato inviato un terminatore stringa. Altrimenti < 0 indica un errore.
  
            if (errno == EINTR){//se viene interrotta da una sys call, si vuole ripetere continuare a reiterare (e quindi ripetere la write)
                continue; // passa alla prossima iterazione
            }
            else{
                exit(nwritten); // altrimenti ritorna il n di byte letti
            }
        }
        nleft -= nwritten; // sottraggo al numero di byte da leggere quelli effettivamente letti
        buf += nwritten; // Sposto il puntatore da buff del n di byte letti
    }
    return (nleft);
}

/*Create a new socket of type TYPE in domain DOMAIN, using
protocol PROTOCOL. If PROTOCOL is zero, one is chosen automatically.
Returns a file descriptor for the new socket, or -1 for errors.*/
int	Socket(int	family,	int	type,	int	protocol){
    int n;
    if	((n	= socket(family, type, protocol)) < 0){
        perror("Socket Error");
        exit(-1);
    }
    return(n);
}

/*Give the socket FD the local address ADDR (which is LEN bytes long).*/
int Bind(int sockfd, const struct sockaddr *addr,socklen_t addrlen){
    int n;
    if ((n = bind(sockfd,addr,addrlen)) < 0){
        perror("Bind Error");
        exit(-1);
    }
    return n;
}

/*Prepare to accept connections on socket FD.
N connection requests will be queued before further requests are refused.
Returns 0 on success, -1 for errors.*/
int Listen(int sockfd, int backlog){
    int n;
    if ((n = listen(sockfd,backlog)) < 0){
        perror("Listen Error");
        exit(-1);
    }
    return n;
}

/*Open a connection on socket FD to peer at ADDR (which LEN bytes long).
For connectionless socket types, just set the default address to send to
and the only address from which to accept transmissions.
Return 0 on success, -1 for errors.

This function is a cancellation point and therefore not marked with
__THROW.*/
int Connect(int sockfd,const struct sockaddr *addr,socklen_t addrlen)
{
    int n;
    if ((n = connect(sockfd,addr,addrlen)) < 0){
        perror("Connect Error");
        exit(-1);
    }
    return n;
}

/*Await a connection on socket FD.
When a connection arrives, open a new socket to communicate with it,
set *ADDR (which is *ADDR_LEN bytes long) to the address of the connecting
peer and *ADDR_LEN to the address's actual length, and return the
new socket's descriptor, or -1 for errors.

This function is a cancellation point and therefore not marked with
__THROW.*/
int Accept(int sockfd, struct sockaddr *addr,socklen_t *addrlen){
    int n;
    if((n = accept(sockfd,addr,addrlen)) < 0){
        perror("Accept Error");
        exit(-1);
    }
    return n;
}

/*Convert from presentation format of an Internet number in buffer
starting at CP to the binary network format and store result for
interface type AF in buffer starting at BUF.*/
int Inet_pton(int af, const char *src, void *dst){
    int n;
    if ((n = inet_pton(af,src,dst)) < 0 )
    {
        perror("PTON Error");
        exit(-1);
    }
	return n;
}

/*Convert a Internet address in binary network format for interface
type AF in buffer starting at CP to presentation form and place
result in buffer of length LEN astarting at BUF.*/
void Inet_ntop(int af, const void *src , char *dst, socklen_t size){ 
	inet_ntop(af,src,dst,size);
	if (dst == NULL){
		perror("NTOP Error");
		exit(-1);
    }
}
/*Clone the calling process, creating an exact copy.
Return -1 for errors, 0 to the new process,
and the process ID of the new process to the old process*/
pid_t Fork(void){
	pid_t n;
	if ((n = fork()) < 0){
		perror ("Fork Error");
		exit (-1);
	}
	return n;
}
/*Allocate NMEMB elements of SIZE bytes each, all initialized to 0.*/
void * Calloc(size_t nelem, size_t elsize){
    void * indirizzo;
    if ( (indirizzo = calloc(nelem,elsize))  == (void *)NULL){
        perror("Calloc Error");
        exit(-1);
    }  
    return indirizzo;
}
/*Return entry from host data base for host with NAME.

This function is a possible cancellation point and therefore not
marked with __THROW.*/
struct hostent * Gethostbyname(const char *name){
    struct hostent * indirizzo;
    if ((indirizzo = gethostbyname(name)) == NULL){
        perror("Gethostbyname Error");
        exit(-1);
    }
    return indirizzo;
}

#endif


