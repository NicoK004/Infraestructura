/******************************************************************************

EJERCICIO 2 - POSIX (Lectores-Escritores)
Requisitos:
- 100 pasajeros (lectores) y 5 oficinistas (escritores).
- Cada oficinista hace al menos 3 cambios.
- Demoras aleatorias: hasta 3s los pasajeros y hasta 5s los oficinistas.
- Mensajes:
  "Pasajero X está mirando el cartel"
  "Oficinista X está modificando el cartel"

Modelo de concurrencia:
- Lectores-Escritores clásico con 2 semáforos:
  * rw_mutex: exclusión total para la ESCRITURA y para el primer/último lector.
  * rc_mutex: protege el contador 'readers' (cuántos lectores están leyendo).

Política:
- Varios lectores pueden leer en paralelo cuando NO hay escritor activo.
- El primer lector que entra BLOQUEA a los escritores (wait en rw_mutex).
- El último lector que sale LIBERA a los escritores (post en rw_mutex).
- Cada escritor entra SOLO, haciendo wait en rw_mutex.

Compilación:
  gcc -pthread -o aeropuerto aeropuerto.c
Ejecución:
  ./aeropuerto

*******************************************************************************/

#define _XOPEN_SOURCE 700
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>

// --------------------- Parámetros de la consigna ----------------------------
#define PASAJEROS 100              // cantidad de hilos lectores
#define OFICINISTAS 5              // cantidad de hilos escritores
#define CAMBIOS 3                  // cambios por oficinista (>= 3)
#define LECTURAS_POR_PASAJERO 3    // cuántas veces mira cada pasajero
#define MAX_SLEEP_READER 3         // segundos máx. para pasajeros (0..3)
#define MAX_SLEEP_WRITER 5         // segundos máx. para oficinistas (0..5)

// ---------------- Sincronización compartida entre hilos ---------------------
sem_t rw_mutex;   // binario: garantiza exclusión entre escritores y lectores (vía 1er/último)
sem_t rc_mutex;   // binario: protege el contador de lectores
int readers = 0;  // contador de lectores actualmente "dentro" de la sección crítica de lectura

// --------------- Utilidad: dormir 0..max segundos al azar -------------------
static void sleep_rand(int max_seconds) {
    // rand() % (max+1) => incluye 0 y max.
    int s = rand() % (max_seconds + 1);
    sleep(s);
}

// -------------------------- Hilo PASAJERO (lector) --------------------------
void* pasajero(void* x) {
    long id = (long)x; // id numérico solo para imprimir

    for (int i = 0; i < LECTURAS_POR_PASAJERO; ++i) {
        // ENTRADA de lector:
        // - rc_mutex protege el acceso y modificaciones al contador 'readers'
        sem_wait(&rc_mutex);          // bloqueo el contador
        readers++;                     // entro como lector
        if (readers == 1) {            // si soy el PRIMER lector que entra
            sem_wait(&rw_mutex);       // bloqueo a los escritores (exclusión)
        }
        sem_post(&rc_mutex);           // libero el contador (otros lectores pueden entrar)

        // SECCIÓN CRÍTICA de LECTURA:
        // - Varios lectores pueden estar aquí en paralelo.
        // - NUNCA habrá un escritor aquí porque el primer lector bloqueó rw_mutex.
        printf("Pasajero %ld está mirando el cartel\n", id);
        fflush(stdout);
        sleep_rand(MAX_SLEEP_READER);  // simulo el tiempo de lectura (0..3s)

        // SALIDA de lector:
        sem_wait(&rc_mutex);           // vuelvo a proteger el contador
        readers--;                     // salgo como lector
        if (readers == 0) {            // si soy el ÚLTIMO lector en salir
            sem_post(&rw_mutex);       // libero a los escritores
        }
        sem_post(&rc_mutex);           // libero el contador

        // ZONA NO CRÍTICA del lector (tiempo fuera de la pantalla)
        sleep_rand(MAX_SLEEP_READER);
    }

    pthread_exit(NULL);
}

// ------------------------- Hilo OFICINISTA (escritor) -----------------------
void* oficinista(void* x) {
    long id = (long)x; // id numérico solo para imprimir

    for (int k = 0; k < CAMBIOS; ++k) {
        // ENTRADA de escritor:
        // - Se bloquea rw_mutex para garantizar exclusión total:
        //   ni lectores ni otros escritores pueden estar dentro.
        sem_wait(&rw_mutex);

        // SECCIÓN CRÍTICA de ESCRITURA:
        printf("Oficinista %ld está modificando el cartel\n", id);
        fflush(stdout);
        sleep_rand(MAX_SLEEP_WRITER);  // simulo la actualización (0..5s)

        // SALIDA de escritor:
        // - Libero rw_mutex para que lectores/escritores puedan entrar.
        sem_post(&rw_mutex);

        // ZONA NO CRÍTICA del escritor (otras tareas fuera del cartel)
        sleep_rand(MAX_SLEEP_WRITER);
    }

    pthread_exit(NULL);
}

// ---------------------------------- MAIN ------------------------------------
int main(void) {
    srand((unsigned int)time(NULL)); // semilla para las demoras aleatorias

    pthread_t pas[PASAJEROS];        // vector de hilos pasajeros
    pthread_t ofi[OFICINISTAS];      // vector de hilos oficinistas
    pthread_attr_t attr;             // atributo estándar de hilos (como en clase)
    pthread_attr_init(&attr);        // inicialización default

    // Inicializo semáforos binarios:
    // - rw_mutex arranca en 1 (cartel libre).
    // - rc_mutex arranca en 1 (contador disponible para leer/escribir).
    sem_init(&rw_mutex, 0, 1);
    sem_init(&rc_mutex, 0, 1);

    // Creo primero a los OFICINISTAS (escritores).
    // El cuarto parámetro pasa el id (cast de long a void*).
    for (long i = 0; i < OFICINISTAS; ++i) {
        pthread_create(&ofi[i], &attr, oficinista, (void*)i);
    }

    // Creo ahora a los PASAJEROS (lectores).
    for (long i = 0; i < PASAJEROS; ++i) {
        pthread_create(&pas[i], &attr, pasajero, (void*)i);
    }

    // Espero a que TERMINE cada OFICINISTA (hace CAMBIOS y finaliza).
    for (int i = 0; i < OFICINISTAS; ++i) {
        pthread_join(ofi[i], NULL);
    }

    // Espero a que TERMINE cada PASAJERO (mira LECTURAS_POR_PASAJERO veces y finaliza).
    for (int i = 0; i < PASAJEROS; ++i) {
        pthread_join(pas[i], NULL);
    }

    // Limpieza de semáforos.
    sem_destroy(&rw_mutex);
    sem_destroy(&rc_mutex);

    return 0;
}
