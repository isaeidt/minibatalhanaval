#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_TIROS 100

int main() {
    FILE *f = fopen("alvos.txt", "r");
    if (!f) {
        perror("Erro ao abrir alvos.txt. Cria o ficheiro com os IPs dos inimigos.");
        return 1;
    }

    char ips[50][50];
    int total_ips = 0;

    while (fgets(ips[total_ips], 50, f)) {
        ips[total_ips][strcspn(ips[total_ips], "\n")] = 0;
        if (strlen(ips[total_ips]) > 0) total_ips++;
    }
    fclose(f);

    if (total_ips == 0) {
        printf("Nenhum IP encontrado em alvos.txt!\n");
        return 1;
    }

    int tiros = 0;
    int acertos = 0;
    int pontos = 0;
    int navios_afundados = 0;

    printf("Iniciando a Varredura Inteligente...\n\n");

    for (int ip_index = 0; ip_index < total_ips && tiros < MAX_TIROS; ip_index++) {
        char *ip_alvo = ips[ip_index];
        printf(">> Analisando inimigo: %s\n", ip_alvo);

        char comando_status[256];
        snprintf(comando_status, sizeof(comando_status), "curl -s \"http://%s:8080/status\"", ip_alvo);
        
        FILE *resp_status = popen(comando_status, "r");
        if (!resp_status) continue;

        char status_json[2048] = {0};
        fread(status_json, 1, sizeof(status_json)-1, resp_status);
        pclose(resp_status);

       
        for (int linha = 0; linha < 10 && tiros < MAX_TIROS; linha++) {
            char busca[20];
            snprintf(busca, sizeof(busca), "\"linha\": %d", linha);
            char busca2[20];
            snprintf(busca2, sizeof(busca2), "\"linha\":%d", linha);

            if (strstr(status_json, busca) != NULL || strstr(status_json, busca2) != NULL) {
                printf("   [!] Navio detetado na linha %d! Iniciando varredura...\n", linha);
                
                for (int coluna = 0; coluna < 10 && tiros < MAX_TIROS; coluna++) {
                    char comando_tiro[256];
                    snprintf(comando_tiro, sizeof(comando_tiro),
                             "curl -s \"http://%s:8080/tiro?linha=%d&coluna=%d\"",
                             ip_alvo, linha, coluna);

                    FILE *resp_tiro = popen(comando_tiro, "r");
                    if (resp_tiro) {
                        char buffer[256] = {0};
                        fgets(buffer, sizeof(buffer), resp_tiro);
                        
                        if (strstr(buffer, "acerto")) {
                            acertos++;
                            navios_afundados++;
                            if (strstr(buffer, "porta_avioes")) pontos += 5;
                            else if (strstr(buffer, "submarino")) pontos += 3;
                            else if (strstr(buffer, "fragata")) pontos += 2;
                            
                            printf("      -> ACERTOU na coluna %d! Total Pontos: %d\n", coluna, pontos);
                        }
                        pclose(resp_tiro);
                    }
                    tiros++;
                }
            }
        }
    }

    printf("\n===== RELATÓRIO DE COMBATE =====\n");
    printf("Tiros disparados: %d / %d\n", tiros, MAX_TIROS);
    printf("Tiros certeiros: %d\n", acertos);
    printf("Navios afundados: %d\n", navios_afundados);
    printf("Pontos totais: %d\n", pontos);
    printf("================================\n");

    return 0;
}