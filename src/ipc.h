#ifndef IPC_H
#define IPC_H

/* * Arquivo de cabeçalho compartilhado entre o Miniwebserver (Isa) e os Navios (Haydee).
 * Define as constantes para a comunicação via FIFO (Pipe Nomeado).
 */

// Nome do arquivo especial da FIFO que será criado na raiz do projeto
#define FIFO_NAME "fifo_navios"

// Tamanho máximo do texto trafegado no pipe
#define BUFFER_SIZE 128

// Tipos padronizados de navios (conforme os requisitos do projeto)
#define TIPO_PORTA_AVIOES "porta_avioes"
#define TIPO_SUBMARINO    "submarino"
#define TIPO_FRAGATA      "fragata"

#endif