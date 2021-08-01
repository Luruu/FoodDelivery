/* Header per funzioni e strutture utilizzate da:
 * Centrale.c - Ristorante.c 
 */

//Utile per identificare e quindi distinguere i singoli thread
struct parametriThreadServer{
    float connfd;
    int n_thread;
    int n_porta;
    pthread_t tid;
};


//Imposta a 0 tutti i tid del vettore di struct thread per indicare che sono liberi.
void inizializzaTid(struct parametriThreadServer thread[], int n_thread){
    for (int i=0; i < n_thread; i++)
        thread[i].tid = 0;
}



void stampaVettoreThread(struct parametriThreadServer thread[], int n_thread){
    printf("vettore thread: ");
    for (int i=0; i < n_thread; i++){
        printf("Thread[%d] = %ld\t", i, thread[i].tid);
    }
    printf("\n");
}

//Ricerca nel vettore di struct dei thread una locazione libera (tid == 0).
int ricercaThreadLibero(struct parametriThreadServer thread[], int n_thread){
    for (int i=0; i < n_thread; i++){
        if(thread[i].tid == 0){//Se è presente un thread libero..
            return i; 
        } 
    }
    return -1;
}

//Fa una kill per ogni thread attivo.
void rimuoviThreads(struct parametriThreadServer thread[], int n_thread){
    for (int i=0; i<n_thread; i++)
        if(thread[i].tid != 0) //Se è presente un thread id (attivo o meno) nella locazione i..
            pthread_kill(thread[i].tid,SIGKILL); 
}

