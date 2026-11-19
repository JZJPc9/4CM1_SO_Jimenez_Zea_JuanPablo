/*
  Compilar: gcc productor_consumidor.c -o productor_consumidor -pthread
  Ejecutar: ./productor_consumidor | tee salida_productor_consumidor.txt
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <unistd.h>

#define TAM_BUFFER        5    /* capacidad del buffer */
#define NUM_PRODUCTORES   2
#define NUM_CONSUMIDORES  2
#define ITEMS_POR_HILO    10   /* cada productor produce 10 y cada consumidor consume 10 */

static int buffer[TAM_BUFFER];
static int entrada = 0;        /* siguiente posición donde se produce */
static int salida  = 0;        /* siguiente posición de donde se consume */
static int cantidad = 0;       /* elementos actualmente en el buffer (solo informativo) */

static sem_t *vacios;
static sem_t *llenos;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *productor(void *arg)
{
    int id = *(int *)arg;

    for (int i = 1; i <= ITEMS_POR_HILO; i++) {
        int item = id * 100 + i;          /* ej. productor 2, item 3 -> 203 */

        usleep((rand() % 300) * 1000);    /* simula el tiempo de producir */

        sem_wait(vacios);                 /* espera un espacio libre */
        pthread_mutex_lock(&mutex);

        buffer[entrada] = item;
        entrada = (entrada + 1) % TAM_BUFFER;
        cantidad++;
        printf("Productor %d produjo %d   [elementos en buffer: %d]\n", id, item, cantidad);
        fflush(stdout);

        pthread_mutex_unlock(&mutex);
        sem_post(llenos);                 /* avisa que hay un elemento más */
    }
    printf("Productor %d termino\n", id);
    return NULL;
}

void *consumidor(void *arg)
{
    int id = *(int *)arg;

    for (int i = 1; i <= ITEMS_POR_HILO; i++) {
        sem_wait(llenos);                 /* espera un elemento disponible */
        pthread_mutex_lock(&mutex);

        int item = buffer[salida];
        salida = (salida + 1) % TAM_BUFFER;
        cantidad--;
        printf("    Consumidor %d consumio %d   [elementos en buffer: %d]\n", id, item, cantidad);
        fflush(stdout);

        pthread_mutex_unlock(&mutex);
        sem_post(vacios);                 /* avisa que hay un espacio libre más */

        usleep((rand() % 500) * 1000);    /* simula el tiempo de consumir */
    }
    printf("    Consumidor %d termino\n", id);
    return NULL;
}

int main(void)
{
    pthread_t prod[NUM_PRODUCTORES], cons[NUM_CONSUMIDORES];
    int ids_p[NUM_PRODUCTORES], ids_c[NUM_CONSUMIDORES];

    srand(42);

    /* Semáforos con nombre (funcionan en Linux y en macOS) */
    sem_unlink("/sem_vacios");
    sem_unlink("/sem_llenos");
    vacios = sem_open("/sem_vacios", O_CREAT | O_EXCL, 0644, TAM_BUFFER);
    llenos = sem_open("/sem_llenos", O_CREAT | O_EXCL, 0644, 0);
    if (vacios == SEM_FAILED || llenos == SEM_FAILED) {
        perror("sem_open");
        return EXIT_FAILURE;
    }

    for (int i = 0; i < NUM_PRODUCTORES; i++) {
        ids_p[i] = i + 1;
        pthread_create(&prod[i], NULL, productor, &ids_p[i]);
    }
    for (int i = 0; i < NUM_CONSUMIDORES; i++) {
        ids_c[i] = i + 1;
        pthread_create(&cons[i], NULL, consumidor, &ids_c[i]);
    }

    for (int i = 0; i < NUM_PRODUCTORES; i++)  pthread_join(prod[i], NULL);
    for (int i = 0; i < NUM_CONSUMIDORES; i++) pthread_join(cons[i], NULL);

    printf("\nFin: se produjeron y consumieron %d elementos. Elementos restantes en buffer: %d\n",
           NUM_PRODUCTORES * ITEMS_POR_HILO, cantidad);

    sem_close(vacios);
    sem_close(llenos);
    sem_unlink("/sem_vacios");
    sem_unlink("/sem_llenos");
    return EXIT_SUCCESS;
}
