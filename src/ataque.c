#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_TIROS 100

int main() {
    FILE *f = fopen("alvos.txt", "r");
    if (!f) {
        perror("Erro ao abrir alvos.txt");
        return 1;
    }

    char ips[50][50];
    int total_ips = 0;

    // Ler IPs
    while (fgets(ips[total_ips], 50, f)) {
        ips[total_ips][strcspn(ips[total_ips], "\n")] = 0;
        total_ips++;
    }
    fclose(f);

    srand(time(NULL));

    int tiros = 0;
    int acertos = 0;
    int pontos = 0;
    int navios_afundados = 0;

    while (tiros < MAX_TIROS) {
        int ip_index = rand() % total_ips;
        int linha = rand() % 10;
        int coluna = rand() % 10;

        char comando[200];
        snprintf(comando, sizeof(comando),
                 "curl -s \"http://%s:8080/tiro?linha=%d&coluna=%d\"",
                 ips[ip_index], linha, coluna);

        FILE *resp = popen(comando, "r");
        if (resp) {
            char buffer[256];
            if (fgets(buffer, sizeof(buffer), resp)) {
                printf("Resposta: %s", buffer);

                if (strstr(buffer, "acerto")) {
                    acertos++;
                    navios_afundados++;

                    if (strstr(buffer, "porta_avioes")) pontos += 5;
                    else if (strstr(buffer, "submarino")) pontos += 3;
                    else if (strstr(buffer, "fragata")) pontos += 2;
                }
            }
            pclose(resp);
        }

        tiros++;
    }

    printf("\n===== RELATORIO FINAL =====\n");
    printf("Tiros: %d\n", tiros);
    printf("Acertos: %d\n", acertos);
    printf("Navios afundados: %d\n", navios_afundados);
    printf("Pontos: %d\n", pontos);

    return 0;
}