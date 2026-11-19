/*
 Compilar: gcc filosofos.c -o filosofos -pthread
 Ejecutar: ./filosofos | tee salida_filosofos.txt
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <unistd.h>

#define N          5     /* número de filósofos y de tenedores */
#define COMIDAS    3     /* veces que come cada filósofo */

static pthread_mutex_t tenedor[N];
static pthread_mutex_t verifica = PTHREAD_MUTEX_INITIALIZER; /* protege 'comiendo[]' y los printf */
static sem_t *comedor;
static int comiendo[N];  /* 1 si el filósofo i está comiendo (solo para verificar la solución) */

static void pensar(int id)
{
    pthread_mutex_lock(&verifica);
    printf("Filosofo %d esta PENSANDO\n", id);
    pthread_mutex_unlock(&verifica);
    usleep((100 + rand() % 400) * 1000);
}

static void comer(int id)
{
    int izq = (id + N - 1) % N, der = (id + 1) % N;

    pthread_mutex_lock(&verifica);
    comiendo[id] = 1;
    printf("Filosofo %d esta COMIENDO (tenedores %d y %d)", id, id, (id + 1) % N);
    if (comiendo[izq] || comiendo[der])
        printf("  <-- ERROR: un vecino tambien come");
    printf("\n");
    pthread_mutex_unlock(&verifica);

    usleep((100 + rand() % 400) * 1000);

    pthread_mutex_lock(&verifica);
    comiendo[id] = 0;
    pthread_mutex_unlock(&verifica);
}

void *filosofo(void *arg)
{
    int id = *(int *)arg;
    int izq = id;            /* tenedor a la izquierda */
    int der = (id + 1) % N;  /* tenedor a la derecha */

    for (int i = 0; i < COMIDAS; i++) {
        pensar(id);

        pthread_mutex_lock(&verifica);
        printf("Filosofo %d tiene HAMBRE\n", id);
        pthread_mutex_unlock(&verifica);

        sem_wait(comedor);                    /* máximo N-1 filósofos compitiendo */
        pthread_mutex_lock(&tenedor[izq]);
        pthread_mutex_lock(&tenedor[der]);

        comer(id);

        pthread_mutex_unlock(&tenedor[der]);
        pthread_mutex_unlock(&tenedor[izq]);
        sem_post(comedor);

        pthread_mutex_lock(&verifica);
        printf("Filosofo %d solto los tenedores %d y %d\n", id, izq, der);
        pthread_mutex_unlock(&verifica);
    }

    pthread_mutex_lock(&verifica);
    printf("Filosofo %d termino de cenar (%d veces)\n", id, COMIDAS);
    pthread_mutex_unlock(&verifica);
    return NULL;
}

int main(void)
{
    pthread_t hilos[N];
    int ids[N];

    srand(7);

    sem_unlink("/sem_comedor");
    comedor = sem_open("/sem_comedor", O_CREAT | O_EXCL, 0644, N - 1);
    if (comedor == SEM_FAILED) {
        perror("sem_open");
        return EXIT_FAILURE;
    }

    for (int i = 0; i < N; i++)
        pthread_mutex_init(&tenedor[i], NULL);

    for (int i = 0; i < N; i++) {
        ids[i] = i;
        pthread_create(&hilos[i], NULL, filosofo, &ids[i]);
    }
    for (int i = 0; i < N; i++)
        pthread_join(hilos[i], NULL);

    printf("\nTodos los filosofos terminaron de cenar. Sin interbloqueo.\n");

    for (int i = 0; i < N; i++)
        pthread_mutex_destroy(&tenedor[i]);
    sem_close(comedor);
    sem_unlink("/sem_comedor");
    return EXIT_SUCCESS;
}
