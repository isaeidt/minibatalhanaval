#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "ipc.h"

#define MAX_COLUNA 9

void loop_navio(int id, char *tipo, int linha) {
    int coluna = 0;

    int fd = open(FIFO_NAME, O_WRONLY);
    if (fd < 0) {
        perror("Erro ao abrir FIFO");
        exit(1);
    }

    while (1) {
        sleep(5);

        coluna++;
        if (coluna > MAX_COLUNA) {
            coluna = 0;
        }

        char buffer[BUFFER_SIZE];
        snprintf(buffer, sizeof(buffer), "%d,%s,%d,%d,%d\n",
                 id, tipo, linha, coluna, getpid());

        write(fd, buffer, strlen(buffer));
    }
}

void iniciar_navios() {
    if (fork() == 0) {
        loop_navio(1, TIPO_PORTA_AVIOES, 0);
        exit(0);
    }

    if (fork() == 0) {
        loop_navio(2, TIPO_SUBMARINO, 1);
        exit(0);
    }

    if (fork() == 0) {
        loop_navio(3, TIPO_SUBMARINO, 2);
        exit(0);
    }

    if (fork() == 0) {
        loop_navio(4, TIPO_FRAGATA, 3);
        exit(0);
    }

    if (fork() == 0) {
        loop_navio(5, TIPO_FRAGATA, 4);
        exit(0);
    }

    if (fork() == 0) {
        loop_navio(6, TIPO_FRAGATA, 5);
        exit(0);
    }
}