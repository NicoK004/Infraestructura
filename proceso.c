#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <time.h>

#define PASAJEROS 100
#define OFICINISTAS 5
#define CAMBIOS 3

sem_t mutex;  // semáforo binario para exclusión mutua (RSR)

// Función de los pasajeros
void *pasajero(void *arg) {
    int id = (int)(intptr_t)arg;
    srand(time(NULL) ^ id);

    while (1) {
        sem_wait(&mutex); // entrada a región crítica
        printf("Pasajero %d está mirando el cartel\n", id);
        fflush(stdout);
        usleep((rand() % 3000 + 1) * 1000); // demora aleatoria hasta 3 seg
        sem_post(&mutex); // salida de región crítica

        usleep((rand() % 3000 + 1) * 1000); // espera antes de volver a mirar
    }
    return NULL;
}

// Función de los oficinistas
void *oficinista(void *arg) {
    int id = (int)(intptr_t)arg;
    srand(time(NULL) ^ id);

    for (int i = 0; i < CAMBIOS; i++) {
        sem_wait(&mutex); // entrada a región crítica
        printf("Oficinista %d está modificando el cartel (cambio %d)\n", id, i + 1);
        fflush(stdout);
        usleep((rand() % 5000 + 1) * 1000); // demora aleatoria hasta 5 seg
        sem_post(&mutex); // salida de región crítica

        usleep((rand() % 2000 + 1) * 1000);
    }
    return NULL;
}

int main() {
    pthread_t th_pasajeros[PASAJEROS];
    pthread_t th_oficinistas[OFICINISTAS];

    // Inicializar semáforo binario (1 = libre)
    sem_init(&mutex, 0, 1);

    // Crear hilos de oficinistas
    for (int i = 0; i < OFICINISTAS; i++)
        pthread_create(&th_oficinistas[i], NULL, oficinista, (void*)(intptr_t)(i + 1));

    // Crear hilos de pasajeros
    for (int i = 0; i < PASAJEROS; i++)
        pthread_create(&th_pasajeros[i], NULL, pasajero, (void*)(intptr_t)(i + 1));

    // Esperar a que los oficinistas terminen sus 3 cambios
    for (int i = 0; i < OFICINISTAS; i++)
        pthread_join(th_oficinistas[i], NULL);

    // Cancelar los pasajeros (lectores infinitos)
    for (int i = 0; i < PASAJEROS; i++)
        pthread_cancel(th_pasajeros[i]);

    sem_destroy(&mutex);
    return 0;
}
