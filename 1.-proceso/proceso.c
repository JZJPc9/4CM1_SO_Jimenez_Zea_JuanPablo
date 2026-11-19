#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(void) {
    /* Salida por lineas: evita que se mezclen lineas a la mitad al redirigir a archivo */
     setvbuf(stdout, NULL, _IOLBF, 0);

    pid_t pid = fork();

    if (pid < 0) {
        perror("Error al crear el proceso con fork()");
        return EXIT_FAILURE;
    }

    if (pid == 0) {
        /* Proceso hijo: de 10,000 a 1 */
        for (int i = 10000; i >= 1; i--) {
            printf("Hijo (PID %d): %d\n", getpid(), i);
        }
    } else {
        /* Proceso padre: de 1 a 10,000 */
         for (int i = 1; i <= 10000; i++) {
            printf("Padre (PID %d): %d\n", getpid(), i);
        }
        wait(NULL); /* Espera a que termine el hijo */
    }

    return EXIT_SUCCESS;
}