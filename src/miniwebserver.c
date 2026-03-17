#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "ipc.h"

#define PORT 8080

typedef struct {
    int id;
    char tipo[50];
    int linha;
    int coluna;
    int ativo;
    int pontos;
} Navio;

Navio navios[6] = {
    {1, TIPO_PORTA_AVIOES, 0, -1, 1, 5},
    {2, TIPO_SUBMARINO, 1, -1, 1, 3},
    {3, TIPO_SUBMARINO, 2, -1, 1, 3},
    {4, TIPO_FRAGATA, 3, -1, 1, 2},
    {5, TIPO_FRAGATA, 4, -1, 1, 2},
    {6, TIPO_FRAGATA, 5, -1, 1, 2}
};

int board_ataques[10][10];

void iniciar_navios();

void atualizar_json_estado() {
    FILE *f = fopen("estado.json", "w");
    if (!f) return;
    
    fprintf(f, "{\n\"navios\": [\n");
    int first = 1;
    for (int i = 0; i < 6; i++) {
        if (navios[i].ativo) {
            if (!first) fprintf(f, ",\n");
            fprintf(f, "  {\"id\": %d, \"tipo\": \"%s\", \"linha\": %d, \"coluna\": %d}", 
                    navios[i].id, navios[i].tipo, navios[i].linha, navios[i].coluna);
            first = 0;
        }
    }
    
    fprintf(f, "\n],\n\"ataques\": [\n");
    first = 1;
    for (int l = 0; l < 10; l++) {
        for (int c = 0; c < 10; c++) {
            if (board_ataques[l][c] != 0) {
                if (!first) fprintf(f, ",\n");
                fprintf(f, "  {\"linha\": %d, \"coluna\": %d, \"resultado\": %d}", l, c, board_ataques[l][c]);
                first = 0;
            }
        }
    }
    fprintf(f, "\n]\n}\n");
    fclose(f);
}

void processar_tiro(int client_socket, int linha, int coluna) {
    char response[1024];
    
    if (linha < 0 || linha >= 10 || coluna < 0 || coluna >= 10) {
        sprintf(response, "HTTP/1.1 400 Bad Request\r\n\r\n{\"erro\":\"parametros_invalidos\"}\n");
        write(client_socket, response, strlen(response));
        return;
    }

    if (board_ataques[linha][coluna] != 0) {
        sprintf(response, "HTTP/1.1 200 OK\r\n\r\n{\"resultado\":\"repetido\"}\n");
        write(client_socket, response, strlen(response));
        return;
    }

    int hit_index = -1;
    for (int i = 0; i < 6; i++) {
        if (navios[i].ativo && navios[i].linha == linha && navios[i].coluna == coluna) {
            hit_index = i;
            break;
        }
    }

    if (hit_index != -1) {
        board_ataques[linha][coluna] = 2;
        navios[hit_index].ativo = 0;
        sprintf(response, "HTTP/1.1 200 OK\r\n\r\n{\"resultado\":\"acerto\", \"tipo\":\"%s\", \"pontos\":%d}\n", 
                navios[hit_index].tipo, navios[hit_index].pontos);
    } else {
        board_ataques[linha][coluna] = 1;
        sprintf(response, "HTTP/1.1 200 OK\r\n\r\n{\"resultado\":\"agua\"}\n");
    }
    
    write(client_socket, response, strlen(response));
    atualizar_json_estado();
}

void processar_status(int client_socket) {
    char response[2048];
    sprintf(response, "HTTP/1.1 200 OK\r\n\r\n{\n\"linhas\": [\n");
    
    int qtd_porta = 0, qtd_sub = 0, qtd_frag = 0;
    int first = 1;

    for (int i = 0; i < 6; i++) {
        if (navios[i].ativo) {
            if (!first) strcat(response, ",\n");
            char linha_str[128];
            sprintf(linha_str, "  {\"linha\": %d, \"tipo\": \"%s\"}", navios[i].linha, navios[i].tipo);
            strcat(response, linha_str);
            first = 0;

            if (strcmp(navios[i].tipo, TIPO_PORTA_AVIOES) == 0) qtd_porta++;
            else if (strcmp(navios[i].tipo, TIPO_SUBMARINO) == 0) qtd_sub++;
            else if (strcmp(navios[i].tipo, TIPO_FRAGATA) == 0) qtd_frag++;
        }
    }

    char footer[512];
    sprintf(footer, "\n],\n\"quantidade\": {\n  \"porta_avioes\": %d,\n  \"submarinos\": %d,\n  \"fragatas\": %d\n}\n}\n",
            qtd_porta, qtd_sub, qtd_frag);
    strcat(response, footer);

    write(client_socket, response, strlen(response));
}

void processar_estado_local(int client_socket) {
    FILE *f = fopen("estado.json", "r");
    if (!f) {
        char *err = "HTTP/1.1 500 Internal Error\r\n\r\n{}";
        write(client_socket, err, strlen(err));
        return;
    }
    
    char response[4096] = "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\n\r\n";
    write(client_socket, response, strlen(response));
    
    char buffer[1024];
    int bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), f)) > 0) {
        write(client_socket, buffer, bytes);
    }
    fclose(f);
}

int main() {
    memset(board_ataques, 0, sizeof(board_ataques));
    
    unlink(FIFO_NAME);
    mkfifo(FIFO_NAME, 0666);

    int fifo_fd = open(FIFO_NAME, O_RDONLY | O_NONBLOCK);
    if (fifo_fd < 0) {
        perror("Erro ao abrir FIFO");
        return 1;
    }

    iniciar_navios();

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 10);

    fd_set readfds;
    char buffer[2048];

    atualizar_json_estado();

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(fifo_fd, &readfds);
        FD_SET(server_fd, &readfds);

        int max_fd = (fifo_fd > server_fd) ? fifo_fd : server_fd;

        int activity = select(max_fd + 1, &readfds, NULL, NULL, NULL);
        if (activity < 0) break;

        if (FD_ISSET(fifo_fd, &readfds)) {
            int bytes = read(fifo_fd, buffer, sizeof(buffer) - 1);
            if (bytes > 0) {
                buffer[bytes] = '\0';
                char *linha_msg = strtok(buffer, "\n");
                while (linha_msg != NULL) {
                    int id, linha, coluna;
                    char tipo[50];
                    if (sscanf(linha_msg, "%d,%[^,],%d,%d", &id, tipo, &linha, &coluna) == 4) {
                        for (int i = 0; i < 6; i++) {
                            if (navios[i].id == id && navios[i].ativo) {
                                navios[i].linha = linha;
                                navios[i].coluna = coluna;
                                atualizar_json_estado();
                                break;
                            }
                        }
                    }
                    linha_msg = strtok(NULL, "\n");
                }
            }
        }

        if (FD_ISSET(server_fd, &readfds)) {
            int client_socket = accept(server_fd, NULL, NULL);
            int bytes = read(client_socket, buffer, sizeof(buffer) - 1);
            if (bytes > 0) {
                buffer[bytes] = '\0';
                if (strncmp(buffer, "GET /tiro", 9) == 0) {
                    int l, c;
                    if (sscanf(buffer, "GET /tiro?linha=%d&coluna=%d", &l, &c) == 2) {
                        processar_tiro(client_socket, l, c);
                    } else {
                        char *err = "HTTP/1.1 400 Bad Request\r\n\r\n{\"erro\":\"comando_invalido\"}\n";
                        write(client_socket, err, strlen(err));
                    }
                } else if (strncmp(buffer, "GET /status", 11) == 0) {
                    processar_status(client_socket);
                } else if (strncmp(buffer, "GET /estado_local", 17) == 0) {
                    processar_estado_local(client_socket);
                } else {
                    char *err = "HTTP/1.1 404 Not Found\r\n\r\n{\"erro\":\"endpoint_inexistente\"}\n";
                    write(client_socket, err, strlen(err));
                }
            }
            close(client_socket);
        }
    }

    close(server_fd);
    close(fifo_fd);
    unlink(FIFO_NAME);
    return 0;
}