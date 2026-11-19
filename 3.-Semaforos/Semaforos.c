/*
  Compilar: gcc semaforos.c -o semaforos -pthread
  Ejecutar: ./semaforos | tee salida.txt
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <unistd.h>

#define LIMITE      10000
#define INICIO_SC   4000
#define FIN_SC      6000

static sem_t *mutex;          /* semáforo binario que protege la sección crítica */

void *contar(void *arg)
{
    int id = *(int *)arg;

    for (int i = 1; i <= LIMITE; i++) {
        if (i == INICIO_SC) {
            sem_wait(mutex);                     /* entra a la sección crítica */
            printf("--- Hilo %d ENTRA a la seccion critica (4000) ---\n", id);
        }

        printf("Hilo %d: %d\n", id, i);

        if (i == FIN_SC) {
            printf("--- Hilo %d SALE de la seccion critica (6000) ---\n", id);
            sem_post(mutex);                     /* libera la sección crítica */
        }

        usleep(50); /* pequeña pausa para que se note la alternancia entre hilos */
    }
    return NULL;
}

int main(void)
{
    pthread_t h1, h2;
    int id1 = 1, id2 = 2;

    /* Semáforo con nombre (funciona en Linux y en macOS) con valor inicial 1 */
    sem_unlink("/sem_practica");
    mutex = sem_open("/sem_practica", O_CREAT | O_EXCL, 0644, 1);
    if (mutex == SEM_FAILED) {
        perror("sem_open");
        return EXIT_FAILURE;
    }

    pthread_create(&h1, NULL, contar, &id1);
    pthread_create(&h2, NULL, contar, &id2);

    pthread_join(h1, NULL);
    pthread_join(h2, NULL);

    sem_close(mutex);
    sem_unlink("/sem_practica");
    return EXIT_SUCCESS;
}
