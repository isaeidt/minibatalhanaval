#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/select.h>
#include "ipc.h"

void iniciar_navios();

int main() {

    mkfifo(FIFO_NAME, 0666);

    int fd = open(FIFO_NAME, O_RDONLY | O_NONBLOCK);
    if (fd < 0) {
        perror("Erro ao abrir FIFO");
        return 1;
    }

    iniciar_navios();

    printf("Miniwebserver rodando...\n");

    fd_set readfds;

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(fd, &readfds);

        int activity = select(fd + 1, &readfds, NULL, NULL, NULL);

        if (activity < 0) {
            perror("Erro no select");
            break;
        }

        if (FD_ISSET(fd, &readfds)) {
            char buffer[BUFFER_SIZE];

            int bytes = read(fd, buffer, sizeof(buffer) - 1);

            if (bytes > 0) {
                buffer[bytes] = '\0';

                int id, linha, coluna;
                char tipo[50];

                sscanf(buffer, "%d,%[^,],%d,%d", &id, tipo, &linha, &coluna);

                printf("Navio %d (%s) na linha %d coluna %d\n",
                       id, tipo, linha, coluna);

            }
        }
    }

    close(fd);
    return 0;
}