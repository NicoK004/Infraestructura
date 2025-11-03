#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>

#define PASAJEROS 100
#define OFICINISTAS 5
#define CAMBIOS 3

sem_t rw_mutex;   // controla acceso al cartel (lectores vs escritores)
sem_t rc_mutex;   // protege contador de lectores
int readers = 0;

// --------- Funciones --------------

void* pasajero(void* x) {
    long id = (long)x;
    int i;
    for (i = 0; i < 3; i++) {
        sem_wait(&rc_mutex);
        readers++;
        if (readers == 1)
            sem_wait(&rw_mutex); // primer lector bloquea a escritores
        sem_post(&rc_mutex);

        // sección crítica de lectura 
        printf("Pasajero %ld está mirando el cartel\n", id);
        fflush(stdout);
        sleep(rand() % 4); // demora random hasta 3s

        // salida
        sem_wait(&rc_mutex);
        readers--;
        if (readers == 0)
            sem_post(&rw_mutex); // último lector libera
        sem_post(&rc_mutex);

        sleep(rand() % 4);
    }
    pthread_exit(NULL);
}

void* oficinista(void* x) {
    long id = (long)x;
    int i;
    for (i = 0; i < CAMBIOS; i++) {
        sem_wait(&rw_mutex); // bloqueo total

        printf("Oficinista %ld está modificando el cartel\n", id);
        fflush(stdout);
        sleep(rand() % 6); // demora random hasta 5s

        sem_post(&rw_mutex);

        sleep(rand() % 6);
    }
    pthread_exit(NULL);
}

int main() {
    srand(time(NULL));

    pthread_t pas[PASAJEROS];
    pthread_t ofi[OFICINISTAS];
    pthread_attr_t attr;
    pthread_attr_init(&attr);

    sem_init(&rw_mutex, 0, 1);
    sem_init(&rc_mutex, 0, 1);

    // crear oficinistas
    for (long i = 0; i < OFICINISTAS; i++) {
        pthread_create(&ofi[i], &attr, oficinista, (void*)i);
    }

    // crear pasajeros
    for (long i = 0; i < PASAJEROS; i++) {
        pthread_create(&pas[i], &attr, pasajero, (void*)i);
    }

    // join oficinistas
    for (int i = 0; i < OFICINISTAS; i++) {
        pthread_join(ofi[i], NULL);
    }

    // join pasajeros
    for (int i = 0; i < PASAJEROS; i++) {
        pthread_join(pas[i], NULL);
    }

    sem_destroy(&rw_mutex);
    sem_destroy(&rc_mutex);

    return 0;
}
